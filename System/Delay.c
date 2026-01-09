#include "stm32f10x.h"
#include "Delay.h"

#include "FreeRTOS.h"
#include "task.h"

#ifndef SystemCoreClock
#define SystemCoreClock 72000000UL
#endif

/* ========== TIM4 用于微秒延时 ========== */
static uint8_t s_tim4_inited = 0;

static void Delay_TIM4_InitOnce(void)
{
    if (s_tim4_inited) return;
    s_tim4_inited = 1;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseInitTypeDef tim;
    TIM_TimeBaseStructInit(&tim);
    tim.TIM_Prescaler = 72 - 1;              /* 72MHz / 72 = 1MHz */
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_Period = 0xFFFF;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4, &tim);

    TIM_SetCounter(TIM4, 0);
    TIM_Cmd(TIM4, ENABLE);
}

static void Delay_BusyWait_us(uint32_t us)
{
    Delay_TIM4_InitOnce();

    uint16_t start = (uint16_t)TIM_GetCounter(TIM4);
    while ((uint16_t)(TIM_GetCounter(TIM4) - start) < (uint16_t)us)
    {
        /* busy wait */
    }
}

/* ARMCC5 下用内联汇编读 IPSR，避免 __get_IPSR 依赖 */
static __inline uint32_t Delay_ReadIPSR(void)
{
    uint32_t result;
    __asm { MRS result, IPSR }
    return result;
}

void Delay_us(uint32_t us)
{
    while (us)
    {
        uint32_t chunk = (us > 60000U) ? 60000U : us;
        Delay_BusyWait_us(chunk);
        us -= chunk;
    }
}

void Delay_ms(uint32_t ms)
{
    if (ms == 0) return;

    /* 非中断上下文 + 调度器已运行 => 用 vTaskDelay 让出 CPU */
    if (Delay_ReadIPSR() == 0)
    {
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
        {
            vTaskDelay(pdMS_TO_TICKS(ms));
            return;
        }
    }

    /* 否则退化为忙等 */
    while (ms--)
    {
        Delay_us(1000);
    }
}

void Delay_s(uint32_t s)
{
    while (s--)
    {
        Delay_ms(1000);
    }
}
