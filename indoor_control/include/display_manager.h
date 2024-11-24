#ifndef DISPLAY_MANAGER_H__
#define DISPLAY_MANAGER_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------
#include "../include/display_dogs164.h"
#include <time.h>
typedef enum
{
    CMD_UNDEFINED = 0,
    START_DISPLAY = 1,
    DOWN = 2,
    UP = 3,
    VF = 4,
    AUX = 5,
    AUXT = 6
} display_event_cmds_t;

typedef struct
{
    uint8_t pwm_value;
    display_event_cmds_t cmd;
    char vege_flora;
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
void display_manager_start(uint8_t, char);
void display_manager_vf(void);
void display_manager_down(void);
void display_manager_up(void);
void display_manager_aux(void);
void display_manager_auxt(void);
esp_err_t display_blink_manager(screen_t, uint8_t);
esp_err_t set_timer(void);
esp_err_t start_timer(void);
esp_err_t stop_timer(void);
esp_err_t reset_timer(void);
esp_err_t clear_line(uint8_t);
esp_err_t display_param_manager(display_event_cmds_t);
esp_err_t screen_one_param(display_event_cmds_t);
esp_err_t screen_two_param(display_event_cmds_t);
esp_err_t screen_three_param(display_event_cmds_t);
esp_err_t param_modified_one(display_event_cmds_t);
esp_err_t param_modified_two(display_event_cmds_t);
esp_err_t param_two_bis(display_event_cmds_t, struct tm *, struct tm *);
esp_err_t param_modified_three(display_event_cmds_t);
esp_err_t save_params(void);
esp_err_t get_params(void);

//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* DISPLAY_MANAGER_H__ */