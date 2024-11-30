//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

#include "../include/current_time_manager.h"
#include "../include/board_def.h"
#include "../include/global_manager.h"
#include "../include/nv_flash_manager.h"
#include "../include/pcf85063.h"
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define DEBUG_MODULE 1

#define QUEUE_ELEMENT_QUANTITY 10
#define SECOND_1 1000
#define SYNC_INTERVAL 60 // 60 seconds (1 minute)

#define MAX_RETRIES 5
#define TIMEOUT_MS 500
//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------
typedef enum
{
    CMD_UNDEFINED = 0,
    SET_CURRENT_TIME = 1,
    GET_CURRENT_TIME = 2,
} current_time_cmds_t;

typedef struct
{
    struct tm current_time;
    current_time_cmds_t cmd;
} current_time_event_t;
//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------
static QueueHandle_t current_time_manager_queue, response_queue;
static time_t local_time_seconds; // Local time in seconds
//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------
static void current_time_manager_task(void *arg);
static void get_current_time(void);
static uint8_t wait_get_current_time_response(struct tm *current_time);
//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
__attribute__((unused)) static void set_manual_time(void)
{
    struct tm manual_time;

    // Configura la hora manualmente
    manual_time.tm_year = 2024 - 1900; // Año - 1900 (2024 en este caso)
    manual_time.tm_mon = 10;           // Mes (0 = Enero, 10 = Noviembre)
    manual_time.tm_mday = 3;           // Día del mes
    manual_time.tm_hour = 18;          // Hora (formato 24 horas)
    manual_time.tm_min = 30;           // Minuto
    manual_time.tm_sec = 0;            // Segundo
    manual_time.tm_isdst = -1;         // Ajuste de horario de verano automático

    // Llama a la función para establecer el tiempo
    current_time_manager_set_current_time(manual_time);
}
//------------------------------------------------------------------------------
static void current_time_manager_task(void *arg)
{
    current_time_event_t ev;
    struct tm updated_time;
    int second_counter = 0;

    pcf85063_init();

    vTaskDelay(300 / portTICK_PERIOD_MS);

    // Initial synchronization with RTC
    pcf85063_get_current_time_info(&updated_time);
    // Print the updated time

    printf("CURRENT TIME: %02d-%02d-%02d %02d:%02d:%02d\n",
           updated_time.tm_year + 1900, updated_time.tm_mon + 1, updated_time.tm_mday,
           updated_time.tm_hour, updated_time.tm_min, updated_time.tm_sec);

    // set_manual_time();

    while (1)
    {
        if (xQueueReceive(current_time_manager_queue, &ev, SECOND_1 / portTICK_PERIOD_MS) == pdTRUE)
        {
            switch (ev.cmd)
            {
            case CMD_UNDEFINED:
                break;
            case SET_CURRENT_TIME:
                local_time_seconds = mktime(&ev.current_time);
                pcf85063_set_current_time(ev.current_time);
                second_counter = 0; // Reset counter after setting time
                printf("Updated Time: %02d-%02d-%02d %02d:%02d:%02d\n",
                       updated_time.tm_year + 1900, updated_time.tm_mon + 1, updated_time.tm_mday,
                       updated_time.tm_hour, updated_time.tm_min, updated_time.tm_sec);
                break;
            case GET_CURRENT_TIME:
                pcf85063_get_current_time_info(&ev.current_time);
                xQueueSend(response_queue, &ev, 10);
                break;
            default:
                break;
            }
        }
        else
        {
            // Incrementar la hora
            updated_time.tm_sec++; // Incrementar segundos
            if (updated_time.tm_sec >= 60)
            {
                updated_time.tm_sec = 0; // Reiniciar segundos
                updated_time.tm_min++;   // Incrementar minutos
                if (updated_time.tm_min >= 60)
                {
                    updated_time.tm_min = 0; // Reiniciar minutos
                    updated_time.tm_hour++;  // Incrementar horas
                    if (updated_time.tm_hour >= 24)
                    {
                        updated_time.tm_hour = 0; // Reiniciar horas
                        updated_time.tm_mday++;   // Incrementar día
                        // Verificar si el mes necesita ser incrementado
                        if (updated_time.tm_mday > 31)
                        {                             // Esto es una simplificación
                            updated_time.tm_mday = 1; // Reiniciar día
                            updated_time.tm_mon++;    // Incrementar mes
                            if (updated_time.tm_mon >= 12)
                            {
                                updated_time.tm_mon = 0; // Reiniciar mes
                                updated_time.tm_year++;  // Incrementar año
                            }
                        }
                    }
                }
            }

            /*//global_manager_update_current_time(updated_time);
            printf("Updated Time: %02d-%02d-%02d %02d:%02d:%02d\n",
                updated_time.tm_year + 1900, updated_time.tm_mon + 1, updated_time.tm_mday,
                updated_time.tm_hour, updated_time.tm_min, updated_time.tm_sec);*/

            // Synchronize with RTC every SYNC_INTERVAL seconds
            second_counter++;
            if (second_counter >= SYNC_INTERVAL)
            {
                pcf85063_get_current_time_info(&updated_time);
                second_counter = 0; // Reset the counter after synchronization
            }
        }
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void current_time_manager_init(void)
{
    current_time_manager_queue = xQueueCreate(QUEUE_ELEMENT_QUANTITY, sizeof(current_time_event_t));
    response_queue = xQueueCreate(QUEUE_ELEMENT_QUANTITY, sizeof(current_time_event_t));

    xTaskCreate(current_time_manager_task, "current_time_manager_task", configMINIMAL_STACK_SIZE * 7,
                NULL, configMAX_PRIORITIES - 5, NULL);
}
//------------------------------------------------------------------------------
void current_time_manager_set_current_time(struct tm current_time)
{
    current_time_event_t ev;

    ev.cmd = SET_CURRENT_TIME;
    ev.current_time = current_time;
    xQueueSend(current_time_manager_queue, &ev, 10);
}
//------------------------------------------------------------------------------
static void get_current_time(void)
{
    current_time_event_t ev;
    ev.cmd = GET_CURRENT_TIME;
    xQueueSend(current_time_manager_queue, &ev, 10);
}

static uint8_t wait_get_current_time_response(struct tm *current_time)
{
    current_time_event_t resp_ev;
    if (xQueueReceive(response_queue, &resp_ev, TIMEOUT_MS / portTICK_PERIOD_MS))
    {
        *current_time = resp_ev.current_time;
        return 1;
    }
    else
    {
#ifdef DEBUG_MODULE
        printf("Error al recibir la respuesta de pwm\n");
#endif
        return 0;
    }
    return 0;
}

uint8_t global_manager_get_current_time_info(struct tm *current_time)
{
    get_current_time();
    if (wait_get_current_time_response(current_time))
    {
        return (1);
    }
    return (0);
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------
