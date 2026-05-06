#include <stdbool.h>
#include <inttypes.h>

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

    bool prev_link_up = false;
    bool prev_ip_up = false;
    bool waiting_link_logged = false;
    uint32_t ping_elapsed_ms = 0;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(APP_NETWORK_FAILSAFE_MS));

        bool link_up = ethernet_is_link_up();
        bool ip_up = ethernet_is_ip_up();
        bool mqtt_up = mqtt_is_up();

        if (link_up != prev_link_up) {
            ESP_LOGI(TAG, "State change: ethernet_link=%d", link_up ? 1 : 0);
            prev_link_up = link_up;
            waiting_link_logged = false;
        }

        if (ip_up != prev_ip_up) {
            ESP_LOGI(TAG, "State change: ethernet_ip=%d", ip_up ? 1 : 0);
            prev_ip_up = ip_up;
        }

        ping_elapsed_ms += APP_NETWORK_FAILSAFE_MS;
        if (ping_elapsed_ms >= APP_LOG_PING_PERIOD_MS) {
            uint32_t uptime_s = (uint32_t)(xTaskGetTickCount() / configTICK_RATE_HZ);
            ESP_LOGI(TAG, "PING uptime=%" PRIu32 "s link=%d ip=%d mqtt=%d", uptime_s,
                     link_up ? 1 : 0, ip_up ? 1 : 0, mqtt_up ? 1 : 0);
            ping_elapsed_ms = 0;
        }

        if (!link_up) {
            if (!waiting_link_logged) {
                ESP_LOGI(TAG, "Ethernet not linked yet, waiting for cable/link...");
                waiting_link_logged = true;
            }
            driver_set_enabled(false);
            esp_err_t stop_err = mqtt_stop();
            if (stop_err != ESP_OK) {
                ESP_LOGW(TAG, "mqtt_stop failed: %s", esp_err_to_name(stop_err));
            }
            continue;
        }

        waiting_link_logged = false;

        if (!ip_up) {
            ESP_LOGI(TAG, "Ethernet link is up, waiting for DHCP IP...");
            continue;
        }

        if (!mqtt_up) {
            ESP_LOGI(TAG, "IP is up, requesting MQTT start");
        }

        esp_err_t start_err = mqtt_start();
        if (start_err != ESP_OK) {
            ESP_LOGW(TAG, "mqtt_start failed: %s", esp_err_to_name(start_err));
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