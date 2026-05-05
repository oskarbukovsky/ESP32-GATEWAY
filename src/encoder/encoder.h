#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t encoder_init(void);
int32_t encoder_get_count(void);
float encoder_get_speed_rps(void);
void encoder_reset_count(void);
bool encoder_is_index_seen(void);