#include "take_photo.hpp"
#include "esp_camera.h"
#include "esp_err.h"
#include "esp_log.h"
#include "http_api.hpp"

static const char *TAG = "take_photo";

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static QueueHandle_t xQueueEvent = NULL;
static take_photo_state_t gEvent = IDLE;

static SemaphoreHandle_t xMutex;

static bool flip = false;

static void flip_horizontal(camera_fb_t *frame) {
    // TODO: error doing
    uint8_t *buf = (uint8_t *)malloc(frame->width * frame->height * 3);
    for (int y = 0; y < frame->height; y++) {
        for (int x = 0; x < frame->width; x++) {
            buf[y * 3 * frame->width + 3 * x] =
                frame->buf[(y + 1) * 3 * frame->width - 3 * x - 3];
            buf[y * 3 * frame->width + 3 * x + 1] =
                frame->buf[(y + 1) * 3 * frame->width - 3 * x - 2];
            buf[y * 3 * frame->width + 3 * x + 2] =
                frame->buf[(y + 1) * 3 * frame->width - 3 * x - 1];
        }
    }
    free(frame->buf);
    frame->buf = buf;
}

static void task_process_handler(void *arg) {
    camera_fb_t *frame = NULL;
    take_photo_state_t _gEvent;

    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    int dire = 0; // out

    while (true) {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        _gEvent = gEvent;
        gEvent = IDLE;
        xSemaphoreGive(xMutex);

        if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY)) {
            if (flip)
                flip_horizontal(frame);

            switch (_gEvent) {
            case CAPTURE_OUT: // 出去车辆拍照
                ets_printf("%s %d\n", TAG, _gEvent);
                dire = 1;
            case CAPTURE_IN: // 进入车辆拍照
                if (frame->format == PIXFORMAT_JPEG) {
                    _jpg_buf = frame->buf;
                    _jpg_buf_len = frame->len;
                } else if (frame2jpg(frame, 80, &_jpg_buf, &_jpg_buf_len)) {
                    http_api_hander(dire, _jpg_buf, _jpg_buf_len);
                } else {
                    ESP_LOGE(TAG, "JPEG compression failed");
                }
            default:
                break;
            }
        }

        // 释放或者放入下一个队列
        if (xQueueFrameO) {
            xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
        } else {
            free(frame);
        }
    }
}

static void task_event_handler(void *arg) {
    take_photo_state_t _gEvent;
    while (true) {
        xQueueReceive(xQueueEvent, &(_gEvent), portMAX_DELAY);
        xSemaphoreTake(xMutex, portMAX_DELAY);
        gEvent = _gEvent;
        xSemaphoreGive(xMutex);
    }
}

void register_take_photo(const QueueHandle_t frame_i, const QueueHandle_t event,
                         const QueueHandle_t frame_o) {
    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    xQueueEvent = event;
    xMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 5, NULL,
                            1);
    if (xQueueEvent)
        xTaskCreatePinnedToCore(task_event_handler, TAG, 4 * 1024, NULL, 5,
                                NULL, 1);
}