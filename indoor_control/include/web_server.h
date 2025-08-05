#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include <esp_wifi.h>
#include <esp_system.h>
#include "nvs_flash.h"
#include "../include/version.h"
#include "../include/current_time_manager.h"
#include "../include/global_manager.h"

typedef struct red
{
    char ID[32];   // maximo 32 caracteres
    char PASS[64]; // maximo 64 caracteres
} red_t;
esp_err_t logo_handler(httpd_req_t *);
esp_err_t index_get_handler(httpd_req_t *);
esp_err_t config_get_handler(httpd_req_t *);
esp_err_t pwm_triac_vege_post_handler(httpd_req_t *);
esp_err_t pwm_post_handler(httpd_req_t *);
esp_err_t red_post_handler(httpd_req_t *);
esp_err_t triac_post_handler(httpd_req_t *);
esp_err_t vegeflor_post_handler(httpd_req_t *);
esp_err_t hora_post_handler(httpd_req_t *);
esp_err_t red_data_handler(httpd_req_t *);
esp_err_t pwm_data_handler(httpd_req_t *);
esp_err_t triac_data_handler(httpd_req_t *);
esp_err_t vegeflor_data_handler(httpd_req_t *);
esp_err_t version_data_handler(httpd_req_t *);
esp_err_t hora_data_handler(httpd_req_t *);
httpd_handle_t start_webserver(void);
esp_err_t stop_webserver(httpd_handle_t);
void disconnect_handler(void *, esp_event_base_t, int32_t, void *);
void connect_handler(void *, esp_event_base_t, int32_t, void *);
/*void parse_pwm_triac_vege(char *);
void parse_pwm(char *);
void parse_triac(char *);
void parse_vegeflor(char *);
void parse_red(char *, red_t *);
void parse_hora(char *, struct tm *);
void analyze_token_pwm_triac_vege(char *);
void analyze_token_pwm(char *);
void analyze_token_triac(char *);
void init_red(red_t *);
void reset_triac_h(triac_config_info_t *);*/