# CleanBot 开发指南

## 目录

1. [开发环境搭建](#开发环境搭建)
2. [项目结构说明](#项目结构说明)
3. [代码规范](#代码规范)
4. [模块开发指南](#模块开发指南)
5. [调试方法](#调试方法)
6. [常见问题](#常见问题)

## 开发环境搭建

### 1. 必需软件

- **Keil MDK-ARM 5.x**: 用于编译和调试
- **STM32CubeMX**: 用于生成HAL库代码和配置
- **STM32CubeProgrammer**: 用于程序下载
- **Git**: 用于版本控制

### 2. 环境配置步骤

1. 安装Keil MDK-ARM
2. 安装STM32F4的Device Pack
3. 安装STM32CubeMX
4. 配置Keil的路径设置

### 3. 项目导入

1. 使用STM32CubeMX打开 `CleanBot.ioc`
2. 生成代码（Generate Code）
3. 使用Keil打开 `MDK-ARM/CleanBot.uvprojx`
4. 编译项目

## 项目结构说明

```
CleanBot/
├── Application/          # 应用层代码
│   ├── CleanBotApp.h    # 应用层头文件
│   └── CleanBotApp.c    # 应用层实现
│
├── Modules/             # 功能模块
│   ├── Motor/          # 电机控制模块
│   ├── Encoder/        # 编码器模块
│   ├── PID/            # PID控制器模块
│   ├── Sensor/         # 传感器模块
│   ├── Indicator/      # 指示器模块
│   └── Communication/  # 通信模块
│
├── Utils/              # 工具模块
│   ├── ring_buffer.h   # 环形缓冲区
│   └── nec_decode.h    # NEC解码
│
├── Config/             # 配置文件
│   ├── hw_config.h     # 硬件配置
│   ├── system_config.h # 系统配置
│   └── cleanbot_config.h # 总配置文件
│
├── Common/             # 公共代码
│   ├── common_def.h    # 公共定义
│   └── common_utils.h  # 工具函数
│
├── Core/               # CubeMX生成的核心代码
│   ├── Inc/
│   └── Src/
│
├── Drivers/            # HAL驱动库
├── Middlewares/        # 中间件（FreeRTOS、USB）
├── USB_DEVICE/         # USB设备配置
└── BSP/                # 板级支持包
```

## 代码规范

### 1. 命名规范

#### 文件命名
- 头文件: `module_name.h`
- 源文件: `module_name.c`
- 使用小写字母和下划线

#### 函数命名
- 模块前缀 + 下划线 + 功能名
- 示例: `Motor_SetSpeed()`, `Encoder_GetSpeed()`

#### 变量命名
- 使用驼峰命名法或下划线命名法
- 全局变量: `g_variableName`
- 静态变量: `s_variableName`
- 局部变量: `variableName`

#### 类型命名
- 结构体: `TypeName_t`
- 枚举: `EnumName_t`
- 宏定义: `MACRO_NAME`

### 2. 代码风格

#### 缩进
- 使用4个空格缩进
- 不使用Tab

#### 注释
- 文件头注释必须包含文件说明、作者、日期
- 函数注释使用Doxygen风格
- 复杂逻辑必须添加注释

#### 示例
```c
/**
 * @brief  设置电机速度
 * @param  motor: 电机对象指针
 * @param  speed: 速度值 (0-1000)
 * @return 状态码
 */
Status_t Motor_SetSpeed(Motor_t *motor, int16_t speed)
{
    // 参数检查
    if (motor == NULL) {
        return STATUS_INVALID_PARAM;
    }
    
    // 速度限制
    speed = CLAMP(speed, 0, MOTOR_SPEED_MAX);
    
    // 设置速度
    motor->targetSpeed = speed;
    
    return STATUS_OK;
}
```

### 3. 模块设计原则

#### 面向对象设计
- 使用结构体封装数据
- 使用函数指针实现多态（如需要）
- 每个模块提供Init、DeInit、Update等标准接口

#### 模块接口
每个模块应该提供以下标准接口：
- `Module_Init()`: 初始化
- `Module_DeInit()`: 反初始化
- `Module_Enable()`: 使能
- `Module_Disable()`: 禁用
- `Module_Update()`: 更新（如果需要周期性更新）

## 模块开发指南

### 1. 创建新模块

#### 步骤1: 创建文件
在 `Modules/` 目录下创建新目录，例如 `NewModule/`，然后创建：
- `new_module.h`: 头文件
- `new_module.c`: 源文件

#### 步骤2: 定义结构体
```c
typedef struct {
    // 模块数据成员
    bool enabled;
    // ...
} NewModule_t;
```

#### 步骤3: 实现接口函数
```c
void NewModule_Init(NewModule_t *module)
{
    // 初始化代码
}

void NewModule_Update(NewModule_t *module)
{
    // 更新代码
}
```

#### 步骤4: 添加到配置文件
在 `Config/cleanbot_config.h` 中添加头文件引用。

### 2. 电机控制模块开发

参考 `Modules/Motor/motor.h` 和 `motor.c`。

关键点：
- 使用PWM控制速度
- 使用GPIO控制方向
- 实现速度限制和保护

### 3. 编码器模块开发

参考 `Modules/Encoder/encoder.h` 和 `encoder.c`。

关键点：
- 使用定时器编码器模式
- 实现速度计算
- 处理溢出情况

### 4. PID控制器模块开发

参考 `Modules/PID/pid_controller.h` 和 `pid_controller.c`。

关键点：
- 实现标准PID算法
- 支持输出限幅和积分限幅
- 处理积分饱和

### 5. 传感器模块开发

参考 `Modules/Sensor/` 目录下的传感器模块。

关键点：
- 使用中断或定时器读取
- 实现滤波算法（如需要）
- 提供状态查询接口

## 调试方法

### 1. 使用Keil调试器

1. 连接ST-Link或J-Link
2. 在Keil中点击Debug按钮
3. 设置断点
4. 使用Watch窗口查看变量

### 2. 使用USB串口调试

1. 通过USB CDC发送调试信息
2. 使用串口助手查看输出
3. 实现调试宏定义

### 3. 使用LED指示

- 使用LED指示系统状态
- 使用LED闪烁表示错误代码

### 4. 使用FreeRTOS任务监控

- 使用FreeRTOS的任务监控功能
- 查看任务堆栈使用情况
- 查看任务执行时间

## 常见问题

### 1. 编译错误

**问题**: 找不到头文件
**解决**: 检查Keil项目的Include Paths设置

**问题**: 链接错误
**解决**: 检查是否所有源文件都添加到项目中

### 2. 运行时错误

**问题**: 系统死机
**解决**: 
- 检查堆栈溢出
- 检查中断优先级
- 检查内存分配

**问题**: 电机不转
**解决**:
- 检查PWM配置
- 检查GPIO配置
- 检查使能标志

### 3. 性能问题

**问题**: 控制周期不稳定
**解决**:
- 优化任务优先级
- 减少中断处理时间
- 优化算法

**问题**: 内存不足
**解决**:
- 减少堆栈大小
- 使用静态分配
- 优化数据结构

### 4. 通信问题

**问题**: USB通信失败
**解决**:
- 检查USB连接
- 检查USB描述符配置
- 检查缓冲区大小

## 开发流程

### 1. 需求分析
- 明确功能需求
- 确定硬件接口
- 设计软件架构

### 2. 模块设计
- 设计模块接口
- 定义数据结构
- 编写接口文档

### 3. 编码实现
- 实现模块功能
- 编写单元测试
- 代码审查

### 4. 集成测试
- 模块集成
- 功能测试
- 性能测试

### 5. 硬件测试
- 实际硬件测试
- 参数调优
- 可靠性测试

## 版本控制

### Git工作流

1. **主分支 (main)**: 稳定版本
2. **开发分支 (develop)**: 开发版本
3. **功能分支 (feature/xxx)**: 新功能开发
4. **修复分支 (fix/xxx)**: Bug修复

### 提交规范

提交信息格式：
```
<type>(<scope>): <subject>

<body>

<footer>
```

类型：
- `feat`: 新功能
- `fix`: Bug修复
- `docs`: 文档更新
- `style`: 代码格式
- `refactor`: 重构
- `test`: 测试
- `chore`: 构建/工具

## 参考资料

- [STM32 HAL库文档](https://www.st.com/resource/en/user_manual/um1725-description-of-stm32f4-hal-and-lowlayer-drivers-stmicroelectronics.pdf)
- [FreeRTOS文档](https://www.freertos.org/Documentation/RTOS_book.html)
- [STM32CubeMX用户手册](https://www.st.com/resource/en/user_manual/um1718-stm32cube-mx-for-stm32-configuration-and-initialization-c-code-generation-stmicroelectronics.pdf)

