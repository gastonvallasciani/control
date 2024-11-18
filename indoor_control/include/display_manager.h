#ifndef DISPLAY_MANAGER_H__
#define DISPLAY_MANAGER_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------
#include "../include/display_dogs164.h"

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
esp_err_t screen_one_param();
esp_err_t screen_two_param();
esp_err_t screen_three_param();
//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* DISPLAY_MANAGER_H__ */