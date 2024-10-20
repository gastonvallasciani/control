#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "esp_log.h"

#include "../include/display_dogs164.h"
// #include "../include/board_def.h"

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
    for (i = 0; i < 16; i++)
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

esp_err_t display_set_power(uint8_t power, arrow_t arrow)
{
    static uint8_t last_power = 0xFF; // Guardar el valor previo del power

    // limpio todo lo de la linea de datos
    display_clean_arrow();
    char numero[6];
    if (arrow == ARROW_UP)
    {
        set_cursor(1, 5);
        display_send_data(0xDE); // flecha arriba
    }
    else if (arrow == ARROW_DOWN)
    {
        set_cursor(1, 5);
        display_send_data(0xE0); // flecha abajo
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); // doy tiempo para que se vea la barra

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
    display_clean_arrow();

    return ESP_OK;
}

esp_err_t display_set_screen_full_start(uint8_t power, uint8_t h, uint8_t m)
{ // en los argumentos de la funcion falta si auto o manual y si vege o flora, que variable?
    char *phy = "PHY";
    char *lumenar = "LUMENAR";
    char *ppf = "PPF";
    char *p = "P";
    char *w = "W";
    char *vege = "VEGE";
    char *flora = "FLORA";
    char *automatic = "AUTO";
    char *manual = "MAN";
    char *ddots = ":";
    char hour[4];
    char min[4];
    char numero[6];

    sprintf(numero, "%u%%", power);
    sprintf(hour, "%u", h);
    sprintf(min, "%u", m);

    init_reset_display_pin();
    gpio_set_level(RESET_PIN_DISPLAY, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);

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
    // display_send_command(COMMAND_SHIFT_SCROLL_ALL_LINES); //
    // display_send_command(COMMAND_DISPLAY_SHIFT_RIGHT);
    // display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    // display_send_command(COMMAND_3LINES_BOTTOM);
    // display_send_command(COMMAND_8BIT_4LINES_RE0_IS0_DH1);
    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_ROM_SELECT);
    display_send_data(COMMAND_ROM_A);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0_DH1);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0);

    set_cursor(0, 0);
    display_write_string(phy);
    set_cursor(0, 9);
    display_write_string(lumenar);
    set_cursor(1, 0);
    display_write_string(numero); // escribo el valor de potencia
    set_cursor(2, 0);
    display_write_string(ppf);
    set_cursor(2, 10);
    display_write_string(p);
    set_cursor(2, 15);
    display_write_string(w);
    set_cursor(3, 0);
    display_write_string(flora);
    set_cursor(3, 6);
    display_write_string(manual);
    set_cursor(3, 13);
    display_write_string(ddots);
    set_cursor(3, 11);
    display_write_string(hour);
    set_cursor(3, 14);
    display_write_string(min);

    // selecciono la ROM A para la flecha
    /*display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_ROM_SELECT);
    display_send_data(COMMAND_ROM_A);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0_DH1);*/
    // muestro la barra de potencia
    display_power_bar(power);

    return ESP_OK;
}
