idf_component_register(SRCS "stream_reader.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES driver esp_wifi nvs_flash)
