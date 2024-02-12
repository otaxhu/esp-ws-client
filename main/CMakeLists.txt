idf_component_register(SRCS "wifi.c" "routes.c" "main.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_http_server
                                esp-cchi-router
                                nvs-dotenv
                                esp_wifi
                                nvs_flash
                                json
                                driver
                       EMBED_TXTFILES assets/index.html)
