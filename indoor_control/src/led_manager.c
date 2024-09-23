//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

#include "../include/led_manager.h"
#include "../include/board_def.h"
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define DEBUG_MODULE 1

#define QUEUE_ELEMENT_QUANTITY 25

#define MANUAL_DEVICE_MODE_TIME 200000
#define MANUAL_TRIAC_TIME 200000
#define RAMPA_PWM_TIME 800000

#define LED_ON 0
#define LED_OFF 1

#define MIN_BLINK_FREQ 1   // Frecuencia mínima de parpadeo (1 Hz)
#define MAX_BLINK_FREQ 10  // Frecuencia máxima de parpadeo (10 Hz)
//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------
typedef enum
{
    CMD_UNDEFINED = 0,
    RELE_VEGE_ON = 1,
    RELE_VEGE_OFF = 2,
    UPDATE_LED_PWM_OUTPUT = 3,
} led_event_cmds_t;

typedef struct
{
    uint8_t pwm_value;
    led_event_cmds_t cmd;
} led_event_t;
//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------
QueueHandle_t led_manager_queue;

//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------
static void config_led_rele_vege_status_up(void);

static void set_rele_vege_on_indicator(void);
static void set_rele_vege_off_indicator(void);

static void led_manager_task(void *arg);

static void timer_led_toggle_pwm_status_callback(void *arg);

//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------
static esp_timer_handle_t blink_timer;
static uint8_t pwm_val_bkp = 0;

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
static int map_value_to_frequency(int value) 
{
    return MIN_BLINK_FREQ + (MAX_BLINK_FREQ - MIN_BLINK_FREQ) * value / 100;
}
//------------------------------------------------------------------------------
void start_led_blink_timer(int pwm_value) {
    int freq = map_value_to_frequency(pwm_value);  // Obtener la frecuencia basada en el valor PWM
    pwm_val_bkp = pwm_value;
    int64_t period = 1000000;
    if(freq != 0)
    {
        period = (1000000 / freq) / 2;  // Periodo en microsegundos, dividido por 2 para el ciclo ON/OFF
    }

    if (blink_timer != NULL) {
        esp_timer_stop(blink_timer);  // Detener cualquier timer activo
    }

    // Crear y empezar el timer para hacer toggle del LED
    esp_timer_create_args_t timer_args = {
        .callback = &timer_led_toggle_pwm_status_callback,
        .name = "led_blink_timer"
    };
    esp_timer_create(&timer_args, &blink_timer);
    esp_timer_start_periodic(blink_timer, period);  // Empezar el timer con el periodo calculado
}
//------------------------------------------------------------------------------
// Callback para alternar el estado del LED
static void timer_led_toggle_pwm_status_callback(void *arg) {
    static bool led_on = false;
    if(pwm_val_bkp != 0)
    {
        gpio_set_level(LED_PWM, led_on ? LED_OFF : LED_ON);
        led_on = !led_on;
    }
    else
    {
        gpio_set_level(LED_PWM, LED_OFF);
        led_on = false;
    }
    
}
//------------------------------------------------------------------------------
static void config_led_rele_vege_status_up(void)
{
    gpio_config_t io_conf;
    
    io_conf.intr_type = GPIO_INTR_DISABLE;                 
    io_conf.mode = GPIO_MODE_OUTPUT;                       
    io_conf.pin_bit_mask = (1ULL << LED_FLORA); // Este pin solo puede ser usado como input GPIO_NUM_35
    io_conf.pull_down_en = 0;                              
    io_conf.pull_up_en = 0;                               
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_DISABLE;                 
    io_conf.mode = GPIO_MODE_OUTPUT;                      
    io_conf.pin_bit_mask = (1ULL << LED_VEGE); 
    io_conf.pull_down_en = 0;                        
    io_conf.pull_up_en = 0;                            
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_DISABLE;                 
    io_conf.mode = GPIO_MODE_OUTPUT;                      
    io_conf.pin_bit_mask = (1ULL << LED_PWM); 
    io_conf.pull_down_en = 0;                        
    io_conf.pull_up_en = 0;                            
    gpio_config(&io_conf);

    gpio_set_level(LED_FLORA, LED_OFF);
    gpio_set_level(LED_VEGE, LED_OFF);
    gpio_set_level(LED_PWM, LED_OFF);
}

//------------------------------------------------------------------------------
static void set_rele_vege_on_indicator(void)
{
#ifdef DEBUG_MODULE
    printf("Led vege ON \n");
    printf("Led flora OFF \n");
#endif
    gpio_set_level(LED_VEGE, LED_ON);
    gpio_set_level(LED_FLORA, LED_OFF);
}
//------------------------------------------------------------------------------
static void set_rele_vege_off_indicator(void)
{
#ifdef DEBUG_MODULE
    printf("Led vege OFF \n");
    printf("Led flora ON \n");
#endif
    gpio_set_level(LED_VEGE, LED_OFF);
    gpio_set_level(LED_FLORA, LED_VEGE);
}
//------------------------------------------------------------------------------
static void led_manager_task(void *arg)
{
    // const char *LED_MANAGER_TASK_TAG = "LED_MANAGER_TASK_TAG";
    led_event_t led_ev;

    while (1)
    {
        if (xQueueReceive(led_manager_queue, &led_ev, portMAX_DELAY) == pdTRUE)
        {
            switch (led_ev.cmd)
            {
            case CMD_UNDEFINED:
                break;
            case RELE_VEGE_ON:
                set_rele_vege_on_indicator();
                break;
            case RELE_VEGE_OFF:
                set_rele_vege_off_indicator();
                break;
            case UPDATE_LED_PWM_OUTPUT:
                start_led_blink_timer(led_ev.pwm_value);
                break;
            default:
                break;
            }
        }
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void led_manager_init(void)
{
    config_led_rele_vege_status_up();

    led_manager_queue = xQueueCreate(QUEUE_ELEMENT_QUANTITY, sizeof(led_event_t));

    xTaskCreate(led_manager_task, "led_manager_task", configMINIMAL_STACK_SIZE * 10,
                NULL, configMAX_PRIORITIES - 2, NULL);

}
//------------------------------------------------------------------------------
void led_manager_rele_vege_on(void)
{
    led_event_t ev;

    ev.cmd = RELE_VEGE_ON;

    xQueueSend(led_manager_queue, &ev, 10);
}
//------------------------------------------------------------------------------
void led_manager_rele_vege_off(void)
{
    led_event_t ev;

    ev.cmd = RELE_VEGE_OFF;

    xQueueSend(led_manager_queue, &ev, 10);
}
//------------------------------------------------------------------------------
void led_manager_pwm_output(uint8_t pwm_value)
{
    led_event_t ev;

    ev.cmd = UPDATE_LED_PWM_OUTPUT;
    ev.pwm_value = pwm_value;

    xQueueSend(led_manager_queue, &ev, 10);
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------