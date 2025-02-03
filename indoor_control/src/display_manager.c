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
char fpower[6];
uint32_t fpowerppf;
flora_vege_status_t vegeflora;
char vegeflorachar; // con esta me manejo en el display
simul_day_status_t dia;
bool diabool; // con este me manejo en e display
pwm_mode_t modo;
bool modobool;    // con esta me manejo en el display
uint8_t contrast; // contraste del display
uint8_t pwm_auto;
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
    start_timerh();
    contrast = 10;
    pwm_auto = 0;
    // aca asgianar valores a todas las variables globales del display

    /*time_device.tm_hour = 12;
    time_device.tm_min = 35;
    time_i1.tm_hour = 12;
    time_i1.tm_min = 35;
    time_f1.tm_hour = 12;
    time_f1.tm_min = 35;
    time_i2.tm_hour = 12;
    time_i2.tm_min = 35;
    time_f2.tm_hour = 12;
    time_f2.tm_min = 35;
    time_i3.tm_hour = 12;
    time_i3.tm_min = 35;
    time_f3.tm_hour = 12;
    time_f3.tm_min = 35;
    time_i4.tm_hour = 12;
    time_i4.tm_min = 35;
    time_f4.tm_hour = 12;
    time_f4.tm_min = 35;
    time_pwmi.tm_hour = 12;
    time_pwmi.tm_min = 35;
    time_pwmf.tm_hour = 12;
    time_pwmf.tm_min = 35;
    power = 45;
    strcpy(fpower, "10000");
    vegeflora = 1;
    dia = pdTRUE;
    modo = pdTRUE;*/
    param_two = 1;
    param_three = 1;
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
                    {
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
                    state = CONFIG_LINE;
                    // dejo de blinkear el caracter
                    save_params();
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
                    display_set_screen_one(&screen, fpower, power, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
                    ESP_LOGI(TAG, "Pantalla %u", screen);

                    break;
                case CONFIG_PARAM:
                    // vuelvo a NORMAL a la pagina correspondiente
                    save_params();
                    state = NORMAL;
                    stop_timer();
                    // vuelvo a la pantalla 1
                    // dejo de blinkear el caracter
                    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_OFF);
                    global_manager_get_current_time_info(&time_device);
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
                    ESP_LOGI("CONFIG_LINE", "line es %u", line);
                    param_two = 1;
                    param_three = 1;
                    display_blink_manager(screen, 1); // 0 es down

                    break;
                case CONFIG_PARAM:
                    // bajo numero a configurar
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
                    printf("El char de vege_flora es %c", vegeflorachar);
                    global_manager_get_current_time_info(&time_device);
                    display_set_screen_one(&screen, fpower, display_ev.pwm_value, vegeflorachar, diabool, modobool, time_device, time_pwmi, time_pwmf);
                    break;
                case CONFIG_LINE:
                    stop_timer();
                    param_two = 1;
                    param_three = 1;
                    display_blink_manager(screen, 0); // 1 es up

                    break;
                case CONFIG_PARAM:
                    // subo numero a configurar
                    display_param_manager(UP);

                    break;

                default:
                    break;
                }
                break;
            case PWM_MANUAL_VALUE:
                get_params();
                display_set_power(display_ev.pwm_value, fpower);
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
    else if (line == 3)
    {
        set_cursor(line, 15);
    }
    else if (line == 1)
    {
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

        break;
    case SCREEN_THREE:
        ESP_LOGI("Display_manager", "Guardo los parametros de la pantalla 3");

        // set horario final e inicial de pwm
        global_manager_set_turn_on_time(time_pwmi);
        global_manager_set_turn_off_time(time_pwmf);
        // set horario device
        current_time_manager_set_current_time(time_device);
        // set contrast (falta funcion)
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
        global_manager_set_automatic_pwm_power(pwm_auto);  ///////////// SET porcentaje de pwm automatico

        break;

    default:
        break;
    }
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
        global_manager_get_pwm_digital_percentage(&power);
    }
    else
    {
        global_manager_get_pwm_analog_percentage(&power);
        printf("La potencia es %u \n", power);
    }

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
        modobool = false;
    }
    else
    {
        modobool = true;
    }
     global_manager_get_automatic_pwm_power(&pwm_auto);
    // FALTA GET CONTRAST
    return ESP_OK;
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------
