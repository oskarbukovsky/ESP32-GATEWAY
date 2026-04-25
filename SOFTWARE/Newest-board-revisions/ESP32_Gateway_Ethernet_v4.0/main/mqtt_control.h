#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t mqtt_control_start(void);
bool mqtt_control_is_started(void);
