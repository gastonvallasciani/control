//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include "../include/board_def.h"
#include "../include/display_manager.h"
#include "../include/display_dogs164.h"
#include "esp_log.h"
static const char *TAG = "I2C";
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void display_manager_init(void)
{
    ESP_LOGI(TAG, "Inicializando I2C");
    ESP_ERROR_CHECK(set_i2c()); // inicio el i2c
    ESP_LOGI(TAG, "Inicializando DISPLAY");
    //display_set_screen(100); // funcion de inicio de pantalla con el valor de potencia
    ESP_LOGI(TAG, "Termina inicializacion del DISPLAY");
    /*display_set_power(75, DOWN); // funcion de ejemplo con el nuevo valor de potencia y si disminuye o aumenta
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    display_set_power(30, DOWN);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    display_set_power(10, DOWN);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    display_set_power(90, UP);
    vTaskDelay(1000 / portTICK_PERIOD_MS);*/
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------