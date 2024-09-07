//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>
#include <assert.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "../include/board_def.h"
#include "../include/flora_vege_manager.h"
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define DEBUG_MODULE
#define GPIO_OUTPUT_PIN_SEL  (1ULL << S_VEGE)
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
void flora_vege_manager_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // desactivar interrupción
    io_conf.mode = GPIO_MODE_OUTPUT; // establecer en modo salida
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL; // configurar pin
    io_conf.pull_down_en = 0; // desactivar pull-down
    io_conf.pull_up_en = 0; // desactivar pull-up
    gpio_config(&io_conf);

    gpio_set_level(S_VEGE, 0);
}
//------------------------------------------------------------------------------
void flora_vege_manager_turn_on(void)
{
    #ifdef DEBUG_MODULE
        printf("TURN ON RELE VEGE \n");
    #endif
    gpio_set_level(S_VEGE, 1);
}
//------------------------------------------------------------------------------
void flora_vege_manager_turn_off(void)
{
    #ifdef DEBUG_MODULE
        printf("TURN OFF RELE VEGE \n");
    #endif
    gpio_set_level(S_VEGE, 0);
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------