// NOTAS: No se puede printear ni loguear desde una ISR se reinicia el ESP32.
//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

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
#include "../include/pwm_manager.h"
#include "../include/jumpers_manager.h"
#include "../include/display_manager.h"

//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define QUEUE_ELEMENT_QUANTITY 20

#define TIEMPO_ANTIRREBOTE_MS 50
#define TIEMPO_PULSADO_MS 3000

#define DEBUG_MODULE
#define PWM_BUTTON_REPEAT_INTERVAL_MS 100 // Intervalo para repetir el evento
#define BUTTON_DEBOUNCE_TIME_MS 50        // Tiempo de anti-rebote

//------------------------------TYPEDEF-----------------------------------------
//------------------------------------------------------------------------------
typedef enum
{
    CMD_UNDEFINED,
    STARTUP,
    VEGE_BUTTON_PUSHED,
    PWM_DOWN_BUTTON_PUSHED,
    PWM_UP_BUTTON_PUSHED,
    AUX_BUTTON_PUSHED,
    AUX_BUTTON_PUSHED_3_SECONDS,
    FABRIC_RESET,
    CALIBRATE_POTE,
    BOTON_PRESIONADO,
    BOTON_LIBERADO,
} cmds_t;

typedef struct
{
    cmds_t cmd;
} button_events_t;
//--------------------DECLARACION DE DATOS INTERNOS-----------------------------
//------------------------------------------------------------------------------
static QueueHandle_t button_manager_queue;

volatile int64_t start_time_flora_vege = 0;
volatile int64_t start_time_aux = 0;

static TimerHandle_t pwm_down_timer;
static TimerHandle_t pwm_up_timer;

static TimerHandle_t aux_button_timer = NULL; // Timer para manejar el tiempo de 3 segundos

static int64_t last_time_pwm_down = 0;
static int64_t last_time_pwm_up = 0;
//--------------------DECLARACION DE FUNCIONES INTERNAS-------------------------
//------------------------------------------------------------------------------
static void button_event_manager_task(void *pvParameters);
static void config_buttons_isr(void);
static void vege_button_interrupt(void *arg);
static void pwm_button_down_interrupt(void *arg);
static void pwm_button_up_interrupt(void *arg);
static void aux_button_interrupt(void *arg);

static void pwm_up_timer_callback(TimerHandle_t xTimer);
static void pwm_down_timer_callback(TimerHandle_t xTimer);

//--------------------DEFINICION DE DATOS INTERNOS------------------------------
//------------------------------------------------------------------------------

//--------------------DEFINICION DE DATOS EXTERNOS------------------------------
//------------------------------------------------------------------------------

//--------------------DEFINICION DE FUNCIONES INTERNAS--------------------------
//------------------------------------------------------------------------------
static void config_buttons_isr(void)
{
    gpio_config_t config;

    gpio_install_isr_service(0);

    config.pin_bit_mask = (1ULL << BT_VE_FLO);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLDOWN_ENABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_ANYEDGE;
    gpio_config(&config);
    gpio_isr_handler_add(BT_VE_FLO, vege_button_interrupt, NULL);

    config.pin_bit_mask = (1ULL << BT_DW);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_ANYEDGE;
    gpio_config(&config);
    gpio_isr_handler_add(BT_DW, pwm_button_down_interrupt, NULL);

    config.pin_bit_mask = (1ULL << BT_UP);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_ANYEDGE;
    gpio_config(&config);
    gpio_isr_handler_add(BT_UP, pwm_button_up_interrupt, NULL);

    config.pin_bit_mask = (1ULL << BT_AUX);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_ANYEDGE;
    gpio_config(&config);
    gpio_isr_handler_add(BT_AUX, aux_button_interrupt, NULL);
}

//------------------------------------------------------------------------------
static void aux_button_timer_callback(TimerHandle_t xTimer)
{
    button_events_t ev;
    ev.cmd = AUX_BUTTON_PUSHED_3_SECONDS;

    // Enviar evento a la cola solo si el botón sigue presionado
    if (gpio_get_level(BT_AUX) == 0)
    {
        xQueueSendFromISR(button_manager_queue, &ev, pdFALSE);
    }
}
//------------------------------------------------------------------------------
static void IRAM_ATTR aux_button_interrupt(void *arg)
{
    button_events_t ev;
    int64_t time_now = esp_timer_get_time();

    if (gpio_get_level(BT_AUX) == 0) // Botón presionado
    {
        start_time_aux = time_now;

        if (aux_button_timer == NULL)
        {
            // Crear el temporizador si no existe
            aux_button_timer = xTimerCreate("Aux Button Timer",
                                            pdMS_TO_TICKS(3000), // 3 segundos
                                            pdFALSE,             // No repetitivo
                                            NULL, aux_button_timer_callback);
        }
        // Reiniciar y empezar el temporizador
        xTimerStartFromISR(aux_button_timer, NULL);
    }
    else // Botón liberado
    {
        // Detener el temporizador en caso de que no haya expirado
        if (aux_button_timer != NULL && xTimerIsTimerActive(aux_button_timer))
        {
            xTimerStopFromISR(aux_button_timer, NULL);
        }

        // Calcular el tiempo que el botón estuvo presionado
        int64_t diff = time_now - start_time_aux;

        if (start_time_aux != 0 && diff >= 3000000) // 3 segundos o más
        {
            //ev.cmd = AUX_BUTTON_PUSHED_3_SECONDS;
        }
        else if (start_time_aux != 0 && diff >= 30000) // 30ms seconds expressed in microseconds
        {
            ev.cmd = AUX_BUTTON_PUSHED;
            xQueueSendFromISR(button_manager_queue, &ev, pdFALSE);
        }

        aux_button_timer = NULL;
        start_time_aux = 0; // Reiniciar el tiempo de inicio
    }
}
//------------------------------------------------------------------------------
static void IRAM_ATTR vege_button_interrupt(void *arg)
{
    button_events_t ev;
    int64_t time_now = esp_timer_get_time();

    if (gpio_get_level(BT_VE_FLO) == 0)
    {
        start_time_flora_vege = time_now;
    }
    else
    {
        if (start_time_flora_vege != 0)
        {
            int64_t diff = time_now - start_time_flora_vege;

            if (diff > 30000) // 30ms seconds expressed in microseconds
            {
                ev.cmd = VEGE_BUTTON_PUSHED;
                xQueueSendFromISR(button_manager_queue, &ev, pdFALSE);
            }
            start_time_flora_vege = 0;
        }
    }
}
//------------------------------------------------------------------------------
static void pwm_down_timer_callback(TimerHandle_t xTimer)
{
    button_events_t ev;
    ev.cmd = PWM_DOWN_BUTTON_PUSHED;
    xQueueSendFromISR(button_manager_queue, &ev, pdFALSE);
}
//------------------------------------------------------------------------------
static IRAM_ATTR void pwm_button_down_interrupt(void *arg)
{
    int64_t time_now = esp_timer_get_time();

    if (gpio_get_level(BT_DW) == 0)
    { // Botón presionado
        if (time_now - last_time_pwm_down > pdMS_TO_TICKS(50))
        {
            last_time_pwm_down = time_now;
            // Iniciar el temporizador si no está ya iniciado
            if (pwm_down_timer == NULL)
            {
                pwm_down_timer = xTimerCreate("PWM Up Timer",
                                              pdMS_TO_TICKS(PWM_BUTTON_REPEAT_INTERVAL_MS),
                                              pdTRUE, NULL, pwm_down_timer_callback);
            }
            xTimerStart(pwm_down_timer, 0);
        }
    }
    else
    { // Botón liberado
        if (pwm_down_timer != NULL)
        {
            xTimerStop(pwm_down_timer, 0);
            pwm_down_timer = 0;
        }
    }
}
//------------------------------------------------------------------------------
static void pwm_up_timer_callback(TimerHandle_t xTimer)
{
    button_events_t ev;
    ev.cmd = PWM_UP_BUTTON_PUSHED;
    xQueueSendFromISR(button_manager_queue, &ev, pdFALSE);
}
//------------------------------------------------------------------------------
static IRAM_ATTR void pwm_button_up_interrupt(void *arg)
{
    int64_t time_now = esp_timer_get_time();

    if (gpio_get_level(BT_UP) == 0)
    { // Botón presionado
        if (time_now - last_time_pwm_up > pdMS_TO_TICKS(50))
        {
            last_time_pwm_up = time_now;
            // Iniciar el temporizador si no está ya iniciado
            if (pwm_up_timer == NULL)
            {
                pwm_up_timer = xTimerCreate("PWM Up Timer",
                                            pdMS_TO_TICKS(PWM_BUTTON_REPEAT_INTERVAL_MS),
                                            pdTRUE, NULL, pwm_up_timer_callback);
            }
            xTimerStart(pwm_up_timer, 0);
        }
    }
    else
    { // Botón liberado
        if (pwm_up_timer != NULL)
        {
            xTimerStop(pwm_up_timer, 0);
            pwm_up_timer = 0;
        }
    }
}

//------------------------------------------------------------------------------
void button_event_manager_task(void *pvParameters)
{
    button_events_t button_ev;
    flora_vege_status_t flora_vege_status;
    uint8_t pwm_digital_per_value = 0;
    pwm_mode_t pwm_mode;
    display_state_t screen_state;
    config_buttons_isr();

    while (true)
    {
        if (xQueueReceive(button_manager_queue, &button_ev, portMAX_DELAY) == pdTRUE)
        {
            switch (button_ev.cmd)
            {
            case BOTON_PRESIONADO:
                printf("BOTON PRESIONADO\n");
                break;
            case BOTON_LIBERADO:
                printf("BOTON LIBERADO\n ");
                break;
            case CMD_UNDEFINED:
                break;
            case VEGE_BUTTON_PUSHED:
                global_manager_get_flora_vege_status(&flora_vege_status);
                get_screen_state(&screen_state);
                if (screen_state == NORMAL)
                {
                    if (flora_vege_status == FLORA_VEGE_OUTPUT_DISABLE)
                    {
#ifdef DEBUG_MODULE
                        printf("FLORA VEGE OUPUT STATUS ENABLE \n");
#endif
                        global_manager_set_flora_vege_status(FLORA_VEGE_OUTPUT_ENABLE);
                        flora_vege_status = FLORA_VEGE_OUTPUT_ENABLE;
                        flora_vege_turn_on();
                    }
                    else if (flora_vege_status == FLORA_VEGE_OUTPUT_ENABLE)
                    {
#ifdef DEBUG_MODULE
                        printf("FLORA VEGE OUPUT STATUS DISABLE \n");
#endif
                        global_manager_set_flora_vege_status(FLORA_VEGE_OUTPUT_DISABLE);
                        flora_vege_status = FLORA_VEGE_OUTPUT_DISABLE;
                        flora_vege_turn_off();
                    }
                }
                display_manager_vf(flora_vege_status);
                break;
            case PWM_DOWN_BUTTON_PUSHED:
                if (is_jp3_teclas_connected() == true)
                {

                    get_screen_state(&screen_state);
                    if (screen_state == NORMAL)
                    {
                        global_manager_get_pwm_digital_percentage(&pwm_digital_per_value);
                        if (pwm_digital_per_value > 10)
                        {
                            pwm_digital_per_value--;
                        }
                        else if (pwm_digital_per_value == 10)
                        {
                            pwm_digital_per_value = 0;
                        }
                        printf("Boton PWM DW presionado, pwm digital value: %d \n", pwm_digital_per_value);
                        global_manager_set_pwm_digital_percentage(pwm_digital_per_value);
                        global_manager_get_pwm_mode(&pwm_mode);
                        if (pwm_mode == PWM_MANUAL)
                        {
                            pwm_manager_turn_on_pwm(pwm_digital_per_value);
                            led_manager_pwm_output(pwm_digital_per_value);
                        }
                    }
                }
                display_manager_down(pwm_digital_per_value, flora_vege_status); // Envio evento button down en al display
                break;
            case PWM_UP_BUTTON_PUSHED:
                if (is_jp3_teclas_connected() == true)
                {
                    get_screen_state(&screen_state);
                    if (screen_state == NORMAL)
                    {
                        global_manager_get_pwm_digital_percentage(&pwm_digital_per_value);

                        if (pwm_digital_per_value == 0)
                        {
                            pwm_digital_per_value = 10;
                        }
                        else if (pwm_digital_per_value < 100)
                        {
                            pwm_digital_per_value++;
                        }
                        printf("Boton PWM UP presionado, pwm digital value: %d \n", pwm_digital_per_value);
                        global_manager_set_pwm_digital_percentage(pwm_digital_per_value);
                        global_manager_get_pwm_mode(&pwm_mode);
                        if (pwm_mode == PWM_MANUAL)
                        {
                            pwm_manager_turn_on_pwm(pwm_digital_per_value);
                            led_manager_pwm_output(pwm_digital_per_value);
                        }
                    }
                }
                display_manager_up(pwm_digital_per_value, flora_vege_status); // Envio evento button up en al display
                break;
            case FABRIC_RESET:
                nv_flash_driver_erase_flash();
                break;
            case AUX_BUTTON_PUSHED:
                display_manager_aux();
                break;
            case AUX_BUTTON_PUSHED_3_SECONDS:

                display_manager_auxt();
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
                configMINIMAL_STACK_SIZE * 5, NULL, configMAX_PRIORITIES - 2, NULL);
}
//--------------------FIN DEL ARCHIVO-------------------------------------------
//------------------------------------------------------------------------------