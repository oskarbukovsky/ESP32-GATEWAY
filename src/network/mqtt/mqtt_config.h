#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void mqtt_config_init(void);
void mqtt_config_get_credentials(const char **username, const char **password);

#ifdef __cplusplus
}
#endif
