//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "sdkconfig.h"

#include "esp_log.h"

#include "../include/board_def.h"
#include "../include/display_manager.h"
#include "../include/display_dogs164.h"
#include "../include/current_time_manager.h"
#include "../include/jumpers_manager.h"
#include "../include/pwm_manager.h"

//  #include "display_dogs164.c"

static const char *TAG = "I2C";
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define DEBUG_MODULE 1

#define QUEUE_ELEMENT_QUANTITY 25
//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

// variables del timer
TimerHandle_t timer;
int interval = 600; // ms de timer
int timerid = 1;
bool clear;

// timer para refrescar la hora en el display
TimerHandle_t timerh;
int intervalh = 1000; // ms de timer, cada 1 segundo refresco la hora
int timeridh = 2;

// timer para parpadear los simbolos del dia cuando está la rampa
TimerHandle_t timer_dia;
int interval_dia = 1000; // ms de timer, cada 1 segundo refresco la hora
int timerid_dia = 3;

// variables globales de parametros a escribir en el display
fading_status_t fade_stat;
struct tm time_device;
struct tm time_i1;
struct tm time_f1;
struct tm time_i2;
struct tm time_f2;
struct tm time_i3;
struct tm time_f3;
struct tm time_i4;
struct tm time_f4;
struct tm time_i1_aux;
struct tm time_f1_aux;
struct tm time_i2_aux;
struct tm time_f2_aux;
struct tm time_i3_aux;
struct tm time_f3_aux;
struct tm time_i4_aux;
struct tm time_f4_aux;
bool flag_times;
uint8_t check_time;
struct tm time_pwmi;
struct tm time_pwmf;
uint8_t pwm_man;  // potencia manual del pwm
uint8_t pwm_auto; // potencia del pwm en automatico
uint8_t power;
uint8_t pwm_dia_rise;
char fpower[6];
uint32_t fpowerppf;
flora_vege_status_t vegeflora;
char vegeflorachar; // con esta me manejo en el display
simul_day_status_t dia;
bool diabool; // con este me manejo en e display
bool flag_dia;
bool amanecer;
pwm_mode_t modo;
bool modobool;    // con esta me manejo en el display
uint8_t contrast; // contraste del display
bool ddots_state; // variable para el timer de la hora y parpadear los dos puntos

uint8_t param_two;
uint8_t param_three;
// comandos de las acciones del display

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
    set_timerh();
    set_timer_dia();
    start_timerh();
    start_timer_dia();
    contrast = 10;
    pwm_auto = 0;
    pwm_man = 0;
    power = 0;
    param_two = 1;
    param_three = 1;
    ddots_state = false;
    flag_times = false;
    flag_dia = false;
    check_time = 0;
    pwm_dia_rise = 0;
    amanecer = pdFALSE;
    get_fading_status(&fade_stat);
    ESP_LOGE("FADING STATUS", "FADING STATUS ES %u", fade_stat);

    while (true)
    {
        if (xQueueReceive(display_manager_queue, &display_ev, portMAX_DELAY) == pdTRUE)
        {
            switch (display_ev.cmd)
            {
            case START_DISPLAY:
                screen = NONE;
                display_init();
                get_params();
                set_contrast(contrast);
                // elijo la potencia en base al modo en que estoy
                if (modobool == false)
                {
                    power = pwm_man;
                }
                else
                {
                    if (compare_times(time_pwmi, time_device) == GREATER) // el horario de inicio del pwm es mayor  que el del equipo, no está la  salida prendida
                    {
                        power = 0;
                    }
                    else if (compare_times(time_pwmi, time_device) == EQUAL) // los horarios son  iguales, ya empieza el pwm pro ende muestro la potencia
                    {
                        power = pwm_auto;
                    }
                    else // horairo de inicio de pwm menor al del equipo
                    {
                        if (compare_times(time_pwmf, time_device) == GREATER) // si el horario final es superior al del equipo, está activa la salida del pwm
                        {
                            power = pwm_auto;
                        }
                        else if (compare_times(time_pwmf, time_device) == EQUAL) // horario final iguala l del dispositivo, ya se acaba asique muestro cero
                        {
                            power = 0;
                        }
                        else // horario final del dispositivo menor al del equipo, la salida pwm esta apagada
                        {
                            power = 0;
                        }
                    }
                }
                display_set_screen_one(&screen, fpower, power, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);

                break;
            case AUX: // BOTON AUX 1 TOQUE
                switch (state)
                {
                case NORMAL:
                    get_params();
                    if (screen == SCREEN_ONE)
                    {
                        display_set_screen_two(&screen, time_i1, time_i2, time_i3, time_i4, time_f1, time_f2, time_f3, time_f4);

                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }
                    else if (screen == SCREEN_TWO)
                    { // elijo la potencia en base al modo en que estoy
                        if (modobool == false)
                        {
                            power = pwm_man;
                        }
                        else
                        {
                            if (compare_times(time_pwmi, time_device) == GREATER) // el horario de inicio del pwm es mayor  que el del equipo, no está la  salida prendida
                            {
                                power = 0;
                            }
                            else if (compare_times(time_pwmi, time_device) == EQUAL) // los horarios son  iguales, ya empieza el pwm pro ende muestro la potencia
                            {
                                power = pwm_dia_rise;
                            }
                            else // horairo de inicio de pwm menor al del equipo
                            {
                                if (compare_times(time_pwmf, time_device) == GREATER) // si el horario final es superior al del equipo, está activa la salida del pwm
                                {
                                    if (fade_stat == FADING_IN_PROGRESS)
                                    {
                                        power = pwm_dia_rise;
                                    }
                                    else
                                    {
                                        power = pwm_auto;
                                    }
                                }
                                else if (compare_times(time_pwmf, time_device) == EQUAL) // horario final iguala l del dispositivo, ya se acaba asique muestro cero
                                {

                                    power = 0;
                                }
                                else // horario final del dispositivo menor al del equipo, la salida pwm esta apagada
                                {
                                    if (time_pwmi.tm_hour > time_pwmf.tm_hour)
                                    {
                                        power = pwm_auto;
                                    }
                                    else
                                    {
                                        power = 0;
                                    }
                                }
                            }
                        }
                        display_set_screen_one(&screen, fpower, power, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }
                    break;
                case CONFIG_LINE:
                    state = CONFIG_PARAM;
                    stop_timer();
                    // la funcion que entra a la linea titilante
                    display_param_manager(AUX); // tengo que pasarle un numero para indicar que boton toqué
                    break;
                case CONFIG_PARAM:
                    if (flag_times == true) // significa que entre a modificar horarios
                    {
                        check_time = checkOverlap(time_i1, time_f1, time_i2, time_f2, time_i3, time_f3, time_i4, time_f4);
                        ESP_LOGW("CHECK TIME", "Check time es %u", check_time);
                        if (check_time > 0) // si hubo overlap, vuelvo a los horarios anteriores
                        {
                            time_i1 = time_i1_aux;
                            time_f1 = time_f1_aux;
                            time_i2 = time_i2_aux;
                            time_f2 = time_f2_aux;
                            time_i3 = time_i3_aux;
                            time_f3 = time_f3_aux;
                            time_i4 = time_i4_aux;
                            time_f4 = time_f4_aux;
                        }
                    }
                    save_params();
                    state = CONFIG_LINE;
                    // dejo de blinkear el caracter
                    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_OFF);
                    // la funcion que me hace salir de la linea y vuelve titilante
                    display_blink_manager(screen, 3); // vuelvo al blink line
                    break;

                default:
                    break;
                }

                break;
            case AUXT: // boton AUX 3 segundos

                switch (state)
                {
                case NORMAL:
                    get_params();
                    // elijo la potencia en base al modo en que estoy
                    if (modobool == false)
                    {
                        power = pwm_man;
                    }
                    else
                    {
                        if (compare_times(time_pwmi, time_device) == GREATER) // el horario de inicio del pwm es mayor  que el del equipo, no está la  salida prendida
                        {
                            power = 0;
                        }
                        else if (compare_times(time_pwmi, time_device) == EQUAL) // los horarios son  iguales, ya empieza el pwm pro ende muestro la potencia
                        {
                            power = pwm_auto;
                        }
                        else // horairo de inicio de pwm menor al del equipo
                        {
                            if (compare_times(time_pwmf, time_device) == GREATER) // si el horario final es superior al del equipo, está activa la salida del pwm
                            {
                                power = pwm_auto;
                            }
                            else if (compare_times(time_pwmf, time_device) == EQUAL) // horario final iguala l del dispositivo, ya se acaba asique muestro cero
                            {
                                power = 0;
                            }
                            else // horario final del dispositivo menor al del equipo, la salida pwm esta apagada
                            {
                                power = 0;
                            }
                        }
                    }
                    // aca tengo que entrar a la primera linea titilante en la pantanlla en la que este
                    if (screen == SCREEN_ONE)
                    {
                        display_set_screen_three(&screen, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);
                        ESP_LOGI(TAG, "Pantalla %u", screen);
                    }

                    state = CONFIG_LINE;
                    line = 0;
                    param_two = 1;
                    param_three = 1;
                    display_blink_manager(screen, 3); // con esta veo que pantalla estoy

                    break;
                case CONFIG_LINE:
                    // vuelvo a NORMAL a la pagina correspondiente
                    state = NORMAL;
                    stop_timer();
                    // dejo de blinkear el caracter
                    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_OFF);
                    // vuelvo a la pantalla 1
                    global_manager_get_current_time_info(&time_device);
                    // elijo la potencia en base al modo en que estoy
                    if (modobool == false)
                    {
                        power = pwm_man;
                    }
                    else
                    {
                        if (compare_times(time_pwmi, time_device) == GREATER) // el horario de inicio del pwm es mayor  que el del equipo, no está la  salida prendida
                        {
                            power = 0;
                        }
                        else if (compare_times(time_pwmi, time_device) == EQUAL) // los horarios son  iguales, ya empieza el pwm pro ende muestro la potencia
                        {
                            power = pwm_dia_rise;
                        }
                        else // horairo de inicio de pwm menor al del equipo
                        {
                            if (compare_times(time_pwmf, time_device) == GREATER) // si el horario final es superior al del equipo, está activa la salida del pwm
                            {
                                if (fade_stat == FADING_IN_PROGRESS)
                                {
                                    power = pwm_dia_rise;
                                }
                                else
                                {
                                    power = pwm_auto;
                                }
                            }
                            else if (compare_times(time_pwmf, time_device) == EQUAL) // horario final iguala l del dispositivo, ya se acaba asique muestro cero
                            {

                                power = 0;
                            }
                            else // horario final del dispositivo menor al del equipo, la salida pwm esta apagada
                            {
                                if (time_pwmi.tm_hour > time_pwmf.tm_hour)
                                {
                                    power = pwm_auto;
                                }
                                else
                                {
                                    power = 0;
                                }
                            }
                        }
                    }
                    display_set_screen_one(&screen, fpower, power, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
                    ESP_LOGI(TAG, "Pantalla %u", screen);

                    break;
                case CONFIG_PARAM:
                    // vuelvo a NORMAL a la pagina correspondiente
                    if (flag_times == true) // significa que entre a modificar horarios
                    {
                        check_time = checkOverlap(time_i1, time_f1, time_i2, time_f2, time_i3, time_f3, time_i4, time_f4);
                        ESP_LOGW("CHECK TIME", "Check time es %u", check_time);
                        if (check_time > 0) // si hubo overlap, vuelvo a los horarios anteriores
                        {
                            time_i1 = time_i1_aux;
                            time_f1 = time_f1_aux;
                            time_i2 = time_i2_aux;
                            time_f2 = time_f2_aux;
                            time_i3 = time_i3_aux;
                            time_f3 = time_f3_aux;
                            time_i4 = time_i4_aux;
                            time_f4 = time_f4_aux;
                        }
                    }
                    save_params();
                    state = NORMAL;
                    stop_timer();
                    // vuelvo a la pantalla 1
                    // dejo de blinkear el caracter
                    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_OFF);
                    global_manager_get_current_time_info(&time_device);
                    // elijo la potencia en base al modo en que estoy
                    if (modobool == false)
                    {
                        power = pwm_man;
                    }
                    else
                    {
                        if (compare_times(time_pwmi, time_device) == GREATER) // el horario de inicio del pwm es mayor  que el del equipo, no está la  salida prendida
                        {
                            power = 0;
                        }
                        else if (compare_times(time_pwmi, time_device) == EQUAL) // los horarios son  iguales, ya empieza el pwm pro ende muestro la potencia
                        {
                            power = pwm_dia_rise;
                        }
                        else // horairo de inicio de pwm menor al del equipo
                        {
                            if (compare_times(time_pwmf, time_device) == GREATER) // si el horario final es superior al del equipo, está activa la salida del pwm
                            {
                                if (fade_stat == FADING_IN_PROGRESS)
                                {
                                    power = pwm_dia_rise;
                                }
                                else
                                {
                                    power = pwm_auto;
                                }
                            }
                            else if (compare_times(time_pwmf, time_device) == EQUAL) // horario final iguala l del dispositivo, ya se acaba asique muestro cero
                            {

                                power = 0;
                            }
                            else // horario final del dispositivo menor al del equipo, la salida pwm esta apagada
                            {
                                if (time_pwmi.tm_hour > time_pwmf.tm_hour)
                                {
                                    power = pwm_auto;
                                }
                                else
                                {
                                    power = 0;
                                }
                            }
                        }
                    }
                    display_set_screen_one(&screen, fpower, power, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
                    ESP_LOGI(TAG, "Pantalla %u", screen);

                    break;
                default:
                    break;
                }
                break;
            case VF:
                switch (state)
                {
                case NORMAL:
                    if (display_ev.vege_flora == FLORA_VEGE_OUTPUT_DISABLE)
                    {
                        vegeflorachar = 'V';
                    }
                    else
                    {
                        vegeflorachar = 'F';
                    }
                    display_set_vege_flora(vegeflorachar);
                    break;
                case CONFIG_LINE:
                    // aca no hace nada
                    break;
                case CONFIG_PARAM:
                    ESP_LOGI("CONFIG_LINE", "Muevo al parametro siguiente");
                    display_param_manager(VF);
                    break;

                default:
                    break;
                }
                break;
            case VFT:
                // cambio el modo (manual o automatico)
                modobool = (bool)display_ev.pwm_mode;
                if (modobool == false)
                {
                    power = pwm_man;
                }
                else
                {
                    if (compare_times(time_pwmi, time_device) == GREATER) // el horario de inicio del pwm es mayor  que el del equipo, no está la  salida prendida
                    {
                        power = 0;
                    }
                    else if (compare_times(time_pwmi, time_device) == EQUAL) // los horarios son  iguales, ya empieza el pwm pro ende muestro la potencia
                    {
                        power = pwm_auto;
                    }
                    else // horairo de inicio de pwm menor al del equipo
                    {
                        if (compare_times(time_pwmf, time_device) == GREATER) // si el horario final es superior al del equipo, está activa la salida del pwm
                        {
                            power = pwm_auto;
                        }
                        else if (compare_times(time_pwmf, time_device) == EQUAL) // horario final iguala l del dispositivo, ya se acaba asique muestro cero
                        {
                            power = 0;
                        }
                        else // horario final del dispositivo menor al del equipo, la salida pwm esta apagada
                        {
                            power = 0;
                        }
                    }
                }
                display_set_screen_one(&screen, fpower, power, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
                break;
            case DOWN:
                switch (state)
                {
                case NORMAL:
                    power = display_ev.pwm_value;
                    if (display_ev.vege_flora == FLORA_VEGE_OUTPUT_DISABLE)
                    {
                        vegeflorachar = 'V';
                    }
                    else
                    {
                        vegeflorachar = 'F';
                    }
                    global_manager_get_current_time_info(&time_device);
                    display_set_screen_one(&screen, fpower, display_ev.pwm_value, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
                    break;
                case CONFIG_LINE:
                    stop_timer();
                    ESP_LOGI("CONFIG_LINE_DOWN", "line es %u", line);
                    param_two = 1;
                    param_three = 1;
                    if (is_jp1_dspy_connected() == 0 && screen == SCREEN_THREE && line == 2)
                    {

                        param_three = 6;
                    }
                    display_blink_manager(screen, 1); // 0 es down

                    break;
                case CONFIG_PARAM:
                    // bajo numero a configurar
                    if (is_jp1_dspy_connected() == 0 && screen == SCREEN_THREE && line == 2)
                    {
                        param_three = 6;
                    }
                    display_param_manager(DOWN);

                    break;

                default:
                    break;
                }
                break;
            case UP:
                switch (state)
                {
                case NORMAL:
                    power = display_ev.pwm_value;
                    if (display_ev.vege_flora == FLORA_VEGE_OUTPUT_DISABLE)
                    {
                        vegeflorachar = 'V';
                    }
                    else
                    {
                        vegeflorachar = 'F';
                    }
                    // printf("El char de vege_flora es %c", vegeflorachar);
                    global_manager_get_current_time_info(&time_device);
                    display_set_screen_one(&screen, fpower, display_ev.pwm_value, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
                    break;
                case CONFIG_LINE:
                    ESP_LOGI("CONFIG_LINE_UP", "line es %u", line);
                    stop_timer();
                    param_two = 1;
                    param_three = 1;
                    if (is_jp1_dspy_connected() == 0 && screen == SCREEN_THREE && line == 1)
                    {
                        param_three = 6;
                    }
                    display_blink_manager(screen, 0); // 1 es up

                    break;
                case CONFIG_PARAM:
                    // subo numero a configurar
                    if (is_jp1_dspy_connected() == 0 && screen == SCREEN_THREE && line == 2)
                    {
                        param_three = 6;
                    }
                    display_param_manager(UP);

                    break;

                default:
                    break;
                }
                break;
            case PWM_MANUAL_VALUE:
                switch (state)
                {
                case NORMAL:
                    // pwm_dia_rise = display_ev.pwm_value;
                    // power = display_ev.pwm_value;
                    get_params();
                    pwm_dia_rise = display_ev.pwm_value;
                    if (screen == SCREEN_ONE)
                    {
                        ESP_LOGI("PWM_MANUAL_VALUE", "PWM_MANUAL_VALUE");
                        display_set_power(display_ev.pwm_value, fpower);
                    }
                    break;
                default:
                    break;
                }
                break;
            case PWM_MODE_UPDATE:
                power = display_ev.pwm_value;
                get_params();
                if (display_ev.vege_flora == FLORA_VEGE_OUTPUT_DISABLE)
                {
                    vegeflorachar = 'V';
                }
                else
                {
                    vegeflorachar = 'F';
                }
                global_manager_get_current_time_info(&time_device);
                display_set_screen_one(&screen, fpower, display_ev.pwm_value, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
                break;
            default:
                break;
            }
        }
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void get_screen_state(display_state_t *state_info)
{
    *state_info = state;
}
//------------------------------------------------------------------------------
void get_screen_number(screen_t *number_info)
{
    *number_info = screen;
}
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
void display_manager_start(uint8_t pwm_value, char vege_flora, pwm_mode_t pwm_mode)
{
    display_event_t display_ev;

    display_ev.cmd = START_DISPLAY;
    display_ev.pwm_value = pwm_value;
    display_ev.vege_flora = vege_flora;
    display_ev.pwm_mode = pwm_mode;
    power = pwm_value;
    vegeflorachar = vege_flora;
    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_down(uint8_t pwm_value, flora_vege_status_t vege_flora)
{
    display_event_t display_ev;

    display_ev.cmd = DOWN;
    display_ev.pwm_value = pwm_value;
    display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_up(uint8_t pwm_value, flora_vege_status_t vege_flora)
{
    display_event_t display_ev;

    display_ev.cmd = UP;
    display_ev.pwm_value = pwm_value;
    display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_pwm_mode_update(uint8_t pwm_value, flora_vege_status_t vege_flora)
{
    display_event_t display_ev;

    display_ev.cmd = PWM_MODE_UPDATE;
    display_ev.pwm_value = pwm_value;
    display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_vf(flora_vege_status_t vege_flora)
{
    display_event_t display_ev;

    display_ev.cmd = VF;
    display_ev.vege_flora = vege_flora;

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
    // display_ev.pwm_value = power;
    // display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}

void display_manager_vft(pwm_mode_t pwm_mode)
{
    display_event_t display_ev;

    display_ev.cmd = VFT;
    display_ev.pwm_mode = pwm_mode;
    // display_ev.pwm_value = power;
    // display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}

void display_manager_manual(uint8_t pwm_value)
{
    display_event_t display_ev;

    printf(" UPDATE DISPLAY PWM VALUE: El valor de pwm_value es %u \n", pwm_value);

    display_ev.cmd = PWM_MANUAL_VALUE;
    display_ev.pwm_value = pwm_value;
    // display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}

esp_err_t display_blink_manager(screen_t screen, uint8_t cmd)
{
    switch (screen)
    {
    /*case SCREEN_ONE:
        // en esta pantalla solo se modifica la ultima linea
        display_set_screen_one(&screen, fpower, power, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
        line = 3;
        start_timer();
        break;*/
    case SCREEN_TWO:
        // chequeo si vino up o down
        display_set_screen_two(&screen, time_i1, time_i2, time_i3, time_i4, time_f1, time_f2, time_f3, time_f4);
        flag_times = true; // entre para configurar los horarios
        /* if (cmd == 0) // es down
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
         }*/
        /*if (line == 0)
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
            ESP_LOGI("CONFIG_LINE", "line es %u", line);
            start_timer();
        }*/
        // start_timer();
        break;
    case SCREEN_THREE:
        display_set_screen_three(&screen, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);

        /*if (cmd == 0) // es down
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
        start_timer();*/
        break;

    default:
        break;
    }
    if (cmd == 0) // es down
    {
        if (line == 0)
        {
            if (is_jp1_dspy_connected() == 0 && screen == SCREEN_THREE) // jumper del GPIO35 no conectado y es la pantalla 3
            {
                line = 2; // si no tengo el jumper, no se ve la ultima linea
            }
            else
            {
                line = 3; // voy a la ultima linea
            }
        }
        else
        {
            --line; // voy a la anterior linea
        }
    }
    else if (cmd == 1) // es up
    {
        if (is_jp1_dspy_connected() == 0 && screen == SCREEN_THREE) // jumper del GPIO35 no conectado y es la pantalla 3
        {
            if (line == 2)
            {
                line = 0; // voy a la primera linea
            }
            else
            {
                ++(line); // voy a la siguiente linea
            }
        }
        else
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
    }
    start_timer();

    return ESP_OK;
}

esp_err_t clear_line(uint8_t linee)
{
    ESP_LOGI("TIMER", "Clear_line");
    char *clearr = "                ";
    set_cursor(linee, 0);
    display_write_string(clearr);
    return ESP_OK;
}

void blink_callback(TimerHandle_t timer)
{
    ESP_LOGI("TIMER", "Entro al callback");
    if (clear == pdFALSE)
    {
        clear_line(line);
        clear = pdTRUE;
    }
    else
    {
        switch (screen)
        {
        /*case SCREEN_ONE:
            // screen_one_line_three(time_device, diabool, modobool);
            clear = pdFALSE;
            break;*/
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
            clear = pdFALSE;
            break;
        case SCREEN_THREE:

            screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);

            clear = pdFALSE;
            break;
        default:
            break;
        }
    }
}

esp_err_t set_timer()
{
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
    ESP_LOGI("TIMER", "Paro el timer");
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

void time_callback(TimerHandle_t timerh)
{ // obtengo la hora del dispositivo y printeo la linea correspondiente

    if (screen == SCREEN_ONE && state == NORMAL)
    {
        global_manager_get_current_time_info(&time_device);
        screen_one_time_device(time_device);
        if (ddots_state == true)
        {
            set_cursor(0, 13);
            display_write_string(":");
            ddots_state = false;
        }
        else
        {
            set_cursor(0, 13);
            display_write_string(" ");
            ddots_state = true;
        }
    }
}

esp_err_t set_timerh()
{
    timerh = xTimerCreate("timerh", pdMS_TO_TICKS(intervalh), pdTRUE, (void *)timeridh, time_callback);
    if (timerh == NULL)
    {
        ESP_LOGI("TIMERh", "No se creó el timer");
    }
    else
    {
        ESP_LOGI("TIMERh", "El timer se creó correctamente");
    }
    return ESP_OK;
}

esp_err_t start_timerh()
{
    if (xTimerStart(timerh, 0) != pdPASS)
    {
        ESP_LOGI("TIMERh", "Error al iniciar el timerh");
    }
    else
    {
        ESP_LOGI("TIMERh", "Inicio de timerh");
    }
    return ESP_OK;
}

esp_err_t stop_timerh()
{
    if (xTimerStop(timerh, 0) != pdPASS)
    {
        printf("Error al detener el temporizadorh de FreeRTOS.\n");
    }
    ESP_LOGI("TIMERh", "Paro el timerh");
    return ESP_OK;
}

esp_err_t reset_timerh()
{
    if (xTimerReset(timerh, 0) != pdPASS)
    {
        printf("Error al reiniciar el temporizadorh de FreeRTOS.\n");
    }
    return ESP_OK;
}

void time_callback_dia(TimerHandle_t timer_dia)
{ //
    if (screen == SCREEN_ONE && state == NORMAL && modobool == pdTRUE && diabool == pdTRUE)
    {
        get_fading_status(&fade_stat);
        if (fade_stat == FADING_IN_PROGRESS)
        {
            set_cursor(3, 1);
            if (flag_dia == false)
            {
                display_write_string("  ");
                flag_dia = pdTRUE;
            }
            else
            {
                display_send_data(0x12);
                set_cursor(3, 2);
                display_send_data(0x13);
                flag_dia = pdFALSE;
            }
        }
        else
        {
            set_cursor(3, 1);
            display_send_data(0x12);
            set_cursor(3, 2);
            display_send_data(0x13);
            flag_dia = pdFALSE;
        }
    }
}

esp_err_t set_timer_dia()
{
    timer_dia = xTimerCreate("timer_dia", pdMS_TO_TICKS(interval_dia), pdTRUE, (void *)timerid_dia, time_callback_dia);
    if (timerh == NULL)
    {
        ESP_LOGI("TIMER_dia", "No se creó el timer");
    }
    else
    {
        ESP_LOGI("TIMER_dia", "El timer se creó correctamente");
    }
    return ESP_OK;
}

esp_err_t start_timer_dia()
{
    if (xTimerStart(timer_dia, 0) != pdPASS)
    {
        ESP_LOGI("TIMER_dia", "Error al iniciar el timer_dia");
    }
    else
    {
        ESP_LOGI("TIMER_dia", "Inicio de timer_dia");
    }
    return ESP_OK;
}

esp_err_t stop_timer_dia()
{
    if (xTimerStop(timer_dia, 0) != pdPASS)
    {
        printf("Error al detener el temporizador_dia de FreeRTOS.\n");
    }
    ESP_LOGI("TIMER_dia", "Paro el timer_dia");
    return ESP_OK;
}

esp_err_t reset_timer_dia()
{
    if (xTimerReset(timer_dia, 0) != pdPASS)
    {
        printf("Error al reiniciar el temporizador_dia de FreeRTOS.\n");
    }
    return ESP_OK;
}

esp_err_t display_param_manager(display_event_cmds_t cmd)
{
    switch (screen)
    {
    /*case SCREEN_ONE:
        display_set_screen_one(&screen, fpower, power, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
        if (cmd == VF || cmd == AUX)
        {
            screen_one_param(cmd);
        }
        else if (cmd == UP)
        {
            ESP_LOGI("PARAM_MANAGER", "Entro a param_modified_one");
            param_modified_one(UP);
            ESP_LOGI("PARAM_MANAGER", "Salgo de param_modified_one");
        }
        else // cmd == DOWN
        {
            // aca subo el numero o cambio el estado
            // param_modified_one(DOWN);
            ESP_LOGI("PARAM_MANAGER", "Entro a param_modified_one");
            param_modified_one(DOWN);
            ESP_LOGI("PARAM_MANAGER", "Salgo de param_modified_one");
        }
        break;*/
    case SCREEN_TWO:
        display_set_screen_two(&screen, time_i1, time_i2, time_i3, time_i4, time_f1, time_f2, time_f3, time_f4);
        if (cmd == VF || cmd == AUX)
        {
            screen_two_param(cmd);
        }
        else if (cmd == UP)
        {
            ESP_LOGI("PARAM_MANAGER", "Entro a param_modified_two");
            param_modified_two(UP);
            ESP_LOGI("PARAM_MANAGER", "Salgo de param_modified_two");
        }
        else // cmd == DOWN
        {
            ESP_LOGI("PARAM_MANAGER", "Entro a param_modified_two");
            param_modified_two(DOWN);
            ESP_LOGI("PARAM_MANAGER", "Salgo de param_modified_two");
        }
        break;
    case SCREEN_THREE:
        display_set_screen_three(&screen, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);
        if (cmd == VF || cmd == AUX)
        {
            ESP_LOGE("PARAM_MANAGER", "Line es %u", line);
            screen_three_param(cmd);
        }
        else if (cmd == UP)
        {
            ESP_LOGI("PARAM_MANAGER", "Entro a param_modified_three");
            param_modified_three(UP);
            ESP_LOGI("PARAM_MANAGER", "Salgo de param_modified_three");
            ESP_LOGI("param_modified_three", "La hora del dispositivo es %u", time_device.tm_hour);
        }
        else // cmd == DOWN
        {
            ESP_LOGI("PARAM_MANAGER", "Entro a param_modified_three");
            param_modified_three(DOWN);
            ESP_LOGI("PARAM_MANAGER", "Salgo de param_modified_three");
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}

/*esp_err_t screen_one_param(display_event_cmds_t cmd)
{

    // screen_one_line_three(time_device, diabool, modobool); // escribo linea para que no quede vacia
    if (cmd == VF)
    {
        if (param_one == 4)
        {
            param_one = 1;
        }
        else
        {
            param_one++;
        }
    }
    switch (param_one)
    {
    case 1:
        set_cursor(3, 4);
        break;

    case 2:
        set_cursor(3, 7);
        break;

    case 3:
        set_cursor(3, 12);
        break;

    case 4:
        set_cursor(3, 15);
        break;

    default:
        break;
    }
    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_ON);

    return ESP_OK;
}*/

esp_err_t screen_two_param(display_event_cmds_t cmd)
{

    switch (line) // escribo la linea para que no quede en blanco
    {
    case 0:
        screen_two_line(line, time_i1, time_f1);
        break;
    case 1:
        screen_two_line(line, time_i2, time_f2);
        break;
    case 2:
        screen_two_line(line, time_i3, time_f3);
        break;
    case 3:
        screen_two_line(line, time_i4, time_f4);
        break;

    default:
        break;
    }
    if (cmd == VF) // si el comando es VF, avanzo al siguiente
    {
        if (param_two == 4)
        {
            param_two = 1;
        }
        else
        {
            ESP_LOGI("Screen_two_param", "Incremento param_two");
            param_two++;
        }
        ESP_LOGI("Screen_two_param", "Param_two valu %u", param_two);
    }

    switch (param_two) // me fijo que parametro modifico
    {
    case 1:
        set_cursor(line, 5);
        break;

    case 2:
        set_cursor(line, 8);
        break;

    case 3:
        set_cursor(line, 12);
        break;

    case 4:
        set_cursor(line, 15);
        break;

    default:
        break;
    }
    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_ON);

    return ESP_OK;
}

esp_err_t screen_three_param(display_event_cmds_t cmd)
{

    screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
    if (line == 0)
    {
        if (cmd == VF)
        {
            if (param_three == 2)
            {
                param_three = 1;
            }
            else
            {
                param_three++;
            }
        }
        switch (param_three) // me fijo que parametro modifico
        {

        case 1:
            set_cursor(line, 12);
            break;

        case 2:
            set_cursor(line, 15);
            break;

        default:
            break;
        }
    }
    else if (line == 2)
    {
        if (is_jp1_dspy_connected() == 1) // jumper del GPIO35 conectado
        {
            if (cmd == VF)
            {
                if (param_three == 6)
                {
                    param_three = 1;
                }
                else
                {
                    param_three++;
                }
            }
            switch (param_three) // me fijo que parametro modifico
            {
            case 1:
                set_cursor(line, 1);
                break;
            case 2:
                set_cursor(line, 2);
                break;
            case 3:
                set_cursor(line, 3);
                break;
            case 4:
                set_cursor(line, 4);
                break;
            case 5:
                set_cursor(line, 5);
                break;
            case 6:
                set_cursor(line, 14);
                break;

            default:
                break;
            }
        }
        else
        {
            set_cursor(line, 14);
        }
    }
    else if (line == 3)
    {
        if (is_jp1_dspy_connected() == 1) // jumper del GPIO35 conectado
        {
            set_cursor(line, 15);
        }
    }
    else if (line == 1)
    {
        ESP_LOGE("PARAM_MANAGER", "Entre en el if del line 1");
        ESP_LOGE("PARAM_MANAGER", "Param_three es %u", param_three);
        if (cmd == VF)
        {

            if (param_three == 5)
            {
                param_three = 1;
            }
            else
            {
                param_three++;
            }
        }
        switch (param_three) // me fijo que parametro modifico
        {
        case 1:
            set_cursor(line, 0);
            break;
        case 2:
            set_cursor(line, 6);
            break;
        case 3:
            set_cursor(line, 9);
            break;
        case 4:
            set_cursor(line, 12);
            break;
        case 5:
            set_cursor(line, 15);
            break;

        default:
            break;
        }
    }

    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_ON);

    return ESP_OK;
}

/*esp_err_t param_modified_one(display_event_cmds_t cmd)
{
    ESP_LOGI("param_modified_one", "Param_one vale %u", param_one);
    if (param_one == 1)
    {
        if (diabool == pdTRUE)
        {
            diabool = pdFALSE;
        }
        else
        {
            diabool = pdTRUE;
        }
        // screen_one_line_three(time_device, diabool, modobool);
        set_cursor(3, 4);
    }
    if (param_one == 2)
    {
        if (modobool == pdTRUE)
        {
            modobool = pdFALSE;
        }
        else
        {
            modobool = pdTRUE;
        }
        // screen_one_line_three(time_device, diabool, modobool);
        set_cursor(3, 7);
    }
    if (param_one == 3)
    {
        if (cmd == UP)
        {
            if (time_device.tm_hour == 23)
            {
                time_device.tm_hour = 0;
            }
            else
            {
                time_device.tm_hour += 1;
            }
            // mktime(&time_device);
            // screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 12);
        }
        else
        {
            if (time_device.tm_hour == 0)
            {
                time_device.tm_hour = 23;
            }
            else
            {
                time_device.tm_hour -= 1;
            }
            // mktime(&time_device);
            // screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 12);
        }
    }
    if (param_one == 4)
    {

        if (cmd == UP)
        {
            if (time_device.tm_min == 59)
            {
                time_device.tm_min = 0;
            }
            else
            {
                time_device.tm_min += 1;
            }
            // mktime(&time_device);
            // screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 15);
        }
        else
        {
            if (time_device.tm_min == 0)
            {
                time_device.tm_min = 59;
            }
            else
            {
                time_device.tm_min -= 1;
            }
            // mktime(&time_device);
            // screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 15);
        }
    }
    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_ON);
    return ESP_OK;
}*/

esp_err_t param_modified_two(display_event_cmds_t cmd)
{
    ESP_LOGI("param_modified_two", "Param_two vale %u", param_two);

    switch (line)
    {
    case 0:
        ESP_LOGI("param_modified_two", "line %u", line);
        ESP_LOGI("param_modified_two", "Entro a param_two_bis");
        param_two_bis(cmd, &time_i1, &time_f1);
        ESP_LOGI("param_modified_two", "Salgo de param_two_bis");
        break;
    case 1:
        ESP_LOGI("param_modified_two", "line %u", line);
        param_two_bis(cmd, &time_i2, &time_f2);
        break;
    case 2:
        ESP_LOGI("param_modified_two", "line %u", line);
        param_two_bis(cmd, &time_i3, &time_f3);
        break;
    case 3:
        ESP_LOGI("param_modified_two", "line %u", line);
        param_two_bis(cmd, &time_i4, &time_f4);
        break;

    default:
        break;
    }

    return ESP_OK;
}

esp_err_t param_two_bis(display_event_cmds_t cmd, struct tm *time_i, struct tm *time_f)
{
    if (param_two == 1)
    {
        if (cmd == UP)
        {
            if (time_i->tm_hour == 23)
            {
                time_i->tm_hour = 0;
            }
            else
            {
                time_i->tm_hour += 1;
            }
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);

            set_cursor(line, 5);
        }
        else
        {
            if (time_i->tm_hour == 0)
            {
                time_i->tm_hour = 23;
            }
            else
            {
                time_i->tm_hour -= 1;
            }
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 5);
        }
    }

    if (param_two == 2)
    {
        if (cmd == UP)
        {
            if (time_i->tm_min == 59)
            {
                time_i->tm_min = 0;
            }
            else
            {
                time_i->tm_min += 1;
            }
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);

            set_cursor(line, 8);
        }
        else
        {
            if (time_i->tm_min == 0)
            {
                time_i->tm_min = 59;
            }
            else
            {
                time_i->tm_min -= 1;
            }
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 8);
        }
    }

    if (param_two == 3)
    {
        if (cmd == UP)
        {

            if (time_f->tm_hour == 23)
            {
                time_f->tm_hour = 0;
            }
            else
            {
                time_f->tm_hour += 1;
            }
            mktime(time_f);
            screen_two_line(line, *time_i, *time_f);

            set_cursor(line, 12);
        }
        else
        {
            if (time_f->tm_hour == 0)
            {
                time_f->tm_hour = 23;
            }
            else
            {
                time_f->tm_hour -= 1;
            }
            mktime(time_f);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 12);
        }
    }

    if (param_two == 4)
    {
        if (cmd == UP)
        {
            if (time_f->tm_min == 59)
            {
                time_f->tm_min = 0;
            }
            else
            {
                time_f->tm_min += 1;
            }
            mktime(time_f);
            screen_two_line(line, *time_i, *time_f);

            set_cursor(line, 15);
        }
        else
        {
            if (time_f->tm_min == 0)
            {
                time_f->tm_min = 59;
            }
            else
            {
                time_f->tm_min -= 1;
            }
            mktime(time_f);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 15);
        }
    }
    return ESP_OK;
}

esp_err_t param_modified_three(display_event_cmds_t cmd)
{
    ESP_LOGI("param_modified_three", "Param_three vale %u", param_three);
    ESP_LOGI("param_modified_three", "Line %u", line);
    if (line == 0)
    {
        switch (param_three)
        {
        case 1:
            if (cmd == UP)
            {
                ESP_LOGI("param_modified_three", "Subo hora del dispositivo");
                if (time_device.tm_hour == 23)
                {
                    time_device.tm_hour = 0;
                }
                else
                {
                    time_device.tm_hour += 1;
                }
                mktime(&time_device);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
                set_cursor(0, 12);
            }
            else
            {
                ESP_LOGI("param_modified_three", "Bajo hora del dispositivo");
                if (time_device.tm_hour == 0)
                {
                    time_device.tm_hour = 23;
                }
                else
                {
                    time_device.tm_hour -= 1;
                }
                ESP_LOGI("param_modified_three", "La hora del dispositivo es %u", time_device.tm_hour);
                mktime(&time_device);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
                set_cursor(0, 12);
            }
            break;

        case 2:
            if (cmd == UP)
            {
                ESP_LOGI("param_modified_three", "Subo minutos del dispositivo");
                if (time_device.tm_min == 59)
                {
                    time_device.tm_min = 0;
                }
                else
                {
                    time_device.tm_min += 1;
                }
                mktime(&time_device);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
                set_cursor(0, 15);
            }
            else
            {
                ESP_LOGI("param_modified_three", "Bajo minutos del dispositivo");
                if (time_device.tm_min == 0)
                {
                    time_device.tm_min = 59;
                }
                else
                {
                    time_device.tm_min -= 1;
                }
                mktime(&time_device);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
                set_cursor(0, 15);
            }
            break;

        default:
            break;
        }
    }
    else if (line == 2)
    {
        switch (param_three)
        {
        case 1:
            if (cmd == UP)
            {
                if (fpower[0] == '9')
                {
                    fpower[0] = '0';
                }
                else
                {

                    fpower[0]++;
                }
            }

            else
            {
                if (fpower[0] == '0')
                {
                    fpower[0] = '9';
                }
                else
                {
                    fpower[0]--;
                }
            }

            screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
            set_cursor(line, 1);
            break;
        case 2:
            if (cmd == UP)
            {
                if (fpower[1] == '9')
                {
                    fpower[1] = '0';
                }
                else
                {

                    fpower[1]++;
                }
            }

            else
            {
                if (fpower[1] == '0')
                {
                    fpower[1] = '9';
                }
                else
                {
                    fpower[1]--;
                }
            }
            screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
            set_cursor(line, 2);
            break;
        case 3:
            if (cmd == UP)
            {
                if (fpower[2] == '9')
                {
                    fpower[2] = '0';
                }
                else
                {
                    fpower[2]++;
                }
            }

            else
            {
                if (fpower[2] == '0')
                {
                    fpower[2] = '9';
                }
                else
                {
                    fpower[2]--;
                }
            }
            screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
            set_cursor(line, 3);
            break;
        case 4:
            if (cmd == UP)
            {
                if (fpower[3] == '9')
                {
                    fpower[3] = '0';
                }
                else
                {
                    fpower[3]++;
                }
            }

            else
            {
                if (fpower[3] == '0')
                {
                    fpower[3] = '9';
                }
                else
                {
                    fpower[3]--;
                }
            }
            screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
            set_cursor(line, 4);
            break;
        case 5:
            if (cmd == UP)
            {
                if (fpower[4] == '9')
                {
                    fpower[4] = '0';
                }
                else
                {
                    fpower[4]++;
                }
            }

            else
            {
                if (fpower[4] == '0')
                {
                    fpower[4] = '9';
                }
                else
                {
                    fpower[4]--;
                }
            }
            screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
            set_cursor(line, 5);
            break;
        case 6:
            if (cmd == UP)
            {
                if (pwm_auto == 100)
                {
                    pwm_auto = 0;
                }
                else
                {

                    pwm_auto++;
                }
            }

            else
            {
                if (pwm_auto == 0)
                {
                    pwm_auto = 100;
                }
                else
                {

                    pwm_auto--;
                }
            }

            screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
            set_cursor(line, 14);
            break;
        default:
            break;
        }
    }
    else if (line == 3) // estoy en contraste
    {
        if (cmd == UP)
        {
            if (contrast == 16)
            {
                contrast = 1;
            }
            else
            {
                contrast++;
            }
        }

        else
        {
            if (contrast == 1)
            {
                contrast = 16;
            }
            else
            {
                contrast--;
            }
        }
        set_contrast(contrast);

        screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
        set_cursor(line, 15);
    }
    else if (line == 1)
    {
        switch (param_three)
        {
        case 1:
            if (diabool == true)
            {
                diabool = false;
            }
            else
            {
                diabool = true;
            }
            screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto); // escribo la linea para que no quede en blanco
            set_cursor(line, 0);
            break;
        case 2:
            if (cmd == UP)
            {
                if (time_pwmi.tm_hour == 23)
                {
                    time_pwmi.tm_hour = 0;
                }
                else
                {
                    time_pwmi.tm_hour += 1;
                }
                mktime(&time_pwmi);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);
                set_cursor(1, 6);
            }
            else
            {
                if (time_pwmi.tm_hour == 0)
                {
                    time_pwmi.tm_hour = 23;
                }
                else
                {
                    time_pwmi.tm_hour -= 1;
                }
                mktime(&time_pwmi);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);
                set_cursor(1, 6);
            }

            break;
        case 3:
            if (cmd == UP)
            {
                if (time_pwmi.tm_min == 59)
                {
                    time_pwmi.tm_min = 0;
                }
                else
                {
                    time_pwmi.tm_min += 1;
                }
                mktime(&time_pwmi);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);
                set_cursor(1, 9);
            }
            else
            {
                if (time_pwmi.tm_min == 0)
                {
                    time_pwmi.tm_min = 59;
                }
                else
                {
                    time_pwmi.tm_min -= 1;
                }
                mktime(&time_pwmi);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);
                set_cursor(1, 9);
            }
            break;
        case 4:
            if (cmd == UP)
            {
                if (time_pwmf.tm_hour == 23)
                {
                    time_pwmf.tm_hour = 0;
                }
                else
                {
                    time_pwmf.tm_hour += 1;
                }
                mktime(&time_pwmf);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);
                set_cursor(1, 12);
            }
            else
            {
                if (time_pwmf.tm_hour == 0)
                {
                    time_pwmf.tm_hour = 23;
                }
                else
                {
                    time_pwmf.tm_hour -= 1;
                }
                mktime(&time_pwmf);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);
                set_cursor(1, 12);
            }
            break;
        case 5:
            if (cmd == UP)
            {
                if (time_pwmf.tm_min == 59)
                {
                    time_pwmf.tm_min = 0;
                }
                else
                {
                    time_pwmf.tm_min += 1;
                }
                mktime(&time_pwmf);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);
                set_cursor(1, 15);
            }
            else
            {
                if (time_pwmf.tm_min == 0)
                {
                    time_pwmf.tm_min = 59;
                }
                else
                {
                    time_pwmf.tm_min -= 1;
                }
                mktime(&time_pwmf);
                screen_three_line(line, time_device, time_pwmi, time_pwmf, fpower, diabool, modobool, contrast, pwm_auto);
                set_cursor(1, 15);
            }
            break;
        default:
            break;
        }
    }
    return ESP_OK;
}

esp_err_t save_params() // el/los parametros los tengo que salvar cuando vuelvo a la linea titilante o salgo del config.
{
    switch (screen)
    {
    /*case SCREEN_ONE:
        // set dia
        ESP_LOGI("Display_manager", "Guardo los parametros de la pantalla 1");
        if (diabool == false)
        {
            dia = SIMUL_DAY_OFF;
        }
        else
        {
            dia = SIMUL_DAY_ON;
        }
        global_manager_set_simul_day_status(dia);
        // set modo
        if (modobool == false)
        {
            modo = PWM_MANUAL;
        }
        else
        {
            modo = PWM_AUTOMATIC;
        }
        global_manager_set_pwm_mode(modo);
        // set horario device
        current_time_manager_set_current_time(time_device);
        break;*/
    case SCREEN_TWO:
        ESP_LOGI("Display_manager", "Guardo los parametros de la pantalla 2");
        // set horario 1 final e inicial
        global_manager_set_s_out_turn_on_time(time_i1, 0);
        global_manager_set_s_out_turn_off_time(time_f1, 0);
        // set horario 2 final e inicial
        global_manager_set_s_out_turn_on_time(time_i2, 1);
        global_manager_set_s_out_turn_off_time(time_f2, 1);
        // set horario 3 final e inicial
        global_manager_set_s_out_turn_on_time(time_i3, 2);
        global_manager_set_s_out_turn_off_time(time_f3, 2);
        // set horario 4 final e inicial
        global_manager_set_s_out_turn_on_time(time_i4, 3);
        global_manager_set_s_out_turn_off_time(time_f4, 3);
        time_i1_aux = time_i1;
        time_f1_aux = time_f1;
        time_i2_aux = time_i2;
        time_f2_aux = time_f2;
        time_i3_aux = time_i3;
        time_f3_aux = time_f3;
        time_i4_aux = time_i4;
        time_f4_aux = time_f4;

        break;
    case SCREEN_THREE:
        ESP_LOGI("Display_manager", "Guardo los parametros de la pantalla 3");

        // set horario final e inicial de pwm
        global_manager_set_turn_on_time(time_pwmi);
        printf("La hora de inicio del pwm guardada es %u : %u \n", time_pwmi.tm_hour, time_pwmi.tm_min);
        global_manager_set_turn_off_time(time_pwmf);
        // set horario device
        current_time_manager_set_current_time(time_device);
        global_manager_set_display_contrast(contrast);
        // set dia
        if (diabool == false)
        {
            dia = SIMUL_DAY_OFF;
        }
        else
        {
            dia = SIMUL_DAY_ON;
        }
        global_manager_set_simul_day_status(dia);
        // set modo
        if (modobool == false)
        {
            modo = PWM_MANUAL;
        }
        else
        {
            modo = PWM_AUTOMATIC;
        }
        global_manager_set_pwm_mode(modo);
        // set potencia total
        fpowerppf = atoi(fpower);
        global_manager_set_ppf(fpowerppf);
        printf("Valor convertido: %lu\n", fpowerppf);
        global_manager_set_automatic_pwm_power(pwm_auto); ///////////// SET porcentaje de pwm automatico

        break;

    default:
        break;
    }
    // GET CONTRAST

    return ESP_OK;
}

esp_err_t get_params()
{
    ESP_LOGI("Display_manager", "Obtengo todos los parametros");
    // obtengo horario del dispositivo
    global_manager_get_current_time_info(&time_device);
    printf("La hora del dispositivo es %u \n", time_device.tm_hour);
    printf("Los minutos del dispositivo es %u \n", time_device.tm_min);
    // obtengo los 4 horarios de s.out
    global_manager_get_s_out_turn_on_time(&time_i1, 0);
    global_manager_get_s_out_turn_off_time(&time_f1, 0);
    //
    global_manager_get_s_out_turn_on_time(&time_i2, 1);
    global_manager_get_s_out_turn_off_time(&time_f2, 1);
    //
    global_manager_get_s_out_turn_on_time(&time_i3, 2);
    global_manager_get_s_out_turn_off_time(&time_f3, 2);
    //
    global_manager_get_s_out_turn_on_time(&time_i4, 3);
    global_manager_get_s_out_turn_off_time(&time_f4, 3);
    time_i1_aux = time_i1;
    time_f1_aux = time_f1;
    time_i2_aux = time_i2;
    time_f2_aux = time_f2;
    time_i3_aux = time_i3;
    time_f3_aux = time_f3;
    time_i4_aux = time_i4;
    time_f4_aux = time_f4;

    // obtengo horario de pwm
    global_manager_get_turn_on_time(&time_pwmi);
    printf("La hora de inicio de PWM es %u:%u \n", time_pwmi.tm_hour, time_pwmi.tm_min);
    global_manager_get_turn_off_time(&time_pwmf);
    printf("La hora de off de PWM es %u:%u \n", time_pwmf.tm_hour, time_pwmf.tm_min);

    // obtengo potencia total
    global_manager_get_ppf(&fpowerppf);
    printf("La potencia total es %d \n", (int)fpowerppf);
    sprintf(fpower, "%05d", (int)fpowerppf);
    // obtengo potencia porcentaje

    if (is_jp3_teclas_connected() == true)
    {
        global_manager_get_pwm_digital_percentage(&pwm_man);
    }
    else
    {
        global_manager_get_pwm_analog_percentage(&pwm_man);
        printf("La potencia es %u \n", power);
    }
    // obtengo el porcentaje de automatico
    global_manager_get_automatic_pwm_power(&pwm_auto);

    // obtengo vegeflora
    global_manager_get_flora_vege_status(&vegeflora);
    if (vegeflora == FLORA_VEGE_OUTPUT_DISABLE)
    {
        vegeflorachar = 'V';
    }
    else
    {
        vegeflorachar = 'F';
    }
    // obtengo dia
    global_manager_get_simul_day_status(&dia);
    if (dia == SIMUL_DAY_OFF)
    {
        diabool = pdFALSE;
    }
    else
    {
        diabool = pdTRUE;
    }
    // obtengo modo
    global_manager_get_pwm_mode(&modo);
    if (modo == PWM_MANUAL)
    {
        printf("MODO MANUAL \n");
        modobool = false;
    }
    else
    {
        printf("MODO AUTOMATIC \n");
        modobool = true;
    }

    // GET CONTRAST
    global_manager_get_display_contrast(&contrast);
    return ESP_OK;
}

compare_t compare_times(struct tm t1, struct tm t2)
{
    if (t1.tm_hour > t2.tm_hour) // hora del 1 mayor a la del 2
    {
        return GREATER; // t1 es mayor a t2 en horas
    }
    if (t1.tm_hour == t2.tm_hour)
    {
        if (t1.tm_min > t2.tm_min)
        {
            return GREATER; // t1 mayor a t2 en minutos
        }
        if (t1.tm_min < t2.tm_min) // los minutos de t1 son menores a los de t2
        {
            return LESSER;
        }
        if (t1.tm_min == t2.tm_min)
        {
            return EQUAL; // los horarios son iguales
        }
    }
    return LESSER;
}

uint8_t colision_times(struct tm ih1, struct tm fh1, struct tm ih2, struct tm fh2) // funcion que me dice si dos horarios se superponen
{
    uint8_t sp = 0;

    ESP_LOGI("Colision_times", "Hora inicial 1 es: %u :%u", ih1.tm_hour, ih1.tm_min);
    ESP_LOGI("Colision_times", "Hora final 1 es: %u :%u", fh1.tm_hour, fh1.tm_min);
    ESP_LOGI("Colision_times", "Hora inicial 2 es: %u :%u", ih2.tm_hour, ih2.tm_min);
    ESP_LOGI("Colision_times", "Hora final 2 es: %u :%u", fh2.tm_hour, fh2.tm_min);

    // caso que horario de encendido sea igual a apagado en h1
    if (ih1.tm_hour == fh1.tm_hour && ih1.tm_min == fh1.tm_min)
    {
        if (ih1.tm_hour == 0 && ih1.tm_min == 0)
        {
            sp = 0;
        }
        else
        {
            sp = 1;
        }
    }
    // caso que horario de encendido sea igual a apagado en h2
    if (ih2.tm_hour == fh2.tm_hour && ih2.tm_min == fh2.tm_min)
    {
        if (ih2.tm_hour == 0 && ih2.tm_min == 0)
        {
            sp = 0;
        }
        else
        {
            sp = 1;
        }
    }
    if (ih1.tm_hour == fh1.tm_hour && ih1.tm_min > fh1.tm_min)
    {
        if (ih2.tm_hour > ih1.tm_hour)
        {
            sp = 1;
        }
        if (ih2.tm_hour == ih1.tm_hour)
        {
            if (ih2.tm_min >= ih1.tm_min || fh2.tm_min > ih1.tm_min)
            {
                sp = 1;
            }
        }
    }
    if (ih2.tm_hour == fh2.tm_hour && ih2.tm_min > fh2.tm_min)
    {
        if (ih1.tm_hour > ih2.tm_hour)
        {
            sp = 1;
        }
        if (ih1.tm_hour == ih2.tm_hour)
        {
            if (ih1.tm_min >= ih2.tm_min || fh1.tm_min > ih2.tm_min)
            {
                sp = 1;
            }
        }
    }
    // caso superposicion en una parte de h2 en h1
    if (ih1.tm_hour <= ih2.tm_hour && ih2.tm_hour <= fh1.tm_hour)
    { // aca falta tener en cuenta los minutos
        if (ih1.tm_hour == 0 && ih1.tm_min == 0)
        {
            sp = 0;
        }
        else if (ih2.tm_hour == fh1.tm_hour && ih2.tm_min >= fh1.tm_min)
        {
            sp = 0;
        }
        else
        {
            sp = 1;
        }
    }
    // caso superposicion en una parte de h1 en h2
    if (ih2.tm_hour <= ih1.tm_hour && ih1.tm_hour <= fh2.tm_hour)
    { // aca falta tener en cuenta los minutos
        if (ih1.tm_hour == 0 && ih1.tm_min == 0)
        {
            sp = 0;
        }
        else if (ih1.tm_hour == fh2.tm_hour && ih1.tm_min >= fh2.tm_min)
        {
            sp = 0;
        }
        else
        {
            sp = 1;
        }
    }
    // caso en que h2 se superponga con misma hora y entre minutos en h1. mismo dia
    if (ih1.tm_hour == fh1.tm_hour && ih2.tm_hour == ih1.tm_hour)
    {
        if (ih2.tm_min >= ih1.tm_min && fh1.tm_min > ih2.tm_min)
        {
            sp = 1;
        }
    }
    // caso en que h1 se superponga con misma hora y entre minutos en h2. mismo dia
    if (ih2.tm_hour == fh2.tm_hour && ih1.tm_hour == ih2.tm_hour)
    {
        if (ih1.tm_min >= ih2.tm_min && fh2.tm_min > ih1.tm_min)
        {
            sp = 1;
        }
    }
    // caso de que se pase al dia siguiente
    if (ih1.tm_hour > fh1.tm_hour)
    {
        if (ih2.tm_hour >= ih1.tm_hour)
        {
            sp = 1;
        }
        if (fh1.tm_hour > ih2.tm_hour)
        {
            sp = 1;
        }
    }
    if (ih2.tm_hour > fh2.tm_hour)
    {
        if (ih1.tm_hour >= ih2.tm_hour)
        {
            sp = 1;
        }
        if (fh2.tm_hour > ih1.tm_hour)
        {
            sp = 1;
        }
    }

    ESP_LOGE("Colision_times", "SP es: %u", sp);
    return sp;
}

uint8_t checkOverlap(struct tm ih1, struct tm fh1, struct tm ih2, struct tm fh2, struct tm ih3, struct tm fh3, struct tm ih4, struct tm fh4)
{
    uint8_t arr[] = {0, 0, 0, 0, 0, 0};
    uint8_t suma = 0;
    uint8_t i = 0;

    arr[0] = colision_times(ih1, fh1, ih2, fh2);

    arr[1] = colision_times(ih1, fh1, ih3, fh3);

    arr[2] = colision_times(ih1, fh1, ih4, fh4);

    arr[3] = colision_times(ih2, fh2, ih3, fh3);

    arr[4] = colision_times(ih2, fh2, ih4, fh4);

    arr[5] = colision_times(ih3, fh3, ih4, fh4);

    for (i = 0; i < 6; i++)
    {
        suma += arr[i];
    }
    return suma;
}

void set_screen_one_from_web()
{
    get_params();
    if (modobool == false) //el modo es manual
    {
        power = pwm_man;
    }
    else //el modo es automatico
    {
        if (compare_times(time_pwmi, time_device) == GREATER) // el horario de inicio del pwm es mayor  que el del equipo, no está la  salida prendida
        {
            power = 0;
        }
        else if (compare_times(time_pwmi, time_device) == EQUAL) // los horarios son  iguales, ya empieza el pwm pro ende muestro la potencia
        {
            power = pwm_dia_rise;
        }
        else // horairo de inicio de pwm menor al del equipo
        {
            if (compare_times(time_pwmf, time_device) == GREATER) // si el horario final es superior al del equipo, está activa la salida del pwm
            {
                if (fade_stat == FADING_IN_PROGRESS)
                {
                    power = pwm_dia_rise;
                }
                else
                {
                    power = pwm_auto;
                }
            }
            else if (compare_times(time_pwmf, time_device) == EQUAL) // horario final iguala l del dispositivo, ya se acaba asique muestro cero
            {

                power = 0;
            }
            else // horario final del dispositivo menor al del equipo, la salida pwm esta apagada
            {
                if (time_pwmi.tm_hour > time_pwmf.tm_hour)
                {
                    power = pwm_auto;
                }
                else
                {
                    power = 0;
                }
            }
        }
    }
    display_set_screen_one(&screen, fpower, power, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
    ESP_LOGI(TAG, "Pantalla %u", screen);
}

void set_screen_two_from_web()
{
    get_params();

    display_set_screen_one(&screen, fpower, power, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
    ESP_LOGI(TAG, "Pantalla %u", screen);
}

//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------
