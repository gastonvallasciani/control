#include <stdio.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "driver/i2c.h"
#include "esp_log.h"

#include "../include/display_dogs164.h"
// #include "../include/display_manager.h"
//  #include "../include/board_def.h"

#define RESET_PIN_DISPLAY 13

#define DISPLAY_ADDRESS 0x3C      // caso 7 bits 78, o sino 0x3C caso 8 bits
#define I2C_MASTER_SCL_IO 22      // Pin SCL
#define I2C_MASTER_SDA_IO 21      // Pin SDA
#define I2C_MASTER_FREQ_HZ 400000 // Frecuencia I2C 400kHz
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

esp_err_t init_reset_display_pin(void)
{
    gpio_reset_pin(RESET_PIN_DISPLAY);
    gpio_set_direction(RESET_PIN_DISPLAY, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

esp_err_t set_i2c(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = false,
        .scl_pullup_en = false,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        .clk_flags = 0};

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));

    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));

    return ESP_OK;
}

esp_err_t display_send_command(uint8_t command)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DISPLAY_ADDRESS << 1) | I2C_MASTER_WRITE, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true)); // Indica que es un comando
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, command, true));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

esp_err_t display_send_data(uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DISPLAY_ADDRESS << 1) | I2C_MASTER_WRITE, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x40, true)); // Indica que es un comando
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, data, true));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

esp_err_t set_cursor(uint8_t row, uint8_t col)
{
    display_send_command(0x80 | (row * 0x20 + col));
    return ESP_OK;
}

esp_err_t display_write_char(char c)
{
    display_send_data(c);

    return ESP_OK;
}

esp_err_t display_write_string(const char *str)
{
    while (*str)
    {
        display_write_char(*str++);
    }
    return ESP_OK;
}

esp_err_t display_clean_arrow(void)
{
    set_cursor(1, 5);
    display_write_string(" ");
    return ESP_OK;
}

esp_err_t display_clean_power_and_bar(void)
{
    uint8_t i;
    for (i = 0; i < 4; i++)
    {
        set_cursor(1, i);
        display_write_string(" ");
    }
    for (i = 6; i < 16; i++)
    {
        set_cursor(1, i);
        display_write_string(" ");
    }
    return ESP_OK;
}

esp_err_t display_power_bar(uint8_t power)
{
    uint8_t full_bars = 0;
    uint8_t minor_bars = 0;
    uint8_t pixel_bars = 0;
    uint8_t i;
    if (power > 100)
    {
        power = 100;
    }

    pixel_bars = power / 2;
    full_bars = pixel_bars / 5;
    minor_bars = pixel_bars % 5;

    // imprimo las barras completas
    for (i = 6; i < 6 + full_bars; i++)
    {
        set_cursor(1, i);
        display_send_data(FIVE_BAR);
    }
    // imprimo la barra incompleta restante
    switch (minor_bars)
    {
    case 1:
        set_cursor(1, 6 + full_bars);
        display_send_data(ONE_BAR);
        break;
    case 2:
        set_cursor(1, 6 + full_bars);
        display_send_data(TWO_BAR);
        break;
    case 3:
        set_cursor(1, 6 + full_bars);
        display_send_data(THREE_BAR);
        break;
    case 4:
        set_cursor(1, 6 + full_bars);
        display_send_data(FOUR_BAR);
        break;
    default:
        break;
    }

    return ESP_OK;
}

esp_err_t display_set_screen(uint8_t power)
{
    char *master = "MASTER";
    char *lumenar = "LUMENAR";
    char numero[6];
    sprintf(numero, "%u%%", power);

    // hago secuencia de reset
    init_reset_display_pin();
    gpio_set_level(RESET_PIN_DISPLAY, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    // comandos para configurar el display
    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_4LINES);
    display_send_command(COMMAND_BOTTOM_VIEW); //
    display_send_command(COMMAND_BS1_1);       //
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS1);
    display_send_command(COMMAND_BS0_1); //
    display_send_command(COMMAND_FOLLOWER_CONTROL_DOGS164);
    display_send_command(COMMAND_POWER_CONTROL_DOGS164);
    display_send_command(COMMAND_CONTRAST_DEFAULT_DOGS164);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0);
    display_send_command(COMMAND_CLEAR_DISPLAY);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    display_send_command(COMMAND_SHIFT_SCROLL_ALL_LINES); //
    display_send_command(COMMAND_DISPLAY_SHIFT_RIGHT);
    // aca defino que el display sea con dos lineas grandes
    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_2LINES);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0_DH1);
    // limpio todo lo de la linea de datos
    display_clean_arrow();
    display_clean_power_and_bar();
    // escribo
    set_cursor(0, 0);
    display_write_string(master); // escribo master
    set_cursor(0, 9);
    display_write_string(lumenar); // escribo lumenar
    set_cursor(1, 0);
    display_write_string(numero); // escribo el valor de potencia
    // selecciono la ROM A para la flecha
    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_ROM_SELECT);
    display_send_data(COMMAND_ROM_A);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0_DH1);
    // muestro la barra de potencia
    display_power_bar(power);

    return ESP_OK;
}

esp_err_t display_set_power(uint8_t power, char vege_flora)
{
    static uint8_t last_power = 0xFF; // Guardar el valor previo del power

    // limpio todo lo de la linea de datos
    char numero[6];

    set_cursor(1, 5);
    display_write_char(vege_flora);

    // Si el valor de power ha cambiado, actualiza la barra y el número
    if (power != last_power)
    {
        display_clean_power_and_bar();
        if (power > 100)
        {
            power = 100;
        }
        sprintf(numero, "%u%%", power);
        set_cursor(1, 0);
        display_write_string(numero);
        display_power_bar(power);
        last_power = power;
    }

    return ESP_OK;
}

esp_err_t display_set_vege_flora(char vege_flora)
{
    set_cursor(1, 4);
    display_write_char(vege_flora);

    return ESP_OK;
}

esp_err_t display_init()
{
    char *lumenar = "LUMENAR";

    char *iniciando = "Iniciando...";

    init_reset_display_pin();
    gpio_set_level(RESET_PIN_DISPLAY, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_4LINES);
    display_send_command(COMMAND_BOTTOM_VIEW); //
    display_send_command(COMMAND_BS1_1);       //
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS1);
    display_send_command(COMMAND_BS0_1); //
    display_send_command(COMMAND_FOLLOWER_CONTROL_DOGS164);
    display_send_command(COMMAND_POWER_CONTROL_DOGS164);
    display_send_command(COMMAND_CONTRAST_DEFAULT_DOGS164);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0);
    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_OFF);
    display_send_command(COMMAND_CLEAR_DISPLAY);
    display_send_command(COMMAND_ENTRY_MODE_SET | ENTRY_MODE_LEFT_TO_RIGHT);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_ROM_SELECT);
    display_send_data(COMMAND_ROM_A);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0_DH1);
    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_3LINES_MIDDLE);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0_DH1);
    set_cursor(1, 4);
    display_write_string(lumenar);
    vTaskDelay(400 / portTICK_PERIOD_MS);
    set_cursor(2, 2);
    display_write_string(iniciando);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    return ESP_OK;
}

esp_err_t display_set_screen_one(screen_t *screen, char *fpower, uint8_t power, char vege_flora, bool dia, bool modo, struct tm time)
{ // en los argumentos de la funcion falta si auto o manual y si vege o flora, que variable?
  // bool dia, 1 si, 0 no.
  // bool modo, 1 manual, 0 auto.
  // para el tiempo, uso la time.h. revisar como es la estructura.
    char *phy = "PHY-03";
    char *lumenar = "LUMENAR";
    char *ppf = "PPF";
    char *p = "P";
    char *w = "W";
    char *modo_m = "MAN";
    char *ddots = ":";
    char *dia_m = "DIA";
    char hour[4];
    char min[4];
    char numero[6];
    char numeroppf[6];
    char potactual[6];
    float ppfn = power * 2.97;

    int fpowercc;
    fpowercc = atoi(fpower);

    int pot_actual = fpowercc * power / 100;

    if (dia == true)
    {
        dia_m = "SI";
    }
    else
    {
        dia_m = "NO";
    }
    if (modo == true)
    {
        modo_m = "MAN";
    }
    else
    {
        modo_m = "AUT";
    }

    *screen = SCREEN_ONE;

    sprintf(numero, "%u%%", power);
    sprintf(hour, "%u", time.tm_hour);
    sprintf(min, "%u", time.tm_min);
    sprintf(numeroppf, "%.f", ppfn);
    sprintf(potactual, "%u", pot_actual);

    display_send_command(COMMAND_CLEAR_DISPLAY);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0);
    // primera fila
    set_cursor(0, 0);
    display_write_string(phy); // pongo el phy-03
    set_cursor(0, 9);
    display_write_string(lumenar); // pongo la marca lumenar
    // segunda fila
    set_cursor(1, 0);
    display_write_string(numero); // escribo el valor de potencia
    set_cursor(1, 5);
    display_write_char(vege_flora); // escribo la letra si es vege o flora
    display_power_bar(power);       // muestro la barra de potencia
    // tercera fila
    set_cursor(2, 0);
    display_write_string(ppf); // escribo la palabra ppf
    set_cursor(2, 3);
    display_write_string(ddots); // escribo los dos puntos
    set_cursor(2, 4);
    display_write_string(numeroppf); // escribo el numero del ppf
    set_cursor(2, 9);
    display_write_string(p); // escribo la letra P de la potencia total
    if (pot_actual <= 99999 && pot_actual > 9999)
    {
        set_cursor(2, 10);
    }
    if (pot_actual <= 9999 && pot_actual > 999)
    {
        set_cursor(2, 11);
    }
    if (pot_actual <= 999 && pot_actual > 99)
    {
        set_cursor(2, 12);
    }
    if (pot_actual <= 99 && pot_actual > 9)
    {
        set_cursor(2, 13);
    }
    if (pot_actual <= 9 && pot_actual >= 0)
    {
        set_cursor(2, 14);
    }

    display_write_string(potactual); // los 4 digitos de la potencia total (pueden ser 5?)
    set_cursor(2, 15);
    display_write_string(w); // escribo la W de la unidad de potencia
    // cuarta fila
    set_cursor(3, 0);
    display_write_string("DIA");
    set_cursor(3, 3);
    display_write_string(ddots);
    set_cursor(3, 4);
    display_write_string(dia_m);
    set_cursor(3, 7);
    display_write_string(modo_m);
    set_cursor(3, 11);
    display_write_string(hour);
    set_cursor(3, 13);
    display_write_string(ddots);
    set_cursor(3, 14);
    display_write_string(min);

    return ESP_OK;
}

esp_err_t screen_one_line_three(struct tm time, bool dia, bool modo)
{
    ESP_LOGI("TIMER", "Entro al screen one line three");
    char *dia_m;
    char *modo_m;
    char hour[4];
    char min[4];
    if (dia == true)
    {
        dia_m = "SI";
    }
    else
    {
        dia_m = "NO";
    }
    if (modo == true)
    {
        modo_m = "MAN";
    }
    else
    {
        modo_m = "AUTO";
    }

    sprintf(hour, "%u", time.tm_hour);
    sprintf(min, "%u", time.tm_min);
    set_cursor(3, 0);
    display_write_string("DIA");
    set_cursor(3, 3);
    display_write_string(":");
    set_cursor(3, 4);
    display_write_string(dia_m);
    set_cursor(3, 7);
    display_write_string(modo_m);
    if (time.tm_hour < 10)
    {
        set_cursor(3, 11);
        display_write_string("0");
        set_cursor(3, 12);
        display_write_string(hour);
    }
    else
    {
        set_cursor(3, 11);
        display_write_string(hour);
    }
    set_cursor(3, 13);
    display_write_string(":");
    if (time.tm_min < 10)
    {
        set_cursor(3, 14);
        display_write_string("0");
        set_cursor(3, 15);
        display_write_string(min);
    }
    else
    {
        set_cursor(3, 14);
        display_write_string(min);
    }
    ESP_LOGI("TIMER", "Salgo del screen one line three");
    return ESP_OK;
}

esp_err_t screen_two_line(uint8_t line, struct tm time_i, struct tm time_f)
{
    ESP_LOGI("TIMER", "Entro al screen two line");
    char *h = "ER";
    // char *ini = "i";
    // char *fin = "f";
    char houri[4];
    char hourf[4];
    char mini[4];
    char minf[4];

    switch (line)
    {
    case 0:
        h = "T1";
        break;
    case 1:
        h = "T2";
        break;
    case 2:
        h = "T3";
        break;
    case 3:
        h = "T4";
        break;

    default:
        break;
    }

    sprintf(houri, "%u", time_i.tm_hour);
    sprintf(mini, "%u", time_i.tm_min);
    sprintf(hourf, "%u", time_f.tm_hour);
    sprintf(minf, "%u", time_f.tm_min);

    set_cursor(line, 0);
    display_write_string(h);
    set_cursor(line, 3);
    display_send_data(0xDE);
    if (time_i.tm_hour < 10)
    {
        set_cursor(line, 4);
        display_write_string("0");
        set_cursor(line, 5);
        display_write_string(houri);
    }
    else
    {
        set_cursor(line, 4);
        display_write_string(houri);
    }
    set_cursor(line, 6);
    display_write_string(":");
    if (time_i.tm_min < 10)
    {
        set_cursor(line, 7);
        display_write_string("0");
        set_cursor(line, 8);
        display_write_string(mini);
    }
    else
    {
        set_cursor(line, 7);
        display_write_string(mini);
    }
    set_cursor(line, 10);
    display_send_data(0xE0);
    if (time_f.tm_hour < 10)
    {
        set_cursor(line, 11);
        display_write_string("0");
        set_cursor(line, 12);
        display_write_string(hourf);
    }
    else
    {
        set_cursor(line, 11);
        display_write_string(hourf);
    }
    set_cursor(line, 13);
    display_write_string(":");
    if (time_f.tm_min < 10)
    {
        set_cursor(line, 14);
        display_write_string("0");
        set_cursor(line, 15);
        display_write_string(minf);
    }
    else
    {
        set_cursor(line, 14);
        display_write_string(minf);
    }
    ESP_LOGI("TIMER", "Salgo del screen two line");
    return ESP_OK;
}

esp_err_t display_set_screen_two(screen_t *screen, struct tm time_i1, struct tm time_i2, struct tm time_i3, struct tm time_i4, struct tm time_f1, struct tm time_f2, struct tm time_f3, struct tm time_f4)
{

    display_send_command(COMMAND_CLEAR_DISPLAY);       // limpio display
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0); // me aseguro qeu se ponga en 4 lineas
    *screen = SCREEN_TWO;
    screen_two_line(0, time_i1, time_f1);
    screen_two_line(1, time_i2, time_f2);
    screen_two_line(2, time_i3, time_f3);
    screen_two_line(3, time_i4, time_f4);

    return ESP_OK;
}

esp_err_t display_set_screen_three(screen_t *screen, struct tm time_pwmi, struct tm time_pwmf, char *fpower, uint8_t level)
{
    char *pwm = "PWM";
    // char *ini = "i";
    // char *fin = "f";
    char *total = "POT.TOTAL";
    char *contraste = "CONTRASTE";
    char houri[4];
    char hourf[4];
    char mini[4];
    char minf[4];
    char contrast_level[4];
    // char fpowerc[6];
    int fpowercc;
    fpowercc = atoi(fpower);
    ESP_LOGI("TAG", "fpower es %u", fpowercc);

    display_send_command(COMMAND_CLEAR_DISPLAY);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0);
    *screen = SCREEN_THREE;

    sprintf(houri, "%u", time_pwmi.tm_hour);
    sprintf(mini, "%u", time_pwmi.tm_min);
    sprintf(hourf, "%u", time_pwmf.tm_hour);
    sprintf(minf, "%u", time_pwmf.tm_min);
    sprintf(contrast_level, "%u", level);
    // sprintf(fpowerc, "%u", fpower);

    set_cursor(0, 0);
    display_write_string(pwm);
    set_cursor(0, 3);
    display_send_data(0xDE);
    set_cursor(0, 4);
    display_write_string(houri);
    set_cursor(0, 6);
    display_write_string(":");
    set_cursor(0, 7);
    display_write_string(mini);
    set_cursor(0, 10);
    display_send_data(0xE0);
    set_cursor(0, 11);
    display_write_string(hourf);
    set_cursor(0, 13);
    display_write_string(":");
    set_cursor(0, 14);
    display_write_string(minf);
    set_cursor(1, 0);
    display_write_string(total);

    /*if (fpowercc <= 99999 && fpowercc > 9999)
    {
        set_cursor(1, 10);
    }
    else if (fpowercc <= 9999 && fpowercc > 999)
    {
        set_cursor(1, 11);
    }
    else if (fpowercc <= 999 && fpowercc > 99)
    {
        set_cursor(1, 12);
    }
    else if (fpowercc <= 99 && fpowercc > 9)
    {
        set_cursor(1, 13);
    }
    else if (fpowercc <= 9 && fpowercc >= 0)
    {
        set_cursor(1, 14);
    }*/
    set_cursor(1, 10);
    display_write_string(fpower);
    set_cursor(1, 15);
    display_write_string("W");
    set_cursor(2, 0);
    display_write_string(contraste);
    set_cursor(2, 14);
    display_write_string(contrast_level);

    return ESP_OK;
}

esp_err_t screen_three_line(uint8_t line, char *fpower, struct tm time_i, struct tm time_f, uint8_t level)
{
    char *pwm = "PWM";
    char *total = "POT.TOTAL";
    char *contraste = "CONTRASTE";
    char houri[4];
    char hourf[4];
    char mini[4];
    char minf[4];
    char fpowerc[6];
    char contrast_level[4];

    sprintf(houri, "%u", time_i.tm_hour);
    sprintf(mini, "%u", time_i.tm_min);
    sprintf(hourf, "%u", time_f.tm_hour);
    sprintf(minf, "%u", time_f.tm_min);
    sprintf(contrast_level, "%u", level);
    // sprintf(fpowerc, "%u", fpower);
    int fpowercc;
    fpowercc = atoi(fpower);
    ESP_LOGI("TAG", "fpower es %u", fpowercc);
    switch (line)
    {
    case 0:
        set_cursor(0, 0);
        display_write_string(pwm);
        set_cursor(0, 3);
        display_send_data(0xDE);
        set_cursor(0, 4);
        display_write_string(houri);
        set_cursor(0, 6);
        display_write_string(":");
        set_cursor(0, 7);
        display_write_string(mini);
        set_cursor(0, 10);
        display_send_data(0xE0);
        set_cursor(0, 11);
        display_write_string(hourf);
        set_cursor(0, 13);
        display_write_string(":");
        set_cursor(0, 14);
        display_write_string(minf);
        break;
    case 1:
        set_cursor(1, 0);
        display_write_string(total);
        /*if (fpower <= 99999 || fpower > 9999)
        {
            set_cursor(1, 10);
        }
        else if (fpower <= 9999 || fpower > 999)
        {
            set_cursor(1, 11);
        }
        else if (fpower <= 999 || fpower > 99)
        {
            set_cursor(1, 12);
        }
        else if (fpower <= 99 || fpower > 9)
        {
            set_cursor(1, 13);
        }
        else if (fpower <= 9 || fpower >= 0)
        {
            set_cursor(1, 14);
        }*/
        set_cursor(1, 10);
        display_write_string(fpower);
        set_cursor(1, 15);
        display_write_string("W");
        break;
    case 2:
        set_cursor(2, 0);
        display_write_string(contraste);
        set_cursor(2, 14);
        display_write_string(contrast_level);
        break;
    default:
        break;
    }

    return ESP_OK;
}

esp_err_t set_contrast(uint8_t level)
{
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS1);
    display_send_command(COMMAND_POWER_CONTROL_DOGS164);
    switch (level)
    {
    case 1:
        display_send_command(0x70);
        break;
    case 2:
        display_send_command(0x71);
        break;
    case 3:
        display_send_command(0x72);
        break;
    case 4:
        display_send_command(0x73);
        break;
    case 5:
        display_send_command(0x74);
        break;
    case 6:
        display_send_command(0x75);
        break;
    case 7:
        display_send_command(0x76);
        break;
    case 8:
        display_send_command(0x77);
        break;
    case 9:
        display_send_command(0x78);
        break;
    case 10:
        display_send_command(0x79);
        break;
    case 11:
        display_send_command(0x7A);
        break;
    case 12:
        display_send_command(0x7B);
        break;
    case 13:
        display_send_command(0x7C);
        break;
    case 14:
        display_send_command(0x7D);
        break;
    case 15:
        display_send_command(0x7E);
        break;
    case 16:
        display_send_command(0x7F);
        break;
    default:
        display_send_command(COMMAND_CONTRAST_DEFAULT_DOGS164);
        break;
    }
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0);
    return ESP_OK;
}