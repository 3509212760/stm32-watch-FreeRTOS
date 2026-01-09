#include "stm32f10x.h"
#include "misc.h"              /* NVIC_PriorityGroupConfig 来自 Library/misc.h */

#include "FreeRTOS.h"
#include "task.h"

#include "OLED.h"
#include "menu.h"
#include "Key.h"
#include "dino.h"

#include "DHT11.h"
#include "LED.h"

/* =========================================================
 * Display notify handle
 * ========================================================= */
static TaskHandle_t s_displayTaskHandle = NULL;

/* 供其他模块调用：请求显示刷新（仅在调度器运行时有效） */
void Display_RequestRefresh(void)
{
    if (s_displayTaskHandle != NULL &&
        xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        xTaskNotifyGive(s_displayTaskHandle);
    }
}

/* ================= 任务声明 ================= */
static void Key_Task(void *pvParameters);
static void Tick_Task(void *pvParameters);
static void UI_Task(void *pvParameters);
static void Sensor_Task(void *pvParameters);
static void Display_Task(void *pvParameters);
static void Step_Task(void *pvParameters);

/* =========================================================
 * main
 * ========================================================= */
int main(void)
{
    SystemInit();

    /* 工程化关键：显式设置 NVIC 优先级分组
     * 推荐 NVIC_PriorityGroup_4：全部抢占优先级，无子优先级
     * 这与 FreeRTOS 的中断优先级规则最匹配
     */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    OLED_Init();
    OLED_Clear();
    OLED_Update();

    Peripheral_Init();   /* 来自 menu.c（你的工程里真实存在） */

    /* 按键扫描任务：1ms */
    xTaskCreate(
        Key_Task,
        "Key",
        128,
        NULL,
        3,
        NULL
    );

    /* 1ms Tick 任务：保持原裸机 TIM2 中断的行为 */
    xTaskCreate(
        Tick_Task,
        "Tick",
        128,
        NULL,
        3,
        NULL
    );

    /* UI / 菜单任务 */
    xTaskCreate(
        UI_Task,
        "UI",
        512,
        NULL,
        2,
        NULL
    );

    /* 传感器任务：2s 采样一次 DHT11，更新缓存 */
    xTaskCreate(
        Sensor_Task,
        "Sensor",
        256,
        NULL,
        1,
        NULL
    );

    /* 专用显示任务：等待通知刷新 OLED */
    xTaskCreate(
        Display_Task,
        "Display",
        256,
        NULL,
        2,
        &s_displayTaskHandle
    );

    /* 步数检测任务 */
    xTaskCreate(
        Step_Task,
        "Step",
        256,
        NULL,
        2,
        NULL
    );

    /* 说明：
     * 你这里原来调用 Display_RequestRefresh() 在 vTaskStartScheduler() 前，
     * 实际不会生效（因为调度器未运行）。
     * 但你已经在启动前 OLED_Update() 画过首屏，同时 UI/传感器里也会触发刷新，
     * 所以这里直接不强求“启动前通知刷新”。
     */

    vTaskStartScheduler();

    while (1)
    {
        /* 一般不会运行到这里 */
    }
}

/* ================= 按键任务 ================= */
static void Key_Task(void *pvParameters)
{
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(1);

    for (;;)
    {
        Key_Tick();
        Key3_Tick();

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* ================= 1ms Tick 任务 ================= */
static void Tick_Task(void *pvParameters)
{
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(1);

    for (;;)
    {
        StopWatch_Tick();
        Dino_Tick();

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* ================= UI 任务 ================= */
static void UI_Task(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        int ret = First_Page_Clock();

        if (ret == 1)
        {
            Menu();
        }
        else if (ret == 2)
        {
            SettingPage();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* ================= 传感器任务 ================= */
static void Sensor_Task(void *pvParameters)
{
    (void)pvParameters;

    /* DHT11 上电后需要稳定时间（很重要） */
    vTaskDelay(pdMS_TO_TICKS(1500));

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(2000);

    /* 启动后先读一次，成功就刷新显示 */
    if (DHT11_CacheUpdate() == 0)
    {
        Display_RequestRefresh();
    }

    for (;;)
    {
        (void)DHT11_CacheUpdate();
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* ================= 显示任务（仅负责 OLED_Update）================= */
static void Display_Task(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        /* 没有刷新请求就阻塞等待，避免无意义刷屏 */
        (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* 关键：锁住 OLED，保证此刻 UI 不会改 DisplayBuf */
        OLED_Lock();
        OLED_Update();
        OLED_Unlock();
    }
}

/* ================= 后台步数检测任务 ================= */
static void Step_Task(void *pvParameters)
{
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(5);   /* 5ms 对应你 Delta_t=0.005 */

    for (;;)
    {
        Steps_BackgroundTick();
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* =========================================================
 * FreeRTOS Hook / Assert
 * ========================================================= */

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    LED_ON();
    while (1)
    {
        /* malloc 失败：死循环定位 */
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    taskDISABLE_INTERRUPTS();
    LED_ON();
    while (1)
    {
        /* 栈溢出：死循环定位 */
    }
}

/* configASSERT 的落点 */
void vAssertCalled(const char *file, int line)
{
    (void)file;
    (void)line;

    taskDISABLE_INTERRUPTS();
    LED_ON();
    while (1)
    {
        /* 断言触发：可在此处下断点查看 file/line */
    }
}

void vApplicationIdleHook(void)
{
    /* 空闲时进入睡眠，等待中断唤醒（SysTick/外设中断等） */
    __asm { WFI }
}
