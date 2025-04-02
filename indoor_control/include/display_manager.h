#ifndef DISPLAY_MANAGER_H__
#define DISPLAY_MANAGER_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------
#include "../include/display_dogs164.h"
#include "../include/global_manager.h"
#include <time.h>
typedef enum
{
    START_DISPLAY = 0,
    DOWN = 1,
    UP = 2,
    VF = 3,
    AUX = 4,
    AUXT = 5,
    VFT = 6,
    PWM_MANUAL_VALUE = 7,
    PWM_MODE_UPDATE = 8
} display_event_cmds_t;

typedef enum
{
    GREATER = 0,
    LESSER = 1,
    EQUAL = 2
} compare_t;

typedef struct
{
    uint8_t pwm_value;
    display_event_cmds_t cmd;
    flora_vege_status_t vege_flora;
    pwm_mode_t pwm_mode;
} display_event_t;

//------------------- MACROS Y DEFINES -----------------------------------------
//------------------------------------------------------------------------------

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS EXTERNOS ----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES EXTERNAS ------------------------
//------------------------------------------------------------------------------
void get_screen_state(display_state_t *);
void display_manager_init(void);
void display_manager_start(uint8_t, char, pwm_mode_t);
void display_manager_vf(flora_vege_status_t);
void display_manager_down(uint8_t, flora_vege_status_t);
void display_manager_up(uint8_t, flora_vege_status_t);
void display_manager_aux(void);
void display_manager_auxt(void);
void display_manager_manual(uint8_t);
void display_manager_vft(pwm_mode_t pwm_mode);
void blink_callback(TimerHandle_t);
void time_callback(TimerHandle_t);
esp_err_t display_blink_manager(screen_t, uint8_t);
esp_err_t set_timer(void);
esp_err_t start_timer(void);
esp_err_t stop_timer(void);
esp_err_t reset_timer(void);
esp_err_t set_timerh(void);
esp_err_t start_timerh(void);
esp_err_t stop_timerh(void);
esp_err_t reset_timerh(void);
esp_err_t clear_line(uint8_t);
esp_err_t display_param_manager(display_event_cmds_t);
/*esp_err_t screen_one_param(display_event_cmds_t);*/
esp_err_t screen_two_param(display_event_cmds_t);
esp_err_t screen_three_param(display_event_cmds_t);
/*esp_err_t param_modified_one(display_event_cmds_t);*/
esp_err_t param_modified_two(display_event_cmds_t);
esp_err_t param_two_bis(display_event_cmds_t, struct tm *, struct tm *);
esp_err_t param_modified_three(display_event_cmds_t);
esp_err_t save_params(void);
esp_err_t get_params(void);
void display_manager_pwm_mode_update(uint8_t pwm_value, flora_vege_status_t vege_flora);
compare_t compare_times(struct tm, struct tm);
uint8_t colision_times(struct tm, struct tm, struct tm, struct tm);
uint8_t checkOverlap(struct tm, struct tm, struct tm, struct tm, struct tm, struct tm, struct tm, struct tm);
//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* DISPLAY_MANAGER_H__ */