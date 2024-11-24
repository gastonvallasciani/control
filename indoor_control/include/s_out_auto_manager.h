#ifndef S_OUT_AUTO_MANAGER_H__
#define S_OUT_AUTO_MANAGER_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------
#include <time.h>
//------------------- MACROS Y DEFINES -----------------------------------------
//------------------------------------------------------------------------------
#define MAX_AUTO_S_OUT_CONFIGS_HOURS 4
#define S_OUT_CALENDAR_1 0
#define S_OUT_CALENDAR_2 1
#define S_OUT_CALENDAR_3 2
#define S_OUT_CALENDAR_4 3
//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------
typedef enum{
    S_OUT_OUTPUT_OFF = 0,
    S_OUT_OUTPUT_ON = 1,
}s_out_output_status_t;

typedef struct{
    struct tm turn_on_time;
    struct tm turn_off_time;    
    uint8_t enable;
}s_out_config_info_t;

typedef struct{
    struct tm current_time;
    s_out_output_status_t output_status;
    s_out_config_info_t s_out_auto[MAX_AUTO_S_OUT_CONFIGS_HOURS];
}s_out_auto_info_t;
//------------------- DECLARACION DE DATOS EXTERNOS ----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES EXTERNAS ------------------------
//------------------------------------------------------------------------------
void s_out_auto_manager_handler(s_out_auto_info_t *info, bool s_out_auto_enable);
void s_out_auto_manager_init(void);
void s_out_auto_manager_update(s_out_auto_info_t *info);
//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* S_OUT_AUTO_MANAGER_H__ */