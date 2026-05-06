#pragma once

#include "driver/gpio.h"

/*
 * Hardware mapping for ESP32-GATEWAY (adjust as needed for your wiring).
 * These are practical defaults and should be verified on your exact revision.
 */
#define APP_PIN_ENCODER_A            GPIO_NUM_32
#define APP_PIN_ENCODER_B            GPIO_NUM_33
#define APP_PIN_ENCODER_INDEX        GPIO_NUM_35
#define APP_PIN_DRIVER_PWM_A         GPIO_NUM_25
#define APP_PIN_DRIVER_PWM_B         GPIO_NUM_26

/* AMT10 setup */
#define APP_ENCODER_PPR              1024
#define APP_ENCODER_CPR              (APP_ENCODER_PPR * 4)
#define APP_ENCODER_USE_INDEX        1
#define APP_ENCODER_SPEED_PERIOD_MS  50

#define APP_PWM_CARRIER_HZ           1000
#define APP_PWM_SINE_HZ              50
/* PWM output to MOSFET gate driver */
#define APP_PWM_FREQUENCY_HZ         APP_PWM_CARRIER_HZ
#define APP_PWM_DUTY_MAX_PERCENT     100.0f
#define APP_PWM_DUTY_MIN_PERCENT     0.0f

/* Position / setup behavior */
#define APP_POSITION_LIMIT_DEG       180.0f
#define APP_POSITION_TOLERANCE_DEG   2.0f
#define APP_POSITION_KP              0.02f
#define APP_POSITION_MAX_RPS         5.0f

/* Lightweight PID setup */
#define APP_PID_PERIOD_MS            20
#define APP_PID_KP                   5.0f
#define APP_PID_KI                   0.8f
#define APP_PID_KD                   0.02f
#define APP_PID_I_MIN                -30.0f
#define APP_PID_I_MAX                30.0f

/* MQTT setup */
#define APP_MQTT_BROKER_URI          "mqtts://l9f0a4f9.ala.eu-central-1.emqxsl.com:8883"
#define APP_MQTT_USERNAME            ""
#define APP_MQTT_PASSWORD            ""
#define APP_MQTT_TOPIC_PREFIX        "gateway/motor"
#define APP_MQTT_STATUS_PERIOD_MS    1000
#define APP_NETWORK_FAILSAFE_MS      500
#define APP_LOG_PING_PERIOD_MS       10000
