#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#include "encoder/encoder.h"
#include "driver/driver.h"
#include "network/network.h"

void app_main(void)
{
    ESP_ERROR_CHECK(encoder_init());
    ESP_ERROR_CHECK(driver_init());
    ESP_ERROR_CHECK(network_init());

    vTaskDelete(NULL);
}