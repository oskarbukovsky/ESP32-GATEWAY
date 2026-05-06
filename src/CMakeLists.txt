idf_component_register(SRCS
                       "main.c"
                       "encoder/encoder.c"
                       "driver/driver.c"
                       "network/ethernet/ethernet.c"
                       "network/mqtt/mqtt.c"
                       "network/mqtt/mqtt_config.c"
                       "network/network.c"
                       INCLUDE_DIRS "."
                       "encoder"
                       "driver"
                       "network/ethernet"
                       "network/mqtt"
                       "network"
                       REQUIRES driver esp_netif esp_eth mqtt lwip)

