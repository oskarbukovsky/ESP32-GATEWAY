#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t rotor_encoder_init(void);
int32_t rotor_encoder_get_count(void);
float rotor_encoder_get_speed_rps(void);
void rotor_encoder_reset_count(void);
bool rotor_encoder_is_index_seen(void);
