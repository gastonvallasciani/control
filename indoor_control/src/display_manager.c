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
int interval = 600; // ms de timer
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
char fpower[6];
char vegeflora;
bool dia;
bool modo;

uint8_t param_one;
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

    // aca asgianar valores a todas las variables globales del display

    time_device.tm_hour = 12;
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
    power = 78;
    strcpy(fpower, "00998");
    vegeflora = 'V';
    dia = pdTRUE;
    modo = pdTRUE;
    param_one = 1;
    param_two = 1;
    param_three = 1;
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

                        display_set_screen_one(&screen, power, vegeflora, dia, modo, time_device);
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
                    save_param();
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
                    // aca tengo que entrar a la primera linea titilante en la pantanlla en la que este
                    state = CONFIG_LINE;
                    line = 0;
                    param_one = 1;
                    param_two = 1;
                    param_three = 1;
                    display_blink_manager(screen, 3); // con esta veo que pantalla estoy

                    break;
                case CONFIG_LINE:
                    // vuelvo a NORMAL a la pagina correspondiente
                    state = NORMAL;
                    stop_timer();
                    if (screen == SCREEN_ONE)
                    {
                        display_set_screen_one(&screen, power, vegeflora, dia, modo, time_device);
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
                    save_param();
                    state = NORMAL;
                    stop_timer();
                    if (screen == SCREEN_ONE)
                    {
                        display_set_screen_one(&screen, power, vegeflora, dia, modo, time_device);
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
                    ESP_LOGI("CONFIG_LINE", "Muevo al parametro siguiente");
                    display_param_manager(VF);
                    break;

                default:
                    break;
                }
                break;
            case DOWN:
                switch (state)
                {
                case NORMAL:
                    power--;
                    display_set_power(display_ev.pwm_value, display_ev.vege_flora);
                    break;
                case CONFIG_LINE:
                    stop_timer();
                    ESP_LOGI("CONFIG_LINE", "line es %u", line);
                    param_one = 1;
                    param_two = 1;
                    param_three = 1;
                    display_blink_manager(screen, 0); // 0 es down

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
                    power++;
                    display_set_power(display_ev.pwm_value, display_ev.vege_flora);
                    break;
                case CONFIG_LINE:
                    stop_timer();
                    param_one = 1;
                    param_two = 1;
                    param_three = 1;
                    display_blink_manager(screen, 1); // 1 es up

                    break;
                case CONFIG_PARAM:
                    ESP_LOGI("UP-CONFIG_PARAM", "Entro al config_param del UP");
                    ESP_LOGI("UP-CONFIG_PARAM", "param_two es %u", param_two);
                    // subo numero a configurar
                    display_param_manager(UP);

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
void display_manager_start(uint8_t pwm_value, char vege_flora)
{
    display_event_t display_ev;

    display_ev.cmd = START_DISPLAY;
    display_ev.pwm_value = power;
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
    // display_ev.pwm_value = power;
    // display_ev.vege_flora = vege_flora;

    xQueueSend(display_manager_queue, &display_ev, 10);
}

esp_err_t display_blink_manager(screen_t screen, uint8_t cmd)
{
    switch (screen)
    {
    case SCREEN_ONE:
        // en esta pantalla solo se modifica la ultima linea
        display_set_screen_one(&screen, power, vegeflora, dia, modo, time_device);
        line = 3;
        start_timer();
        break;
    case SCREEN_TWO:
        // chequeo si vino up o down
        display_set_screen_two(&screen, time_i1, time_i2, time_i3, time_i4, time_f1, time_f2, time_f3, time_f4);
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
            ESP_LOGI("CONFIG_LINE", "line es %u", line);
            start_timer();
        }

        break;
    case SCREEN_THREE:
        display_set_screen_three(&screen, time_pwmi, time_pwmf, fpower);
        if (cmd == 0) // es down
        {
            if (line == 0)
            {
                line = 1; // voy a la ultima linea
            }
            else if (line == 1)
            {
                line = 0;
            }
        }
        else if (cmd == 1) // es up
        {
            if (line == 0)
            {
                line = 1; // voy a la primera linea
            }
            else if (line == 1)
            {
                line = 0;
            }
        }
        start_timer();
        break;

    default:
        break;
    }

    return ESP_OK;
}

esp_err_t clear_line(uint8_t linee)
{
    ESP_LOGI("TIMER", "Entro a clear_line");
    char *clearr = "                ";
    set_cursor(linee, 0);
    display_write_string(clearr);
    ESP_LOGI("TIMER", "salgo de clear_line");
    return ESP_OK;
}

void blink_callback(TimerHandle_t timer)
{
    ESP_LOGI("TIMER", "Entro al callback");
    if (clear == pdFALSE)
    {
        ESP_LOGI("TIMER", "Entro al clear del callback");
        clear_line(line);
        clear = pdTRUE;
        ESP_LOGI("TIMER", "Salgo del clear del callback");
    }
    else
    {
        switch (screen)
        {
            ESP_LOGI("TIMER", "Entro al switch del callback");
        case SCREEN_ONE:
            screen_one_line_three(time_device, dia, modo);
            clear = pdFALSE;
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
            clear = pdFALSE;
            break;
        case SCREEN_THREE:
            screen_three_line(line, fpower, time_pwmi, time_pwmf);
            clear = pdFALSE;
            break;
        default:
            break;
        }
        ESP_LOGI("TIMER", "Salgo del switch del callback");
    }
    ESP_LOGI("TIMER", "Salgo del callback");
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

esp_err_t display_param_manager(display_event_cmds_t cmd)
{
    switch (screen)
    {
    case SCREEN_ONE:
        display_set_screen_one(&screen, power, vegeflora, dia, modo, time_device);
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
        break;
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
        display_set_screen_three(&screen, time_pwmi, time_pwmf, fpower);
        if (cmd == VF || cmd == AUX)
        {
            screen_three_param(cmd);
        }
        else if (cmd == UP)
        {
            ESP_LOGI("PARAM_MANAGER", "Entro a param_modified_three");
            param_modified_three(UP);
            ESP_LOGI("PARAM_MANAGER", "Salgo de param_modified_three");
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

esp_err_t screen_one_param(display_event_cmds_t cmd)
{

    screen_one_line_three(time_device, dia, modo); // escribo linea para que no quede vacia
    switch (param_one)
    {
    case 1:
        set_cursor(3, 4);
        break;

    case 2:
        set_cursor(3, 7);
        break;

    case 3:
        set_cursor(3, 11);
        break;

    case 4:
        set_cursor(3, 12);
        break;

    case 5:
        set_cursor(3, 14);
        break;

    case 6:
        set_cursor(3, 15);
        break;

    default:
        break;
    }
    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_ON);
    if (cmd == VF)
    {
        if (param_one == 6)
        {
            param_one = 1;
        }
        else
        {
            param_one++;
        }
    }

    return ESP_OK;
}

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

    switch (param_two) // me fijo que parametro modifico
    {
    case 1:
        set_cursor(line, 4);
        break;
    case 2:
        set_cursor(line, 5);
        break;
    case 3:
        set_cursor(line, 7);
        break;
    case 4:
        set_cursor(line, 8);
        break;
    case 5:
        set_cursor(line, 11);
        break;
    case 6:
        set_cursor(line, 12);
        break;
    case 7:
        set_cursor(line, 14);
        break;
    case 8:
        set_cursor(line, 15);
        break;

    default:
        break;
    }
    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_ON);
    if (cmd == VF)
    {
        if (param_two == 8)
        {
            param_two = 1;
        }
        else
        {
            param_two++;
        }
    }
    return ESP_OK;
}

esp_err_t screen_three_param(display_event_cmds_t cmd)
{
    screen_three_line(line, fpower, time_pwmi, time_pwmf);
    if (line == 0)
    {
        switch (param_three) // me fijo que parametro modifico
        {
        case 1:
            set_cursor(line, 4);
            break;
        case 2:
            set_cursor(line, 5);
            break;
        case 3:
            set_cursor(line, 7);
            break;
        case 4:
            set_cursor(line, 8);
            break;
        case 5:
            set_cursor(line, 11);
            break;
        case 6:
            set_cursor(line, 12);
            break;
        case 7:
            set_cursor(line, 14);
            break;
        case 8:
            set_cursor(line, 15);
            break;

        default:
            break;
        }
        if (cmd == VF)
        {
            if (param_three == 8)
            {
                param_three = 1;
            }
            else
            {
                param_three++;
            }
        }
    }
    else // line == 1
    {
        switch (param_three) // me fijo que parametro modifico
        {
        case 1:
            set_cursor(line, 10);
            break;
        case 2:
            set_cursor(line, 11);
            break;
        case 3:
            set_cursor(line, 12);
            break;
        case 4:
            set_cursor(line, 13);
            break;
        case 5:
            set_cursor(line, 14);
            break;

        default:
            break;
        }
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
    }

    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_ON);

    return ESP_OK;
}

esp_err_t param_modified_one(display_event_cmds_t cmd)
{
    ESP_LOGI("param_modified_one", "Param_one vale %u", param_one);
    if (param_one == 1)
    {
        if (dia == pdTRUE)
        {
            dia = pdFALSE;
        }
        else
        {
            dia = pdTRUE;
        }
        // aca debo guardarla en la funcion de gaston
        screen_one_line_three(time_device, dia, modo);
        set_cursor(3, 4);
    }
    if (param_one == 2)
    {
        if (modo == pdTRUE)
        {
            modo = pdFALSE;
        }
        else
        {
            modo = pdTRUE;
        }
        // aca debo guardarla en la funcion de gaston
        screen_one_line_three(time_device, dia, modo);
        set_cursor(3, 7);
    }
    if (param_one == 3)
    {
        if (cmd == UP)
        {
            time_device.tm_hour += 10;
            mktime(&time_device);
            screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 11);
        }
        else
        {
            time_device.tm_hour -= 10;
            mktime(&time_device);
            screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 11);
        }
    }
    if (param_one == 4)
    {
        if (cmd == UP)
        {
            time_device.tm_hour += 1;
            mktime(&time_device);
            screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 12);
        }
        else
        {
            time_device.tm_hour -= 1;
            mktime(&time_device);
            screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 12);
        }
    }
    if (param_one == 5)
    {
        if (cmd == UP)
        {
            time_device.tm_min += 10;
            mktime(&time_device);
            screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 14);
        }
        else
        {
            time_device.tm_min -= 10;
            mktime(&time_device);
            screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 14);
        }
    }
    if (param_one == 6)
    {
        if (cmd == UP)
        {
            time_device.tm_min += 1;
            mktime(&time_device);
            screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 15);
        }
        else
        {
            time_device.tm_min -= 1;
            mktime(&time_device);
            screen_one_line_three(time_device, dia, modo);
            ESP_LOGI("time", "la hora vale %u", time_device.tm_hour);
            ESP_LOGI("time", "los minutos valen %u", time_device.tm_min);
            set_cursor(3, 15);
        }
    }

    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_ON);
    return ESP_OK;
}

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
            time_i->tm_hour += 10;
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);

            set_cursor(line, 4);
        }
        else
        {
            time_i->tm_hour -= 10;
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 4);
        }
    }
    if (param_two == 2)
    {
        if (cmd == UP)
        {
            time_i->tm_hour += 1;
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);

            set_cursor(line, 5);
        }
        else
        {
            time_i->tm_hour -= 1;
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 5);
        }
    }
    if (param_two == 3)
    {
        if (cmd == UP)
        {
            time_i->tm_min += 10;
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 7);
        }
        else
        {
            time_i->tm_min -= 10;
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 7);
        }
    }
    if (param_two == 4)
    {
        if (cmd == UP)
        {
            time_i->tm_min += 1;
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);

            set_cursor(line, 8);
        }
        else
        {
            time_i->tm_min -= 1;
            mktime(time_i);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 8);
        }
    }
    if (param_two == 5)
    {
        if (cmd == UP)
        {
            time_f->tm_hour += 10;
            mktime(time_f);
            screen_two_line(line, *time_i, *time_f);

            set_cursor(line, 11);
        }
        else
        {
            time_f->tm_hour -= 10;
            mktime(time_f);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 11);
        }
    }
    if (param_two == 6)
    {
        if (cmd == UP)
        {
            time_f->tm_hour += 1;
            mktime(time_f);
            screen_two_line(line, *time_i, *time_f);

            set_cursor(line, 12);
        }
        else
        {
            time_f->tm_hour -= 1;
            mktime(time_f);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 12);
        }
    }
    if (param_two == 7)
    {
        if (cmd == UP)
        {
            time_f->tm_min += 10;
            mktime(time_f);
            screen_two_line(line, *time_i, *time_f);

            set_cursor(line, 14);
        }
        else
        {
            time_f->tm_min -= 10;
            mktime(time_f);
            screen_two_line(line, *time_i, *time_f);
            set_cursor(line, 14);
        }
    }
    if (param_two == 8)
    {
        if (cmd == UP)
        {
            time_f->tm_min += 1;
            mktime(time_f);
            screen_two_line(line, *time_i, *time_f);

            set_cursor(line, 15);
        }
        else
        {
            time_f->tm_min -= 1;
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
    if (line == 0)
    {
        switch (param_three)
        {
        case 1:
            if (cmd == UP)
            {
                time_pwmi.tm_hour += 10;
                mktime(&time_pwmi);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 4);
            }
            else
            {
                time_pwmi.tm_hour -= 10;
                mktime(&time_pwmi);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 4);
            }

            break;
        case 2:
            if (cmd == UP)
            {
                time_pwmi.tm_hour += 1;
                mktime(&time_pwmi);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 5);
            }
            else
            {
                time_pwmi.tm_hour -= 1;
                mktime(&time_pwmi);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 5);
            }
            break;
        case 3:
            if (cmd == UP)
            {
                time_pwmi.tm_min += 10;
                mktime(&time_pwmi);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 7);
            }
            else
            {
                time_pwmi.tm_min -= 10;
                mktime(&time_pwmi);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 7);
            }

            break;
        case 4:
            if (cmd == UP)
            {
                time_pwmi.tm_min += 1;
                mktime(&time_pwmi);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 8);
            }
            else
            {
                time_pwmi.tm_min -= 1;
                mktime(&time_pwmi);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 8);
            }
            break;
        case 5:
            if (cmd == UP)
            {
                time_pwmf.tm_hour += 10;
                mktime(&time_pwmf);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 11);
            }
            else
            {
                time_pwmf.tm_hour -= 10;
                mktime(&time_pwmf);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 11);
            }
            break;
        case 6:
            if (cmd == UP)
            {
                time_pwmf.tm_hour += 1;
                mktime(&time_pwmf);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 12);
            }
            else
            {
                time_pwmf.tm_hour -= 1;
                mktime(&time_pwmf);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 12);
            }
            break;
        case 7:
            if (cmd == UP)
            {
                time_pwmf.tm_min += 10;
                mktime(&time_pwmf);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 14);
            }
            else
            {
                time_pwmf.tm_min -= 10;
                mktime(&time_pwmf);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 14);
            }
            break;
        case 8:
            if (cmd == UP)
            {
                time_pwmf.tm_min += 1;
                mktime(&time_pwmf);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 15);
            }
            else
            {
                time_pwmf.tm_min -= 1;
                mktime(&time_pwmf);
                screen_three_line(line, fpower, time_pwmi, time_pwmf);
                set_cursor(1, 15);
            }
            break;

        default:
            break;
        }
    }
    else
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
                    ESP_LOGI("param_modified_three", "Entre en el ++");
                    fpower[0]++;
                    ESP_LOGI("param_modified_three", "Sali del ++");
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
                    ESP_LOGI("param_modified_three", "Entre en el --");
                    fpower[0]--;
                }
            }

            screen_three_line(line, fpower, time_pwmi, time_pwmf);
            set_cursor(line, 10);
            break;
        case 2:
            if (cmd == UP)
            {
                if (fpower[0] == '9')
                {
                    fpower[0] = '0';
                }
                else
                {
                    ESP_LOGI("param_modified_three", "Entre en el ++");
                    fpower[0]++;
                    ESP_LOGI("param_modified_three", "Sali del ++");
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
                    ESP_LOGI("param_modified_three", "Entre en el --");
                    fpower[1]--;
                }
            }
            screen_three_line(line, fpower, time_pwmi, time_pwmf);
            set_cursor(line, 11);
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
                    ESP_LOGI("param_modified_three", "Entre en el ++");
                    fpower[2]++;
                    ESP_LOGI("param_modified_three", "Sali del ++");
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
                    ESP_LOGI("param_modified_three", "Entre en el --");
                    fpower[2]--;
                }
            }
            screen_three_line(line, fpower, time_pwmi, time_pwmf);
            set_cursor(line, 12);
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
                    ESP_LOGI("param_modified_three", "Entre en el ++");
                    fpower[3]++;
                    ESP_LOGI("param_modified_three", "Sali del ++");
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
                    ESP_LOGI("param_modified_three", "Entre en el --");
                    fpower[3]--;
                }
            }
            screen_three_line(line, fpower, time_pwmi, time_pwmf);
            set_cursor(line, 13);
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
                    ESP_LOGI("param_modified_three", "Entre en el ++");
                    fpower[4]++;
                    ESP_LOGI("param_modified_three", "Sali del ++");
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
                    ESP_LOGI("param_modified_three", "Entre en el --");
                    fpower[4]--;
                }
            }
            screen_three_line(line, fpower, time_pwmi, time_pwmf);
            set_cursor(line, 14);
            break;

        default:
            break;
        }
    }
    return ESP_OK;
}

esp_err_t save_param() // el/los parametros los tengo que salvar cuando vuelvo a la linea titilante o salgo del config.
{
    switch (screen)
    {
    case SCREEN_ONE:
        // set dia
        // set modo
        // set horario device
        break;
    case SCREEN_TWO:
        if (line == 0)
        {
            // set horario 1 final e inicial
        }
        if (line == 1)
        {
            // set horario 2 final e inicial
        }
        if (line == 2)
        {
            // set horario 3 final e inicial
        }
        if (line == 3)
        {
            // set horario 4 final e inicial
        }
        break;
    case SCREEN_THREE:
        if (line == 0)
        {
            // set horario final e inicial de pwm
        }
        if (line == 1)
        {
            // set potencia total
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------
