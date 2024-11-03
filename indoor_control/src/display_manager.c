//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sdkconfig.h"

#include "esp_log.h"

// #include "../include/board_def.h"
#include "../include/display_manager.h"
#include "../include/display_dogs164.h"
// #include "display_dogs164.c"

static const char *TAG = "I2C";
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define DEBUG_MODULE 1

#define QUEUE_ELEMENT_QUANTITY 25
//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------
// comandos de las acciones del display
typedef enum
{
    CMD_UNDEFINED = 0,
    START_DISPLAY = 1,
    UPDATE_DISPLAY = 2,
    UPDATE_VEGE_FLORA_ON_SCREEN = 3,
    CHANGE_SCREEN = 4,
    CONFIG = 5
} display_event_cmds_t;

typedef struct
{
    uint8_t pwm_value;
    display_event_cmds_t cmd;
    char vege_flora;
} display_event_t;

//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------
static QueueHandle_t display_manager_queue;

//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
static void display_manager_task(void *arg)
{
    display_event_t display_ev;

    // display_ev.pwm_value = 75;
    // display_ev.vege_flora = 'V';

    screen_t screen = SCREEN_ONE;

    display_state_t state = NORMAL;

    while (true)
    {
        if (xQueueReceive(display_manager_queue, &display_ev, portMAX_DELAY) == pdTRUE)
        {
            switch (display_ev.cmd)
            {
            case CMD_UNDEFINED:
                break;
            case START_DISPLAY:
                display_init();
                display_set_screen_one(&screen, display_ev.pwm_value, display_ev.vege_flora, true, true, 10, 10);
                // display_set_screen(display_ev.pwm_value, display_ev.vege_flora);
                break;
            case UPDATE_DISPLAY:
                display_set_power(display_ev.pwm_value, display_ev.vege_flora);
                break;
            case UPDATE_VEGE_FLORA_ON_SCREEN:
                display_set_vege_flora(display_ev.vege_flora);
                break;
            case CHANGE_SCREEN:
                if (screen == SCREEN_ONE)
                {
                    display_set_screen_two(&screen);
                    ESP_LOGI(TAG, "Pantalla %u", screen);
                }
                else if (screen == SCREEN_TWO)
                {
                    display_set_screen_three(&screen);
                    ESP_LOGI(TAG, "Pantalla %u", screen);
                }
                else // screen = SCREEN_THREE
                {
                    display_set_screen_one(&screen, display_ev.pwm_value, display_ev.vege_flora, true, true, 10, 10);
                    ESP_LOGI(TAG, "Pantalla %u", screen);
                }
                break;
            case CONFIG:
                if (screen == SCREEN_ONE)
                {
                    display_set_screen_two(&screen);
                }
                else if (screen == SCREEN_TWO)
                {
                    display_set_screen_three(&screen);
                }
                else // screen = SCREEN_THREE
                {
                    display_set_screen_one(&screen, display_ev.pwm_value, display_ev.vege_flora, true, true, 10, 10);
                }
                break;

            default:
                break;
            }
        }
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void display_manager_init(void)
{
    set_i2c();
    ESP_LOGI(TAG, "DISPLAY inicializado");

    display_manager_queue = xQueueCreate(QUEUE_ELEMENT_QUANTITY, sizeof(display_event_t));

    xTaskCreate(display_manager_task, "display_manager_task", configMINIMAL_STACK_SIZE * 10,
                NULL, configMAX_PRIORITIES - 2, NULL);
}
//------------------------------------------------------------------------------
void display_manager_start(uint8_t pwm_value, char vege_flora)
{
    display_event_t display_ev;

    display_ev.cmd = START_DISPLAY;
    display_ev.pwm_value = pwm_value;
    display_ev.vege_flora = vege_flora;
    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_refresh(uint8_t pwm_value, char vege_flora)
{
    display_event_t display_ev;

    display_ev.cmd = UPDATE_DISPLAY;
    display_ev.pwm_value = pwm_value;
    display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_refreshvege_flora(char vege_flora)
{
    display_event_t display_ev;

    display_ev.cmd = UPDATE_VEGE_FLORA_ON_SCREEN;
    display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_change_screen(uint8_t pwm_value, char vege_flora)
{
    display_event_t display_ev;

    display_ev.cmd = CHANGE_SCREEN;
    display_ev.pwm_value = pwm_value;
    display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}

void display_manager_config_screen()
{
    display_event_t display_ev;

    display_ev.cmd = CONFIG;
    // display_ev.pwm_value = pwm_value;
    // display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}

//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------
