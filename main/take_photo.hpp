#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

typedef enum {
    CAPTURE_IN = 0,
    CAPTURE_OUT,
    IDLE
} take_photo_state_t;

void register_take_photo(const QueueHandle_t frame_i, const QueueHandle_t event,
                         const QueueHandle_t frame_o);
