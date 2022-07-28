#pragma once

#include "esp_http_client.h"
#include "esp_log.h"

#define ESP_SERVER_HOST CONFIG_ESP_SERVER_HOST
#define ESP_SERVER_PORT CONFIG_ESP_SERVER_PORT
#define ESP_SERVER_PATH CONFIG_ESP_SERVER_PATH

/**
 * @brief
 *
 * @param dire  车辆的触发方向
 * @param jpg_buf 图片缓冲
 * @param jpg_size  图片大小
 */

void http_api(int dire, uint8_t *jpg_buf, size_t jpg_size);