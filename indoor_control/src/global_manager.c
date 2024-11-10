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
#include "../include/nv_flash_manager.h"
#include "../include/display_manager.h"
#include "../include/s_out_manager.h"
#include "../include/current_time_manager.h"
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
static uint8_t nv_init_pwm_digital_value(void);
static pwm_mode_t nv_init_pwm_mode(void);
static simul_day_status_t nv_init_simul_day_status(void);
static calendar_auto_pwm_t nv_init_pwm_calendar(void);
static uint8_t nv_init_auto_percent_power(void);
static flora_vege_status_t nv_init_flora_vege_status(void);

static uint8_t global_manager_init_automatic_pwm_params(pwm_auto_info_t pwm_auto);
static uint8_t global_manager_init_pwm_mode(pwm_mode_t pwm_mode);
static uint8_t global_manager_init_flora_vege_status(flora_vege_status_t flora_vege_status);
//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
static uint8_t global_manager_init_automatic_pwm_params(pwm_auto_info_t pwm_auto)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        global_manager_info.nv_info.pwm_auto = pwm_auto;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
static uint8_t global_manager_init_pwm_mode(pwm_mode_t pwm_mode)
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
static uint8_t global_manager_init_pwm_digital_percentage(uint8_t pwm_digital_per_value)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {  
        global_manager_info.pwm_digital_percent_power = pwm_digital_per_value;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
static uint8_t global_manager_init_flora_vege_status(flora_vege_status_t flora_vege_status)
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
static calendar_auto_pwm_t nv_init_pwm_calendar(void)
{
    pwm_auto_info_t pwm_calendar;
    calendar_auto_pwm_t calendar;

    if (read_date_from_flash(PWM_DATE_ON_KEY, &pwm_calendar.turn_on_time))
    {
#ifdef DEBUG_MODULE
        printf("TURN ON TIME HOUR READ: %d \n", pwm_calendar.turn_on_time.tm_hour);
        printf("TURN ON TIME min READ: %d \n", pwm_calendar.turn_on_time.tm_min);
#endif
        calendar.turn_on_time = pwm_calendar.turn_on_time;
        calendar.read_ok = true;
    }
    else
    {
        calendar.read_ok = false;
#ifdef DEBUG_MODULE
        printf("TURN ON CALENDAR READING FAILED \n");
#endif
    }
    if (read_date_from_flash(PWM_DATE_OFF_KEY, &pwm_calendar.turn_off_time))
    {
#ifdef DEBUG_MODULE
        printf("TURN OFF TIME HOUR READ: %d \n", pwm_calendar.turn_off_time.tm_hour);
        printf("TURN OFF TIME min READ: %d \n", pwm_calendar.turn_off_time.tm_min);
#endif
        calendar.turn_off_time = pwm_calendar.turn_off_time;
        calendar.read_ok = true;
    }
    else
    {
        calendar.read_ok = false;
#ifdef DEBUG_MODULE
        printf("TURN OFF CALENDAR READING FAILED \n");
#endif
    }

    return(calendar);
}
//------------------------------------------------------------------------------
static uint8_t nv_init_auto_percent_power(void)
{
    uint32_t value;
    if (read_uint32_from_flash(PWM_PERCENT_POWER_KEY, &value))
    {
#ifdef DEBUG_MODULE
        printf("AUTO PERCENT POWER READ: %ld \n", value);
#endif
        //global_manager_set_pwm_power_value_auto((uint8_t)value, true);
    }
    else
    {
        value = 0;
#ifdef DEBUG_MODULE
        printf("AUTO PERCENT POWER READING FAILED \n");
#endif
    }
    return((uint8_t)value);
}
//------------------------------------------------------------------------------
static simul_day_status_t nv_init_simul_day_status(void)
{
    uint32_t value;
    simul_day_status_t simul_day_status;
    if (read_uint32_from_flash(SIMUL_DAY_STATUS_KEY, &value))
    {
        simul_day_status = (simul_day_status_t)value;
#ifdef DEBUG_MODULE
        printf("SIMUL DAY STATUS READ: %d \n", simul_day_status);
#endif
    }
    else
    {
#ifdef DEBUG_MODULE
        printf("SIMUL DAY STATUS READING FAILED \n");
#endif
        simul_day_status = SIMUL_DAY_UNDEFINED;
    }

    return(simul_day_status);
}
//------------------------------------------------------------------------------
static uint8_t nv_init_pwm_digital_value(void)
{
    uint32_t value;
    uint8_t ret_value;

    if (read_uint32_from_flash(PWM_DIGITAL_VALUE_KEY, &value))
    {
#ifdef DEBUG_MODULE
        printf("PWM_DIGITAL_VALUE READ: %lu \n", value);
#endif

        ret_value = (uint8_t)value;
    }
    else
    {
#ifdef DEBUG_MODULE
        printf("PWM_DIGITAL_VALUE READING FAILED \n");
#endif
        ret_value = 0;
    }
    return(ret_value);
}
//------------------------------------------------------------------------------
static pwm_mode_t nv_init_pwm_mode(void)
{
    uint32_t value;
    pwm_mode_t pwm_mode;
    if (read_uint32_from_flash(PWM_MODE_KEY, &value))
    {
        pwm_mode = (pwm_mode_t)value;
#ifdef DEBUG_MODULE
        printf("PWM MODE READ: %d \n", pwm_mode);
#endif
        return pwm_mode;
        //global_manager_set_pwm_mode(pwm_mode);
    }
    else
    {
#ifdef DEBUG_MODULE
        printf("PWM MODE READING FAILED \n");
#endif
        return PWM_UNDEFINED;
    }
}
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
    
    pwm_auto_info_t pwm_auto_info;
    struct tm current_time;
    bool pwm_auto_status = false;

    pwm_auto_info.output_status = PWM_OUTPUT_OFF;
    pwm_auto_info.update_calendar = false;

    uint8_t pwm_digital_value = nv_init_pwm_digital_value();
    pwm_mode_t pwm_mode = nv_init_pwm_mode();
    calendar_auto_pwm_t pwm_calendar = nv_init_pwm_calendar();
    flora_vege_status_t flora_vege_status = nv_init_flora_vege_status();


    pwm_auto_info.percent_power = nv_init_auto_percent_power();
    pwm_auto_info.simul_day_status = nv_init_simul_day_status();
    if(pwm_calendar.read_ok)
    {
        pwm_auto_info.turn_on_time = pwm_calendar.turn_off_time;
        pwm_auto_info.turn_off_time = pwm_calendar.turn_off_time;
    }

    global_manager_init_automatic_pwm_params(pwm_auto_info);
    global_manager_init_pwm_mode(pwm_mode);
    global_manager_init_pwm_digital_percentage(pwm_digital_value);
    global_manager_init_flora_vege_status(flora_vege_status);

    printf("INICIO PRINT DEBUG \n");
    printf("PWM DIGITAL VALUE %d \n", pwm_digital_value);
    printf("PWM MODE VALUE %d \n", pwm_mode);

    printf("TURN OFF TIME: %02d-%02d-%02d %02d:%02d:%02d\n", 
        pwm_calendar.turn_off_time.tm_year + 1900, pwm_calendar.turn_off_time.tm_mon + 1, pwm_calendar.turn_off_time.tm_mday, 
        pwm_calendar.turn_off_time.tm_hour, pwm_calendar.turn_off_time.tm_min, pwm_calendar.turn_off_time.tm_sec);

    printf("TURN ON TIME: %02d-%02d-%02d %02d:%02d:%02d\n", 
        pwm_calendar.turn_on_time.tm_year + 1900, pwm_calendar.turn_on_time.tm_mon + 1, pwm_calendar.turn_on_time.tm_mday, 
        pwm_calendar.turn_on_time.tm_hour, pwm_calendar.turn_on_time.tm_min, pwm_calendar.turn_on_time.tm_sec);

    printf("FLORA VEGE VALUE %d \n", flora_vege_status);

    printf("AUTO PERCENT POWER %d \n",  pwm_auto_info.percent_power);
    printf("SIMUL DAY STATUS %d \n",  pwm_auto_info.simul_day_status);
    printf("FIN PRINT DEBUG \n");

    if(flora_vege_status == FLORA_VEGE_OUTPUT_ENABLE)
    {
        flora_vege_turn_on();
    }
    
    if(is_jp3_teclas_connected() == false)
    {
        global_manager_get_pwm_analog_percentage(&pwm_manual_value);
        pwm_manager_turn_on_pwm(pwm_manual_value);
        led_manager_pwm_output(pwm_manual_value);
        if(flora_vege_status == FLORA_VEGE_OUTPUT_ENABLE)
            display_manager_start(pwm_manual_value, 'V');
        else
            display_manager_start(pwm_manual_value, 'F');
        pwm_value_bkp = pwm_manual_value;
    }
    else if (is_jp3_teclas_connected() == true)
    {
        pwm_manager_turn_on_pwm(pwm_digital_value);
        led_manager_pwm_output(pwm_digital_value);
        if(flora_vege_status == FLORA_VEGE_OUTPUT_ENABLE)
            display_manager_start(pwm_digital_value, 'V');
        else
            display_manager_start(pwm_digital_value, 'F');
    }

    while(true)
    {
        global_manager_get_pwm_mode(&pwm_mode);

        if(pwm_mode == PWM_MANUAL)
        {
            if(is_jp3_teclas_connected() == false)
            {
                global_manager_get_pwm_analog_percentage(&pwm_manual_value);
                global_manager_get_flora_vege_status(&flora_vege_status);
                
                pwm_diff = (int8_t)pwm_manual_value - (int8_t)pwm_value_bkp;
                
                if((pwm_diff > 1) || (pwm_diff < -1))
                {
                    #ifdef DEBUG_MODULE
                        printf("Update PWM to value: %d \n", pwm_manual_value);
                    #endif  
                    pwm_manager_update_pwm(pwm_manual_value);
                    led_manager_pwm_output(pwm_manual_value);

                    if(flora_vege_status == FLORA_VEGE_OUTPUT_ENABLE)
                        display_manager_refresh(pwm_manual_value, 'V');
                    else
                        display_manager_refresh(pwm_manual_value, 'F');
        
                    pwm_value_bkp = pwm_manual_value;
                }
            }
        }
        
        global_manager_get_pwm_automatic_info(&pwm_auto_info);
        if(global_manager_get_current_time_info(&current_time))
            pwm_auto_info.current_time = current_time;

        if(pwm_mode == PWM_AUTOMATIC)
            pwm_auto_status = true;
        else
            pwm_auto_status = false;

        pwm_auto_manager_handler(&pwm_auto_info, pwm_auto_status);
        global_manager_set_pwm_automatic_info(pwm_auto_info);


        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void global_manager_init(void)
{
    device_mode_t device_mode;
    global_manager_semaph = xSemaphoreCreateBinary(); 

    led_manager_init();
    pwm_manager_init();
    button_manager_init();
    display_manager_init();
    s_out_manager_init();
    current_time_manager_init();


    xTaskCreate(global_manager_task, "global_manager_task", 
        configMINIMAL_STACK_SIZE*4, NULL, configMAX_PRIORITIES-2, NULL);

    if(global_manager_semaph != NULL)
        xSemaphoreGive(global_manager_semaph);
    else
        assert(0);

    jumpers_manager_init();
    

    device_mode = global_manager_find_device_mode();
    global_manager_set_device_mode(device_mode);

    global_manager_set_pwm_mode(PWM_MANUAL);

    pote_input_manager_init();

    flora_vege_manager_init();
}
//------------------------------------------------------------------------------

uint8_t global_manager_set_pwm_analog_percentage(uint8_t pwm_analog_per_value)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.pwm_analog_percent_power = pwm_analog_per_value;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_pwm_analog_percentage(uint8_t* pwm_analog_per_value)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *pwm_analog_per_value = global_manager_info.pwm_analog_percent_power;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------

uint8_t global_manager_set_pwm_digital_percentage(uint8_t pwm_digital_per_value)
{
    uint8_t aux = 0;
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {  
        aux =  global_manager_info.pwm_digital_percent_power;
        global_manager_info.pwm_digital_percent_power = pwm_digital_per_value;
        xSemaphoreGive(global_manager_semaph);
        if(aux != pwm_digital_per_value)
            write_parameter_on_flash_uint32(PWM_DIGITAL_VALUE_KEY, (uint32_t)pwm_digital_per_value);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_pwm_digital_percentage(uint8_t* pwm_digital_per_value)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *pwm_digital_per_value = global_manager_info.pwm_digital_percent_power;
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
    pwm_mode_t pwm_mode_aux;
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {   
        pwm_mode_aux = global_manager_info.nv_info.pwm_mode;   
        global_manager_info.nv_info.pwm_mode = pwm_mode;
        xSemaphoreGive(global_manager_semaph);
        if(pwm_mode_aux != pwm_mode)
        {
            write_parameter_on_flash_uint32(PWM_MODE_KEY, (uint32_t)pwm_mode);
        }
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
    uint8_t aux = 0;
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        aux = global_manager_info.nv_info.flora_vege_status;
        global_manager_info.nv_info.flora_vege_status = flora_vege_status;
        xSemaphoreGive(global_manager_semaph);
        if(aux != flora_vege_status)
            write_parameter_on_flash_uint32(RELE_VEGE_STATUS_KEY, (uint32_t)flora_vege_status);
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
//------------------------------------------------------------------------------
uint8_t global_manager_set_simul_day_status(simul_day_status_t simul_day_status)
{
    simul_day_status_t aux;
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        aux = global_manager_info.nv_info.pwm_auto.simul_day_status;
        global_manager_info.nv_info.pwm_auto.simul_day_status = simul_day_status;
        xSemaphoreGive(global_manager_semaph);
        if(aux != simul_day_status)
            write_parameter_on_flash_uint32(SIMUL_DAY_STATUS_KEY, (uint32_t)simul_day_status);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_simul_day_status(simul_day_status_t* simul_day_status)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *simul_day_status = global_manager_info.nv_info.pwm_auto.simul_day_status;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_set_turn_on_time(struct tm turn_on_time)
{
    struct tm date_aux;

    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        date_aux = global_manager_info.nv_info.pwm_auto.turn_on_time;
        global_manager_info.nv_info.pwm_auto.turn_on_time = turn_on_time;
        xSemaphoreGive(global_manager_semaph);
        if((date_aux.tm_hour != turn_on_time.tm_hour) || (date_aux.tm_min != turn_on_time.tm_min))
        {
            write_date_on_flash(PWM_DATE_OFF_KEY, global_manager_info.nv_info.pwm_auto.turn_on_time);
        }
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_turn_on_time(struct tm* turn_on_time)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *turn_on_time = global_manager_info.nv_info.pwm_auto.turn_on_time;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_set_turn_off_time(struct tm turn_off_time)
{
    struct tm date_aux;

    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        date_aux = global_manager_info.nv_info.pwm_auto.turn_off_time;
        global_manager_info.nv_info.pwm_auto.turn_off_time = turn_off_time;
        xSemaphoreGive(global_manager_semaph);

        if((date_aux.tm_hour != turn_off_time.tm_hour) || (date_aux.tm_min != turn_off_time.tm_min))
        {
            write_date_on_flash(PWM_DATE_OFF_KEY, global_manager_info.nv_info.pwm_auto.turn_off_time);
        }
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_turn_off_time(struct tm* turn_off_time)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *turn_off_time = global_manager_info.nv_info.pwm_auto.turn_off_time;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_update_pwm_calendar_info(void)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.nv_info.pwm_auto.update_calendar = true;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_set_pwm_automatic_info(pwm_auto_info_t pwm_auto)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.nv_info.pwm_auto = pwm_auto;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_pwm_automatic_info(pwm_auto_info_t *pwm_auto)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *pwm_auto = global_manager_info.nv_info.pwm_auto;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_set_automatic_pwm_power(uint8_t auto_pwm_power)
{
    uint8_t aux = 0;

    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        aux = global_manager_info.nv_info.pwm_auto.percent_power;     
        global_manager_info.nv_info.pwm_auto.percent_power = auto_pwm_power;
        xSemaphoreGive(global_manager_semaph);
        if(aux != auto_pwm_power)
            write_parameter_on_flash_uint32(PWM_PERCENT_POWER_KEY, (uint32_t)auto_pwm_power);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_automatic_pwm_power(uint8_t *auto_pwm_power)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *auto_pwm_power = global_manager_info.nv_info.pwm_auto.percent_power;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_update_output_percent_power(void)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.nv_info.pwm_auto.update_output_percent_power = true;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_set_automatic_pwm_output_status(uint8_t auto_pwm_output_status)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.nv_info.pwm_auto.output_status = auto_pwm_output_status;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_automatic_pwm_output_status(uint8_t *auto_pwm_output_status)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *auto_pwm_output_status = global_manager_info.nv_info.pwm_auto.output_status;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------