#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t network_init(void);
bool network_is_link_up(void);
bool network_is_mqtt_up(void);