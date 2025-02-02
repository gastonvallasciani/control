#ifndef GLOBAL_MANAGER_H__
#define GLOBAL_MANAGER_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>
#include "pwm_auto_manager.h"
#include "../include/s_out_auto_manager.h"

//------------------- MACROS Y DEFINES -----------------------------------------
//------------------------------------------------------------------------------

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------
typedef enum
{
    PWM_MANUAL = 0,
    PWM_AUTOMATIC = 1,
    PWM_UNDEFINED = 2,
} pwm_mode_t;

typedef enum
{
    MODE_1 = 0,
    MODE_2 = 1,
    MODE_3 = 2,
    MODE_4 = 3,
} device_mode_t;

typedef enum
{
    FLORA_VEGE_OUTPUT_DISABLE = 0,
    FLORA_VEGE_OUTPUT_ENABLE = 1,
} flora_vege_status_t;

typedef struct
{
    pwm_mode_t pwm_mode;
    flora_vege_status_t flora_vege_status;
    pwm_auto_info_t pwm_auto;
    s_out_auto_info_t s_out_auto;
    uint32_t ppf;
} global_manager_nv_t;

typedef struct
{
    device_mode_t device_mode;
    uint8_t pwm_analog_percent_power;
    uint8_t pwm_digital_percent_power;
    global_manager_nv_t nv_info;
} global_manager_t;

typedef struct
{
    struct tm turn_on_time;
    struct tm turn_off_time;
    bool read_ok;
} calendar_t;

typedef struct
{
    calendar_t calendar;
    bool enable;
} s_out_conf_t;

//------------------- DECLARACION DE DATOS EXTERNOS ----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES EXTERNAS ------------------------
//------------------------------------------------------------------------------
void global_manager_init(void);

uint8_t global_manager_is_device_in_phase_3(void);


uint8_t global_manager_set_pwm_analog_percentage(uint8_t pwm_analog_per_value);
uint8_t global_manager_get_pwm_analog_percentage(uint8_t *pwm_analog_per_value);

uint8_t global_manager_set_pwm_mode(pwm_mode_t pwm_mode);
uint8_t global_manager_get_pwm_mode(pwm_mode_t *pwm_mode);

uint8_t global_manager_set_device_mode(device_mode_t device_mode);
uint8_t global_manager_get_device_mode(device_mode_t *device_mode);

uint8_t global_manager_set_flora_vege_status(flora_vege_status_t flora_vege_status);
uint8_t global_manager_get_flora_vege_status(flora_vege_status_t *flora_vege_status);

uint8_t global_manager_set_pwm_digital_percentage(uint8_t pwm_digital_per_value);
uint8_t global_manager_get_pwm_digital_percentage(uint8_t *pwm_digital_per_value);

uint8_t global_manager_set_pwm_automatic_info(pwm_auto_info_t pwm_auto);
uint8_t global_manager_get_pwm_automatic_info(pwm_auto_info_t *pwm_auto);

uint8_t global_manager_set_simul_day_status(simul_day_status_t simul_day_status);
uint8_t global_manager_get_simul_day_status(simul_day_status_t *simul_day_status);

uint8_t global_manager_set_turn_on_time(struct tm turn_on_time);
uint8_t global_manager_get_turn_on_time(struct tm *turn_on_time);

uint8_t global_manager_set_turn_off_time(struct tm turn_off_time);
uint8_t global_manager_get_turn_off_time(struct tm *turn_off_time);

uint8_t global_manager_update_pwm_calendar_info(void);

uint8_t global_manager_set_automatic_pwm_power(uint8_t auto_pwm_power);  ///////////// SET porcentaje de pwm automatico
uint8_t global_manager_get_automatic_pwm_power(uint8_t *auto_pwm_power); ////////////// GET porcentaje de pwm de salida autoamtico

uint8_t global_manager_update_output_percent_power(void);

uint8_t global_manager_set_automatic_pwm_output_status(uint8_t auto_pwm_output_status);
uint8_t global_manager_get_automatic_pwm_output_status(uint8_t *auto_pwm_output_status);

uint8_t global_manager_set_s_out_automatic_info(s_out_auto_info_t s_out_auto);
uint8_t global_manager_get_s_out_automatic_info(s_out_auto_info_t *s_out_auto);

uint8_t global_manager_set_s_out_turn_off_time(struct tm turn_off_time, uint8_t s_out_index);
uint8_t global_manager_get_s_out_turn_off_time(struct tm *turn_off_time, uint8_t s_out_index);

uint8_t global_manager_set_s_out_turn_on_time(struct tm turn_on_time, uint8_t s_out_index);
uint8_t global_manager_get_s_out_turn_on_time(struct tm *turn_on_time, uint8_t s_out_index);

uint8_t global_manager_set_s_out_time_enable_status(uint8_t time_enable_status, uint8_t s_out_index);
uint8_t global_manager_get_s_out_time_enable_status(uint8_t *time_enable_status, uint8_t s_out_index);

uint8_t global_manager_get_ppf(uint32_t *ppf);
uint8_t global_manager_set_ppf(uint32_t ppf);

/*
    UPDATE PWM CALENDAR
    -------------------
    Al momento de actualizar el calendario de pwm automatico se debe llamar:

    global_manager_set_turn_on_time(turn_on_time);
    global_manager_set_turn_off_time(turn_off_time);
    global_manager_update_pwm_calendar_info();

    UPDATE AUTO PWM POWER
    ---------------------
    global_manager_set_automatic_pwm_power(auto_pwm_power);
    global_manager_update_output_percent_power();

    Se actualiza tmb cuando el modo del PWM pasa de funcionamiento MANUAL pasa a AUTOMATICO:
    global_manager_set_pwm_mode(PWM_AUTOMATIC);
    global_manager_update_output_percent_power();

*/
//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif