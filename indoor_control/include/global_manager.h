#ifndef GLOBAL_MANAGER_H__
#define GLOBAL_MANAGER_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>
//------------------- MACROS Y DEFINES -----------------------------------------
//------------------------------------------------------------------------------

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------
typedef enum{
    MANUAL = 0,
    AUTOMATIC = 1
}pwm_mode_t;

typedef enum{
    MODE_1 = 0,
    MODE_2 = 1,
    MODE_3 = 2,
    MODE_4 = 3,
}device_mode_t;

typedef struct{
    uint8_t percent_power;

}pwm_automatic_info_t;

typedef struct{
    pwm_mode_t pwm_mode;
    pwm_automatic_info_t pwm_auto_info;
}global_manager_nv_t;

typedef struct{
    device_mode_t device_mode;
    uint8_t pwm_manual_percent_power;
    global_manager_nv_t nv_info;
}global_manager_t;
//------------------- DECLARACION DE DATOS EXTERNOS ----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES EXTERNAS ------------------------
//------------------------------------------------------------------------------
void global_manager_init(void);

uint8_t global_manager_set_pwm_manual_percentage(uint8_t pwm_manual_per_value);
uint8_t global_manager_get_pwm_manual_percentage(uint8_t* pwm_manual_per_value);

uint8_t global_manager_set_pwm_mode(pwm_mode_t pwm_mode);
uint8_t global_manager_get_pwm_mode(pwm_mode_t* pwm_mode);

uint8_t global_manager_set_device_mode(device_mode_t device_mode);
uint8_t global_manager_get_device_mode(device_mode_t *device_mode);

//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* GLOBAL_MANAGER_H__ */