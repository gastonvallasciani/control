#ifndef BOARD_DEF_H__
#define BOARD_DEF_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------

//------------------- MACROS Y DEFINES -----------------------------------------
//------------------------------------------------------------------------------

#define MAX_PERCENTAGE_POWER_VALUE 100
#define MIN_PERCENTAGE_POWER_VALUE 0

// LED DEFINITIONS
#define LED_PWM GPIO_NUM_34
#define LED_FLORA GPIO_NUM_35
#define LED_VEGE GPIO_NUM_32

// ADC POTE
#define ADC_POTE_INPUT ADC_CHANNEL_5 // GPIO 33 ADC1 channel 5 - IMPORTANTE: must be LOW during boot 

// BUTTONS
#define BT_DW
#define BT_UP
#define BT_VE_FLO

// I2C
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22

// PWM
#define PWM_OUTPUT GPIO_NUM_23

// OUTPUTS
#define S_RUN GPIO_NUM_23
#define S_VEGE GPIO_NUM_23
#define S_OUT GPIO_NUM_18

// JUMPERS 
#define JP1
#define JP2
#define JP3







//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS EXTERNOS ----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES EXTERNAS ------------------------
//------------------------------------------------------------------------------

//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* BOARD_DEF_H__ */