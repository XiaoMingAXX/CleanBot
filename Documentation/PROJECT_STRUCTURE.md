# CleanBot 项目结构说明

## 目录结构总览

本项目采用分层架构和模块化设计，遵循面向对象的设计思想。

## 详细目录说明

### 1. Application/ - 应用层

**职责**: 应用层代码，整合所有模块，实现业务逻辑。

**文件**:
- `CleanBotApp.h/c`: 主应用类，管理所有硬件模块

**设计思想**:
- 单一应用对象，统一管理所有模块
- 提供高级API接口
- 实现状态机控制

### 2. Modules/ - 功能模块层

**职责**: 实现具体的硬件功能模块。

#### 2.1 Motor/ - 电机控制模块

**设计思想**: 面向对象设计，支持多种电机类型。

**核心结构**:
- `Motor_t`: 电机对象
- `MotorVTable_t`: 虚拟函数表（支持多态）

**功能**:
- PWM速度控制
- 方向控制
- 速度限制

#### 2.2 Encoder/ - 编码器模块

**设计思想**: 封装编码器读取和速度计算。

**核心结构**:
- `Encoder_t`: 编码器对象

**功能**:
- 脉冲计数
- 速度计算（RPM）
- 增量计算

#### 2.3 PID/ - PID控制器模块

**设计思想**: 标准PID算法实现。

**核心结构**:
- `PIDController_t`: PID控制器对象

**功能**:
- PID计算
- 输出限幅
- 积分限幅

#### 2.4 Sensor/ - 传感器模块

**设计思想**: 统一的传感器接口。

**子模块**:
- `ir_sensor`: 红外传感器（支持NEC解码）
- `photo_gate`: 光电门

#### 2.5 Indicator/ - 指示器模块

**设计思想**: 简单的GPIO/PWM控制。

**子模块**:
- `led`: LED控制
- `buzzer`: 蜂鸣器控制

#### 2.6 Communication/ - 通信模块

**设计思想**: 使用环形缓冲区实现数据缓冲。

**子模块**:
- `usb_comm`: USB CDC虚拟串口通信

### 3. Config/ - 配置层

**职责**: 集中管理所有配置信息。

**文件**:
- `hw_config.h`: 硬件配置（引脚、定时器、PID参数等）
- `system_config.h`: 系统配置（任务优先级、堆栈大小等）
- `cleanbot_config.h`: 总配置文件（包含所有头文件）

**设计思想**:
- 配置与代码分离
- 便于修改和移植
- 使用宏定义

### 4. Common/ - 公共代码层

**职责**: 提供公共定义和工具函数。

**文件**:
- `common_def.h`: 公共类型定义、宏定义
- `common_utils.h`: 工具函数（时间、数学、字符串等）

**设计思想**:
- 避免代码重复
- 提供通用功能
- 独立于硬件

### 5. Utils/ - 工具模块层

**职责**: 提供通用的工具模块。

**文件**:
- `ring_buffer.h/c`: 环形缓冲区实现
- `nec_decode.h/c`: NEC红外解码实现

**设计思想**:
- 可复用的工具模块
- 独立的功能单元

### 6. BSP/ - 板级支持包

**职责**: 提供板级相关的封装函数。

**文件**:
- `bsp_gpio.h/c`: GPIO封装
- `bsp_tim.h/c`: 定时器封装

**设计思想**:
- 封装HAL库调用
- 提供统一的BSP接口

### 7. Core/ - 核心代码层

**职责**: STM32CubeMX生成的核心代码。

**目录**:
- `Inc/`: 头文件
- `Src/`: 源文件

**注意**: 此目录下的文件由CubeMX自动生成，修改后重新生成代码会被覆盖。

### 8. Drivers/ - 驱动层

**职责**: STM32 HAL驱动库。

**目录**:
- `CMSIS/`: ARM CMSIS核心文件
- `STM32F4xx_HAL_Driver/`: STM32F4 HAL驱动库

### 9. Middlewares/ - 中间件层

**职责**: 第三方中间件。

**目录**:
- `ST/STM32_USB_Device_Library/`: ST官方USB设备库
- `Third_Party/FreeRTOS/`: FreeRTOS实时操作系统

### 10. USB_DEVICE/ - USB设备配置

**职责**: USB设备相关配置。

**目录**:
- `App/`: USB应用层代码
- `Target/`: USB目标配置

### 11. MDK-ARM/ - Keil工程

**职责**: Keil MDK-ARM工程文件。

**文件**:
- `CleanBot.uvprojx`: Keil项目文件

### 12. Documentation/ - 文档目录

**职责**: 项目文档。

**文件**:
- `ARCHITECTURE.md`: 架构设计文档
- `DEVELOPMENT_GUIDE.md`: 开发指南
- `PIN_CONFIGURATION.md`: 引脚配置说明
- `PROJECT_STRUCTURE.md`: 项目结构说明（本文档）

## 代码组织原则

### 1. 分层设计

```
应用层 (Application)
    ↓
模块层 (Modules)
    ↓
BSP层 (BSP)
    ↓
HAL层 (Drivers)
```

### 2. 模块化设计

- 每个模块独立
- 接口清晰
- 低耦合高内聚

### 3. 面向对象设计

- 使用结构体封装数据
- 使用函数指针实现多态
- 提供标准接口（Init、Update等）

### 4. 配置与代码分离

- 所有配置集中在Config目录
- 使用宏定义
- 便于修改和移植

## 文件命名规范

### 头文件
- 格式: `module_name.h`
- 示例: `motor.h`, `encoder.h`

### 源文件
- 格式: `module_name.c`
- 示例: `motor.c`, `encoder.c`

### 函数命名
- 格式: `Module_Function()`
- 示例: `Motor_SetSpeed()`, `Encoder_GetSpeed()`

### 类型命名
- 格式: `TypeName_t`
- 示例: `Motor_t`, `Encoder_t`

### 宏定义
- 格式: `MACRO_NAME`
- 示例: `MOTOR_SPEED_MAX`, `PID_KP`

## 依赖关系

### 依赖层次

```
Application
    ↓ depends on
Modules + Config + Common
    ↓ depends on
BSP + Utils
    ↓ depends on
Core + Drivers + Middlewares
```

### 依赖规则

1. **上层可以依赖下层，下层不能依赖上层**
2. **同层模块尽量独立，减少相互依赖**
3. **公共功能放在Common或Utils**
4. **配置信息集中在Config**

## 添加新模块步骤

1. 在 `Modules/` 下创建新目录
2. 创建 `module_name.h` 和 `module_name.c`
3. 实现模块接口（Init、Update等）
4. 在 `Config/cleanbot_config.h` 中添加头文件引用
5. 在Keil项目中添加源文件
6. 编写文档说明

## 注意事项

1. **不要修改Core目录下的CubeMX生成文件**（除非必要）
2. **配置修改集中在Config目录**
3. **遵循命名规范**
4. **添加必要的注释**
5. **保持模块独立性**

