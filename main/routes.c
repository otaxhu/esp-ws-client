#include <esp_http_server.h>
#include <esp_log.h>
#include <cJSON.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp_check.h>

static const char *TAG = "routes";

struct async_ws_data {
    int fd;
    httpd_handle_t handle;
    cJSON *data;
};

#define __ROUTES_MAX_WS_CLIENTS 10

static bool led_level = false;

static void handle_async_ws(void *args) {
    struct async_ws_data *data = (struct async_ws_data*)args;
    httpd_ws_frame_t frame = { 0 };
    frame.type = HTTPD_WS_TYPE_TEXT;
    if (cJSON_IsFalse(data->data)) {
        led_level = false;
        frame.payload = (uint8_t*)"false";
        frame.len = 5;
    } else if (cJSON_IsTrue(data->data)) {
        led_level = true;
        frame.payload = (uint8_t*)"true";
        frame.len = 4;
    } else if (cJSON_IsObject(data->data)) {
        if (led_level) {
            frame.payload = (uint8_t*)"true";
            frame.len = 4;
        } else {
            frame.payload = (uint8_t*)"false";
            frame.len = 5;
        }
    } else {
        ESP_LOGE(TAG, "invalid json");
        goto exit;
    }
    gpio_set_level(2, led_level);
    httpd_ws_send_frame_async(data->handle, data->fd, &frame);
exit:
    cJSON_Delete(data->data);
    free(data);
}

esp_err_t handle_ping(httpd_req_t *r) {
    if (r->method == HTTP_GET) {
        return ESP_OK;
    }
    httpd_ws_frame_t frame = {0};
    esp_err_t err = httpd_ws_recv_frame(r, &frame, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "error ocurred during getting WS payload lenght");
        return err;
    }
    char ws_content[frame.len];
    frame.payload = (uint8_t*)ws_content;
    err = httpd_ws_recv_frame(r, &frame, frame.len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "error ocurred during getting WS payload content");
        return err;
    }
    cJSON *json = cJSON_ParseWithLength((const char *)frame.payload, frame.len);
    if (json == NULL) {
        ESP_LOGE(TAG, "error ocurred during parsing json");
        return ESP_FAIL;
    }
    struct async_ws_data *data = malloc(sizeof(struct async_ws_data));
    if (data == NULL) {
        goto fail;
    }
    data->fd = httpd_req_to_sockfd(r);
    if (data->fd == -1) {
        ESP_LOGE(TAG, "fd is -1");
        goto fail;
    }
    data->data = json;
    data->handle = r->handle;
    err = httpd_queue_work(r->handle, handle_async_ws, data);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "error during queuing of request");
    fail:
        cJSON_Delete(json);
        free(data);
        return ESP_FAIL;
    }
    return ESP_OK;
}

extern const uint8_t index_html_file[] asm("_binary_index_html_start");

esp_err_t handle_webpage(httpd_req_t *r) {
    return httpd_resp_send(r, (const char*)index_html_file, -1);
}
