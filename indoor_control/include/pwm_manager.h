#ifndef PWM_MANAGER_H__
#define PWM_MANAGER_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>
//------------------- MACROS Y DEFINES -----------------------------------------
//------------------------------------------------------------------------------

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------
typedef enum
{
    FADING_IN_PROGRESS = 1,
    FADING_STOP = 2,
} fading_status_t;
//------------------- DECLARACION DE DATOS EXTERNOS ----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES EXTERNAS ------------------------
//------------------------------------------------------------------------------

/// @brief Inits pwm module. Must be call in main function.
/// @param
void pwm_manager_init(void);
void pwm_manager_turn_on_pwm(uint8_t pwm_power_percent);
void pwm_manager_turn_off_pwm(void);
void pwm_manager_update_pwm(uint8_t pwm_power_percent);

void pwm_manager_turn_on_pwm_simul_day_on(uint8_t pwm_power_percent);
void pwm_manager_turn_off_pwm_simul_day_on(uint8_t duty_cycle);
void pwm_manager_resume_fading_state_function(void);
void pwm_manager_only_turn_off_pwm(void);
uint8_t is_fading_in_progress(void);
void get_fading_status(fading_status_t *);

//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* LED_MANAGER_H__ */