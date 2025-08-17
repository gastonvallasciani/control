#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include <esp_wifi.h>
#include "esp_system.h"
// #include "nvs_flash.h"
#include "web_server.h"
#include "cJSON.h"
#include "../include/version.h"
#include "../include/current_time_manager.h"
#include "../include/global_manager.h"
#include "../include/led_manager.h"
#include "../include/dia.h"
#include <time.h>
#include "esp_timer.h"
#include "../include/jumpers_manager.h"
#include "../include/button_manager.h"
#include "../include/display_manager.h"
#include "esp_log.h"

static const char *TAG = "WEBSERVER";
static const char *VERSIONN = "VERSION";
static const char *HORA = "HORA";
static const char *VEGEFLOR = "VEGEFLOR";
static const char *PWM = "PWM";
static const char *TRIAC = "TRIAC";
static const char *MAIN = "MAIN";

static esp_timer_handle_t timer_reset_esp32;
static void timer_reset_esp32_callback(void *arg);
int flag_modo = 0;
//---------------RED-------------------

red_t red; // variable para leer el ssid y pass de la red

//---------------Hora-------------------

struct tm aux_hora; // variable para setear y leer la hora actual

//---------------PWM------------------

// calendar_auto_mode_t pwm_hora; // variable para setear la hora del pwm

// simul_day_status_t dia; // variable para setear la simulacion dia del PWM

// output_mode_t modo_pwm; // variable setear y leer el modo del pwm

struct tm time_pwmi_web; // variable con la info para leer el pwm
struct tm time_pwmf_web; // variable con la info para leer el pwm
pwm_mode_t modo_pwm_web;
simul_day_status_t dia_web;
uint8_t pwm_auto_web;
uint8_t pwm_man_web;

uint8_t auto_pwm_output_status;

//---------------TRIAC------------------

/*output_mode_t modo_triac; // variable para setear y leer el modo del triac

calendar_auto_mode_t triac_h; // variable para setear el modo del triac
*/
struct tm triac_h1f; // variable para setear los horarios del triac
struct tm triac_h1i; // variable para setear los horarios del triac

struct tm triac_h2f; // variable para setear los horarios del triac
struct tm triac_h2i; // variable para setear los horarios del triac

struct tm triac_h3f; // variable para setear los horarios del triac
struct tm triac_h3i; // variable para setear los horarios del triac

struct tm triac_h4f; // variable para setear los horarios del triac
struct tm triac_h4i; // variable para setear los horarios del triac

uint8_t en1 = 0; // 0 apagado, 1 activado
uint8_t en2 = 0;
uint8_t en3 = 0;
uint8_t en4 = 0;

// triac_auto_info_t triac_auto_info; // variable para leer toda la data del triac (y los 4 horarios)

//---------------VEGEFLOR------------------

flora_vege_status_t vegeflor; // variable para leer el estado del relé vegeflora

// comandos al equipo

button_events_t ev;

//----------FUNCIONES------------//

/*void reset_triac_h(struct tm h1f,struct tm h1i,struct tm h2f,struct tm h2i,struct tm h3f,struct tm h3i,struct tm h4f,struct tm h4i)
{
    //triac_h->enable = 0;
    h1i.tm_hour = 0;
    h1i.tm_min = 0;
    h1f.tm_hour = 0;
    h1f.tm_min = 0;
    //
    h2i.tm_hour = 0;
    h2i.tm_min = 0;
    h2f.tm_hour = 0;
    h2f.tm_min = 0;
    //
    h3i.tm_hour = 0;
    h3i.tm_min = 0;
    h3f.tm_hour = 0;
    h3f.tm_min = 0;
    //
    h4i.tm_hour = 0;
    h4i.tm_min = 0;
    h4f.tm_hour = 0;
    h4f.tm_min = 0;
}*/
/*
void init_red(red_t *red)
{
    memset(red->ID, '\0', sizeof(red->ID));
    strcpy(red->ID, "-");
    memset(red->PASS, '\0', sizeof(red->PASS));
    strcpy(red->PASS, "-");
}
*/
void print_red(red_t *red)
{

    ESP_LOGW(TAG, "ID:%s", red->ID);

    ESP_LOGW(TAG, "PASS:%s", red->PASS);
}

void analyze_token_pwm_triac_vege(char *token)
{
    int dh, dm; // unidades y decenas de horas y minutos
    uint8_t inten = 0;
    // output_mode_t triac_mode;
    // triac_auto_info_t triac_auto;

    switch (token[0])
    {
    case 't': // parseo modo, solo se ve el automatico //LISTO
        flag_modo = 1;
        global_manager_set_pwm_mode(PWM_MANUAL);

        break;
    case 'r':
        ESP_LOGI(PWM, "%d", strlen(token)); // LISTO? CHEQUEAR QUE PASA EN MANUAL Y AUTO
        if (strlen(token) == 7)             // caso de que sea un numero de un solo digito
        {
            inten = (uint8_t)atoi(&token[6]);
            if (flag_modo == 1) // estoy en modo automatico, entonces guardo el valor del pwm en automatico.
            {
                global_manager_set_automatic_pwm_power(inten);
            }
            else
            {
                global_manager_set_pwm_digital_percentage(inten);
            }
        }
        else if (strlen(token) == 8) // caso de un numero de dos digitos
        {
            dh = atoi(&token[6]);
            inten = (uint8_t)atoi(&token[6]);
            ESP_LOGI(TAG, "%d", dh);
            if (flag_modo == 1) // estoy en modo automatico, entonces guardo el valor del pwm en automatico.
            {
                global_manager_set_automatic_pwm_power(inten);
            }
            else
            {
                global_manager_set_pwm_digital_percentage(inten);
            }
        }
        else if (strlen(token) == 9) // caso 100
        {
            if (flag_modo == 1) // estoy en modo automatico, entonces guardo el valor del pwm en automatico.
            {
                global_manager_set_automatic_pwm_power(inten);
            }
            else
            {
                global_manager_set_pwm_digital_percentage(inten);
            }
        }
        else
        {
            ESP_LOGE(PWM, "Error en parseo del rango del pwm");
        }

        break;
    case 'O': // seteo dia si o no LISTO

        if (token[9] == 'S')
        {
            global_manager_set_simul_day_status(SIMUL_DAY_ON);
        }
        else if (token[9] == 'N')
        {
            global_manager_set_simul_day_status(SIMUL_DAY_OFF);
        }
        else
        {
            ESP_LOGE(PWM, "Error en parseo del amanecer/atardecer del pwm");
        }

        break;
    case 'i':                // este es muy largo porque tengo que contemplar los horarios de los dos. LISTO
        if (token[3] == 'p') // es horario inicial del pwm
        {
            dh = atoi(&token[7]);
            dm = atoi(&token[12]);

            time_pwmi_web.tm_hour = dh;
            time_pwmi_web.tm_min = dm;

            break;
        }
        else if (token[3] == '=') // es horario inicial del triac
        {
            dh = atoi(&token[4]);
            dm = atoi(&token[9]);
            if (token[2] == '1')
            {

                triac_h1i.tm_hour = dh;
                triac_h1i.tm_min = dm;
            }
            else if (token[2] == '2')
            {

                triac_h2i.tm_hour = dh;
                triac_h2i.tm_min = dm;
            }
            else if (token[2] == '3')
            {

                triac_h3i.tm_hour = dh;
                triac_h3i.tm_min = dm;
            }
            else if (token[2] == '4')
            {

                triac_h4i.tm_hour = dh;
                triac_h4i.tm_min = dm;
            }
        }
        break;
    case 'f':                // este igual
        if (token[3] == 'p') // es horario final del pwm
        {
            dh = atoi(&token[7]);
            dm = atoi(&token[12]);

            time_pwmf_web.tm_hour = dh;
            time_pwmf_web.tm_min = dm;

            break;
        }
        else if (token[3] == '=') // es horario final del triac
        {
            dh = atoi(&token[4]);
            dm = atoi(&token[9]);
            if (token[2] == '1')
            {

                triac_h1f.tm_hour = dh;
                triac_h1f.tm_min = dm;
            }
            else if (token[2] == '2')
            {

                triac_h2f.tm_hour = dh;
                triac_h2f.tm_min = dm;
            }
            else if (token[2] == '3')
            {

                triac_h3f.tm_hour = dh;
                triac_h3f.tm_min = dm;
            }
            else if (token[2] == '4')
            {

                triac_h4f.tm_hour = dh;
                triac_h4f.tm_min = dm;
            }
        }
        else
        {
            ESP_LOGE(MAIN, "Error en parseo de horarios");
        }
        break;
    case 'm': // parseo el estado del triac
        ESP_LOGW(MAIN, "entre al case m");
        ESP_LOGW(MAIN, "el token 12 es %c", token[12]);
        ESP_LOGW(MAIN, "el token 5 es %c", token[5]);
        ESP_LOGW(MAIN, "el token 14 es %c", token[14]);
        /*if (token[12] == 'E') //elimino la opcion de prender o apagar la salida 220V
        {
            global_manager_set_triac_mode_manual_on(false);
        }
        else if (token[12] == 'A')
        {
            global_manager_set_triac_mode_off(false);
        }
        else */
        if (token[5] == 'v')
        {
            if (token[14] == 'V')
            {

                global_manager_set_flora_vege_status(FLORA_VEGE_OUTPUT_DISABLE);
            }
            else if (token[14] == 'F')
            {
                global_manager_set_flora_vege_status(FLORA_VEGE_OUTPUT_ENABLE);
            }
        }
        else
        {
            ESP_LOGE(TRIAC, "Error en parseo de vegeflor");
        }
        break;

    case 'c': // los checkbox del triac
        if (token[9] == '1')
        {
            en1 = 1;
        }
        if (token[9] == '2')
        {
            en2 = 1;
        }
        if (token[9] == '3')
        {
            en3 = 1;
        }
        if (token[9] == '4')
        {
            en4 = 1;
        }
        break;
    default:
        break;
    }
}

void parse_pwm_triac_vege(char *buff)
{
    // el & es el separador de los campos
    ESP_LOGI(MAIN, "Testeo del MAIN parseo");
    char delim[2] = "&";
    char *token;
    // output_mode_t triac_mode;
    // triac_auto_info_t triac_auto;
    //  seteo un par de cosas del triac para que ya tenga los valores de antes
    // int status = global_manager_get_triac_info(&modo_triac, &triac_auto_info);

    en1 = 0;
    en2 = 0;
    en3 = 0;
    en4 = 0;

    // obtengo los 4 horarios de s.out
    global_manager_get_s_out_turn_on_time(&triac_h1i, 0);
    global_manager_get_s_out_turn_off_time(&triac_h1f, 0);
    //
    global_manager_get_s_out_turn_on_time(&triac_h2i, 1);
    global_manager_get_s_out_turn_off_time(&triac_h2f, 1);
    //
    global_manager_get_s_out_turn_on_time(&triac_h3i, 2);
    global_manager_get_s_out_turn_off_time(&triac_h3f, 2);
    //
    global_manager_get_s_out_turn_on_time(&triac_h4i, 3);
    global_manager_get_s_out_turn_off_time(&triac_h4f, 3);

    // Pongo el flag del modo del  pwm en manual por default
    flag_modo = 0;
    //   hago los token del header para parsear
    token = strtok(buff, delim);
    while (token != NULL)
    {
        analyze_token_pwm_triac_vege(token);
        ESP_LOGI(MAIN, "%s", token);
        token = strtok(NULL, delim);
    }
    // condicion para ver si se mando el modo automatico o no
    ESP_LOGE(MAIN, "el flag modo es: %u", flag_modo);
    if (flag_modo == 0) // significa que sigo en manual
    {
        global_manager_set_pwm_mode(PWM_AUTOMATIC);
    }
    if (en1 != 1)
        en1 = 0;
    if (en2 != 1)
        en2 = 0;
    if (en3 != 1)
        en3 = 0;
    if (en4 != 1)
        en4 = 0;
    // guardo los horarios de los triac

    // set horario 1 final e inicial
    global_manager_set_s_out_turn_on_time(triac_h1i, 0);
    global_manager_set_s_out_turn_off_time(triac_h1f, 0);
    // set horario 2 final e inicial
    global_manager_set_s_out_turn_on_time(triac_h2i, 1);
    global_manager_set_s_out_turn_off_time(triac_h2f, 1);
    // set horario 3 final e inicial
    global_manager_set_s_out_turn_on_time(triac_h3i, 2);
    global_manager_set_s_out_turn_off_time(triac_h3f, 2);
    // set horario 4 final e inicial
    global_manager_set_s_out_turn_on_time(triac_h4i, 3);
    global_manager_set_s_out_turn_off_time(triac_h4f, 3);

    // guardo el horario del pwm

    global_manager_set_turn_on_time(time_pwmi_web);
    global_manager_set_turn_off_time(time_pwmf_web);

    // mando comando para actualizar el modo
    change_mode_device();

    // led_manager_new_update();

    ESP_LOGI(MAIN, "Salgo del parseo MAIN");
}

void parse_red(char *buff, red_t *red)
{
    // el & es el separador de los campos
    ESP_LOGI(TAG, "Testeo del parseo de RED");
    uint8_t status = 0;
    char *e;
    int j = 0;
    int len = strlen(buff);
    int index_amp;
    int equalIndex = 6;
    e = strchr(buff, '&');
    index_amp = (int)(e - buff);
    int secondEqualIndex = index_amp + 12;
    ESP_LOGW(TAG, "%d", index_amp);
    for (int i = equalIndex + 1; i < index_amp; i++)
    {
        red->ID[j] = buff[i];
        j++;
    }
    red->ID[j] = '\0';
    j = 0;
    // status = global_manager_set_wifi_ssid(red->ID, pdFALSE);
    for (int i = secondEqualIndex + 1; i <= len; i++)
    {
        red->PASS[j] = buff[i];
        j++;
    }
    // status = global_manager_set_wifi_password(red->PASS, pdFALSE);

    esp_timer_start_once(timer_reset_esp32, 2000000);
    ESP_LOGI(TAG, "Salgo del parseo de RED");

    // led_manager_new_update();
};

void parse_hora(char *buff, struct tm *aux)
{
    // el & es el separador de los campos
    ESP_LOGI(HORA, "Entro al parseo de HORA");

    aux->tm_hour = atoi(&buff[5]);
    if (buff[6] == '%')
    {
        aux->tm_min = atoi(&buff[9]);
    }
    else
    {
        aux->tm_min = atoi(&buff[10]);
    }

    current_time_manager_set_current_time(*aux);

    ESP_LOGI(HORA, "Salgo del parseo HORA");
};

//----------URL´S------------//
httpd_uri_t index_uri = {
    .uri = "/index",
    .method = HTTP_GET,
    .handler = index_get_handler,
    .user_ctx = NULL};

httpd_uri_t config_uri = {
    .uri = "/config",
    .method = HTTP_GET,
    .handler = config_get_handler,
    .user_ctx = NULL};

httpd_uri_t red_post = {
    .uri = "/red",
    .method = HTTP_POST,
    .handler = red_post_handler,
    .user_ctx = NULL};

httpd_uri_t pwm_triac_vege_post = {
    .uri = "/pwm_triac_vege",
    .method = HTTP_POST,
    .handler = pwm_triac_vege_post_handler,
    .user_ctx = NULL};

httpd_uri_t hora_post = {
    .uri = "/hora",
    .method = HTTP_POST,
    .handler = hora_post_handler,
    .user_ctx = NULL};

httpd_uri_t data_red_uri = {
    .uri = "/data_red",
    .method = HTTP_GET,
    .handler = red_data_handler,
    .user_ctx = NULL};

httpd_uri_t data_pwm_uri = {
    .uri = "/data_pwm",
    .method = HTTP_GET,
    .handler = pwm_data_handler,
    .user_ctx = NULL};

httpd_uri_t data_triac_uri = {
    .uri = "/data_triac",
    .method = HTTP_GET,
    .handler = triac_data_handler,
    .user_ctx = NULL};

httpd_uri_t data_vegeflor_uri = {
    .uri = "/data_vegeflor",
    .method = HTTP_GET,
    .handler = vegeflor_data_handler,
    .user_ctx = NULL};

httpd_uri_t version_data_uri = {
    .uri = "/version",
    .method = HTTP_GET,
    .handler = version_data_handler,
    .user_ctx = NULL};

httpd_uri_t hora_data_uri = {
    .uri = "/data_hora",
    .method = HTTP_GET,
    .handler = hora_data_handler,
    .user_ctx = NULL};

httpd_uri_t image = {
    .uri = "/logo",
    .method = HTTP_GET,
    .handler = logo_handler,
    .user_ctx = NULL};

//----------HANDLERS PARA LOS HTML------------//
esp_err_t index_get_handler(httpd_req_t *req)
{
    extern unsigned char index_start[] asm("_binary_index_html_start");
    extern unsigned char index_end[] asm("_binary_index_html_end");
    size_t index_len = index_end - index_start;
    size_t chunk_size = 2048; // podés ajustar este tamaño
    size_t offset = 0;

    while (offset < index_len)
    {
        size_t bytes_to_send = (index_len - offset > chunk_size) ? chunk_size : (index_len - offset);
        esp_err_t err = httpd_resp_send_chunk(req, (const char *)&index_start[offset], bytes_to_send);
        if (err != ESP_OK)
        {
            ESP_LOGE("HTTP", "Error al enviar chunk: %s", esp_err_to_name(err));
            return err;
        }
        offset += bytes_to_send;
    }

    // Señalar el final del contenido
    return httpd_resp_send_chunk(req, NULL, 0);
}

esp_err_t config_get_handler(httpd_req_t *req)
{
    extern unsigned char config_start[] asm("_binary_config_html_start");
    extern unsigned char config_end[] asm("_binary_config_html_end");
    size_t config_len = config_end - config_start;
    size_t chunk_size = 2048; // ajustá este valor si querés chunks más grandes o más chicos
    size_t offset = 0;

    while (offset < config_len)
    {
        size_t bytes_to_send = (config_len - offset > chunk_size) ? chunk_size : (config_len - offset);

        esp_err_t err = httpd_resp_send_chunk(req, (const char *)&config_start[offset], bytes_to_send);
        if (err != ESP_OK)
        {
            ESP_LOGE("HTTP", "Error al enviar chunk en offset %d: %s", (int)offset, esp_err_to_name(err));
            return err;
        }

        offset += bytes_to_send;
    }

    // Señal de fin de transmisión
    return httpd_resp_send_chunk(req, NULL, 0);
}

esp_err_t logo_handler(httpd_req_t *req)
{
    extern unsigned char logo_start[] asm("_binary_logo_png_start");
    extern unsigned char logo_end[] asm("_binary_logo_png_end");
    size_t logo_len = logo_end - logo_start;
    size_t chunk_size = 2048; // o 512 si querés ser más conservador
    size_t offset = 0;

    // Importante: establecer el tipo MIME antes de enviar chunks
    httpd_resp_set_type(req, "image/png");

    while (offset < logo_len)
    {
        size_t bytes_to_send = (logo_len - offset > chunk_size) ? chunk_size : (logo_len - offset);

        esp_err_t err = httpd_resp_send_chunk(req, (const char *)&logo_start[offset], bytes_to_send);
        if (err != ESP_OK)
        {
            ESP_LOGE("HTTP", "Error al enviar chunk en offset %d: %s", (int)offset, esp_err_to_name(err));
            return err;
        }

        offset += bytes_to_send;
    }

    // Señalar fin de la respuesta
    return httpd_resp_send_chunk(req, NULL, 0);
}

//----------HANDLERS PARA LOS POST DE LAS SECCIONES------------//
esp_err_t pwm_triac_vege_post_handler(httpd_req_t *req)
{
    // Enviar una respuesta HTTP predeterminada
    ESP_LOGI(TAG, "ENTRE AL MAIN HANDLER");
    char buff[300];
    int ret, remaining = 0;
    remaining = req->content_len;
    ret = req->content_len;
    ESP_LOGI(TAG, "%d", ret);
    ESP_LOGI(TAG, "%d", remaining);
    if (remaining >= sizeof(buff))
    {
        // Buffer de datos insuficiente
        ESP_LOGI(TAG, "PAYLOAD MUY GRANDE");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Payload too large");
        return ESP_FAIL;
    }
    else
    {
        while (remaining > 0)
        {
            // Leer los datos del formulario
            ret = httpd_req_recv(req, buff, sizeof(buff)); // en buff se pone lo que estaba en el req y devuelve el numero de bytes que se pasaron al buffer
            if (ret <= 0)                                  // si es 0 o menor es que hubo error
            {
                if (ret == HTTPD_SOCK_ERR_TIMEOUT)
                {
                    // El tiempo de espera para recibir los datos ha expirado
                    httpd_resp_send_408(req);
                }
                return ESP_FAIL;
            }
            remaining -= ret; // resto la cantidad que se pasaron para pasar en el siguiente ciclo el resto. aca cuidado porque los esstaria sobrescribiendo
            // Procesar los datos recibidos, por ejemplo, almacenarlos en una variable
        }
        ESP_LOGI(TAG, "%s", buff);
        parse_pwm_triac_vege(buff);
        ESP_LOGI(TAG, "Salgo del MAIN HANDLER");
        httpd_resp_send(req, NULL, 0);

        set_screen_one_from_web();

        return ESP_OK;
    }
}

esp_err_t red_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "ENTRE AL HANDLER DE LA RED");
    char buff[50];
    memset(buff, '\0', sizeof(buff));
    int ret, remaining = 0;
    remaining = req->content_len;
    ret = req->content_len;
    ESP_LOGI(TAG, "%d", ret);
    ESP_LOGI(TAG, "%d", remaining);
    if (remaining >= sizeof(buff))
    {
        // Buffer de datos insuficiente
        ESP_LOGI(TAG, "PAYLOAD MUY GRANDE");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Payload too large");
        return ESP_FAIL;
    }
    else
    {
        while (remaining > 0)
        {
            // Leer los datos del formulario
            ret = httpd_req_recv(req, buff, sizeof(buff)); // en buff se pone lo que estaba en el req y devuelve el numero de bytes que se pasaron al buffer
            if (ret <= 0)                                  // si es 0 o menor es que hubo error
            {
                if (ret == HTTPD_SOCK_ERR_TIMEOUT)
                {
                    // El tiempo de espera para recibir los datos ha expirado
                    httpd_resp_send_408(req);
                }
                return ESP_FAIL;
            }
            remaining -= ret; // resto la cantidad que se pasaron para pasar en el siguiente ciclo el resto. aca cuidado porque los esstaria sobrescribiendo
            // Procesar los datos recibidos, por ejemplo, almacenarlos en una variable
        }
        ESP_LOGI(TAG, "%s", buff);
        parse_red(buff, &red);
        print_red(&red);
        ESP_LOGI(TAG, "Salgo del RED HANDLER");
    }
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t hora_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "ENTRE AL HANDLER DE LA HORA");
    char buff[30];
    int ret, remaining = 0;
    remaining = req->content_len;
    ret = req->content_len;
    ESP_LOGI(TAG, "%d", ret);
    ESP_LOGI(TAG, "%d", remaining);
    if (remaining >= sizeof(buff))
    {
        // Buffer de datos insuficiente
        ESP_LOGI(VEGEFLOR, "PAYLOAD MUY GRANDE");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Payload too large");
        return ESP_FAIL;
    }
    else
    {
        while (remaining > 0)
        {
            // Leer los datos del formulario
            ret = httpd_req_recv(req, buff, sizeof(buff)); // en buff se pone lo que estaba en el req y devuelve el numero de bytes que se pasaron al buffer
            if (ret <= 0)                                  // si es 0 o menor es que hubo error
            {
                if (ret == HTTPD_SOCK_ERR_TIMEOUT)
                {
                    // El tiempo de espera para recibir los datos ha expirado
                    httpd_resp_send_408(req);
                }
                return ESP_FAIL;
            }
            remaining -= ret; // resto la cantidad que se pasaron para pasar en el siguiente ciclo el resto. aca cuidado porque los esstaria sobrescribiendo
            // Procesar los datos recibidos, por ejemplo, almacenarlos en una variable
        }
        ESP_LOGI(TAG, "%s", buff);
        parse_hora(buff, &aux_hora);
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }
}

//----------HANDLERS PARA LEER LOS DATOS------------/

esp_err_t red_data_handler(httpd_req_t *req)
{
    uint8_t status = 0;
    status = global_manager_get_net_info(red.ID, red.PASS);
    if (status == 1)
    {
        cJSON *json_object = cJSON_CreateObject();
        cJSON_AddStringToObject(json_object, "ID", red.ID);
        cJSON_AddStringToObject(json_object, "PASS", red.PASS);
        char *json_str = cJSON_Print(json_object);
        ESP_LOGI("RED", "JSON ES: %s", json_str);
        cJSON_Delete(json_object);

        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json_str, strlen(json_str));

        free(json_str);

        return ESP_OK;
    }
    else
    {
        ESP_LOGI(TAG, "ERROR EN OBTENER SSID y PASSWORD");
        return ESP_FAIL;
    }
}

esp_err_t pwm_data_handler(httpd_req_t *req)
{
    char *modo;
    uint8_t status = 0;
    // status = global_manager_get_pwm_info(&modo_pwm, &pwm_info);
    status = global_manager_get_turn_on_time(&time_pwmi_web);
    global_manager_get_turn_off_time(&time_pwmf_web);
    global_manager_get_simul_day_status(&dia_web);
    ESP_LOGE("DIADIA", " EL DIA ES %u", dia_web);
    global_manager_get_pwm_mode(&modo_pwm_web);
    global_manager_get_automatic_pwm_power(&pwm_auto_web);

    if (is_jp3_teclas_connected() == true)
    {
        global_manager_get_pwm_digital_percentage(&pwm_man_web);
    }
    else
    {
        global_manager_get_pwm_analog_percentage(&pwm_man_web);
    }

    if (status == 1)
    {
        cJSON *json_object = cJSON_CreateObject();
        // cJSON_AddNumberToObject(json_object, "intensidad", pwm_info.percent_power);

        if (modo_pwm_web == PWM_MANUAL)
        {
            // ESP_LOGE("TAG", "ENTRE EN MODO PWM_MANUAL");

            modo = "Manual";
            cJSON_AddNumberToObject(json_object, "intensidad", pwm_man_web);
            cJSON_AddNumberToObject(json_object, "ih1h", time_pwmi_web.tm_hour);
            cJSON_AddNumberToObject(json_object, "ih1m", time_pwmi_web.tm_min);
            cJSON_AddNumberToObject(json_object, "fh1h", time_pwmf_web.tm_hour);
            cJSON_AddNumberToObject(json_object, "fh1m", time_pwmf_web.tm_min);
            // ESP_LOGE("TAG", "Las horas son: %u : %u y %u : %u", time_pwmi_web.tm_hour, time_pwmi_web.tm_min, time_pwmf_web.tm_hour, time_pwmf_web.tm_min);
        }
        /*else if (modo_pwm == MANUAL_OFF)
        {
            modo = "OFF";
            cJSON_AddNumberToObject(json_object, "ih1h", pwm_info.turn_on_time.tm_hour);
            cJSON_AddNumberToObject(json_object, "ih1m", pwm_info.turn_on_time.tm_min);
            cJSON_AddNumberToObject(json_object, "fh1h", pwm_info.turn_off_time.tm_hour);
            cJSON_AddNumberToObject(json_object, "fh1m", pwm_info.turn_off_time.tm_min);
        }*/
        else if (modo_pwm_web == PWM_AUTOMATIC)
        {
            modo = "Automatico";
            cJSON_AddNumberToObject(json_object, "intensidad", pwm_auto_web);
            cJSON_AddNumberToObject(json_object, "ih1h", time_pwmi_web.tm_hour);
            cJSON_AddNumberToObject(json_object, "ih1m", time_pwmi_web.tm_min);
            cJSON_AddNumberToObject(json_object, "fh1h", time_pwmf_web.tm_hour);
            cJSON_AddNumberToObject(json_object, "fh1m", time_pwmf_web.tm_min);
        }
        else
        {
            modo = "Indefinido";
            cJSON_AddStringToObject(json_object, "ih1h", "-");
            cJSON_AddStringToObject(json_object, "ih1m", "-");
            cJSON_AddStringToObject(json_object, "fh1h", "-");
            cJSON_AddStringToObject(json_object, "fh1m", "-");
        }
        cJSON_AddStringToObject(json_object, "Modo", modo);
        if (dia_web == SIMUL_DAY_ON)
        {
            modo = "Si";
        }
        else
        {
            modo = "No";
        }
        cJSON_AddStringToObject(json_object, "DIA", modo);
        global_manager_get_automatic_pwm_output_status(&auto_pwm_output_status);
        if (auto_pwm_output_status == PWM_OUTPUT_ON) // pwm_info.output_status == PWM_OUTPUT_ON)
        {
            modo = "ON";
            cJSON_AddStringToObject(json_object, "State", modo);
        }
        else
        {
            modo = "OFF";
            cJSON_AddStringToObject(json_object, "State", modo);
        }

        char *json_str = cJSON_Print(json_object);
        ESP_LOGI(PWM, "JSON ES: %s", json_str);
        cJSON_Delete(json_object);

        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json_str, strlen(json_str));

        free(json_str);
        return ESP_OK;
    }
    else
    {
        ESP_LOGI(TAG, "ERROR EN OBTENER PWM");
        return ESP_FAIL;
    }
}

esp_err_t triac_data_handler(httpd_req_t *req)
{
    char *modo;
    uint8_t status = 0;
    // status = global_manager_get_triac_info(&modo_triac, &triac_auto_info);
    status = global_manager_get_s_out_turn_on_time(&triac_h1i, 0);
    global_manager_get_s_out_turn_off_time(&triac_h1f, 0);
    //
    global_manager_get_s_out_turn_on_time(&triac_h2i, 1);
    global_manager_get_s_out_turn_off_time(&triac_h2f, 1);
    //
    global_manager_get_s_out_turn_on_time(&triac_h3i, 2);
    global_manager_get_s_out_turn_off_time(&triac_h3f, 2);
    //
    global_manager_get_s_out_turn_on_time(&triac_h4i, 3);
    global_manager_get_s_out_turn_off_time(&triac_h4f, 3);
    if (status == 1)
    {
        cJSON *json_object = cJSON_CreateObject();
        /*if (1 == 1) // modo_triac == MANUAL_ON)
        {
            modo = "Encendido";
        }*/

        // ESP_LOGE(TRIAC, " EL ENABLE DEL 1 ES %d", triac_auto_info.triac_auto[0].enable);
        // cJSON_AddStringToObject(json_object, "Modo", modo);
        cJSON_AddBoolToObject(json_object, "cb1", 1);
        cJSON_AddNumberToObject(json_object, "ih1h", triac_h1i.tm_hour);
        cJSON_AddNumberToObject(json_object, "ih1m", triac_h1i.tm_min);
        cJSON_AddNumberToObject(json_object, "fh1h", triac_h1f.tm_hour);
        cJSON_AddNumberToObject(json_object, "fh1m", triac_h1f.tm_min);
        cJSON_AddBoolToObject(json_object, "cb2", 1);
        cJSON_AddNumberToObject(json_object, "ih2h", triac_h2i.tm_hour);
        cJSON_AddNumberToObject(json_object, "ih2m", triac_h2i.tm_min);
        cJSON_AddNumberToObject(json_object, "fh2h", triac_h2f.tm_hour);
        cJSON_AddNumberToObject(json_object, "fh2m", triac_h2f.tm_min);
        cJSON_AddBoolToObject(json_object, "cb3", 1);
        cJSON_AddNumberToObject(json_object, "ih3h", triac_h3i.tm_hour);
        cJSON_AddNumberToObject(json_object, "ih3m", triac_h3i.tm_min);
        cJSON_AddNumberToObject(json_object, "fh3h", triac_h3f.tm_hour);
        cJSON_AddNumberToObject(json_object, "fh3m", triac_h3f.tm_min);
        cJSON_AddBoolToObject(json_object, "cb4", 1);
        cJSON_AddNumberToObject(json_object, "ih4h", triac_h4i.tm_hour);
        cJSON_AddNumberToObject(json_object, "ih4m", triac_h4i.tm_min);
        cJSON_AddNumberToObject(json_object, "fh4h", triac_h4f.tm_hour);
        cJSON_AddNumberToObject(json_object, "fh4m", triac_h4f.tm_min);

        if (1 == 1) // triac_auto_info.output_status == TRIAC_OUTPUT_ON)
        {
            modo = "ON";
            cJSON_AddStringToObject(json_object, "State", modo);
        }
        else
        {
            modo = "OFF";
            cJSON_AddStringToObject(json_object, "State", modo);
        }

        char *json_str = cJSON_Print(json_object);
        ESP_LOGI(TRIAC, "JSON ES: %s", json_str);
        cJSON_Delete(json_object);

        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json_str, strlen(json_str));

        free(json_str);

        return ESP_OK;
    }
    else
    {
        ESP_LOGI(TAG, "ERROR EN OBTENER DATOS DE TRIAC");
        return ESP_FAIL;
    }
}

esp_err_t vegeflor_data_handler(httpd_req_t *req)
{
    char *modo;
    uint8_t status = 0;
    status = global_manager_get_flora_vege_status(&vegeflor);
    if (status == 1)
    {
        cJSON *json_object = cJSON_CreateObject();
        if (vegeflor == FLORA_VEGE_OUTPUT_DISABLE)
        {
            modo = "Vegetativo";
        }
        else
        {
            modo = "Floracion";
        }
        cJSON_AddStringToObject(json_object, "Modo", modo);

        char *json_str = cJSON_Print(json_object);
        ESP_LOGI(VEGEFLOR, "JSON ES: %s", json_str);
        cJSON_Delete(json_object);

        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json_str, strlen(json_str));

        free(json_str);

        return ESP_OK;
    }
    else
    {
        ESP_LOGI(TAG, "ERROR EN OBTENER VEGEFLOR");
        return ESP_FAIL;
    }
}

esp_err_t version_data_handler(httpd_req_t *req)
{
    cJSON *json_object = cJSON_CreateObject();
    char version[10];
    uint8_t len_ver;
    get_version(version, &len_ver);
    cJSON_AddStringToObject(json_object, "Version", version);

    char *json_str = cJSON_Print(json_object);
    ESP_LOGI(VERSIONN, "JSON ES: %s", json_str);
    cJSON_Delete(json_object);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_str, strlen(json_str));

    free(json_str);

    return ESP_OK;
}

esp_err_t hora_data_handler(httpd_req_t *req)
{

    uint8_t status = 0;
    status = global_manager_get_current_time_info(&aux_hora);
    if (status == 1)
    {
        cJSON *json_object = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_object, "Hora_h", aux_hora.tm_hour);
        cJSON_AddNumberToObject(json_object, "Hora_m", aux_hora.tm_min);
        char *json_str = cJSON_Print(json_object);
        ESP_LOGI(HORA, "JSON ES: %s", json_str);
        cJSON_Delete(json_object);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json_str, strlen(json_str));
        free(json_str);
        return ESP_OK;
    }
    else
    {
        ESP_LOGI(TAG, "ERROR EN OBTENER HORA");
        return ESP_FAIL;
    }
}

//---------FUNCIONES DEL WEBSERVER-------------//

static void timer_reset_esp32_callback(void *arg)
{
    esp_restart();
}

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG(); // Configuracion por default del server
    config.stack_size = 300000;
    config.max_uri_handlers = 16;
    config.max_resp_headers = 16;

    esp_err_t ret = httpd_start(&server, &config);
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (ret == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        // ESP_LOGI(TAG, "Registering HTML");
        httpd_register_uri_handler(server, &index_uri);
        httpd_register_uri_handler(server, &config_uri);
        httpd_register_uri_handler(server, &pwm_triac_vege_post);
        httpd_register_uri_handler(server, &red_post);
        httpd_register_uri_handler(server, &hora_post);
        httpd_register_uri_handler(server, &data_red_uri);
        httpd_register_uri_handler(server, &version_data_uri);
        httpd_register_uri_handler(server, &hora_data_uri);
        httpd_register_uri_handler(server, &data_vegeflor_uri);
        httpd_register_uri_handler(server, &data_pwm_uri);
        httpd_register_uri_handler(server, &data_triac_uri);
        httpd_register_uri_handler(server, &image);
        // init_red(&red);

        esp_timer_create_args_t timer_reset_esp32_args = {
            .callback = timer_reset_esp32_callback,
            .arg = NULL,
            .name = "timer_reset_esp32"};
        esp_timer_create(&timer_reset_esp32_args, &timer_reset_esp32);

        return server;
    }
    else if (ret == ESP_ERR_HTTPD_ALLOC_MEM)
    {
        ESP_LOGI(TAG, "ESP_ERR_HTTPD_ALLOC_MEM \n");
    }
    else if (ret == ESP_ERR_HTTPD_TASK)
    {
        ESP_LOGI(TAG, "ESP_ERR_HTTPD_TASK \n");
    }
    else
    {
        ESP_LOGI(TAG, "ret = %i \n", (int)ret);
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

void disconnect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK)
        {
            *server = NULL;
        }
        else
        {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

void connect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}
