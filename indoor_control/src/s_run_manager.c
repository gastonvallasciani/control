
//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "../include/s_run_manager.h"
#include "../include/board_def.h"

//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------
static void s_run_config(void);
static void s_run_manager_task(void *arg);

//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
static void s_run_config(void)
{
    gpio_config_t io_conf;
    
    io_conf.intr_type = GPIO_INTR_DISABLE;                 
    io_conf.mode = GPIO_MODE_OUTPUT;                       
    io_conf.pin_bit_mask = (1ULL << S_RUN); 
    io_conf.pull_down_en = 0;                              
    io_conf.pull_up_en = 0;                               
    gpio_config(&io_conf);

    gpio_set_level(S_RUN, 0);
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void s_run_manager_init(void)
{
    s_run_config();

    xTaskCreate(s_run_manager_task, "s_run_manager_task", configMINIMAL_STACK_SIZE * 2,
                NULL, configMAX_PRIORITIES - 2, NULL);

}
//------------------------------------------------------------------------------
static void s_run_manager_task(void *arg)
{

    while (true)
    { 
        gpio_set_level(S_RUN, 1);  
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(S_RUN, 0);  
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(S_RUN, 1);  
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(S_RUN, 0);  
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(S_RUN, 1);  
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(S_RUN, 0);  
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(S_RUN, 1); 
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(S_RUN, 0);  
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------