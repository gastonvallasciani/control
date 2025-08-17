#ifndef BUTTON_MANAGER_H__
#define BUTTON_MANAGER_H__
//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include "../include/global_manager.h"

//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------

//--------------------TYPEDEF---------------------------------------------------
//------------------------------------------------------------------------------
typedef enum
{
    DEVICE_IN_MANUAL = 0,
    DEVICE_IN_AUTOMATIC = 1,
} device_mode_status_t;

typedef enum
{
    CMD_UNDEFINED,
    STARTUP,
    VEGE_BUTTON_PUSHED,
    PWM_DOWN_BUTTON_PUSHED,
    PWM_UP_BUTTON_PUSHED,
    AUX_BUTTON_PUSHED,
    AUX_BUTTON_PUSHED_3_SECONDS,
    FABRIC_RESET,
    CALIBRATE_POTE,
    BOTON_PRESIONADO,
    BOTON_LIBERADO,
    VEGE_BUTTON_PUSHED_3_SECONDS,
} cmds_t;

typedef struct
{
    cmds_t cmd;
} button_events_t;

//--------------------DECLARACION DE DATOS EXTERNOS-----------------------------
//------------------------------------------------------------------------------

//--------------------DECLARACION DE FUNCIONES EXTERNAS-------------------------
//------------------------------------------------------------------------------
void button_manager_init(void);

//--------------------FIN DEL ARCHIVO-------------------------------------------
#endif /* BUTTON_MANAGER_H__ */