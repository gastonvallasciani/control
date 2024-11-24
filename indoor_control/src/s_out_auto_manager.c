//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>
#include <assert.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "../include/board_def.h"
#include "../include/s_out_auto_manager.h"
#include "../include/s_out_manager.h"
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define MAX_S_OUT_CALENDARS 4
#define DEBUG_MODULE
//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------
static bool s_out_mark_on[4];
//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------
static void s_out_auto_manager_handler_per_s_out(s_out_auto_info_t *info, uint8_t triac_index);
static uint8_t is_date1_grater_than_date2(struct tm date1, struct tm date2);
static void is_first_turn_on_time_greater_than_current_time(s_out_auto_info_t *info);
static int is_within_range(struct tm target, struct tm start, struct tm end);
static void must_be_s_out_output_off(s_out_auto_info_t *info);
//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
static uint8_t is_date1_grater_than_date2(struct tm date1, struct tm date2)
{
    if((date1.tm_hour > date2.tm_hour) \
        || ((date1.tm_hour == date2.tm_hour) \
        && (date1.tm_min > date2.tm_min)) \
        || ((date1.tm_hour == date2.tm_hour) \
        && (date1.tm_min == date2.tm_min) 
        && (date1.tm_sec > date2.tm_sec)))
    {
        return 1;
    }
    return 0;
}
//------------------------------------------------------------------------------
static int is_within_range(struct tm target, struct tm start, struct tm end)
{
    if (start.tm_hour < end.tm_hour || 
       (start.tm_hour == end.tm_hour && start.tm_min < end.tm_min) ||
       (start.tm_hour == end.tm_hour && start.tm_min == end.tm_min && start.tm_sec <= end.tm_sec)) {
        // Caso normal (ejemplo: de 9:00:00 a 17:00:00)
        if ((target.tm_hour > start.tm_hour || 
            (target.tm_hour == start.tm_hour && target.tm_min > start.tm_min) ||
            (target.tm_hour == start.tm_hour && target.tm_min == start.tm_min && target.tm_sec >= start.tm_sec)) &&
            (target.tm_hour < end.tm_hour || 
            (target.tm_hour == end.tm_hour && target.tm_min < end.tm_min) ||
            (target.tm_hour == end.tm_hour && target.tm_min == end.tm_min && target.tm_sec <= end.tm_sec))) {
            return 1;
        } else {
            return 0;
        }
    } else {
        // Rango cruza la medianoche (ejemplo: de 22:00:00 a 2:00:00)
        if ((target.tm_hour > start.tm_hour || 
            (target.tm_hour == start.tm_hour && target.tm_min > start.tm_min) ||
            (target.tm_hour == start.tm_hour && target.tm_min == start.tm_min && target.tm_sec >= start.tm_sec)) ||
            (target.tm_hour < end.tm_hour || 
            (target.tm_hour == end.tm_hour && target.tm_min < end.tm_min) ||
            (target.tm_hour == end.tm_hour && target.tm_min == end.tm_min && target.tm_sec <= end.tm_sec))) {
            return 1;
        } else {
            return 0;
        }
    }
}
//------------------------------------------------------------------------------
static void s_out_auto_manager_handler_per_s_out(s_out_auto_info_t *info, uint8_t s_out_index)
{
    if(info->s_out_auto[s_out_index].enable == true)
    {
        if(info->output_status == S_OUT_OUTPUT_OFF)
        {
            if(is_within_range(info->current_time, info->s_out_auto[s_out_index].turn_on_time, info->s_out_auto[s_out_index].turn_off_time))
            {
                s_out_manager_turn_on_s_out();
                info->output_status = S_OUT_OUTPUT_ON;
                s_out_mark_on[s_out_index] = true;
            }
        }
        else if(info->output_status == S_OUT_OUTPUT_ON)
        {
            if((is_date1_grater_than_date2(info->current_time, info->s_out_auto[s_out_index].turn_off_time) == 1) \
                && (is_date1_grater_than_date2(info->s_out_auto[s_out_index].turn_off_time, info->s_out_auto[s_out_index].turn_on_time) == 1) && (s_out_mark_on[s_out_index] == true))
            {
                s_out_manager_turn_off_s_out();
                info->output_status = S_OUT_OUTPUT_OFF;
                s_out_mark_on[s_out_index] = false;

            }
            else if((is_date1_grater_than_date2(info->current_time, info->s_out_auto[s_out_index].turn_off_time) == 1) \
                && (is_date1_grater_than_date2(info->s_out_auto[s_out_index].turn_on_time, info->current_time) == 1) \
                && (is_date1_grater_than_date2(info->s_out_auto[s_out_index].turn_on_time, info->s_out_auto[s_out_index].turn_off_time) == 1) && (s_out_mark_on[s_out_index] == true))
            {
                s_out_manager_turn_off_s_out();
                info->output_status = S_OUT_OUTPUT_OFF;
                s_out_mark_on[s_out_index] = false;
            }
        }
    }
    else
    {
        return;
    }
}
//------------------------------------------------------------------------------
static void is_first_turn_on_time_greater_than_current_time(s_out_auto_info_t *info)
{
    uint8_t s_out_index = 0;
    uint8_t index_youngest = 0;
    struct tm ton_youngest;

    for(s_out_index = 0; s_out_index < MAX_S_OUT_CALENDARS; s_out_index++)
    {
        if(info->s_out_auto[s_out_index].enable == true)
        {
            ton_youngest = info->s_out_auto[s_out_index].turn_on_time;
            index_youngest = s_out_index;
            break;
        }
    }

    for(s_out_index = 0; s_out_index < MAX_S_OUT_CALENDARS; s_out_index++)
    {
        if(info->s_out_auto[s_out_index].enable == true)
        {
            if(is_date1_grater_than_date2(ton_youngest, info->s_out_auto[s_out_index].turn_on_time))
            {
                ton_youngest = info->s_out_auto[s_out_index].turn_on_time;
                index_youngest = s_out_index;
            }
        }
    }
    if(info->output_status == S_OUT_OUTPUT_ON)
    {
        if((is_date1_grater_than_date2(ton_youngest, info->current_time) == 1) \
        && (is_date1_grater_than_date2(info->s_out_auto[index_youngest].turn_off_time, info->s_out_auto[index_youngest].turn_on_time) == 1))
        {
           //printf("ton_youngest: %s \n", asctime(&ton_youngest));
            //printf("Hora actual: %s \n", asctime(&info->current_time));
            info->output_status = S_OUT_OUTPUT_OFF;
            s_out_manager_turn_off_s_out();
        }
    }
}
//------------------------------------------------------------------------------
static void must_be_s_out_output_off(s_out_auto_info_t *info)
{
    uint8_t s_out_index = 0;
    char out_of_range_mark[MAX_S_OUT_CALENDARS];

    if(info->output_status == S_OUT_OUTPUT_ON)
    {
        for(s_out_index = 0; s_out_index < MAX_S_OUT_CALENDARS; s_out_index++)
        {
            // chequeo si current time no se encuentra dentro de ningun rango
            if(is_within_range(info->current_time, info->s_out_auto[s_out_index].turn_on_time, info->s_out_auto[s_out_index].turn_off_time) == 0)
            {
                out_of_range_mark[s_out_index] = 1;
            }
        }
        if((out_of_range_mark[0] == 1) && (out_of_range_mark[1] == 1) && (out_of_range_mark[2] == 1) && (out_of_range_mark[3] == 1))
        {
            info->output_status = S_OUT_OUTPUT_OFF;
            s_out_manager_turn_off_s_out();

            for(s_out_index = 0; s_out_index < MAX_S_OUT_CALENDARS; s_out_index++)
            {
                s_out_mark_on[s_out_index] = false;
            }
        }
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void s_out_auto_manager_init(void)
{
    uint8_t s_out_index = 0;
    for(s_out_index = 0; s_out_index < MAX_S_OUT_CALENDARS; s_out_index++)
    {
        s_out_mark_on[s_out_index] = false;
    }
}
void s_out_auto_manager_update(s_out_auto_info_t *info)
{
    uint8_t s_out_index = 0;

    for(s_out_index = 0; s_out_index < MAX_S_OUT_CALENDARS; s_out_index++)
    {
        s_out_mark_on[s_out_index] = false;
    }

    for(s_out_index = 0; s_out_index < MAX_S_OUT_CALENDARS; s_out_index++)
    {
        if(is_within_range(info->current_time, info->s_out_auto[s_out_index].turn_on_time, info->s_out_auto[s_out_index].turn_off_time))
        {
            if(info->output_status == S_OUT_OUTPUT_ON)
            {
                s_out_mark_on[s_out_index] = true;
            } 
        }
        
    }
}
//------------------------------------------------------------------------------
void s_out_auto_manager_handler(s_out_auto_info_t *info, bool s_out_auto_enable)
{
    uint8_t s_out_index = 0;

    if(s_out_auto_enable == true)
    {
        must_be_s_out_output_off(info);


        for(s_out_index = 0; s_out_index < MAX_S_OUT_CALENDARS; s_out_index++)
        {
            s_out_auto_manager_handler_per_s_out(info, s_out_index);
        }
    }
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------