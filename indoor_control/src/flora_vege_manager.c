//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>
#include <assert.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../include/board_def.h"
#include "../include/flora_vege_manager.h"
#include "../include/pwm_manager.h"
#include "../include/jumpers_manager.h"
#include "../include/global_manager.h"
#include "../include/led_manager.h"

//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define DEBUG_MODULE
#define GPIO_OUTPUT_PIN_SEL  (1ULL << S_VEGE)
//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------


//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------
static void flora_vege_manager_turn_on(void);
static void flora_vege_manager_turn_off(void);
//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
static void flora_vege_manager_turn_on(void)
{
    uint8_t pwm_manual_value = 0;
    #ifdef DEBUG_MODULE
        printf("TURN ON RELE VEGE \n");
    #endif

    // Cuando el pwm se maneja por potenciometro y no por teclas
    if(!is_jp3_teclas_connected())
    {
        global_manager_get_pwm_analog_percentage(&pwm_manual_value);
        pwm_manager_turn_off_pwm();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(S_VEGE, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        pwm_manager_turn_on_pwm(pwm_manual_value);
    }
    else
    {
        gpio_set_level(S_VEGE, 1);
    }
    
}
//------------------------------------------------------------------------------
static void flora_vege_manager_turn_off(void)
{
    uint8_t pwm_manual_value = 0;
    #ifdef DEBUG_MODULE
        printf("TURN OFF RELE VEGE \n");
    #endif
    // Cuando el pwm se maneja por potenciometro y no por teclas
    if(!is_jp3_teclas_connected())
    {
        global_manager_get_pwm_analog_percentage(&pwm_manual_value);
        pwm_manager_turn_off_pwm();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(S_VEGE, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        pwm_manager_turn_on_pwm(pwm_manual_value);
    }
    else
    {
        gpio_set_level(S_VEGE, 0);
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void flora_vege_manager_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // desactivar interrupción
    io_conf.mode = GPIO_MODE_OUTPUT; // establecer en modo salida
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL; // configurar pin
    io_conf.pull_down_en = 0; // desactivar pull-down
    io_conf.pull_up_en = 0; // desactivar pull-up
    gpio_config(&io_conf);

    gpio_set_level(S_VEGE, 0);
}
//------------------------------------------------------------------------------
void flora_vege_turn_on(void)
{
    flora_vege_manager_turn_on();
    led_manager_rele_vege_on();
}
//------------------------------------------------------------------------------
void flora_vege_turn_off(void)
{
    flora_vege_manager_turn_off();
    led_manager_rele_vege_off();
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------