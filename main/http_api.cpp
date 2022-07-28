#include "http_api.hpp"

static const char *TAG = "esp_http_client";

static const char *body_header =
    "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: "
    "form-data; name=\"file\"; filename=\"img.jpg\"\r\nContent-Type: "
    "image/jpeg\r\n\r\n";
static const char *boundary =
    "\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";


/* http client  */
esp_err_t _http_event_handle(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
        printf("%.*s", evt->data_len, (char *)evt->data);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client)) {
            printf("%.*s\n", evt->data_len, (char *)evt->data);
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}

static int dire;
static uint8_t *_jpg_buf;
static size_t _jpg_size;

static void http_api_hander(void *arg) {
    int body_header_size = strlen(body_header);
    int boundary_size = strlen(boundary);

    char *body_data = (char *)malloc(1024 + _jpg_size);
    memset(body_data, 0, 1024 + _jpg_size);
    // 设置方向
    char *query = (char *)malloc(128);
    sprintf(query, "dire=%d", dire);
    ets_printf("%s\n", query);

    esp_http_client_config_t config = {
        .host = ESP_SERVER_HOST,    // default "192.168.8.125"
        .port = ESP_SERVER_PORT,    // default 6123
        .path = ESP_SERVER_PATH,    // default "/api/v1/services/test"
        .query = query,             // "dire=%d", dire
        .method = HTTP_METHOD_POST, //
        .event_handler = _http_event_handle,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // http header
    esp_http_client_set_header(
        client, "Content-Type",
        "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");
    // http body
    memcpy(body_data, body_header, body_header_size);
    memcpy(body_data + body_header_size, _jpg_buf, _jpg_size);
    memcpy(body_data + body_header_size + _jpg_size, boundary, boundary_size);
    esp_http_client_set_post_field(client, body_data,
                                   body_header_size + boundary_size + _jpg_size);
    // 发送请求
    esp_err_t err = esp_http_client_perform(client);
    // 请求结果
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    esp_http_client_cleanup(client);
    free(body_data);
    free(query);

    vTaskDelete(NULL);
}

void http_api(int dire, uint8_t *jpg_buf, size_t jpg_size) {
    // _jpg_buf = (uint8_t *)malloc(jpg_size);
    // memccpy(_jpg_buf, jpg_buf, jpg_size);
    _jpg_buf = jpg_buf;
    _jpg_size = jpg_size;
    xTaskCreatePinnedToCore(http_api_hander, TAG, 4 * 1024, NULL, 5, NULL, 1);
}