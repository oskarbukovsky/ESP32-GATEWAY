#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "app_config.h"
#include "driver/driver.h"
#include "network/ethernet/ethernet.h"
#include "network/mqtt/mqtt.h"
#include "network/network.h"

static bool s_started = false;
static const char *TAG = "network";

static void network_task(void *arg)
{
    (void)arg;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(APP_NETWORK_FAILSAFE_MS));

        if (!ethernet_is_link_up()) {
            driver_set_enabled(false);
            esp_err_t stop_err = mqtt_stop();
            if (stop_err != ESP_OK) {
                ESP_LOGW(TAG, "mqtt_stop failed: %s", esp_err_to_name(stop_err));
            }
            continue;
        }

        if (ethernet_is_ip_up()) {
            esp_err_t start_err = mqtt_start();
            if (start_err != ESP_OK) {
                ESP_LOGW(TAG, "mqtt_start failed: %s", esp_err_to_name(start_err));
            }
        }
    }
}

esp_err_t network_init(void)
{
    if (s_started) {
        return ESP_OK;
    }

    ESP_ERROR_CHECK(ethernet_init());
    xTaskCreate(network_task, "net_mgr", 3072, NULL, 3, NULL);
    s_started = true;
    return ESP_OK;
}

bool network_is_link_up(void)
{
    return ethernet_is_link_up();
}

bool network_is_mqtt_up(void)
{
    return mqtt_is_up();
}