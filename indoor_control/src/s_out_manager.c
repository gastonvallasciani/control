


//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "../include/s_out_manager.h"
#include "../include/board_def.h"
#include "../include/global_manager.h"
#include "../include/jumpers_manager.h"

//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------
static void s_out_config(void);
static void s_out_manager_task(void *arg);

//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
static void s_out_config(void)
{
    gpio_config_t io_conf;
    
    io_conf.intr_type = GPIO_INTR_DISABLE;                 
    io_conf.mode = GPIO_MODE_OUTPUT;                       
    io_conf.pin_bit_mask = (1ULL << S_OUT); // Este pin solo puede ser usado como input GPIO_NUM_35
    io_conf.pull_down_en = 0;                              
    io_conf.pull_up_en = 0;                               
    gpio_config(&io_conf);

    gpio_set_level(S_OUT, 0);
}
//------------------------------------------------------------------------------
static void s_out_manager_task(void *arg)
{
    uint8_t pwm_analog_per_value, pwm_analog_per_value_ant;
    uint8_t pwm_digital_per_value, pwm_digital_per_value_ant;
    pwm_mode_t pwm_mode;
    uint8_t auto_pwm_output_status;

    pwm_analog_per_value_ant = 0;
    pwm_digital_per_value_ant = 0;
    gpio_set_level(S_OUT, 0);

    while (true)
    {
        if(global_manager_is_device_in_phase_3() == true)
        {
            global_manager_get_pwm_mode(&pwm_mode);

            if (pwm_mode == PWM_MANUAL)
            {
                if(is_jp3_teclas_connected() == false)
                {
                    global_manager_get_pwm_analog_percentage(&pwm_analog_per_value);
                    if(pwm_analog_per_value != pwm_analog_per_value_ant)
                    {
                        if(pwm_analog_per_value > 0)
                        {
                            gpio_set_level(S_OUT, 1);
                        }
                        else
                        {
                            gpio_set_level(S_OUT, 0);
                        }   
                    }
                    pwm_analog_per_value_ant = pwm_analog_per_value;

                }
                else if (is_jp3_teclas_connected() == true)
                {
                    global_manager_get_pwm_digital_percentage(&pwm_digital_per_value);
                    if(pwm_digital_per_value != pwm_digital_per_value_ant)
                    {
                        if(pwm_digital_per_value > 0)
                        {
                            gpio_set_level(S_OUT, 1);
                        }
                        else
                        {
                            gpio_set_level(S_OUT, 0);
                        }   
                    }
                    pwm_digital_per_value_ant = pwm_digital_per_value;
                }
            }
            else if(pwm_mode == PWM_AUTOMATIC)
            {
                global_manager_get_automatic_pwm_output_status(&auto_pwm_output_status);

                if(auto_pwm_output_status == 1)
                {
                    gpio_set_level(S_OUT, 1);
                }
                else
                {
                    gpio_set_level(S_OUT, 0);
                }

            }    
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void s_out_manager_init(void)
{
    s_out_config();
   
    xTaskCreate(s_out_manager_task, "s_out_manager_task", configMINIMAL_STACK_SIZE * 4,
                NULL, configMAX_PRIORITIES - 2, NULL);

}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void s_out_manager_turn_on_s_out(void)
{
    if(global_manager_is_device_in_phase_3() == false)
    {
        gpio_set_level(S_OUT, 1);
    }
}
//------------------------------------------------------------------------------
void s_out_manager_turn_off_s_out(void)
{
    if(global_manager_is_device_in_phase_3() == false)
    {
        gpio_set_level(S_OUT, 0);
    }
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------