# CleanBot - 扫地机器人STM32工程

## 项目简介

这是一个基于STM32F407ZGT6的扫地机器人控制系统，使用Keil MDK-ARM和STM32CubeMX开发。项目采用面向对象的设计思想，模块化结构，便于维护和扩展。

## 硬件配置

- **主控芯片**: STM32F407ZGT6
- **操作系统**: FreeRTOS
- **通信方式**: USB CDC虚拟串口（与树莓派通信）

## 硬件清单

### 电机
- 2个轮电机（左右轮）
- 2个边刷电机
- 1个吸尘电机
- 1个水箱增压电机

### 编码器
- 3个霍尔编码器（左轮、右轮、风机）

### 传感器
- 4个红外传感器（支持NEC解码）
- 2个光电门

### 指示器
- 4个LED
- 1个蜂鸣器

## 工程结构

```
CleanBot/
├── Application/              # 应用层
│   ├── CleanBotApp.h         # 应用层头文件
│   └── CleanBotApp.c         # 应用层实现
│
├── Modules/                   # 功能模块（面向对象设计）
│   ├── Motor/                # 电机控制模块
│   │   ├── motor.h
│   │   └── motor.c
│   ├── Encoder/              # 编码器模块
│   │   ├── encoder.h
│   │   └── encoder.c
│   ├── PID/                  # PID控制器模块
│   │   ├── pid_controller.h
│   │   └── pid_controller.c
│   ├── Sensor/               # 传感器模块
│   │   ├── ir_sensor.h       # 红外传感器（支持NEC解码）
│   │   ├── ir_sensor.c
│   │   ├── photo_gate.h      # 光电门
│   │   └── photo_gate.c
│   ├── Indicator/            # 指示器模块
│   │   ├── led.h             # LED控制
│   │   ├── led.c
│   │   ├── buzzer.h          # 蜂鸣器控制
│   │   └── buzzer.c
│   └── Communication/        # 通信模块
│       ├── usb_comm.h        # USB CDC虚拟串口通信
│       └── usb_comm.c
│
├── Config/                   # 配置文件
│   ├── hw_config.h           # 硬件配置（引脚、定时器等）
│   ├── system_config.h       # 系统配置（任务、优先级等）
│   └── cleanbot_config.h     # 项目总配置文件
│
├── Common/                   # 公共代码
│   ├── common_def.h          # 公共定义（类型、宏等）
│   └── common_utils.h        # 工具函数
│
├── Utils/                    # 工具模块
│   ├── ring_buffer.h         # 环形缓冲区
│   ├── ring_buffer.c
│   ├── nec_decode.h          # NEC红外解码
│   └── nec_decode.c
│
├── BSP/                      # 板级支持包
│   ├── bsp_gpio.h
│   ├── bsp_gpio.c
│   ├── bsp_tim.h
│   └── bsp_tim.c
│
├── Core/                     # CubeMX生成的核心文件
│   ├── Inc/                  # 头文件
│   └── Src/                  # 源文件
│
├── Drivers/                  # STM32 HAL驱动库
│   ├── CMSIS/                # CMSIS核心文件
│   └── STM32F4xx_HAL_Driver/ # HAL驱动
│
├── Middlewares/              # 中间件
│   ├── ST/                   # ST官方中间件
│   │   └── STM32_USB_Device_Library/ # USB设备库
│   └── Third_Party/          # 第三方中间件
│       └── FreeRTOS/       # FreeRTOS实时操作系统
│
├── USB_DEVICE/               # USB设备配置
│   ├── App/                  # USB应用层
│   └── Target/               # USB目标配置
│
├── MDK-ARM/                  # Keil工程文件
│   └── CleanBot.uvprojx     # Keil项目文件
│
├── Documentation/           # 文档目录
│   ├── ARCHITECTURE.md       # 架构设计文档
│   ├── DEVELOPMENT_GUIDE.md  # 开发指南
│   └── PIN_CONFIGURATION.md  # 引脚配置说明
│
├── CleanBot.ioc              # STM32CubeMX配置文件
├── README.md                 # 项目说明
└── .gitignore               # Git忽略文件
```

## 模块说明

### 1. 电机控制模块 (Motor)
- 支持多种电机类型（轮电机、边刷电机、吸尘电机、水泵电机）
- 使用PWM控制速度
- 支持方向控制（正转/反转）
- 面向对象设计，易于扩展

### 2. 编码器模块 (Encoder)
- 支持霍尔编码器
- 自动计算速度（RPM）
- 支持脉冲计数和增量计算

### 3. PID控制器模块 (PID)
- 标准PID控制器实现
- 支持输出限幅和积分限幅
- 可配置PID参数

### 4. 传感器模块 (Sensor)
- **红外传感器**: 支持NEC协议解码
- **光电门**: 检测碰撞和障碍物

### 5. 指示器模块 (Indicator)
- **LED**: 4个LED状态指示
- **蜂鸣器**: 支持PWM音调输出

### 6. 通信模块 (Communication)
- **USB通信**: USB CDC虚拟串口
- 使用环形缓冲区进行数据缓冲
- 支持与树莓派通信

## 使用说明

### 1. 环境配置
- Keil MDK-ARM 5.x
- STM32CubeMX
- STM32F4 HAL库

### 2. 编译和下载
1. 使用STM32CubeMX打开 `CleanBot.ioc` 文件
2. 配置硬件（GPIO、定时器、USB等）
3. 生成代码
4. 使用Keil打开 `MDK-ARM/CleanBot.uvprojx`
5. 编译并下载到开发板

### 3. 配置说明

#### GPIO配置
根据 `main.h` 中的引脚定义，配置以下GPIO：
- 电机PWM和方向控制
- 编码器输入
- 传感器输入
- LED和蜂鸣器输出

#### 定时器配置
- TIM1: 轮电机PWM
- TIM2: 左轮编码器
- TIM3: 右轮编码器/边刷电机PWM
- TIM4: 风机编码器/吸尘电机PWM
- TIM5: 水泵电机PWM
- TIM10: 蜂鸣器PWM

#### USB配置
- 配置为USB Device模式
- 使用CDC类（虚拟串口）

### 4. 应用层使用

```c
#include "CleanBotApp.h"

/* 初始化 */
CleanBotApp_t *app = CleanBotApp_GetInstance();
CleanBotApp_Init(app);

/* 启动 */
CleanBotApp_Start(app);

/* 设置速度 */
CleanBotApp_SetWheelSpeed(app, 500, 500);  // 左右轮速度
CleanBotApp_SetBrushSpeed(app, 300, 300);  // 边刷速度
CleanBotApp_SetFanSpeed(app, 800);         // 风机速度

/* 周期性更新（在FreeRTOS任务中调用） */
void CleanBotTask(void *argument)
{
    while(1)
    {
        CleanBotApp_Update(app);
        osDelay(10);
    }
}
```

## PID参数调优

PID参数需要根据实际硬件进行调优：

```c
/* 在CleanBotApp_Init中调整PID参数 */
PID_Init(&app->pidWheelLeft, 1.0f, 0.1f, 0.05f);  // Kp, Ki, Kd
```

## 注意事项

1. **引脚配置**: 请根据实际硬件连接修改 `CleanBotApp.c` 中的引脚配置
2. **编码器参数**: 根据实际编码器的PPR（每转脉冲数）和减速比调整参数
3. **PID参数**: 需要根据实际系统特性进行调优
4. **定时器配置**: 确保PWM频率和编码器模式配置正确
5. **USB通信**: 需要在USB CDC回调函数中调用 `USB_Comm_RxCpltCallback` 和 `USB_Comm_TxCpltCallback`

## 开发计划

- [x] 基础模块框架
- [x] 电机控制模块
- [x] 编码器模块
- [x] PID控制器模块
- [x] 传感器模块
- [x] 指示器模块
- [x] USB通信模块
- [ ] 路径规划算法
- [ ] 避障算法
- [ ] 充电管理
- [ ] 数据日志记录

## 许可证

本项目采用 MIT 许可证。

## 作者

CleanBot Team

## 更新日志

### 2025-01-XX
- 初始版本
- 完成基础模块框架
- 实现电机控制、编码器、PID、传感器等模块

