# stm32-watch-FreeRTOS

一个基于 **STM32F10x + FreeRTOS** 的“智能手表/小型多功能终端”项目，集成 OLED UI、多级菜单、按键交互、温湿度采集、姿态/计步、秒表与小游戏等功能。工程自带 Keil uVision 工程文件，可直接编译下载。

> 工程名：`stm32_Watch-FreeRTOS`
>  主要入口：`User/main.c`
>  FreeRTOS 配置：`User/FreeRTOSConfig.h`

## 1. 项目概览

### 1.1 核心特性

- **FreeRTOS 多任务架构**：按键扫描、UI、传感器采集、显示刷新、后台计步各自独立任务运行
- **OLED 图形界面**：菜单/时钟/页面绘制写入显示缓冲区，通过通知机制触发刷新
- **传感器与外设**
  - DHT11 温湿度（带 RTOS 缓存与互斥保护）
  - MPU6050 姿态解算/偏航角（用于计步逻辑）
  - ADC 采样（`ADC1` 通道0）
  - RTC（LSE 驱动，带备份寄存器标志判断是否首次配置）
- **应用功能**：时钟首页、设置页、多级菜单、秒表、计步页面/记录页面、小游戏（Dino）等

### 1.2 硬件总体清单（BOM 级）

| 硬件模块     | 型号 / 说明          | 用途                    |
| ------------ | -------------------- | ----------------------- |
| 主控 MCU     | STM32F103c8t6        | 系统核心，运行 FreeRTOS |
| 0.96寸显示屏 | OLED（SSD1306，I²C） | 图形界面显示            |
| 温湿度传感器 | DHT11                | 环境温湿度采集          |
| 姿态传感器   | MPU6050              | 姿态检测 / 计步         |
| 按键         | 4 个（含长按）       | 人机交互                |
| LED          | 普通 LED             | 状态指示                |
| RTC          | MCU 内部 RTC + LSE   | 实时时钟                |
| 晶振         | 32.768kHz            | RTC 低速外部时钟        |
| 电源         | 3.3V                 | 系统供电                |

#### 引脚总览速查表

| 模块     | 引脚          |
| -------- | ------------- |
| OLED     | PB8, PB9      |
| MPU6050  | PB10, PB11    |
| DHT11    | PA2           |
| 按键     | PB1, PA6, PA4 |
| LED      | PB15          |
| ADC      | PA0           |
| 扩展 IO  | PB12, PB13    |
| RTC 晶振 | PC14, PC15    |

### 2. 功能列表（按模块）

> 具体页面与业务逻辑主要集中在 `Hardware/menu.c`（菜单与 UI 绘制）与 `Hardware/dino.c`（小游戏）中。

- **时钟首页**：`First_Page_Clock()`（`Hardware/menu.c`）

- **菜单系统**：`Menu()`（`Hardware/menu.c`）

- **设置页面**：`SettingPage()`（`Hardware/menu.c`）

- **秒表**：`StopWatch_Tick()` / `StopWatch()`（`Hardware/menu.c`）

- **DHT11 温湿度**

  - 底层读数：`DHT11_Read_Data()`（`Hardware/DHT11.c`）
  - RTOS 缓存：`DHT11_CacheInit()` / `DHT11_CacheUpdate()` / `DHT11_CacheGet()`（`Hardware/DHT11.c`）

- **MPU6050**

  - 初始化：`MPU6050_Init()`（`Hardware/MPU6050.c`，依赖 `Hardware/MyI2C.c` 软件 I2C）
  - 计算：`MPU6050_Calculation()`（在 `Hardware/menu.c` 的计步逻辑中被调用）

- **计步**

  - 后台计步接口：`Steps_BackgroundInit()` / `Steps_SetRunning()` / `Steps_ResetSession()` / `Steps_GetSession()` / `Steps_BackgroundTick()`（`Hardware/menu.c`）
  - 页面：`Steps()` / `Start_Steps()` / `Show_Record()` / `Show_Steps_UI()`（`Hardware/menu.c`）

- **OLED 显示**

  - 驱动：`Hardware/OLED.c`、字模/图像数据：`Hardware/OLED_Data.c`

  - 互斥锁：`OLED_MutexInit()` / `OLED_Lock()` / `OLED_Unlock()`（`Hardware/OLED.c`）

    

## 3. 工程结构与文件说明（真实目录）

### 3.1 应用层（入口与 RTOS）

- `User/main.c`
  - 创建 FreeRTOS 任务：`Key_Task` / `Tick_Task` / `UI_Task` / `Sensor_Task` / `Display_Task` / `Step_Task`
  - 显示刷新通知接口：`Display_RequestRefresh()`
  - `vApplicationStackOverflowHook()`、`vApplicationIdleHook()` 等 FreeRTOS Hook
- `User/FreeRTOSConfig.h`：FreeRTOS 配置
- `User/stm32f10x_it.c/.h`：中断处理框架（工程自带）
- `User/stm32f10x_conf.h`：外设库配置

### 3.2 Hardware（外设驱动 & UI/应用）

- 按键：`Hardware/Key.c/.h`
- LED：`Hardware/LED.c/.h`
- OLED：`Hardware/OLED.c/.h`、`Hardware/OLED_Data.c/.h`
- DHT11：`Hardware/DHT11.c/.h`
- MPU6050：`Hardware/MPU6050.c/.h`、`Hardware/MPU6050_Reg.h`
- 软件 I2C：`Hardware/MyI2C.c/.h`
- ADC：`Hardware/AD.c/.h`
- 菜单/页面/计步逻辑：`Hardware/menu.c/.h`
- 小游戏：`Hardware/dino.c/.h`
- 时间设置页面：`Hardware/SetTime.c/.h`

### 3.3 System（系统支持）

- 延时：`System/Delay.c/.h`
- 定时器：`System/Timer.c/.h`
- Flash：`System/MyFLASH.c/.h`
- RTC：`System/MyRTC.c/.h`
- 存储封装：`System/Store.c/.h`

### 3.4 Start / Library / FreeRTOS

- 启动文件：`Start/startup_stm32f10x_*.s`
- CMSIS & system：`Start/system_stm32f10x.c/.h`
- 标准外设库：`Library/stm32f10x_*.c/.h`
- FreeRTOS 内核：`FreeRTOS/include/*`、`FreeRTOS/source/*`、`FreeRTOS/portable/*`

## 4. 硬件设计：各模块引脚分配（来自代码真实配置）

> 下表均来自各外设 `Init()` 或宏定义中的 GPIO 配置（例如 `GPIO_InitStructure.GPIO_Pin = ...`、以及 DHT11 宏里标注的端口）。

### 4.1 OLED（软件 I2C，开漏）

文件：`Hardware/OLED.c` → `OLED_GPIO_Init()`

- **PB8**：SCL
- **PB9**：SDA
   GPIO 模式：开漏输出 `GPIO_Mode_Out_OD`

### 4.2 MPU6050（软件 I2C，开漏）

文件：`Hardware/MyI2C.c` → `MyI2C_Init()`

- **PB10**：SCL
- **PB11**：SDA
   GPIO 模式：开漏输出 `GPIO_Mode_Out_OD`

> MPU6050 初始化：`Hardware/MPU6050.c` → `MPU6050_Init()`（内部先调用 `MyI2C_Init()`）

### 4.3 DHT11

文件：`Hardware/DHT11.c`

- **PA2**：数据线 DQ
   宏定义（文件内）明确标注：`PA2 推挽输出 / 上拉输入`

### 4.4 ADC（ADC1 通道0）

文件：`Hardware/AD.c` → `AD_Init()`

- **PA0**：ADC 输入（`ADC_Channel_0`）
   GPIO 模式：模拟输入 `GPIO_Mode_AIN`

### 4.5 按键（上拉输入）

文件：`Hardware/Key.c` → `Key_Init()` & `Key_GetState()`

- **PB1**：Key state = 1
- **PA6**：Key state = 2
- **PA4**：Key state = 3（短按）
- **PA4 长按**：Key state = 4（由 `press_time > 1000` 判断）

GPIO 模式：上拉输入 `GPIO_Mode_IPU`

### 4.6 LED / 提示输出

文件：`Hardware/LED.c` → `LED_Init()`

- **PB15**：LED（`LED_ON()` 拉低点亮，`LED_OFF()` 拉高熄灭）
- **PB12 / PB13**：同样被初始化为推挽输出（并在 `Hardware/menu.c` 中出现对 PB12/PB13 的置位/复位控制）

> PB12/PB13 在工程中确实被当作输出脚使用，但具体外接硬件（蜂鸣器/背光/震动/扩展灯等）需要以你的原理图为准；README 这里按“提示输出/扩展 IO”描述最稳妥。

### 4.7 RTC（LSE）

文件：`System/MyRTC.c`

- 使用 **LSE 32.768kHz**：代码中调用 `RCC_LSEConfig(RCC_LSE_ON)` 并等待 `RCC_FLAG_LSERDY`
- 若使用外部 32.768kHz 晶振，通常对应 **PC14/PC15（OSC32_IN/OSC32_OUT）**（这是芯片硬件固定功能脚）

## 5. 软件设计

### 5.1 FreeRTOS 任务划分（来自 `User/main.c`）

`User/main.c` 创建任务如下（名称/周期/职责）：

| 任务           | 周期/触发                | 主要调用                                      | 说明                                 |
| -------------- | ------------------------ | --------------------------------------------- | ------------------------------------ |
| `Key_Task`     | 1ms 周期                 | `Key_Tick()`、`Key3_Tick()`                   | 按键扫描与事件产生                   |
| `Tick_Task`    | 1ms 周期                 | `StopWatch_Tick()`、`Dino_Tick()`             | 秒表与小游戏的周期 Tick              |
| `UI_Task`      | 循环 + 10ms delay        | `First_Page_Clock()` → `Menu()/SettingPage()` | UI 状态机/页面调度                   |
| `Sensor_Task`  | 2s 周期（启动延迟 1.5s） | `DHT11_CacheUpdate()`                         | 温湿度后台更新，首次成功会触发刷新   |
| `Display_Task` | 阻塞等待通知             | `ulTaskNotifyTake()` → `OLED_Update()`        | 唯一“真正刷屏”的任务（被通知才刷新） |
| `Step_Task`    | 5ms 周期                 | `Steps_BackgroundTick()`                      | 后台计步（依赖 MPU6050 偏航角）      |



### 5.2 任务流程图（Mermaid）

#### 一、系统初始化与任务创建阶段（主线流程）

1. **入口起点**：程序从`main()`函数开始执行，首先完成核心初始化工作，包括系统级初始化（SystemInit）、OLED 显示屏初始化（OLED_Init）以及各类外设初始化（Peripheral_Init）。
2. **任务创建**：初始化完成后，调用`xTaskCreate()`函数创建 6 个 FreeRTOS 任务，分别是：按键任务（Key_Task）、时基任务（Tick_Task）、UI 界面任务（UI_Task）、传感器任务（Sensor_Task）、显示刷新任务（Display_Task）、计步任务（Step_Task）。
3. **启动调度器**：所有任务创建完毕后，调用`vTaskStartScheduler()`启动 FreeRTOS 任务调度器，系统开始按任务优先级和周期调度执行各个任务。

#### 二、各任务的具体执行逻辑（并行运行）

##### 1. 按键任务（Key_Task，执行周期 1ms）

- 首先执行`Key_Tick()`函数，完成通用按键的扫描与状态更新；
- 接着执行`Key3_Tick()`函数，专门处理 Key3 按键的状态检测与 Tick 计数。

##### 2. 时基任务（Tick_Task，执行周期 1ms）

- 先执行`StopWatch_Tick()`函数，为秒表功能提供时间基准；
- 再执行`Dino_Tick()`函数，为恐龙小游戏提供运行时基。

##### 3. UI 界面任务（UI_Task，无固定周期，按需执行）

- 初始显示时钟首页（First_Page_Clock()）；

  - 若返回值`ret=1`，进入菜单页面（`Menu()`）；
  - 若返回值`ret=2`，进入设置页面（`SettingPage()`）；

  

- 菜单页面或设置页面执行完成后，将对应的界面内容绘制到 OLED 显示缓冲区（`OLED_DisplayBuf`）；

- 绘制完成后调用`Display_RequestRefresh()`函数，请求刷新 OLED 显示。

##### 4. 传感器任务（Sensor_Task，执行周期 2s）

- 任务启动后先延迟 1500ms，避免系统初始化阶段的资源冲突；

- 延迟结束后执行DHT11_CacheUpdate()函数，更新 DHT11 温湿度传感器的缓存数据；

  - 若本次是传感器首次成功获取数据，调用`Display_RequestRefresh()`函数请求刷新显示；
  - 无论是否首次成功，任务都会按 2000ms 的周期重复执行数据更新操作。

  

##### 5. 计步任务（Step_Task，执行周期 5ms）

- 首先执行`Steps_BackgroundTick()`函数，完成计步功能的后台时基处理；
- 接着调用`MPU6050_Calculation()`函数，对 MPU6050 传感器数据进行计算处理；
- 最后根据计算得到的偏航角（Yaw）进行阈值判定，符合条件则累计步数。

##### 6. 显示刷新任务（Display_Task，阻塞等待触发）

- 任务通过`ulTaskNotifyTake()`函数阻塞等待外部的刷新通知（不占用 CPU 资源）；
- 收到通知后先调用`OLED_Lock()`函数锁定 OLED 资源，防止多任务冲突；
- 然后执行`OLED_Update()`函数，将显示缓冲区的数据刷新到 OLED 屏幕；
- 刷新完成后调用`OLED_Unlock()`函数解锁 OLED 资源，释放占用。

#### 三、任务间的交互逻辑

- UI 任务中调用的`Display_RequestRefresh()`会触发显示刷新任务的`ulTaskNotifyTake()`，使其退出阻塞并执行显示更新；
- 传感器任务首次成功获取数据时调用的`Display_RequestRefresh()`，同样会触发显示刷新任务执行 OLED 更新。

### 总结

1. 系统核心流程：初始化 → 创建 6 个任务 → 启动调度器，各任务并行执行；
2. 任务特性：不同任务有不同执行周期（1ms/2s/5ms），显示任务为阻塞触发式，UI 任务按需执行；
3. 交互核心：UI 任务和传感器任务通过`Display_RequestRefresh()`触发显示任务，完成 OLED 屏幕刷新，保证显示数据的实时性。

## 6. 关键代码机制分析（基于真实函数）

### 6.1 显示刷新：任务通知 + 显示互斥

- 刷屏任务：`Display_Task`（`User/main.c`）
  - 使用 `ulTaskNotifyTake(pdTRUE, portMAX_DELAY)` 阻塞等待
  - 收到通知后 `OLED_Lock()` → `OLED_Update()` → `OLED_Unlock()`
- 通知接口：`Display_RequestRefresh()`（`User/main.c`）
  - 判断 `s_displayTaskHandle != NULL`
  - 且调度器已运行：`xTaskGetSchedulerState() == taskSCHEDULER_RUNNING`
  - 调用 `xTaskNotifyGive(s_displayTaskHandle)` 唤醒 `Display_Task`
- OLED 互斥：`OLED_MutexInit()` / `OLED_Lock()` / `OLED_Unlock()`（`Hardware/OLED.c`）
  - 使用 **递归互斥锁** `xSemaphoreCreateRecursiveMutex()`，允许 `OLED_Update()` 内部/外部重复加锁的场景更安全

**优点：**

- UI/传感器/应用层只负责“改缓冲区 + 发通知”，刷新时机统一由 `Display_Task` 管控
- 避免多处直接 `OLED_Update()` 导致的撕裂/竞争

------

### 6.2 DHT11：RTOS 缓存 + 互斥保护 + 重试

文件：`Hardware/DHT11.c`

- 初始化：
  - `DHT11_Init()`：初始化 GPIO（PA2）
  - `DHT11_CacheInit()`：创建互斥量 `xSemaphoreCreateMutex()`
- 周期更新：`DHT11_CacheUpdate()`
  - 最多重试 3 次
  - 读时序关键区间使用短暂关中断（代码中有 `__asm { CPSID I }` / `__asm { CPSIE I }`）
  - 成功后写入缓存：`s_temp / s_humi / s_valid`
  - **失败不清空上一次有效值**：保持“最后一次有效值”策略，UI 体验更稳定
- 读取：`DHT11_CacheGet()`
  - 若互斥量存在则保护读取，返回有效标志 `s_valid`

------

### 6.3 按键：扫描 Tick + 队列（兼容 RTOS/非 RTOS）

文件：`Hardware/Key.c`

- `Key_Task`（1ms）周期调用：
  - `Key_Tick()`：每 20ms 做一次去抖与状态跳变检测
  - 释放沿触发：`PreState != 0 && CurrentState == 0` 时认为一次按键完成
  - 将按键码写入：
    - 传统变量：`Key_Num`
    - RTOS 队列：`xQueueSend(s_keyQueue, &k, 0)`（若调度器已运行）
- `Key_GetNum()`：
  - 若队列可用优先 `xQueueReceive()` 非阻塞取键值
  - 否则回退到 `Key_Num` 的传统逻辑（提高鲁棒性）
- `Key_GetState()` 映射（来自代码判断）：
  - PB1 → 1
  - PA6 → 2
  - PA4 短按 → 3
  - PA4 长按（`press_time > 1000`）→ 4

------

### 6.4 计步：后台 5ms Tick + 偏航角阈值触发

文件：`Hardware/menu.c`（计步后台逻辑） + `Hardware/MPU6050.c`

- `Step_Task`（`User/main.c`）每 5ms 调用 `Steps_BackgroundTick()`
- `Steps_BackgroundTick()`（`Hardware/menu.c`）核心逻辑：
  1. 若 `g_steps_running == 0` 直接返回（后台计步开关）
  2. 调用 `MPU6050_Calculation()` 更新姿态（使用全局 `Yaw`）
  3. 以 `Yaw > 20` 和 `Yaw < -20` 做左右摆动判定
     - `left_step_flag/right_step_flag` 防止单侧重复触发
  4. `steps_num_temp` 每触发一次摆动 +1，**两次摆动算一步**：`g_steps_session++`

## 7. 编译与下载

- 推荐环境：**Keil uVision**
- 工程文件：
  - `Project.uvprojx`
  - `Project.uvoptx`
- 依赖：
  - STM32F10x 标准外设库（已包含在 `Library/`）
  - FreeRTOS 内核（已包含在 `FreeRTOS/`）

基本步骤：

1. 用 Keil 打开 `Project.uvprojx`
2. 选择对应芯片型号/启动文件（工程已提供多种 `startup_stm32f10x_*.s`）
3. 编译
4. 使用 ST-Link/J-Link 下载运行

## 8. 可扩展方向（建议）

- 增加低功耗：在 `UI` 空闲时降低刷新频率/进入睡眠，结合 `vApplicationIdleHook()` 的 `WFI`
- 增加更多传感器页面：复用 `Display_RequestRefresh()` 的显示通知机制
- 将 UI 绘制与业务逻辑进一步解耦：以“页面对象/状态机”组织 `menu.c`

------

## 9. 许可

如无特殊说明，可添加：

- `MIT License`（建议用于开源分享）
- 或按你的课程/比赛要求选择对应 License

## 10. UI 页面结构与导航说明（基于真实页面函数）

> 以下所有页面/跳转逻辑均来自 `Hardware/menu.c`、`Hardware/dino.c`、`Hardware/SetTime.c` 中**已经存在的函数**。

------

### 10.1 页面总览

| 页面类型 | 入口函数             | 说明                                        |
| -------- | -------------------- | ------------------------------------------- |
| 时钟首页 | `First_Page_Clock()` | 默认启动页面，显示 RTC 时间、温湿度、步数等 |
| 主菜单   | `Menu()`             | 所有功能入口                                |
| 设置页面 | `SettingPage()`      | 时间设置等系统配置                          |
| 秒表     | `StopWatch()`        | 计时功能                                    |
| 计步页面 | `Steps()`            | 当前计步会话                                |
| 计步记录 | `Show_Record()`      | 历史步数查看                                |
| 小游戏   | `Dino()`             | 恐龙跳跃小游戏                              |

------

### 10.2 页面跳转关系（逻辑真实）

1. **初始启动**：设备上电后，系统首先进入时钟首页（`First_Page_Clock`），这是整个界面系统的默认初始页面。

2. **触发菜单**：在时钟首页（`First_Page_Clock`）下，用户按下指定按键后，界面会从时钟首页跳转到主菜单页面（`Menu`）。

3. 菜单分支选择

   ：主菜单页面（Menu）包含 5 个可选择的功能项，用户可通过按键选择进入对应功能页面：

   - 选择第一项：进入秒表功能页面（`StopWatch`）；
   - 选择第二项：进入计步功能页面（`Steps`）；
   - 选择第三项：进入记录查看页面（`Show_Record`）；
   - 选择第四项：进入系统设置页面（`SettingPage`）；
   - 选择第五项：进入恐龙小游戏页面（`Dino`）。

   

4. **返回主菜单**：无论进入上述哪一个功能页面（`StopWatch`/`Steps`/`Show_Record`/`SettingPage`/`Dino`），用户均可通过按键操作返回主菜单页面（`Menu`），形成闭环的界面交互逻辑。

### 总结

1. 核心流程：上电→时钟首页→（按键触发）主菜单→各功能页面→（按键）返回主菜单；
2. 交互特点：所有功能页面均以主菜单为中转，最终均可返回主菜单，界面跳转逻辑闭环；
3. 触发方式：界面跳转的核心触发条件是**按键操作**，无自动跳转逻辑。

- **所有页面最终都回到 `Menu()`**
- 页面切换通过按键值（`Key_GetNum()`）驱动
- 页面绘制只写 OLED 显存，不直接刷新屏幕

------

### 10.3 时钟首页（`First_Page_Clock()`）

主要职责：

- 调用 `RTC_GetTime()` / `RTC_GetDate()`
- 通过 `DHT11_CacheGet()` 获取温湿度
- 通过 `Steps_GetSession()` 获取当前步数
- 使用 OLED 绘图接口写入显示缓冲区
- 返回值决定下一页面：
  - `1` → 进入 `Menu()`
  - `2` → 进入 `SettingPage()`

**刷新策略**

- 页面逻辑中只在数据变化或按键触发时调用 `Display_RequestRefresh()`
- 实际刷屏由 `Display_Task` 统一完成

------

### 10.4 菜单系统（`Menu()`）

- 菜单项绘制、光标移动逻辑集中在 `Menu()`
- 使用按键：
  - 上/下：菜单项切换
  - 确认：进入对应功能
- 菜单项最终映射到函数调用：
  - 秒表 → `StopWatch()`
  - 计步 → `Steps()`
  - 游戏 → `Dino()`
  - 设置 → `SettingPage()`

------

### 10.5 秒表（`StopWatch()` / `StopWatch_Tick()`）

- `StopWatch_Tick()`
  - 在 `Tick_Task`（1ms）中周期调用
  - 负责计时自增
- `StopWatch()`
  - 负责 UI 绘制、按键处理
  - 不直接操作计数，只读状态

**体现“逻辑 Tick 与 UI 解耦”的 RTOS 设计思想**

------

### 10.6 计步系统（`Steps()` / `Steps_BackgroundTick()`）

### 前台（UI）

- `Steps()`：显示当前会话步数
- `Show_Record()`：显示历史记录（Flash/Store）

### 后台（实时）

- `Steps_BackgroundTick()`
  - 由 `Step_Task` 每 5ms 调用
  - 使用 MPU6050 的 `Yaw` 判断左右摆动
  - 两次摆动计为一步

### 控制接口

- `Steps_SetRunning(uint8_t on)`
- `Steps_ResetSession()`
- `Steps_GetSession()`

UI 页面只调用接口，不直接参与传感器计算

------

### 10.7 Dino 小游戏（`Dino()` / `Dino_Tick()`）

- `Dino_Tick()`
  - 在 `Tick_Task` 中 1ms 周期运行
  - 控制重力、跳跃、障碍物移动
- `Dino()`
  - 页面绘制与按键处理
  - 游戏状态机（开始 / 运行 / 结束）

------

## 11. FreeRTOS 调度与优先级设计

来自 `User/main.c` 的真实 `xTaskCreate()` 参数：

| 任务         | 优先级 | 设计理由                      |
| ------------ | ------ | ----------------------------- |
| Key_Task     | 高     | 防止按键丢失                  |
| Tick_Task    | 中高   | 秒表 / 游戏需要稳定 Tick      |
| Step_Task    | 中     | 姿态采样需要连续性            |
| UI_Task      | 中     | 页面逻辑                      |
| Display_Task | 低     | 刷新可被延后                  |
| Sensor_Task  | 低     | DHT11 2s 周期，实时性要求最低 |

**设计思想总结：**

- **输入优先于输出**
- **实时逻辑优先于显示**
- **慢速传感器最低优先级**

------

## 12. 项目亮点总结

这个项目在 README 里可以**明确强调**这些点：

- ✔ **真正使用 FreeRTOS 多任务，而非“为了 RTOS 而 RTOS”**
- ✔ 使用 **任务通知 + 互斥锁** 管理 OLED，避免并发刷新
- ✔ DHT11 使用 **缓存 + 失败保护策略**
- ✔ 计步逻辑运行在后台 Task，与 UI 完全解耦
- ✔ 所有 UI 页面仅负责绘制和状态切换，不直接操作硬件
- ✔ 同时包含：
  - 实时系统
  - 图形界面
  - 传感器融合
  - 应用层（游戏）
