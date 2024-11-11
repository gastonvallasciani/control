//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "sdkconfig.h"

#include "esp_log.h"

// #include "../include/board_def.h"
#include "../include/display_manager.h"
#include "../include/display_dogs164.h"
// #include "display_dogs164.c"

static const char *TAG = "I2C";
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define DEBUG_MODULE 1

#define QUEUE_ELEMENT_QUANTITY 25
//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

// variables del timer
TimerHandle_t timer;
int interval = 400; // 500ms de timer
int timerid = 1;
bool clear;

// variables globales de parametros a escribir en el display
struct tm time_device;
struct tm time_i1;
struct tm time_f1;
struct tm time_i2;
struct tm time_f2;
struct tm time_i3;
struct tm time_f3;
struct tm time_i4;
struct tm time_f4;
struct tm time_pwmi;
struct tm time_pwmf;
uint8_t power;
int fpower;
char vegeflora;
bool dia;
bool modo;

// comandos de las acciones del display
typedef enum
{
    CMD_UNDEFINED = 0,
    START_DISPLAY = 1,
    DOWN = 2,
    UP = 3,
    VF = 4,
    AUX = 5,
    AUXT = 6
} display_event_cmds_t;

typedef struct
{
    uint8_t pwm_value;
    display_event_cmds_t cmd;
    char vege_flora;
} display_event_t;

// variables globales de informacion y posicion
uint8_t line;
screen_t screen;       // variable para saber en que pantalla estoy
display_state_t state; // variable para saber el estado del display

//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------
static QueueHandle_t display_manager_queue;

//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
static void display_manager_task(void *arg)
{
    display_event_t display_ev;
    line = 0;
    screen = SCREEN_ONE; // variable para saber en que pantalla estoy
    state = NORMAL;
    clear = pdFALSE;
    set_timer();

    // aca asgianr valores a todas las variables globales del display
    while (true)
    {
        if (xQueueReceive(display_manager_queue, &display_ev, portMAX_DELAY) == pdTRUE)
        {
            switch (display_ev.cmd)
            {
            case CMD_UNDEFINED:
                break;
            case START_DISPLAY:
                display_init();
                display_set_screen_one(&screen, power, vegeflora, dia, modo, time_device);
                break;
            case AUX: // BOTON AUX 1 TOQUE
                switch (state)
                {
                case NORMAL:
                    if (screen == SCREEN_ONE)
                    {
                        display_set_screen_two(&screen, time_i1, time_i2, time_i3, time_i4, time_f1, time_f2, time_f3, time_f4);
                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }
                    else if (screen == SCREEN_TWO)
                    {
                        display_set_screen_three(&screen, time_pwmi, time_pwmf, fpower);
                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }
                    else // screen = SCREEN_THREE
                    {

                        display_set_screen_one(&screen, 80, "V", true, true, time_device);
                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }
                    break;
                case CONFIG_LINE:
                    state = CONFIG_PARAM;
                    // la funcion que entra a la linea titilante
                    break;
                case CONFIG_PARAM:
                    state = CONFIG_LINE;
                    // la funcion que me hace salir de la linea y vuelve titilante
                    break;

                default:
                    break;
                }

                break;
            case AUXT: // boton AUX 3 segundos
                switch (state)
                {
                case NORMAL:
                    // aca tengo que entrar a la primera linea titilante en la pantanlla en la que este
                    state = CONFIG_LINE;
                    line = 0;
                    display_blink_manager(screen, line, 3); // con esta veo que pantalla estoy

                    break;
                case CONFIG_LINE:
                    // vuelvo a NORMAL a la pagina correspondiente
                    state = NORMAL;
                    stop_timer();
                    if (screen == SCREEN_ONE)
                    {
                        display_set_screen_one(&screen, display_ev.pwm_value, display_ev.vege_flora, true, true, time_device);
                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }
                    else if (screen == SCREEN_TWO)
                    {
                        display_set_screen_two(&screen, time_i1, time_i2, time_i3, time_i4, time_f1, time_f2, time_f3, time_f4);
                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }
                    else // screen = SCREEN_THREE
                    {
                        display_set_screen_three(&screen, time_pwmi, time_pwmf, fpower);
                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }
                    break;
                case CONFIG_PARAM:
                    // vuelvo a NORMAL a la pagina correspondiente
                    state = NORMAL;
                    stop_timer();
                    if (screen == SCREEN_ONE)
                    {
                        display_set_screen_one(&screen, display_ev.pwm_value, display_ev.vege_flora, true, true, time_device);
                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }
                    else if (screen == SCREEN_TWO)
                    {
                        display_set_screen_two(&screen, time_i1, time_i2, time_i3, time_i4, time_f1, time_f2, time_f3, time_f4);
                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }
                    else // screen = SCREEN_THREE
                    {
                        display_set_screen_three(&screen, time_pwmi, time_pwmf, fpower);
                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }
                    break;
                default:
                    break;
                }

            case VF:
                switch (state)
                {
                case NORMAL:
                    display_set_vege_flora(display_ev.vege_flora);
                    break;
                case CONFIG_LINE:
                    // aca no hace nada
                    break;
                case CONFIG_PARAM:
                    // funcion para ir al siguiente numero a modificar
                    break;

                default:
                    break;
                }
                break;
            case DOWN:
                switch (state)
                {
                case NORMAL:
                    display_set_power(display_ev.pwm_value, display_ev.vege_flora);
                    break;
                case CONFIG_LINE:
                    stop_timer();
                    display_blink_manager(screen, line, 0); // 0 es down

                    break;
                case CONFIG_PARAM:
                    // bajo numero a configurar
                    break;

                default:
                    break;
                }
                break;
            case UP:
                switch (state)
                {
                case NORMAL:
                    display_set_power(display_ev.pwm_value, display_ev.vege_flora);
                    break;
                case CONFIG_LINE:
                    stop_timer();
                    display_blink_manager(screen, line, 1); // 1 es up

                    break;
                case CONFIG_PARAM:
                    // subo numero a configurar
                    break;

                default:
                    break;
                }
                break;
            default:
                break;
            }
        }
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void display_manager_init(void)
{
    set_i2c();
    ESP_LOGI(TAG, "DISPLAY inicializado");

    display_manager_queue = xQueueCreate(QUEUE_ELEMENT_QUANTITY, sizeof(display_event_t));

    xTaskCreate(display_manager_task, "display_manager_task", configMINIMAL_STACK_SIZE * 10,
                NULL, configMAX_PRIORITIES - 2, NULL);
}
//------------------------------------------------------------------------------
void display_manager_start(uint8_t pwm_value, char vege_flora)
{
    display_event_t display_ev;

    display_ev.cmd = START_DISPLAY;
    display_ev.pwm_value = pwm_value;
    display_ev.vege_flora = vege_flora;
    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_down()
{
    display_event_t display_ev;

    display_ev.cmd = DOWN;
    // display_ev.pwm_value = pwm_value;
    // display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_up()
{
    display_event_t display_ev;

    display_ev.cmd = UP;
    // display_ev.pwm_value = pwm_value;
    // display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_vf()
{
    display_event_t display_ev;

    display_ev.cmd = VF;
    // display_ev.pwm_value = pwm_value;
    // display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_aux()
{
    display_event_t display_ev;

    display_ev.cmd = AUX;
    // display_ev.pwm_value = pwm_value;
    // display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_auxt()
{
    display_event_t display_ev;

    display_ev.cmd = AUXT;
    // display_ev.pwm_value = pwm_value;
    // display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}

esp_err_t display_blink_manager(screen_t screen, uint8_t line, uint8_t cmd)
{
    switch (screen)
    {
    case SCREEN_ONE:
        // en esta pantalla solo se modifica la ultima linea
        line = 3;
        start_timer();
        break;
    case SCREEN_TWO:
        // chequeo si vino up o down
        if (cmd == 0) // es down
        {
            if (line == 0)
            {
                line = 3; // voy a la ultima linea
            }
            else
            {
                --line; // voy a la anterior linea
            }
        }
        else if (cmd == 1) // es up
        {
            if (line == 3)
            {
                line = 0; // voy a la primera linea
            }
            else
            {
                ++(line); // voy a la siguiente linea
            }
        }
        if (line == 0)
        {
            // blink renglon 1
            start_timer();
        }
        else if (line == 1)
        {
            // blink renglon 2
            start_timer();
        }
        else if (line == 2)
        {
            // blink renglon 3
            start_timer();
        }
        else if (line == 3)
        {
            // blink renglon 4
            start_timer();
        }

        break;
    case SCREEN_THREE:
        if (cmd == 0) // es down
        {
            if (line == 0)
            {
                line = 3; // voy a la ultima linea
            }
            else if (line == 3)
            {
                line = 1;
            }
            else
            {
                --line; // voy a la anterior linea
            }
        }
        else if (cmd == 1) // es up
        {
            if (line == 3)
            {
                line = 0; // voy a la primera linea
            }
            else if (line == 1)
            {
                line = 3;
            }
            else
            {
                ++line; // voy a la siguiente linea
            }
        }
        start_timer();
        break;

    default:
        break;
    }

    return ESP_OK;
}

esp_err_t clear_line(uint8_t line)
{
    char *clear = "                ";
    set_cursor(line, 0);
    display_write_string(clear);
    return ESP_OK;
}

void blink_callback(TimerHandle_t timer)
{
    ESP_LOGI("TIMER", "Entro al callback");
    if (clear == pdFALSE)
    {
        clear_line(line);
    }
    else
    {
        switch (screen)
        {
        case SCREEN_ONE:
            screen_one_line_three(time_device, dia, modo);
            break;
        case SCREEN_TWO:
            if (line == 0)
            {
                screen_two_line(line, time_i1, time_f1);
            }
            else if (line == 1)
            {
                screen_two_line(line, time_i2, time_f2);
            }
            else if (line == 2)
            {
                screen_two_line(line, time_i3, time_f3);
            }
            else // line == 3
            {
                screen_two_line(line, time_i4, time_f4);
            }

            break;
        case SCREEN_THREE:
            screen_three_line(line, fpower, time_pwmi, time_pwmf);
            break;
        default:
            break;
        }
    }
}

esp_err_t set_timer()
{
    ESP_LOGI("TIMER", "Inicializo timer");
    timer = xTimerCreate("timer", pdMS_TO_TICKS(interval), pdTRUE, (void *)timerid, blink_callback);
    if (timer == NULL)
    {
        ESP_LOGI("TIMER", "No se creó el timer");
    }
    else
    {
        ESP_LOGI("TIMER", "El timer se creó correctamente");
    }
    return ESP_OK;
}

esp_err_t start_timer()
{
    if (xTimerStart(timer, 0) != pdPASS)
    {
        ESP_LOGI("TIMER", "Error al iniciar el timer");
    }
    else
    {
        ESP_LOGI("TIMER", "Inicio de timer");
    }
    return ESP_OK;
}

esp_err_t stop_timer()
{
    if (xTimerStop(timer, 0) != pdPASS)
    {
        printf("Error al detener el temporizador de FreeRTOS.\n");
    }
    return ESP_OK;
}

esp_err_t reset_timer()
{
    if (xTimerReset(timer, 0) != pdPASS)
    {
        printf("Error al reiniciar el temporizador de FreeRTOS.\n");
    }
    return ESP_OK;
}

//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------
