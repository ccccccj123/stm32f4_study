//
// Created by cj on 2026/5/14.
//

#ifndef STM32F4_WS2812_HPP
#define STM32F4_WS2812_HPP

#include "main.h"
#include <cstdint>

class Ws2812 {
public:
    static constexpr uint8_t LED_COUNT = 3;

    explicit Ws2812(TIM_HandleTypeDef *timer);

    void setPixel(uint8_t index, uint8_t red, uint8_t green, uint8_t blue);
    void clear();
    void show();

    void onDmaFinished(TIM_HandleTypeDef *timer);

private:
    struct Color {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    static constexpr uint32_t TIMER_CHANNEL = TIM_CHANNEL_4;
    static constexpr uint32_t BIT_0_DUTY = 30;
    static constexpr uint32_t BIT_1_DUTY = 60;
    static constexpr uint32_t RESET_SLOTS = 50;
    static constexpr uint32_t BUFFER_SIZE = LED_COUNT * 24 + RESET_SLOTS;

    void encode();

    TIM_HandleTypeDef *timer_;
    Color pixels_[LED_COUNT]{};
    uint32_t pwm_buffer_[BUFFER_SIZE]{};
    volatile bool dma_finished_ = true;
};

#endif //STM32F4_WS2812_HPP
