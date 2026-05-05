#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Topic suffixes under APP_MQTT_TOPIC_PREFIX */
#define MQTT_TOPIC_STATUS                  "status"
#define MQTT_TOPIC_CMD_WILDCARD            "cmd/#"
#define MQTT_TOPIC_EVENT_LIMIT             "event/limit"

#define MQTT_TOPIC_CMD_ENABLE              "cmd/enable"
#define MQTT_TOPIC_CMD_PID_ENABLE          "cmd/pid_enable"
#define MQTT_TOPIC_CMD_TARGET_RPS          "cmd/target_rps"
#define MQTT_TOPIC_CMD_MANUAL_PWM          "cmd/manual_pwm"
#define MQTT_TOPIC_CMD_DIRECTION           "cmd/direction"
#define MQTT_TOPIC_CMD_SETUP               "cmd/setup"
#define MQTT_TOPIC_CMD_HOME                "cmd/home"
#define MQTT_TOPIC_CMD_TARGET_ANGLE_DEG    "cmd/target_angle_deg"
#define MQTT_TOPIC_CMD_LIMIT_DEG           "cmd/limit_deg"
#define MQTT_TOPIC_CMD_RESET_ENCODER       "cmd/reset_encoder"

/* Payload shapes */
typedef struct {
    int32_t count;
    float speed_rps;
    float target_rps;
    float pwm;
    bool enabled;
    bool pid;
    bool dir;
    bool pos_mode;
    float angle_deg;
    float target_angle_deg;
    float limit_deg;
    bool limit_hit;
    bool link_up;
    bool ip_up;
    bool mqtt_up;
    bool index;
} mqtt_status_shape_t;

#define MQTT_STATUS_JSON_FORMAT "{\"count\":%ld,\"speed_rps\":%.4f,\"target_rps\":%.4f,\"pwm\":%.2f,\"enabled\":%d,\"pid\":%d,\"dir\":%d,\"pos_mode\":%d,\"angle_deg\":%.2f,\"target_angle_deg\":%.2f,\"limit_deg\":%.2f,\"limit_hit\":%d,\"link_up\":%d,\"ip_up\":%d,\"mqtt_up\":%d,\"index\":%d}"

#define MQTT_EVENT_LIMIT_HIT_JSON          "{\"limit_hit\":true}"
