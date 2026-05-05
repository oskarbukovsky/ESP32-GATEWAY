#include "mqtt_config.h"
#include "esp_log.h"

static const char *TAG = "mqtt_config";

void mqtt_config_init(void)
{
    /* Placeholder: initialize NVS or config storage here. For now, no-op. */
    ESP_LOGD(TAG, "mqtt_config_init (stub)");
}

void mqtt_config_get_credentials(const char **username, const char **password)
{
    /* Placeholder: attempt to read stored credentials from NVS.
       Return NULLs to indicate use of compile-time defaults in app_config.h.
       Implement NVS reads here when ready.
    */
    if (username) *username = NULL;
    if (password) *password = NULL;
}
