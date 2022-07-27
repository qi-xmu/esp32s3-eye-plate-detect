#include <stdio.h>
#include "event_logic.hpp"
#include "who_button.h"
#include "take_photo.hpp"

typedef enum
{
    MENU = 1,
    PLAY,
    UP,
    DOWN
} key_name_t;

static QueueHandle_t xQueueKeyStateIGPIO = NULL;
static QueueHandle_t xQueueKeyStateIADC = NULL;
static QueueHandle_t xQueueEventO = NULL;
static key_state_t key_state;
static key_name_t adc_button_name;
static take_photo_state_t detector_state;

void event_generate_from_gpio_button(void *arg)
{
    take_photo_state_t mode = CAPTURE_IN;
    while (1)
    {
        xQueueReceive(xQueueKeyStateIGPIO, &key_state, portMAX_DELAY);
        xQueueSend(xQueueEventO, &mode, portMAX_DELAY);
    }
}

void event_generate_from_adc_button(void *arg)
{
    while (1)
    {
        xQueueReceive(xQueueKeyStateIADC, &adc_button_name, portMAX_DELAY);
        switch (adc_button_name)
        {
        case MENU:
            detector_state = CAPTURE_IN;
            break;

        case PLAY:
            detector_state = CAPTURE_IN;
            break;

        case UP:
            detector_state = CAPTURE_OUT;
            break;

        case DOWN:
            detector_state = CAPTURE_OUT;
            break;

        default:
            detector_state = CAPTURE_IN;
            break;
        }
        xQueueSend(xQueueEventO, &detector_state, portMAX_DELAY);
    }
}

void register_event(const QueueHandle_t key_state_i_adc, const QueueHandle_t key_state_i_gpio, const QueueHandle_t event_o)
{
    xQueueKeyStateIADC = key_state_i_adc;
    xQueueKeyStateIGPIO = key_state_i_gpio;
    xQueueEventO = event_o;
    if (xQueueKeyStateIADC != NULL)
        xTaskCreatePinnedToCore(event_generate_from_adc_button, "event_logic_task1", 1024, NULL, 5, NULL, 0);
    if (xQueueKeyStateIGPIO != NULL)
        xTaskCreatePinnedToCore(event_generate_from_gpio_button, "event_logic_task2", 1024, NULL, 5, NULL, 0);
}