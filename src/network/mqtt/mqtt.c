#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "app_config.h"
#include "driver/driver.h"
#include "encoder/encoder.h"
#include "network/ethernet/ethernet.h"
#include "network/mqtt/mqtt.h"
#include "network/mqtt/mqtt_schema.h"
#include "network/mqtt/mqtt_config.h"
#include "freertos/semphr.h"

static const char *TAG = "mqtt";
static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static bool s_mqtt_up = false;
static bool s_client_created = false;
static bool s_client_started = false;
static bool s_limit_event_sent = false;
static bool s_status_task_started = false;

static SemaphoreHandle_t s_mqtt_lock = NULL;
static TaskHandle_t s_reconnect_task = NULL;
static uint32_t s_reconnect_delay_ms = 1000; /* start 1s */
static const uint32_t MQTT_RECONNECT_MAX_MS = 60000; /* 60s */

static void mqtt_reconnect_task(void *arg);
static void set_mqtt_up(bool v);
static bool get_mqtt_up(void);

static void log_payload_preview(const char *prefix, const char *topic, const char *payload, int payload_len)
{
    char payload_buf[96];
    int copy_len = payload_len;
    if (copy_len > (int)(sizeof(payload_buf) - 1)) {
        copy_len = (int)sizeof(payload_buf) - 1;
    }
    if (copy_len > 0) {
        memcpy(payload_buf, payload, (size_t)copy_len);
    }
    payload_buf[copy_len] = '\0';

    ESP_LOGI(TAG, "%s topic=%s payload=%s%s", prefix, topic, payload_buf,
             payload_len > (int)(sizeof(payload_buf) - 1) ? "..." : "");
}

static void log_heartbeat(void)
{
    uint32_t uptime_s = (uint32_t)(xTaskGetTickCount() / configTICK_RATE_HZ);
    time_t now = time(NULL);

    if (now > 1700000000) {
        struct tm tm_now;
        localtime_r(&now, &tm_now);
        char ts[32];
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm_now);
        ESP_LOGI(TAG, "PING uptime=%" PRIu32 "s now=%s link=%d ip=%d mqtt=%d", uptime_s, ts,
                 ethernet_is_link_up() ? 1 : 0,
                 ethernet_is_ip_up() ? 1 : 0,
                 get_mqtt_up() ? 1 : 0);
    } else {
        ESP_LOGI(TAG, "PING uptime=%" PRIu32 "s now=unsynced link=%d ip=%d mqtt=%d", uptime_s,
                 ethernet_is_link_up() ? 1 : 0,
                 ethernet_is_ip_up() ? 1 : 0,
                 get_mqtt_up() ? 1 : 0);
    }
}

static void build_topic(char *out, size_t out_size, const char *suffix)
{
    snprintf(out, out_size, "%s/%s", APP_MQTT_TOPIC_PREFIX, suffix);
}

static bool topic_equals(const char *topic, int topic_len, const char *suffix)
{
    char expected[96];
    build_topic(expected, sizeof(expected), suffix);
    return (strlen(expected) == (size_t)topic_len) && (strncmp(topic, expected, (size_t)topic_len) == 0);
}

static bool payload_to_bool(const char *payload, int len)
{
    return ((len == 1) && payload[0] == '1') || ((len == 4) && (strncasecmp(payload, "true", 4) == 0));
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

static void mqtt_publish_event(const char *suffix, const char *payload)
{
    if (!s_mqtt_up || s_mqtt_client == NULL) {
        return;
    }

    char topic[96];
    build_topic(topic, sizeof(topic), suffix);
    log_payload_preview("TX event", topic, payload, (int)strlen(payload));
    esp_mqtt_client_publish(s_mqtt_client, topic, payload, 0, 1, 0);
}

static void publish_status(void)
{
    if (!s_mqtt_up || s_mqtt_client == NULL) {
        return;
    }

    char topic[96];
    char payload[320];

    mqtt_status_shape_t status = {
        .count = encoder_get_count(),
        .speed_rps = encoder_get_speed_rps(),
        .target_rps = driver_get_target_rps(),
        .pwm = driver_get_output_duty(),
        .enabled = driver_is_enabled(),
        .pid = driver_is_pid_enabled(),
        .dir = driver_get_direction(),
        .pos_mode = driver_is_position_mode(),
        .angle_deg = driver_get_current_angle_deg(),
        .target_angle_deg = driver_get_target_angle_deg(),
        .limit_deg = driver_get_limit_deg(),
        .limit_hit = driver_is_limit_hit(),
        .link_up = ethernet_is_link_up(),
        .ip_up = ethernet_is_ip_up(),
        .mqtt_up = s_mqtt_up,
        .index = encoder_is_index_seen(),
    };

    build_topic(topic, sizeof(topic), MQTT_TOPIC_STATUS);
    snprintf(payload, sizeof(payload),
             MQTT_STATUS_JSON_FORMAT,
             (long)status.count,
             status.speed_rps,
             status.target_rps,
             status.pwm,
             status.enabled ? 1 : 0,
             status.pid ? 1 : 0,
             status.dir ? 1 : 0,
             status.pos_mode ? 1 : 0,
             status.angle_deg,
             status.target_angle_deg,
             status.limit_deg,
             status.limit_hit ? 1 : 0,
             status.link_up ? 1 : 0,
             status.ip_up ? 1 : 0,
             status.mqtt_up ? 1 : 0,
             status.index ? 1 : 0);

    log_payload_preview("TX status", topic, payload, (int)strlen(payload));
    esp_mqtt_client_publish(s_mqtt_client, topic, payload, 0, 0, 0);
}

static void handle_command(const char *topic, int topic_len, const char *data, int data_len)
{
    char topic_buf[96];
    int topic_copy_len = topic_len;
    if (topic_copy_len > (int)(sizeof(topic_buf) - 1)) {
        topic_copy_len = (int)sizeof(topic_buf) - 1;
    }
    memcpy(topic_buf, topic, (size_t)topic_copy_len);
    topic_buf[topic_copy_len] = '\0';

    log_payload_preview("RX command", topic_buf, data, data_len);

    if (topic_equals(topic, topic_len, MQTT_TOPIC_CMD_ENABLE)) {
        driver_set_enabled(payload_to_bool(data, data_len));
        ESP_LOGI(TAG, "Action: set enabled=%d", driver_is_enabled() ? 1 : 0);
        return;
    }

    if (topic_equals(topic, topic_len, MQTT_TOPIC_CMD_PID_ENABLE)) {
        driver_set_pid_enabled(payload_to_bool(data, data_len));
        ESP_LOGI(TAG, "Action: set pid=%d", driver_is_pid_enabled() ? 1 : 0);
        return;
    }

    if (topic_equals(topic, topic_len, MQTT_TOPIC_CMD_TARGET_RPS)) {
        driver_set_target_rps(payload_to_float(data, data_len));
        ESP_LOGI(TAG, "Action: set target_rps=%.3f", driver_get_target_rps());
        return;
    }

    if (topic_equals(topic, topic_len, MQTT_TOPIC_CMD_MANUAL_PWM)) {
        driver_set_manual_pwm(payload_to_float(data, data_len));
        ESP_LOGI(TAG, "Action: set manual_pwm=%.3f", driver_get_output_duty());
        return;
    }

    if (topic_equals(topic, topic_len, MQTT_TOPIC_CMD_DIRECTION)) {
        driver_set_direction(payload_to_bool(data, data_len));
        ESP_LOGI(TAG, "Action: set direction=%d", driver_get_direction() ? 1 : 0);
        return;
    }

    if (topic_equals(topic, topic_len, MQTT_TOPIC_CMD_SETUP) || topic_equals(topic, topic_len, MQTT_TOPIC_CMD_HOME)) {
        driver_set_home();
        driver_set_position_mode(true);
        driver_set_enabled(true);
        ESP_LOGI(TAG, "Action: setup/home -> home set, position_mode=1, enabled=1");
        return;
    }

    if (topic_equals(topic, topic_len, MQTT_TOPIC_CMD_TARGET_ANGLE_DEG)) {
        driver_set_position_mode(true);
        driver_set_target_angle_deg(payload_to_float(data, data_len));
        driver_set_enabled(true);
        ESP_LOGI(TAG, "Action: set target_angle_deg=%.2f (position_mode=1, enabled=1)", driver_get_target_angle_deg());
        return;
    }

    if (topic_equals(topic, topic_len, MQTT_TOPIC_CMD_LIMIT_DEG)) {
        driver_set_limit_deg(payload_to_float(data, data_len));
        ESP_LOGI(TAG, "Action: set limit_deg=%.2f", driver_get_limit_deg());
        return;
    }

    if (topic_equals(topic, topic_len, MQTT_TOPIC_CMD_RESET_ENCODER)) {
        encoder_reset_count();
        ESP_LOGI(TAG, "Action: encoder reset");
        return;
    }

    ESP_LOGW(TAG, "Action: unknown command topic=%s", topic_buf);
}

static void status_task(void *arg)
{
    (void)arg;
    uint32_t ping_elapsed_ms = 0;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(APP_MQTT_STATUS_PERIOD_MS));

        ping_elapsed_ms += APP_MQTT_STATUS_PERIOD_MS;
        if (ping_elapsed_ms >= APP_LOG_PING_PERIOD_MS) {
            log_heartbeat();
            ping_elapsed_ms = 0;
        }

        if (s_mqtt_up) {
            publish_status();

            if (driver_is_limit_hit() && !s_limit_event_sent) {
                mqtt_publish_event(MQTT_TOPIC_EVENT_LIMIT, MQTT_EVENT_LIMIT_HIT_JSON);
                s_limit_event_sent = true;
            }

            if (!driver_is_limit_hit()) {
                s_limit_event_sent = false;
            }
        }
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    switch ((esp_mqtt_event_id_t)event->event_id) {
    case MQTT_EVENT_CONNECTED: {
        char topic[96];
        set_mqtt_up(true);
        /* if a reconnect task was running, stop it */
        if (s_reconnect_task != NULL) {
            vTaskDelete(s_reconnect_task);
            s_reconnect_task = NULL;
            s_reconnect_delay_ms = 1000;
        }
        build_topic(topic, sizeof(topic), MQTT_TOPIC_CMD_WILDCARD);
        esp_mqtt_client_subscribe(s_mqtt_client, topic, 0);
        ESP_LOGI(TAG, "MQTT connected, subscribed to %s", topic);
        break;
    }
    case MQTT_EVENT_DISCONNECTED:
        set_mqtt_up(false);
        ESP_LOGW(TAG, "MQTT disconnected");
        /* spawn reconnect task if not running */
        if (s_reconnect_task == NULL && s_client_created && s_mqtt_client != NULL) {
            BaseType_t ok = xTaskCreate(mqtt_reconnect_task, "mqtt_reconnect", 4096, NULL, 3, &s_reconnect_task);
            if (ok != pdPASS) {
                ESP_LOGW(TAG, "Failed to create mqtt_reconnect task");
                s_reconnect_task = NULL;
            }
        }
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

static esp_err_t ensure_client(void)
{
    if (s_client_created) {
        return ESP_OK;
    }

    /* initialize mqtt config (NVS-backed stub) and lock */
    mqtt_config_init();
    if (s_mqtt_lock == NULL) {
        s_mqtt_lock = xSemaphoreCreateMutex();
    }

    const char *cfg_user = NULL;
    const char *cfg_pass = NULL;
    mqtt_config_get_credentials(&cfg_user, &cfg_pass);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = APP_MQTT_BROKER_URI,
        .credentials.username = cfg_user ? cfg_user : APP_MQTT_USERNAME,
        .credentials.authentication.password = cfg_pass ? cfg_pass : APP_MQTT_PASSWORD,
    };

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (s_mqtt_client == NULL) {
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    s_client_created = true;

    if (!s_status_task_started) {
        xTaskCreate(status_task, "mqtt_status", 4096, NULL, 4, NULL);
        s_status_task_started = true;
    }

    return ESP_OK;
}

static void mqtt_reconnect_task(void *arg)
{
    (void)arg;
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(s_reconnect_delay_ms));
        ESP_LOGI(TAG, "Attempting MQTT reconnect (delay %" PRIu32 " ms)", s_reconnect_delay_ms);
        esp_err_t r = esp_mqtt_client_reconnect(s_mqtt_client);
        if (r == ESP_OK) {
            /* reconnect will cause CONNECTED event handler to run */
            break;
        }
        /* increase delay exponentially */
        s_reconnect_delay_ms = s_reconnect_delay_ms * 2;
        if (s_reconnect_delay_ms > MQTT_RECONNECT_MAX_MS) {
            s_reconnect_delay_ms = MQTT_RECONNECT_MAX_MS;
        }
    }
    /* mark task handle cleared and exit */
    s_reconnect_task = NULL;
    vTaskDelete(NULL);
}

esp_err_t mqtt_start(void)
{
    if (s_client_started) {
        return ESP_OK;
    }

    ESP_ERROR_CHECK(ensure_client());
    ESP_ERROR_CHECK(esp_mqtt_client_start(s_mqtt_client));
    s_client_started = true;
    return ESP_OK;
}

esp_err_t mqtt_stop(void)
{
    if (!s_client_created || s_mqtt_client == NULL) {
        return ESP_OK;
    }

    if (s_client_started) {
        esp_err_t err = esp_mqtt_client_stop(s_mqtt_client);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            return err;
        }
    }
    /* stop any reconnect task */
    if (s_reconnect_task != NULL) {
        vTaskDelete(s_reconnect_task);
        s_reconnect_task = NULL;
    }

    set_mqtt_up(false);
    s_client_started = false;
    s_limit_event_sent = false;
    return ESP_OK;
}

bool mqtt_is_up(void)
{
    return get_mqtt_up();
}

static void set_mqtt_up(bool v)
{
    if (s_mqtt_lock) xSemaphoreTake(s_mqtt_lock, portMAX_DELAY);
    s_mqtt_up = v;
    if (s_mqtt_lock) xSemaphoreGive(s_mqtt_lock);
}

static bool get_mqtt_up(void)
{
    bool v;
    if (s_mqtt_lock) xSemaphoreTake(s_mqtt_lock, portMAX_DELAY);
    v = s_mqtt_up;
    if (s_mqtt_lock) xSemaphoreGive(s_mqtt_lock);
    return v;
}