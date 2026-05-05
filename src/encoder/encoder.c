#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "app_config.h"
#include "encoder/encoder.h"

static const char *TAG = "encoder";

static volatile int32_t s_count = 0;
static volatile bool s_index_seen = false;
static volatile uint8_t s_prev_ab = 0;
static float s_speed_rps = 0.0f;
static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;

static const int8_t s_qdec_lut[16] = {
    0, -1, +1, 0,
    +1, 0, 0, -1,
    -1, 0, 0, +1,
    0, +1, -1, 0,
};

static inline uint8_t read_ab_state(void)
{
    uint8_t a = (uint8_t)gpio_get_level(APP_PIN_ENCODER_A);
    uint8_t b = (uint8_t)gpio_get_level(APP_PIN_ENCODER_B);
    return (uint8_t)((a << 1) | b);
}

static void IRAM_ATTR encoder_ab_isr(void *arg)
{
    (void)arg;
    uint8_t curr = read_ab_state();
    uint8_t idx = (uint8_t)((s_prev_ab << 2) | curr);

    portENTER_CRITICAL_ISR(&s_lock);
    s_count += s_qdec_lut[idx];
    s_prev_ab = curr;
    portEXIT_CRITICAL_ISR(&s_lock);
}

static void IRAM_ATTR encoder_index_isr(void *arg)
{
    (void)arg;
    portENTER_CRITICAL_ISR(&s_lock);
    s_index_seen = true;
    portEXIT_CRITICAL_ISR(&s_lock);
}

static void speed_task(void *arg)
{
    (void)arg;
    int32_t last_count = encoder_get_count();
    const float dt_s = (float)APP_ENCODER_SPEED_PERIOD_MS / 1000.0f;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(APP_ENCODER_SPEED_PERIOD_MS));
        int32_t now_count = encoder_get_count();
        int32_t delta = now_count - last_count;
        last_count = now_count;

        float speed = ((float)delta / (float)APP_ENCODER_CPR) / dt_s;

        portENTER_CRITICAL(&s_lock);
        s_speed_rps = speed;
        portEXIT_CRITICAL(&s_lock);
    }
}

esp_err_t encoder_init(void)
{
    gpio_config_t io_cfg = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << APP_PIN_ENCODER_A) | (1ULL << APP_PIN_ENCODER_B),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_cfg));

#if APP_ENCODER_USE_INDEX
    gpio_config_t idx_cfg = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << APP_PIN_ENCODER_INDEX),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&idx_cfg));
#endif

    s_prev_ab = read_ab_state();

    esp_err_t isr_install = gpio_install_isr_service(0);
    if (isr_install != ESP_OK && isr_install != ESP_ERR_INVALID_STATE) {
        return isr_install;
    }

    ESP_ERROR_CHECK(gpio_isr_handler_add(APP_PIN_ENCODER_A, encoder_ab_isr, NULL));
    ESP_ERROR_CHECK(gpio_isr_handler_add(APP_PIN_ENCODER_B, encoder_ab_isr, NULL));
#if APP_ENCODER_USE_INDEX
    ESP_ERROR_CHECK(gpio_isr_handler_add(APP_PIN_ENCODER_INDEX, encoder_index_isr, NULL));
#endif

    xTaskCreate(speed_task, "enc_speed", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "Encoder initialized (PPR=%d, CPR=%d)", APP_ENCODER_PPR, APP_ENCODER_CPR);
    return ESP_OK;
}

int32_t encoder_get_count(void)
{
    int32_t value;
    portENTER_CRITICAL(&s_lock);
    value = s_count;
    portEXIT_CRITICAL(&s_lock);
    return value;
}

float encoder_get_speed_rps(void)
{
    float value;
    portENTER_CRITICAL(&s_lock);
    value = s_speed_rps;
    portEXIT_CRITICAL(&s_lock);
    return value;
}

void encoder_reset_count(void)
{
    portENTER_CRITICAL(&s_lock);
    s_count = 0;
    s_speed_rps = 0.0f;
    s_index_seen = false;
    portEXIT_CRITICAL(&s_lock);
}

bool encoder_is_index_seen(void)
{
    bool seen;
    portENTER_CRITICAL(&s_lock);
    seen = s_index_seen;
    portEXIT_CRITICAL(&s_lock);
    return seen;
}