#include <esp_http_server.h>
// #include <esp_cchi.h>
#include <main/routes.h>
#include <main/wifi.h>
#include <esp_wifi.h>
#include <nvs_dotenv.h>
#include <nvs_flash.h>
#include <driver/gpio.h>



httpd_uri_t ping_uri = {
    .handler = handle_ping,
    .method = HTTP_GET,
    .uri = "/toggle_led",
    .is_websocket = true
};

httpd_uri_t webpage_uri = {
    .handler = handle_webpage,
    .method = HTTP_GET,
    .uri = "/",
};

void app_main(void) {
    
    ESP_ERROR_CHECK(nvs_dotenv_load());
    printf("%s\n", getenv("WIFI_SSID"));
    printf("%s\n", getenv("WIFI_PASSWORD"));
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    wifi_init_sta();
    httpd_config_t hd_cfg = HTTPD_DEFAULT_CONFIG();
    // ESP_ERROR_CHECK(esp_cchi_setup_hd_config(&hd_cfg));
    // ESP_ERROR_CHECK(esp_cchi_setup_hd_uri(&ping_uri));
    httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(httpd_start(&server, &hd_cfg));
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &ping_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &webpage_uri));
    vTaskDelete(NULL);
}
