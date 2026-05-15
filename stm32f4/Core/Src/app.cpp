//
// Created by cj on 2026/5/14.
//

#include "app.h"
#include "main.h"
#include "cmsis_os.h"
#include "app.h"
#include "ws2812.hpp"

extern TIM_HandleTypeDef htim5;

class Led
{
public:
    Led(GPIO_TypeDef *port, uint16_t pin) : port_(port), pin_(pin) {}

    void toggle()
        {
            HAL_GPIO_TogglePin(port_, pin_);
        }

private:
    GPIO_TypeDef* port_;
    uint16_t pin_;
};

static Led led(GPIOB,GPIO_PIN_2);

static void led_task(void const * argument)
{
    (void)argument;

    while (true)
    {
        led.toggle();
        osDelay(300);
    }
}

static Ws2812 rgb(&htim5);

static void rgb_task(void const *argument)
{
    (void)argument;

    osDelay(100);

    while (true) {
        rgb.clear();
        rgb.setPixel(0, 128, 0, 0);
        rgb.setPixel(1, 0, 128, 0);
        rgb.setPixel(2, 0, 0, 64);
    }
}

extern "C" void app_start(void)
{
    osThreadDef(ledTask, led_task, osPriorityNormal, 0, 128);
    osThreadCreate(osThread(ledTask), nullptr);
    osThreadDef(rgbTask, rgb_task, osPriorityNormal, 0, 256);
    osThreadCreate(osThread(rgbTask), nullptr);
}
