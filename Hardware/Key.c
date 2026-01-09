#include "stm32f10x.h"
#include "Delay.h"
#include "Key.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

uint8_t Key_Num;

/* ========== 新增：按键事件队列 ========== */
static QueueHandle_t s_keyQueue = NULL;
#define KEY_QUEUE_LEN   8

static BaseType_t Key_IsSchedulerRunning(void)
{
    return (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING);
}

void Key_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_4;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 创建按键事件队列（仅创建一次） */
    if (s_keyQueue == NULL)
    {
        s_keyQueue = xQueueCreate(KEY_QUEUE_LEN, sizeof(uint8_t));
    }
}

/* 兼容旧接口：非阻塞取键值（优先队列） */
uint8_t Key_GetNum(void)
{
    uint8_t temp = 0;

    if (s_keyQueue != NULL && Key_IsSchedulerRunning())
    {
        if (xQueueReceive(s_keyQueue, &temp, 0) == pdPASS)
        {
            return temp;
        }
        return 0;
    }

    /* 裸机/调度器未启动时：走你原先 Key_Num 逻辑 */
    if (Key_Num)
    {
        temp = Key_Num;
        Key_Num = 0;
        return temp;
    }
    return 0;
}

/* 新增：非阻塞读取（等价于 Key_GetNum，但名字更语义化） */
uint8_t Key_PeekNum(void)
{
    return Key_GetNum();
}

/* 新增：阻塞等待按键事件（timeout_ms=0 立即返回） */
uint8_t Key_WaitNum(uint32_t timeout_ms)
{
    uint8_t temp = 0;

    if (s_keyQueue != NULL && Key_IsSchedulerRunning())
    {
        TickType_t to = (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);
        if (xQueueReceive(s_keyQueue, &temp, to) == pdPASS)
        {
            return temp;
        }
        return 0;
    }

    /* 裸机/调度器未启动：退化为轮询 + 延时 */
    while (timeout_ms--)
    {
        temp = Key_GetNum();
        if (temp) return temp;
        Delay_ms(1);
    }
    return 0;
}

/* ===== 你原有逻辑保持不变 ===== */

int press_time;

void Key3_Tick(void)
{
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0)
    {
        press_time++;
    }

    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 1)
    {
        press_time = 0;
    }
}

uint8_t Key_GetState(void)
{
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
    {
        return 1;
    }
    else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == 0)
    {
        return 2;
    }
    else if ((GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0) && press_time > 1000)
    {
        return 4;
    }
    else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0)
    {
        return 3;
    }
    else
    {
        return 0;
    }
}

void Key_Tick(void)
{
    static uint8_t Count;
    static uint8_t CurrentState, PreState;

    Count++;
    if (Count >= 20)
    {
        Count = 0;
        PreState = CurrentState;
        CurrentState = Key_GetState();

        if (PreState != 0 && CurrentState == 0)
        {
            /* 产生一个按键事件（抬起触发） */
            uint8_t k = PreState;

            /* 兼容旧逻辑 */
            Key_Num = k;

            /* RTOS：推送到队列 */
            if (s_keyQueue != NULL && Key_IsSchedulerRunning())
            {
                (void)xQueueSend(s_keyQueue, &k, 0);
            }
        }
    }
}
