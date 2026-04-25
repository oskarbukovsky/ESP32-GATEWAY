#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "esp_log.h"

#include "app_config.h"
#include "mqtt_control.h"
#include "motor_control.h"
#include "rotor_encoder.h"

static const char *TAG = "mqtt_control";

static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static bool s_started = false;
static bool s_connected = false;

static void build_topic(char *out, size_t out_size, const char *suffix)
{
    snprintf(out, out_size, "%s/%s", APP_MQTT_TOPIC_PREFIX, suffix);
}

static bool topic_equals(const char *topic, int topic_len, const char *suffix)
{
    char expected[96];
    size_t expected_len;

    build_topic(expected, sizeof(expected), suffix);
    expected_len = strlen(expected);
    return (expected_len == (size_t)topic_len) && (strncmp(topic, expected, (size_t)topic_len) == 0);
}

static bool payload_to_bool(const char *payload, int len)
{
    if ((len == 1) && (payload[0] == '1')) {
        return true;
    }
    if ((len == 4) && (strncasecmp(payload, "true", 4) == 0)) {
        return true;
    }
    return false;
}

static float payload_to_float(const char *payload, int len)
{
    char buf[32];
    int copy_len = len;

    if (copy_len > (int)(sizeof(buf) - 1)) {
        copy_len = (int)sizeof(buf) - 1;
    }
    memcpy(buf, payload, (size_t)copy_len);
    buf[copy_len] = '\0';

    return strtof(buf, NULL);
}

static void handle_command(const char *topic, int topic_len, const char *data, int data_len)
{
    if (topic_equals(topic, topic_len, "cmd/enable")) {
        motor_control_set_enabled(payload_to_bool(data, data_len));
        return;
    }

    if (topic_equals(topic, topic_len, "cmd/pid_enable")) {
        motor_control_set_pid_enabled(payload_to_bool(data, data_len));
        return;
    }

    if (topic_equals(topic, topic_len, "cmd/target_rps")) {
        motor_control_set_target_rps(payload_to_float(data, data_len));
        return;
    }

    if (topic_equals(topic, topic_len, "cmd/manual_pwm")) {
        motor_control_set_manual_pwm(payload_to_float(data, data_len));
        return;
    }

    if (topic_equals(topic, topic_len, "cmd/reset_encoder")) {
        rotor_encoder_reset_count();
        return;
    }
}

static void publish_status(void)
{
    char topic[96];
    char payload[256];

    int32_t count = rotor_encoder_get_count();
    float speed_rps = rotor_encoder_get_speed_rps();
    float target_rps = motor_control_get_target_rps();
    float pwm = motor_control_get_output_duty();
    bool enabled = motor_control_is_enabled();
    bool pid_enabled = motor_control_is_pid_enabled();
    bool idx_seen = rotor_encoder_is_index_seen();

    build_topic(topic, sizeof(topic), "status");

    snprintf(payload, sizeof(payload),
             "{\"count\":%ld,\"speed_rps\":%.4f,\"target_rps\":%.4f,\"pwm\":%.2f,\"enabled\":%d,\"pid\":%d,\"index\":%d}",
             (long)count, speed_rps, target_rps, pwm, enabled ? 1 : 0, pid_enabled ? 1 : 0, idx_seen ? 1 : 0);

    esp_mqtt_client_publish(s_mqtt_client, topic, payload, 0, 0, 0);
}

static void status_task(void *arg)
{
    (void)arg;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(APP_MQTT_STATUS_PERIOD_MS));
        if (s_connected && s_mqtt_client != NULL) {
            publish_status();
        }
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    switch ((esp_mqtt_event_id_t)event->event_id) {
    case MQTT_EVENT_CONNECTED: {
        char topic[96];
        s_connected = true;

        build_topic(topic, sizeof(topic), "cmd/#");
        esp_mqtt_client_subscribe(s_mqtt_client, topic, 0);
        ESP_LOGI(TAG, "MQTT connected, subscribed to %s", topic);
        break;
    }
    case MQTT_EVENT_DISCONNECTED:
        s_connected = false;
        ESP_LOGW(TAG, "MQTT disconnected");
        break;
    case MQTT_EVENT_DATA:
        handle_command(event->topic, event->topic_len, event->data, event->data_len);
        break;
    default:
        break;
    }

    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)handler_args;
    (void)base;
    (void)event_id;

    mqtt_event_handler_cb((esp_mqtt_event_handle_t)event_data);
}

esp_err_t mqtt_control_start(void)
{
    if (s_started) {
        return ESP_OK;
    }

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = APP_MQTT_BROKER_URI,
        .username = APP_MQTT_USERNAME,
        .password = APP_MQTT_PASSWORD,
    };

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (s_mqtt_client == NULL) {
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(s_mqtt_client));

    xTaskCreate(status_task, "mqtt_status", 4096, NULL, 4, NULL);
    s_started = true;

    ESP_LOGI(TAG, "MQTT started (%s)", APP_MQTT_BROKER_URI);
    return ESP_OK;
}

bool mqtt_control_is_started(void)
{
    return s_started;
}
