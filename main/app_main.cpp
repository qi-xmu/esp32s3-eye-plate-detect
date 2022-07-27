#include "app_httpd.hpp"
#include "app_mdns.h"
#include "app_wifi.h"
#include "who_adc_button.h"
#include "who_button.h"
#include "who_camera.h"
#include "who_lcd.h"

#include <stdio.h>
#include <string.h>

#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "event_logic.hpp"
#include "take_photo.hpp"

static const char *TAG = "HTTP_CLIENT";
#define MAX_HTTP_OUTPUT_BUFFER 2048

static QueueHandle_t xQueueFrame = NULL;
static QueueHandle_t xQueueLCDFrame = NULL;
static QueueHandle_t xQueueADCKeyState = NULL;
static QueueHandle_t xQueueGPIOKeyState = NULL;
static QueueHandle_t xQueueEventLogic = NULL;
static button_adc_config_t buttons[4] = {
    {1, 2800, 3000}, {2, 2250, 2450}, {3, 300, 500}, {4, 850, 1050}};

#define GPIO_BOOT GPIO_NUM_0

extern "C" void app_main() {
    app_wifi_main();
    app_mdns_main();

    wifi_ap_record_t ap_info;
    while (esp_wifi_sta_get_ap_info(&ap_info) != ESP_OK) {
        ESP_LOGI(TAG, "Haven't to connect to a suitable AP now!");
        vTaskDelay(100);
    }
    // 确定连接wifi
    ESP_LOGI(TAG, "连接wifi %s", ap_info.ssid);

    gpio_config_t gpio_conf;
    gpio_conf.mode = GPIO_MODE_OUTPUT_OD;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.pin_bit_mask = 1LL << GPIO_NUM_1;
    gpio_config(&gpio_conf);

    xQueueFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));

    xQueueADCKeyState = xQueueCreate(1, sizeof(int));
    xQueueGPIOKeyState = xQueueCreate(1, sizeof(int));
    xQueueEventLogic = xQueueCreate(1, sizeof(int));

    register_camera(PIXFORMAT_RGB565, FRAMESIZE_240X240, 2, xQueueFrame);

    register_adc_button(buttons, 4, xQueueADCKeyState);
    register_button(GPIO_BOOT, xQueueGPIOKeyState);
    register_event(xQueueADCKeyState, xQueueGPIOKeyState, xQueueEventLogic);
    register_take_photo(xQueueFrame, xQueueEventLogic, xQueueLCDFrame);
    
    register_lcd(xQueueLCDFrame, NULL, true);
}
