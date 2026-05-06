#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"

#include "encoder/encoder.h"
#include "driver/driver.h"
#include "network/network.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Boot start: initializing encoder");
    ESP_ERROR_CHECK(encoder_init());

    ESP_LOGI(TAG, "Initializing driver");
    ESP_ERROR_CHECK(driver_init());

    ESP_LOGI(TAG, "Initializing network stack (Ethernet + MQTT manager)");
    ESP_ERROR_CHECK(network_init());

    ESP_LOGI(TAG, "System init complete; runtime logs available on USB serial");

    vTaskDelete(NULL);
}