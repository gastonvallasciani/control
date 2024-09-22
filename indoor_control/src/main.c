#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "esp_log.h"

#define RESET_PIN_DISPLAY 13
#define LED_PIN 18

#define DISPLAY_ADDRESS 0x3C      // caso 7 bits 78, o sino 0x3C caso 8 bits
#define I2C_MASTER_SCL_IO 22      // Pin SCL
#define I2C_MASTER_SDA_IO 21      // Pin SDA
#define I2C_MASTER_FREQ_HZ 400000 // Frecuencia I2C 400kHz
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
// comandos display
static const char *TAG = "I2C";

uint8_t led_level = 0;

static esp_err_t init_led_pin(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

static esp_err_t blink_led(void)
{
    led_level = !led_level;
    gpio_set_level(LED_PIN, led_level);
    return ESP_OK;
}

static esp_err_t init_reset_display_pin(void)
{
    gpio_reset_pin(RESET_PIN_DISPLAY);
    gpio_set_direction(RESET_PIN_DISPLAY, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

static esp_err_t set_i2c(void)
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

/*static esp_err_t display_send_command(uint8_t command)
{
    uint8_t buffer[2];
    buffer[0] = 0x00;    // Control byte (0x00 = comando)
    buffer[1] = command; // Comando a enviar
    esp_err_t err = i2c_master_write_to_device(I2C_NUM_0, DISPLAY_ADDRESS, buffer, sizeof(buffer), 1000 / portTICK_PERIOD_MS);
    if (err == ESP_FAIL)
    {
        ESP_LOGE(TAG, "Error en el envio de comando al display");
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGI(TAG, "Comando enviado al display con exito");
        return ESP_OK;
    }
}*/

static esp_err_t display_send_command(uint8_t command)
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

static esp_err_t display_init(void)
{
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
    ESP_LOGI(TAG, "Envio primer comando");
    display_send_command(0x3A);
    display_send_command(0x09);
    display_send_command(0x06);
    display_send_command(0x1E);
    display_send_command(0x39);
    display_send_command(0x1B);
    display_send_command(0x6C);
    display_send_command(0x54);
    display_send_command(0x70);
    display_send_command(0x72);
    display_send_command(0x00);
    display_send_command(0x38);
    display_send_command(0x0F);
    display_send_command(0x01);

    return ESP_OK;
}

void i2c_scanner()
{
    printf("Escaneando el bus I2C...\n");
    for (int i = 1; i < 127; i++)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if (err == ESP_OK)
        {
            printf("Dispositivo encontrado en la dirección 0x%02X\n", i);
        }
    }
    printf("Escaneo completado\n");
}

void app_main()
{
    init_led_pin();
    ESP_LOGI(TAG, "Inicializando I2C");
    ESP_ERROR_CHECK(set_i2c());
    ESP_LOGI(TAG, "Escaneo");
    i2c_scanner();
    // ESP_LOGI(TAG, "Inicializando DISPLAY");
    // display_init();
    // ESP_LOGI(TAG, "Termina inicializacion del DISPLAY");

    while (true)
    {
        ESP_LOGI("wait", "...");
        // blink_led();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}