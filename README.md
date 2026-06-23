# STM32F4 学习工程

这是一个基于立创天空星 STM32F4 的嵌入式学习项目，主板使用立创筑基星开发板。工程以 STM32CubeMX 生成代码为基础，使用 STM32 HAL、FreeRTOS、CMake 和少量 C++ 业务代码，当前主要用于熟悉 STM32F4 外设配置、RTOS 任务组织、定时器 PWM、DMA 以及 WS2812 RGB 灯带驱动。

## 项目定位

本项目不是一次性完成的成品应用，而是一个持续推进的 STM32F4 学习仓库。当前目标是先把开发板跑起来，逐步沉淀外设驱动、RTOS 任务模型和工程化构建方式，后续再扩展到更多传感器、通信接口和小型综合应用。

硬件基础：

- 核心/主控：立创天空星 STM32F4，当前 CubeMX 目标芯片为 `STM32F407V(E-G)Tx`
- 开发板：立创筑基星开发板
- 当前已用 GPIO：`PB2`，普通推挽输出，用于 LED 翻转测试
- 当前已用 PWM 引脚：`PA3`，复用为 `TIM5_CH4`，用于 WS2812 数据输出
- 当前系统时钟：HSI + PLL，`SYSCLK = 84 MHz`

软件基础：

- STM32 HAL Driver
- CMSIS
- FreeRTOS，CMSIS-OS v1 接口
- CMake + Ninja 构建
- `arm-none-eabi-gcc` 工具链
- 可选使用 `pyOCD` 下载固件

## 当前项目程度

### 1. 基础工程已经搭好

`stm32f4/` 目录是主要固件工程，包含 CubeMX 生成的启动文件、链接脚本、HAL 驱动、CMSIS、FreeRTOS 和 CMake 构建配置。

当前工程已经完成：

- STM32F407 工程框架生成
- 启动文件 `startup_stm32f407xx.s`
- 链接脚本 `STM32F407XX_FLASH.ld`
- HAL 初始化流程
- 系统时钟配置
- GPIO 初始化
- DMA 初始化
- TIM5 PWM 初始化
- FreeRTOS 调度器启动
- CMake Preset 构建入口

工程入口仍然保持 CubeMX 的典型结构：

- `Core/Src/main.c`：芯片初始化、外设初始化、RTOS 启动
- `Core/Src/freertos.c`：FreeRTOS 相关生成代码
- `Core/Src/app.cpp`：用户应用层逻辑
- `Core/Src/ws2812.cpp`：WS2812 驱动实现

### 2. 已经引入 C++ 应用层

项目不是完全停留在 C 语言 CubeMX 模板上，当前已经通过 `app.h` / `app.cpp` 增加了 C++ 应用层，并通过 `extern "C"` 暴露 `app_start()` 给 `main.c` 调用。

目前 `app_start()` 会创建两个用户任务：

- `ledTask`：周期翻转 `PB2`
- `rgbTask`：准备 WS2812 RGB 像素数据

这种结构比较适合作为后续学习项目的基础：CubeMX 负责底层初始化，用户逻辑集中放在 `app.cpp` 和自定义驱动模块中，避免把业务代码塞进 `main.c`。

### 3. FreeRTOS 已经跑起来

当前启用了 FreeRTOS，并使用 CMSIS-OS v1 风格 API：

- 默认任务 `defaultTask`
- 用户任务 `ledTask`
- 用户任务 `rgbTask`

`ledTask` 每 300 ms 翻转一次 `PB2`，可以作为系统是否正常运行、调度器是否启动的基础验证点。

当前任务栈大小：

- `defaultTask`：128 words
- `ledTask`：128 words
- `rgbTask`：256 words

后续如果驱动和业务逻辑变复杂，需要继续观察任务栈余量，并考虑增加调试手段。

### 4. WS2812 驱动已经有雏形

当前已经实现了一个 `Ws2812` C++ 类，核心思路是：

- 使用 `TIM5_CH4` 输出 PWM 波形
- 使用 `DMA1_Stream1` 将 PWM 占空比序列搬运到定时器比较寄存器
- 用缓冲区编码 RGB 数据
- DMA 传输完成后通过 `HAL_TIM_PWM_PulseFinishedCallback()` 回调通知驱动

相关文件：

- `Core/Inc/ws2812.hpp`
- `Core/Src/ws2812.cpp`

当前配置：

- LED 数量：`3`
- 定时器通道：`TIM_CHANNEL_4`
- `0` 码占空比：`30`
- `1` 码占空比：`60`
- reset 空槽：`50`
- PWM 周期：`104`

需要注意：当前 `rgb_task()` 中已经设置了 3 个像素的颜色，但还没有调用 `rgb.show()`，因此 WS2812 刷新流程还没有真正闭环。也就是说，驱动框架已经写好，但 RGB 实际显示效果还需要继续验证和完善。

### 5. 构建系统已经具备基础可用性

工程提供了 CMake 配置和 Preset：

- `Debug`
- `Release`

并且在 `CMakeLists.txt` 中加入了 `pyOCD` 下载目标。如果本机已经安装 `pyocd`，可以通过 CMake 自定义目标进行烧录。

## 目录结构

```text
.
├── README.md
└── stm32f4
    ├── CMakeLists.txt
    ├── CMakePresets.json
    ├── STM32F407XX_FLASH.ld
    ├── startup_stm32f407xx.s
    ├── stm32f4.ioc
    ├── Core
    │   ├── Inc
    │   │   ├── app.h
    │   │   ├── main.h
    │   │   └── ws2812.hpp
    │   └── Src
    │       ├── app.cpp
    │       ├── main.c
    │       └── ws2812.cpp
    ├── Drivers
    │   ├── CMSIS
    │   └── STM32F4xx_HAL_Driver
    ├── Middlewares
    │   └── Third_Party
    │       └── FreeRTOS
    └── cmake
        ├── gcc-arm-none-eabi.cmake
        ├── starm-clang.cmake
        └── stm32cubemx
```

## 构建方法

进入固件工程目录：

```powershell
cd stm32f4
```

配置 Debug 工程：

```powershell
cmake --preset Debug
```

编译 Debug 固件：

```powershell
cmake --build --preset Debug
```

配置 Release 工程：

```powershell
cmake --preset Release
```

编译 Release 固件：

```powershell
cmake --build --preset Release
```

如果已经安装并配置好 `pyOCD`，可以尝试下载：

```powershell
cmake --build --preset Debug --target flash
```

当前 `pyOCD` 目标名在 CMake 中配置为：

```text
stm32f407vetx
```

## 当前验证清单

已经具备的验证点：

- 工程文件已生成
- FreeRTOS 已接入
- `PB2` LED 翻转任务已创建
- TIM5 PWM + DMA 配置已生成
- WS2812 编码和 DMA 发送流程已有实现
- CMake 构建入口已配置

仍需要进一步验证的点：

- 本地工具链是否完整可用
- 固件是否可以成功编译
- `PB2` 对应的实际硬件 LED 或外接 LED 是否按预期闪烁
- `PA3` 是否正确连接到 WS2812 DIN
- WS2812 颜色顺序是否正确
- WS2812 时序是否满足实际灯珠要求
- `rgb_task()` 是否需要补充 `rgb.show()` 和合理延时
- DMA 完成回调是否稳定触发

## 接下来方向

### 第一阶段：把基础闭环跑稳

优先目标是让最小系统稳定运行，建议按这个顺序推进：

1. 确认 CMake 工程可以稳定编译。
2. 确认调试器或下载器可以正常连接开发板。
3. 烧录后观察 `PB2` LED 是否 300 ms 周期翻转。
4. 明确筑基星开发板上 `PB2` 和 `PA3` 的实际引脚位置。
5. 给 `rgb_task()` 补充 `rgb.show()` 和任务延时，避免空转占用 CPU。
6. 使用逻辑分析仪或示波器观察 `PA3` 上的 WS2812 波形。
7. 实测 WS2812 显示红、绿、蓝三色是否正确。

这一阶段完成后，项目就从“工程能启动”进入“外设控制可验证”的状态。

### 第二阶段：完善 WS2812 驱动

WS2812 是一个很适合练习定时器、DMA 和 RTOS 配合的模块。后续可以继续做：

- 修正颜色编码顺序，确认实际灯珠使用的是 RGB、GRB 还是其他顺序
- 增加 `show()` 调用节流，避免重复刷新占满总线
- 增加亮度缩放接口
- 增加单色、流水灯、呼吸灯、彩虹灯等效果
- 将 LED 数量从固定 `3` 扩展为可配置
- 增加忙状态判断，避免 DMA 未完成时重复启动
- 将 DMA 完成等待从轮询延时优化为信号量或任务通知

这一阶段完成后，`Ws2812` 可以沉淀成项目中的第一个相对完整驱动模块。

### 第三阶段：整理工程结构

随着学习内容增加，建议逐步把代码分层：

- `Core/Src/main.c`：只保留 CubeMX 初始化和 RTOS 启动
- `Core/Src/app.cpp`：只负责任务创建和应用编排
- `Core/Src/drivers/`：放自定义硬件驱动
- `Core/Src/tasks/`：放各类 FreeRTOS 任务
- `Core/Src/boards/`：放和筑基星开发板相关的引脚定义

推荐后续把板级引脚封装起来，例如：

```cpp
namespace Board {
    constexpr auto LED_PORT = GPIOB;
    constexpr uint16_t LED_PIN = GPIO_PIN_2;
}
```

这样以后换引脚或换板子时，不需要在业务代码里到处搜索 GPIO。

### 第四阶段：继续扩展 STM32F4 外设学习

在 LED 和 WS2812 跑通后，可以按难度逐步扩展：

- USART：串口日志、命令行交互
- I2C：OLED、温湿度传感器、IMU
- SPI：屏幕、Flash、无线模块
- ADC：电位器、电压采样、简单滤波
- PWM：蜂鸣器、舵机、电机调速
- EXTI：按键中断、事件触发
- RTC：时间保持
- IWDG/WWDG：看门狗
- Flash：参数保存

建议每增加一个外设，就保留一个最小可运行示例和一段学习记录，方便后面复盘。

### 第五阶段：做一个综合小项目

当基础外设比较熟悉后，可以用筑基星开发板做一个小型综合项目，例如：

- RGB 氛围灯控制器：按键切换模式，串口修改颜色，WS2812 显示效果
- 桌面状态灯：串口接收电脑状态，用 RGB 灯显示不同状态
- 小型环境监测器：I2C 传感器采集数据，OLED 显示，串口输出日志
- FreeRTOS 练习平台：多个任务、队列、信号量、软件定时器协同运行

比较推荐的第一个综合项目是“RGB 氛围灯控制器”，因为当前工程已经有 LED、TIM、DMA、FreeRTOS 和 WS2812 基础，顺着现有代码继续扩展最自然。

## 近期 TODO

- [ ] 确认本机 `arm-none-eabi-gcc`、CMake、Ninja 是否配置完成
- [ ] 跑通 Debug 构建
- [ ] 烧录固件并验证 `PB2` LED 闪烁
- [ ] 查清筑基星开发板上 `PB2`、`PA3` 的实际接口位置
- [ ] 在 `rgb_task()` 中加入 `rgb.show()` 和 `osDelay()`
- [ ] 实测 WS2812 第一颗灯珠是否能显示固定颜色
- [ ] 根据实际显示效果确认 RGB/GRB 顺序
- [ ] 给串口日志预留接口，方便后续调试
- [ ] 将板级引脚和驱动代码进一步分层

## 学习记录建议

后续可以在 README 或单独的 `docs/` 目录中记录每次学习内容。推荐格式：

```text
日期：
目标：
修改内容：
遇到的问题：
解决方法：
下一步：
```

例如：

```text
日期：2026-06-23
目标：验证 FreeRTOS 下 LED 任务是否正常运行
修改内容：创建 ledTask，每 300 ms 翻转 PB2
遇到的问题：需要确认 PB2 是否连接到开发板 LED
解决方法：查原理图或用万用表确认引脚
下一步：烧录后观察 LED 状态，继续验证 WS2812 输出
```

## 备注

当前项目已经完成了从 CubeMX 模板到“可继续扩展的学习工程”的第一步。后续最重要的不是一次性堆很多外设，而是每完成一个小模块都做到能编译、能烧录、能观察、能复盘。这样这个仓库会慢慢变成一套属于自己的 STM32F4 学习笔记和代码积累。
