#include <math.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"

#include "app_config.h"
#include "driver/driver.h"
#include "encoder/encoder.h"

static const char *TAG = "driver";
static const float kPi = 3.14159265358979323846f;

static bool s_enabled = false;
static bool s_pid_enabled = true;
static bool s_position_mode = false;
static bool s_direction_forward = true;
static bool s_limit_hit = false;
static float s_target_rps = 0.0f;
static float s_manual_duty = 0.0f;
static float s_output_duty = 0.0f;
static float s_current_angle_deg = 0.0f;
static float s_target_angle_deg = 0.0f;
static float s_limit_deg = APP_POSITION_LIMIT_DEG;
static int32_t s_home_count = 0;
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

static float count_to_angle_deg(int32_t count)
{
    int32_t relative = count - s_home_count;
    return ((float)relative / (float)APP_ENCODER_CPR) * 360.0f;
}

static void apply_pwm_percent_channels(float duty_a_percent, float duty_b_percent)
{
    const uint32_t max_duty = (1U << LEDC_TIMER_13_BIT) - 1U;
    float clamped_a = clampf(duty_a_percent, APP_PWM_DUTY_MIN_PERCENT, APP_PWM_DUTY_MAX_PERCENT);
    float clamped_b = clampf(duty_b_percent, APP_PWM_DUTY_MIN_PERCENT, APP_PWM_DUTY_MAX_PERCENT);
    uint32_t duty_a = (uint32_t)((clamped_a / 100.0f) * (float)max_duty);
    uint32_t duty_b = (uint32_t)((clamped_b / 100.0f) * (float)max_duty);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_a);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty_b);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}

static void driver_task(void *arg)
{
    (void)arg;
    const int samples = APP_PWM_CARRIER_HZ / APP_PWM_SINE_HZ;
    uint32_t control_period_ms = (uint32_t)(1000 / APP_PWM_CARRIER_HZ);
    TickType_t control_delay;
    int idx = 0;

    float i_term = 0.0f;
    float prev_error = 0.0f;

    if (samples <= 0) {
        ESP_LOGE(TAG, "Invalid SPWM config: carrier=%dHz sine=%dHz", APP_PWM_CARRIER_HZ, APP_PWM_SINE_HZ);
        vTaskDelete(NULL);
    }

    if (control_period_ms == 0U) {
        control_period_ms = 1U;
    }
    control_delay = pdMS_TO_TICKS(control_period_ms);
    if (control_delay == 0) {
        control_delay = 1;
    }

    if ((APP_PWM_CARRIER_HZ % APP_PWM_SINE_HZ) != 0) {
        ESP_LOGW(TAG, "Carrier (%dHz) is not divisible by sine (%dHz), waveform may jitter", APP_PWM_CARRIER_HZ, APP_PWM_SINE_HZ);
    }

    while (true) {
        vTaskDelay(control_delay);

        bool enabled;
        bool pid_enabled;
        bool position_mode;
        bool dir_forward;
        float target_rps;
        float manual_duty;
        float target_angle_deg;
        float limit_deg;

        portENTER_CRITICAL(&s_ctrl_lock);
        enabled = s_enabled;
        pid_enabled = s_pid_enabled;
        position_mode = s_position_mode;
        dir_forward = s_direction_forward;
        target_rps = s_target_rps;
        manual_duty = s_manual_duty;
        target_angle_deg = s_target_angle_deg;
        limit_deg = s_limit_deg;
        portEXIT_CRITICAL(&s_ctrl_lock);

        int32_t count = encoder_get_count();
        float current_angle_deg = count_to_angle_deg(count);
        bool limit_hit = false;

        if (position_mode) {
            float clamped_target = clampf(target_angle_deg, -limit_deg, limit_deg);
            if (fabsf(clamped_target - target_angle_deg) > 0.001f) {
                limit_hit = true;
            }

            float pos_error = clamped_target - current_angle_deg;
            if (fabsf(pos_error) <= APP_POSITION_TOLERANCE_DEG) {
                target_rps = 0.0f;
            } else {
                dir_forward = (pos_error >= 0.0f);
                target_rps = clampf(fabsf(pos_error) * APP_POSITION_KP, 0.0f, APP_POSITION_MAX_RPS);
            }

        }

        float amplitude_percent = 0.0f;
        if (!enabled) {
            i_term = 0.0f;
            prev_error = 0.0f;
            amplitude_percent = 0.0f;
        } else if (pid_enabled) {
            const float dt_s = 1.0f / (float)APP_PWM_CARRIER_HZ;
            float measured_rps = encoder_get_speed_rps();
            float error = target_rps - measured_rps;
            i_term = clampf(i_term + (error * dt_s), APP_PID_I_MIN, APP_PID_I_MAX);
            float d_term = (error - prev_error) / dt_s;
            prev_error = error;
            amplitude_percent = clampf((APP_PID_KP * error) + (APP_PID_KI * i_term) + (APP_PID_KD * d_term),
                                       APP_PWM_DUTY_MIN_PERCENT,
                                       APP_PWM_DUTY_MAX_PERCENT);
        } else {
            amplitude_percent = clampf(manual_duty, APP_PWM_DUTY_MIN_PERCENT, APP_PWM_DUTY_MAX_PERCENT);
        }

        if (position_mode && !enabled) {
            amplitude_percent = 0.0f;
        }

        float phase = 2.0f * kPi * ((float)idx / (float)samples);
        float phase_shift = dir_forward ? (kPi * 0.5f) : (-kPi * 0.5f);
        float samp_a = (sinf(phase) + 1.0f) * 0.5f;
        float samp_b = (sinf(phase + phase_shift) + 1.0f) * 0.5f;

        if (limit_hit) {
            amplitude_percent = 0.0f;
        }

        apply_pwm_percent_channels(amplitude_percent * samp_a, amplitude_percent * samp_b);

        portENTER_CRITICAL(&s_ctrl_lock);
        s_current_angle_deg = current_angle_deg;
        s_target_rps = target_rps;
        s_direction_forward = dir_forward;
        s_limit_hit = limit_hit;
        s_output_duty = amplitude_percent;
        portEXIT_CRITICAL(&s_ctrl_lock);

        idx = (idx + 1) % samples;
    }
}

esp_err_t driver_init(void)
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
        .gpio_num = APP_PIN_DRIVER_PWM_A,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_cfg));

    ledc_channel_config_t ch_cfg_b = {
        .channel = LEDC_CHANNEL_1,
        .duty = 0,
        .gpio_num = APP_PIN_DRIVER_PWM_B,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_cfg_b));

    xTaskCreate(driver_task, "driver", 4096, NULL, 6, NULL);
    ESP_LOGI(TAG, "Driver initialized on GPIOs %d and %d", APP_PIN_DRIVER_PWM_A, APP_PIN_DRIVER_PWM_B);
    return ESP_OK;
}

void driver_set_enabled(bool enabled)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_enabled = enabled;
    portEXIT_CRITICAL(&s_ctrl_lock);
}

void driver_set_pid_enabled(bool enabled)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_pid_enabled = enabled;
    portEXIT_CRITICAL(&s_ctrl_lock);
}

void driver_set_position_mode(bool enabled)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_position_mode = enabled;
    portEXIT_CRITICAL(&s_ctrl_lock);
}

void driver_set_target_rps(float target_rps)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_target_rps = target_rps;
    s_position_mode = false;
    portEXIT_CRITICAL(&s_ctrl_lock);
}

void driver_set_manual_pwm(float duty_percent)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_manual_duty = duty_percent;
    portEXIT_CRITICAL(&s_ctrl_lock);
}

void driver_set_direction(bool forward)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_direction_forward = forward;
    portEXIT_CRITICAL(&s_ctrl_lock);
}

void driver_set_home(void)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_home_count = encoder_get_count();
    s_target_angle_deg = 0.0f;
    s_current_angle_deg = 0.0f;
    s_limit_hit = false;
    portEXIT_CRITICAL(&s_ctrl_lock);
}

void driver_set_target_angle_deg(float target_deg)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_target_angle_deg = clampf(target_deg, -s_limit_deg, s_limit_deg);
    s_position_mode = true;
    s_limit_hit = (s_target_angle_deg != target_deg);
    portEXIT_CRITICAL(&s_ctrl_lock);
}

void driver_set_limit_deg(float limit_deg)
{
    portENTER_CRITICAL(&s_ctrl_lock);
    s_limit_deg = (limit_deg > 0.0f) ? limit_deg : APP_POSITION_LIMIT_DEG;
    s_target_angle_deg = clampf(s_target_angle_deg, -s_limit_deg, s_limit_deg);
    portEXIT_CRITICAL(&s_ctrl_lock);
}

bool driver_is_enabled(void)
{
    bool value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_enabled;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

bool driver_is_pid_enabled(void)
{
    bool value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_pid_enabled;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

bool driver_is_position_mode(void)
{
    bool value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_position_mode;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

bool driver_get_direction(void)
{
    bool value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_direction_forward;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

bool driver_is_limit_hit(void)
{
    bool value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_limit_hit;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

float driver_get_target_rps(void)
{
    float value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_target_rps;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

float driver_get_output_duty(void)
{
    float value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_output_duty;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

float driver_get_current_angle_deg(void)
{
    float value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_current_angle_deg;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

float driver_get_target_angle_deg(void)
{
    float value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_target_angle_deg;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}

float driver_get_limit_deg(void)
{
    float value;
    portENTER_CRITICAL(&s_ctrl_lock);
    value = s_limit_deg;
    portEXIT_CRITICAL(&s_ctrl_lock);
    return value;
}