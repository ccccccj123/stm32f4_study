//
// Created by cj on 2026/5/14.
//

#include "ws2812.hpp"
#include "cmsis_os.h"

static Ws2812 *active_ws2812 = nullptr;

Ws2812::Ws2812(TIM_HandleTypeDef *timer) : timer_(timer)
{
    active_ws2812 = this;
}

void Ws2812::setPixel(uint8_t index, uint8_t red, uint8_t green, uint8_t blue)
{
    if (index >= LED_COUNT) {
        return;
    }

    pixels_[index].red = red;
    pixels_[index].green = green;
    pixels_[index].blue = blue;
}

void Ws2812::clear()
{
    for (uint8_t i = 0; i < LED_COUNT; ++i) {
        setPixel(i, 0, 0, 0);
    }
}

void Ws2812::show()
{
    encode();

    HAL_TIM_PWM_Stop_DMA(timer_, TIMER_CHANNEL);
    __HAL_TIM_SET_COMPARE(timer_, TIMER_CHANNEL, 0);
    osDelay(1);

    dma_finished_ = false;

    HAL_StatusTypeDef status = HAL_TIM_PWM_Start_DMA(
        timer_,
        TIMER_CHANNEL,
        pwm_buffer_,
        BUFFER_SIZE
    );

    if (status != HAL_OK) {
        dma_finished_ = true;
        return;
    }

    uint32_t timeout = 10;
    while (!dma_finished_ && timeout > 0) {
        osDelay(1);
        --timeout;
    }

    HAL_TIM_PWM_Stop_DMA(timer_, TIMER_CHANNEL);
    __HAL_TIM_SET_COMPARE(timer_, TIMER_CHANNEL, 0);
}

void Ws2812::encode()
{
    uint32_t pos = 0;

    for (uint8_t i = 0; i < LED_COUNT; ++i) {
        uint8_t color[3] = {
            pixels_[i].blue,
            pixels_[i].green,
            pixels_[i].red
        };

        for (uint8_t c = 0; c < 3; ++c) {
            for (int8_t bit = 7; bit >= 0; --bit) {
                pwm_buffer_[pos++] = (color[c] & (1U << bit)) ? BIT_1_DUTY : BIT_0_DUTY;
            }
        }
    }

    while (pos < BUFFER_SIZE) {
        pwm_buffer_[pos++] = 0;
    }
}

void Ws2812::onDmaFinished(TIM_HandleTypeDef *timer)
{
    if (timer->Instance == TIM5) {
        dma_finished_ = true;
    }
}

extern "C" void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (active_ws2812 != nullptr) {
        active_ws2812->onDmaFinished(htim);
    }
}
