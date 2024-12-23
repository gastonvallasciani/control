#ifndef BOARD_DEF_H__
#define BOARD_DEF_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------

//------------------- MACROS Y DEFINES -----------------------------------------
//------------------------------------------------------------------------------
#define MAX_PERCENTAGE_POWER_VALUE 100
#define MIN_PERCENTAGE_POWER_VALUE 0

// PIN DEFINITIONS - Micro PWM v.0.1
// LED DEFINITIONS
#define LED_PWM GPIO_NUM_13 
#define LED_FLORA GPIO_NUM_19 
#define LED_VEGE GPIO_NUM_32

// ADC POTE
#define ADC_POTE_INPUT ADC_CHANNEL_5 // GPIO 33 ADC1 channel 5 - IMPORTANTE: must be LOW during boot 

// BUTTONS
#define BT_DW GPIO_NUM_27
#define BT_UP GPIO_NUM_14
#define BT_VE_FLO GPIO_NUM_12
#define BT_AUX GPIO_NUM_39

// I2C
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22

// PWM
#define PWM_OUTPUT GPIO_NUM_23

// OUTPUTS
#define S_RUN GPIO_NUM_25
#define S_VEGE GPIO_NUM_26
#define S_OUT GPIO_NUM_18

// JUMPERS 
#define JP1_DSPY GPIO_NUM_35 // usa pull up externo
#define JP2_FASE3 GPIO_NUM_34 // usa pull up externo  
#define JP3_TECLAS GPIO_NUM_4

#define J1 GPIO_NUM_5

// DISPLAY
#define RST_DSPY GPIO_NUM_15

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS EXTERNOS ----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES EXTERNAS ------------------------
//------------------------------------------------------------------------------

//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* BOARD_DEF_H__ */