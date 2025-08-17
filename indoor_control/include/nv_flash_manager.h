#ifndef NV_FLASH_MANAGER_H__
#define NV_FLASH_MANAGER_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>
#include <time.h>

#include "../include/nv_flash_driver.h"
//------------------- MACROS Y DEFINES -----------------------------------------
//------------------------------------------------------------------------------
// FLASH KEYs
// Tamaño maximo de key 15 caracteres
#define RELE_VEGE_STATUS_KEY "rele_vege_key\0"
#define PWM_DIGITAL_VALUE_KEY "pwm_dig_key\0"
#define PWM_MODE_KEY "pwm_mode_key\0"
#define SIMUL_DAY_STATUS_KEY "sim_day_key\0"
#define PWM_DATE_ON_KEY "pwm_on_key\0"
#define PWM_DATE_OFF_KEY "pwm_off_key\0"
#define PWM_PERCENT_POWER_KEY "pwm_power_key\0"
#define S_OUT_1_DATE_ON_KEY "s_out_1_on\0"
#define S_OUT_1_DATE_OFF_KEY "s_out_1_off\0"
#define S_OUT_1_DATE_ENABLE "s_out_1_enable\0"
#define S_OUT_2_DATE_ON_KEY "s_out_2_on\0"
#define S_OUT_2_DATE_OFF_KEY "s_out_2_off\0"
#define S_OUT_2_DATE_ENABLE "s_out_2_enable\0"
#define S_OUT_3_DATE_ON_KEY "s_out_3_on\0"
#define S_OUT_3_DATE_OFF_KEY "s_out_3_off\0"
#define S_OUT_3_DATE_ENABLE "s_out_3_enable\0"
#define S_OUT_4_DATE_ON_KEY "s_out_4_on\0"
#define S_OUT_4_DATE_OFF_KEY "s_out_4_off\0"
#define S_OUT_4_DATE_ENABLE "s_out_4_enable\0"
#define PPF_KEY "ppf_key\0"
#define DISPLAY_CONTRAST_KEY "cont_key\0"
#define WIFI_AP_SSID_KEY "ssid_ap_key\0"
#define WIFI_AP_PASSWORD_KEY "pass_ap_key\0"

// DEFAULT VALUES
#define RELE_VEGE_STATUS_DEFAULT 0     // OFF
#define PWM_DIGITAL_VALUE_DEFAULT 0    // pwm value configurado pro tecla. Rango: 0-100
#define PWM_MODE_DEFAULT 0             // PWM_MANUAL
#define PWM_SIMUL_DAY_STATUS_DEFAULT 0 // SIMUL_DAY_OFF
#define PWM_PERCENT_POWER_DEFAULT 50   // 50 percent
#define S_OUT_1_DATE_ENABLE_DEFAULT 1  // enable
#define S_OUT_2_DATE_ENABLE_DEFAULT 0  // disable
#define S_OUT_3_DATE_ENABLE_DEFAULT 0  // disable
#define S_OUT_4_DATE_ENABLE_DEFAULT 0  // disable
#define PPF_DEFAULT 1000
#define CONTRASTE_DEFAULT 10
#define WIFI_AP_SSID_DEFAULT "Lumenar01" // el tamaño maximo del ssid no debe exceder los 32 caracteres
#define WIFI_AP_PASSWORD_DEFAULT "12345678"

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS EXTERNOS ----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES EXTERNAS ------------------------
//------------------------------------------------------------------------------
void nv_flash_manager_init(void);

void init_date_parameter_in_flash(char *key, struct tm time_info_default);
uint8_t read_date_from_flash(char *key, struct tm *time_info);
void write_date_on_flash(char *key, struct tm time_info);
uint8_t read_uint32_from_flash(char *key, uint32_t *value);
uint8_t read_str_from_flash(char *key, char *str_val);
//------------------------------------------------------------------------------
/*
Ejemplo de lectura de una fecha de dataflash

struct tm time_info_aux;
    if(read_date_from_flash(TRIAC2_DATE_OFF_KEY, &time_info_aux))
    {
        printf("Año: %d\n", time_info_aux.tm_year + 1900);
        printf("Mes: %d\n", time_info_aux.tm_mon + 1);
        printf("Día: %d\n", time_info_aux.tm_mday);
        printf("Hora: %d\n", time_info_aux.tm_hour);
        printf("Minuto: %d\n", time_info_aux.tm_min);
        printf("Segundo: %d\n", time_info_aux.tm_sec);
    }
*/
//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* NV_FLASH_MANAGER_H__ */