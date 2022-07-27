#pragma once

/**
 * @brief
 *
 * @param dire  车辆的触发方向
 * @param jpg_buf 图片缓冲
 * @param jpg_size  图片大小
 */

#define ESP_SERVER_HOST CONFIG_ESP_SERVER_HOST
#define ESP_SERVER_PORT CONFIG_ESP_SERVER_PORT
#define ESP_SERVER_PATH CONFIG_ESP_SERVER_PATH
esp_err_t http_api_hander(int dire, uint8_t *jpg_buf, size_t jpg_size);