// NOTAS: No se puede printear ni loguear desde una ISR se reinicia el ESP32.
//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "esp_timer.h"

#include "../include/button_manager.h"
#include "../include/board_def.h"
#include "../include/global_manager.h"
#include "../include/nv_flash_manager.h"
#include "../include/flora_vege_manager.h"
#include "../include/led_manager.h"

//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define QUEUE_ELEMENT_QUANTITY 20

#define TIEMPO_ANTIRREBOTE_MS 50
#define TIEMPO_PULSADO_MS 3000

#define DEBUG_MODULE
//------------------------------TYPEDEF-----------------------------------------
//------------------------------------------------------------------------------
typedef enum{
    CMD_UNDEFINED,
    STARTUP,
    VEGE_BUTTON_PUSHED,
    FABRIC_RESET,
    CALIBRATE_POTE,
}cmds_t;

typedef struct{
    cmds_t cmd;
}button_events_t;
//--------------------DECLARACION DE DATOS INTERNOS-----------------------------
//------------------------------------------------------------------------------
static QueueHandle_t button_manager_queue;

volatile int64_t start_time_flora_vege = 0;
//--------------------DECLARACION DE FUNCIONES INTERNAS-------------------------
//------------------------------------------------------------------------------
static void button_event_manager_task(void * pvParameters);
static void config_buttons_isr(void);
static void vege_button_interrupt(void *arg);

static flora_vege_status_t nv_init_flora_vege_status(void);
static void nv_save_flora_vege_status(flora_vege_status_t flora_vege_status);
//--------------------DEFINICION DE DATOS INTERNOS------------------------------
//------------------------------------------------------------------------------

//--------------------DEFINICION DE DATOS EXTERNOS------------------------------
//------------------------------------------------------------------------------

//--------------------DEFINICION DE FUNCIONES INTERNAS--------------------------
//------------------------------------------------------------------------------
static flora_vege_status_t nv_init_flora_vege_status(void)
{
    uint32_t value;
    flora_vege_status_t ret_value;

    if (read_uint32_from_flash(RELE_VEGE_STATUS_KEY, &value))
    {
#ifdef DEBUG_MODULE
        printf("RELE VEGE STATUS READ: %lu \n", value);
#endif

        if (value == 1)
        {
            ret_value = FLORA_VEGE_OUTPUT_ENABLE;
        }
        else
        {
            ret_value = FLORA_VEGE_OUTPUT_DISABLE;
        }
    }
    else
    {
#ifdef DEBUG_MODULE
        printf("RELE VEGE STATUS READING FAILED \n");
#endif
        ret_value = FLORA_VEGE_OUTPUT_DISABLE;
    }
    return(ret_value);
}

//------------------------------------------------------------------------------
static void nv_save_flora_vege_status(flora_vege_status_t flora_vege_status)
{
    write_parameter_on_flash_uint32(RELE_VEGE_STATUS_KEY, (uint32_t)flora_vege_status);
}

//------------------------------------------------------------------------------
static void config_buttons_isr(void)
{
    gpio_config_t config;

    config.pin_bit_mask = (1ULL << BT_VE_FLO);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLDOWN_ENABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_ANYEDGE;
    gpio_config(&config);
    
    // Configurar la interrupción del botón
    gpio_install_isr_service(0);

    gpio_isr_handler_add(BT_VE_FLO, vege_button_interrupt, NULL);
}

//------------------------------------------------------------------------------
static void IRAM_ATTR vege_button_interrupt(void *arg) 
{
    button_events_t ev;
    int64_t time_now = esp_timer_get_time();

    if(gpio_get_level(BT_VE_FLO) == 0)
    {
        start_time_flora_vege = time_now;
    }
    else 
    {
        if (start_time_flora_vege != 0)
        {
            int64_t diff = time_now - start_time_flora_vege;

            if (diff > 30000)  // 30ms seconds expressed in microseconds
            {
                ev.cmd = VEGE_BUTTON_PUSHED;
                xQueueSendFromISR(button_manager_queue, &ev, pdFALSE);
            }
            start_time_flora_vege = 0;
        }
    }
}

//------------------------------------------------------------------------------
void button_event_manager_task(void * pvParameters)
{
    button_events_t button_ev;    
    flora_vege_status_t flora_vege_status = nv_init_flora_vege_status();

    #ifdef DEBUG_MODULE
        printf("flora_vege_status: %d \n", flora_vege_status);
    #endif

    config_buttons_isr();

    global_manager_set_flora_vege_status(flora_vege_status);

    if(flora_vege_status == FLORA_VEGE_OUTPUT_ENABLE)
    {
        flora_vege_manager_turn_on();
        led_manager_rele_vege_on();
    }
    

    while(true)
    {
        if(xQueueReceive(button_manager_queue, &button_ev, portMAX_DELAY) == pdTRUE)
        {
            switch(button_ev.cmd)
            {
                case CMD_UNDEFINED:
                    break;
                case VEGE_BUTTON_PUSHED:
                    global_manager_get_flora_vege_status(&flora_vege_status);
                    if(flora_vege_status == FLORA_VEGE_OUTPUT_DISABLE)
                    {
                        #ifdef DEBUG_MODULE
                                printf("FLORa VEGE OUPUT STATUS ENABLE \n");
                        #endif
                        global_manager_set_flora_vege_status(FLORA_VEGE_OUTPUT_ENABLE);
                        flora_vege_status = FLORA_VEGE_OUTPUT_ENABLE;
                        flora_vege_manager_turn_on();
                        led_manager_rele_vege_on();
                    }
                    else if(flora_vege_status == FLORA_VEGE_OUTPUT_ENABLE)
                    {
                        #ifdef DEBUG_MODULE
                                printf("FLORa VEGE OUPUT STATUS DISABLE \n");
                        #endif
                        global_manager_set_flora_vege_status(FLORA_VEGE_OUTPUT_DISABLE);
                        flora_vege_status = FLORA_VEGE_OUTPUT_DISABLE;
                        flora_vege_manager_turn_off();
                        led_manager_rele_vege_off();
                    }
                    nv_save_flora_vege_status(flora_vege_status);
                    break;
                case FABRIC_RESET:
                    //led_manager_new_update();
                    nv_flash_driver_erase_flash();
                    break;
                default:
                break;
            }
        }
    }
}
//--------------------DEFINICION DE FUNCIONES EXTERNAS--------------------------
//------------------------------------------------------------------------------
void button_manager_init(void)
{
    button_manager_queue = xQueueCreate(QUEUE_ELEMENT_QUANTITY, sizeof(button_events_t));

    xTaskCreate(button_event_manager_task, "button_event_manager_task", 
               configMINIMAL_STACK_SIZE*5, NULL, configMAX_PRIORITIES, NULL);             
}
//--------------------FIN DEL ARCHIVO-------------------------------------------
//------------------------------------------------------------------------------