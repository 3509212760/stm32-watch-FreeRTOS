#include "stm32f10x_it.h"

/* =========================
 * Cortex-M3 异常处理
 * ========================= */

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

/* =========================
 * FreeRTOS 使用的中断
 * 注意：这里只做“声明”，不做实现
 * 实现位于 FreeRTOS 的 port.c
 * ========================= */

void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
