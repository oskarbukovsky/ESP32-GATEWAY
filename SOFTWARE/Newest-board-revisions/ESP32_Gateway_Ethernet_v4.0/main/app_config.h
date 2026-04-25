#pragma once

#include "driver/gpio.h"

/*
 * Hardware mapping for ESP32-GATEWAY (adjust as needed for your wiring).
 * These are practical defaults and should be verified on your exact revision.
 */
#define APP_PIN_ENCODER_A            GPIO_NUM_32
#define APP_PIN_ENCODER_B            GPIO_NUM_33
#define APP_PIN_ENCODER_INDEX        GPIO_NUM_35
#define APP_PIN_MOSFET_PWM           GPIO_NUM_25

/* AMT10 setup */
#define APP_ENCODER_PPR              1024
#define APP_ENCODER_CPR              (APP_ENCODER_PPR * 4)
#define APP_ENCODER_USE_INDEX        1
#define APP_ENCODER_SPEED_PERIOD_MS  50

/* PWM output to MOSFET gate driver */
#define APP_PWM_FREQUENCY_HZ         20000
#define APP_PWM_DUTY_MAX_PERCENT     100.0f
#define APP_PWM_DUTY_MIN_PERCENT     0.0f

/* Lightweight PID setup */
#define APP_PID_PERIOD_MS            20
#define APP_PID_KP                   5.0f
#define APP_PID_KI                   0.8f
#define APP_PID_KD                   0.02f
#define APP_PID_I_MIN                -30.0f
#define APP_PID_I_MAX                30.0f

/* MQTT setup */
#define APP_MQTT_BROKER_URI          "mqtt://192.168.1.100:1883"
#define APP_MQTT_USERNAME            ""
#define APP_MQTT_PASSWORD            ""
#define APP_MQTT_TOPIC_PREFIX        "gateway/motor"
#define APP_MQTT_STATUS_PERIOD_MS    1000
