//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "sdkconfig.h"

#include "../include/global_manager.h"
#include "../include/jumpers_manager.h"
#include "../include/pote_input_manager.h"
#include "../include/button_manager.h"
#include "../include/nv_flash_driver.h"
#include "../include/led_manager.h"
#include "../include/flora_vege_manager.h"
#include "../include/pwm_manager.h"
#include "../include/led_manager.h"

//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define DEBUG_MODULE
//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------
static SemaphoreHandle_t global_manager_semaph;
static global_manager_t global_manager_info;
//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------
static void global_manager_task(void* arg);

//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
device_mode_t global_manager_find_device_mode(void)
{
    device_mode_t device_mode;

    if((is_jp2_reloj_connected() == false) && (is_jp3_teclas_connected() == false) 
        && (is_jp1_dspy_connected() == false))
    {
        device_mode = MODE_1; // FASE 1
    }
    else if((is_jp2_reloj_connected() == false) && (is_jp3_teclas_connected() == false) 
        && (is_jp1_dspy_connected() == true))
    {
        device_mode = MODE_2; // FASE 2 con pote
    }
    else if((is_jp2_reloj_connected() == false) && (is_jp3_teclas_connected() == true) 
        && (is_jp1_dspy_connected() == true))
    {
        device_mode = MODE_3; // FASE 2 con tecla
    }
    else if((is_jp2_reloj_connected() == true) && (is_jp3_teclas_connected() == true) 
        && (is_jp1_dspy_connected() == true))
    {
        device_mode = MODE_4; // FASE 3 con reloj montado y teclas
    }
    else
    {
        device_mode = MODE_1; // FASE 1
    }

    return(device_mode);
}
//------------------------------------------------------------------------------
static void global_manager_task(void* arg)
{
    uint8_t pwm_manual_value = 0, pwm_value_bkp = 0;
    int8_t pwm_diff = 0;
    if(is_jp3_teclas_connected() == false)
    {
        global_manager_get_pwm_manual_percentage(&pwm_manual_value);
        pwm_manager_turn_on_pwm(pwm_manual_value);
        led_manager_pwm_output(pwm_manual_value);

        pwm_value_bkp = pwm_manual_value;
    }

    while(true)
    {
        if(is_jp3_teclas_connected() == false)
        {
            global_manager_get_pwm_manual_percentage(&pwm_manual_value);
            
            pwm_diff = (int8_t)pwm_manual_value - (int8_t)pwm_value_bkp;
            
            if((pwm_diff > 1) || (pwm_diff < -1))
            {
                #ifdef DEBUG_MODULE
                    printf("Update PWM to value: %d \n", pwm_manual_value);
                #endif  
                pwm_manager_update_pwm(pwm_manual_value);
                led_manager_pwm_output(pwm_manual_value);
                pwm_value_bkp = pwm_manual_value;
            }
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void global_manager_init(void)
{
    device_mode_t device_mode;
    global_manager_semaph = xSemaphoreCreateBinary(); 

    xTaskCreate(global_manager_task, "global_manager_task", 
        configMINIMAL_STACK_SIZE*4, NULL, configMAX_PRIORITIES-2, NULL);

    if(global_manager_semaph != NULL)
        xSemaphoreGive(global_manager_semaph);
    else
        assert(0);

    jumpers_manager_init();

    device_mode = global_manager_find_device_mode();
    global_manager_set_device_mode(device_mode);

    global_manager_set_pwm_mode(MANUAL);

    pote_input_manager_init();

    led_manager_init();

    flora_vege_manager_init();

    button_manager_init();

    pwm_manager_init();

}
//------------------------------------------------------------------------------

uint8_t global_manager_set_pwm_manual_percentage(uint8_t pwm_manual_per_value)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.pwm_manual_percent_power = pwm_manual_per_value;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_pwm_manual_percentage(uint8_t* pwm_manual_per_value)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *pwm_manual_per_value = global_manager_info.pwm_manual_percent_power;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------

uint8_t global_manager_set_device_mode(device_mode_t device_mode)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.device_mode = device_mode;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------

uint8_t global_manager_get_device_mode(device_mode_t *device_mode)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        *device_mode = global_manager_info.device_mode;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------

uint8_t global_manager_set_pwm_mode(pwm_mode_t pwm_mode)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.nv_info.pwm_mode = pwm_mode;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_pwm_mode(pwm_mode_t* pwm_mode)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *pwm_mode = global_manager_info.nv_info.pwm_mode;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------

uint8_t global_manager_set_flora_vege_status(flora_vege_status_t flora_vege_status)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.nv_info.flora_vege_status = flora_vege_status;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_flora_vege_status(flora_vege_status_t* flora_vege_status)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *flora_vege_status = global_manager_info.nv_info.flora_vege_status;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------