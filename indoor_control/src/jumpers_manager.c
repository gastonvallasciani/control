//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include "../include/jumpers_manager.h"
#include "../include/board_def.h"
#include "driver/gpio.h"

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
void jumpers_manager_init(void)
{
    gpio_config_t io_conf;
    
    io_conf.intr_type = GPIO_INTR_DISABLE;          
    io_conf.mode = GPIO_MODE_INPUT;                   
    io_conf.pin_bit_mask = (1ULL << JP1_DSPY) | (1ULL << JP2_RELOJ) | (1ULL << JP3_TECLAS) | (1ULL << J1); 
    io_conf.pull_down_en = 0;                         
    io_conf.pull_up_en = 1;  

    gpio_config(&io_conf);
}
//------------------------------------------------------------------------------
// Devuelve 1 cuando el jumper esta conectado y 0 cuando esta desconectado
uint8_t is_jp1_dspy_connected(void)
{
    uint8_t status = gpio_get_level(JP1_DSPY);
    if(status)
        return 0;
    else
        return 1;
}
//------------------------------------------------------------------------------
// Devuelve 1 cuando el jumper esta conectado y 0 cuando esta desconectado
uint8_t is_jp2_reloj_connected(void)
{
    uint8_t status = gpio_get_level(JP2_RELOJ);
    if(status)
        return 0;
    else
        return 1;
}
//------------------------------------------------------------------------------
// Devuelve 1 cuando el jumper esta conectado y 0 cuando esta desconectado
uint8_t is_jp3_teclas_connected(void)
{
    uint8_t status = gpio_get_level(JP3_TECLAS);

    if(status)
        return 0;
    else
        return 1;
}
//------------------------------------------------------------------------------
// Devuelve 1 cuando el jumper esta conectado y 0 cuando esta desconectado
uint8_t is_j1_connected(void)
{
    uint8_t status = gpio_get_level(J1);
    if(status)
        return 0;
    else
        return 1;
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------