#ifndef JUMPERS_H__
#define JUMPERS_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>
//------------------- MACROS Y DEFINES -----------------------------------------
//------------------------------------------------------------------------------

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS EXTERNOS ----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES EXTERNAS ------------------------
//------------------------------------------------------------------------------
void jumpers_manager_init(void);

uint8_t is_jp1_dspy_connected(void);
uint8_t is_jp2_fase3_connected(void);
uint8_t is_jp3_teclas_connected(void);
uint8_t is_j1_connected(void);
uint8_t is_j4_connected(void);
//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* JUMPERS_H__ */