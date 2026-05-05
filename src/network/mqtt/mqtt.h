#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t mqtt_start(void);
esp_err_t mqtt_stop(void);
bool mqtt_is_up(void);