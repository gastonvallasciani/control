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
void display_manager_init(void);
void display_manager_start(uint8_t pwm_value, char vege_flora);
void display_manager_refresh(uint8_t pwm_value, char vege_flora);
void display_manager_refreshvege_flora(char vege_flora);
void display_manager_change_screen(uint8_t pwm_value, char vege_flora);
//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* DISPLAY_MANAGER_H__ */