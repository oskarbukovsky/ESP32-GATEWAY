#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"

#include "app_config.h"
#include "motor_control.h"
#include "rotor_encoder.h"

static const char *TAG = "motor_control";

static bool s_enabled = false;
static bool s_pid_enabled = true;
static float s_target_rps = 0.0f;
static float s_manual_duty = 0.0f;
static float s_output_duty = 0.0f;

static portMUX_TYPE s_ctrl_lock = portMUX_INITIALIZER_UNLOCKED;

static float clampf(float val, float min_val, float max_val)
{
    if (val < min_val) {
        return min_val;
    }
    if (val > max_val) {
        return max_val;
    }
    return val;
}

static void apply_pwm_percent(float duty_percent)
{
    const uint32_t max_duty = (1U << LEDC_TIMER_13_BIT) - 1U;
    float clamped = clampf(duty_percent, APP_PWM_DUTY_MIN_PERCENT, APP_PWM_DUTY_MAX_PERCENT);
    uint32_t duty = (uint32_t)((clamped / 100.0f) * (float)max_duty);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

static void pid_task(void *arg)
{
    (void)arg;

    const float dt_s = (float)APP_PID_PERIOD_MS / 1000.0f;
    float i_term = 0.0f;
    float prev_error = 0.0f;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(APP_PID_PERIOD_MS));

        bool enabled;
        bool pid_enabled;
        float target_rps;
        float manual_duty;

        portENTER_CRITICAL(&s_ctrl_lock);
        enabled = s_enabled;
        pid_enabled = s_pid_enabled;
        target_rps = s_target_rps;
        manual_duty = s_manual_duty;
        portEXIT_CRITICAL(&s_ctrl_lock);

        if (!enabled) {
            i_term = 0.0f;
            prev_error = 0.0f;
            apply_pwm_percent(0.0f);

            portENTER_CRITICAL(&s_ctrl_lock);
            s_output_duty = 0.0f;
            portEXIT_CRITICAL(&s_ctrl_lock);
            continue;
        }

        float output;
        if (pid_enabled) {
            float measured_rps = rotor_encoder_get_speed_rps();
            float error = target_rps - measured_rps;
            i_term = clampf(i_term + (error * dt_s), APP_PID_I_MIN, APP_PID_I_MAX);
            float d_term = (error - prev_error) / dt_s;
            prev_error = error;

            output = (APP_PID_KP * error) + (APP_PID_KI * i_term) + (APP_PID_KD * d_term);
        } else {
            output = manual_duty;
        }

        output = clampf(output, APP_PWM_DUTY_MIN_PERCENT, APP_PWM_DUTY_MAX_PERCENT);
        apply_pwm_percent(output);

        portENTER_CRITICAL(&s_ctrl_lock);
        s_output_duty = output;
        portEXIT_CRITICAL(&s_ctrl_lock);
    }
}

esp_err_t motor_control_init(void)
{
    ledc_timer_config_t timer_cfg = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = APP_PWM_FREQUENCY_HZ,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    ledc_channel_config_t ch_cfg = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = APP_PIN_MOSFET_PWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_cfg));

    xTaskCreate(pid_task, "pid_loop", 3072, NULL, 6, NULL);

    ESP_LOGI(TAG, "Motor control initialized on GPIO %d", APP_PIN_MOSFET_PWM);
    return ESP_OK;
}

void motor_control_set_enabled(bool enabled)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_enabled = enabled;
    portEXIT_CRITICAL(&s_ctrl_lock);
}

void motor_control_set_pid_enabled(bool enabled)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_pid_enabled = enabled;
    portEXIT_CRITICAL(&s_ctrl_lock);
}

void motor_control_set_target_rps(float target_rps)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_target_rps = target_rps;
    portEXIT_CRITICAL(&s_ctrl_lock);
}

void motor_control_set_manual_pwm(float duty_percent)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_manual_duty = duty_percent;
    portEXIT_CRITICAL(&s_ctrl_lock);
}

bool motor_control_is_enabled(void)
{
    bool value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_enabled;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

bool motor_control_is_pid_enabled(void)
{
    bool value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_pid_enabled;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

float motor_control_get_target_rps(void)
{
    float value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_target_rps;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

float motor_control_get_output_duty(void)
{
    float value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_output_duty;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}
