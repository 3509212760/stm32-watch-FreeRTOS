#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

/* =========================================================
 * FreeRTOSConfig for STM32F103 (Cortex-M3)
 * 工程化关键点：
 * 1) 明确优先级位数与 MAX_SYSCALL 配置
 * 2) 打开 configASSERT，便于定位中断优先级/API误用问题
 * ========================================================= */

/* 基本配置 */
#define configCPU_CLOCK_HZ                    ( ( unsigned long ) 72000000 )
#define configTICK_RATE_HZ                    ( ( TickType_t ) 1000 )

#define configUSE_PREEMPTION                  1
#define configUSE_IDLE_HOOK                   1
#define configUSE_TICK_HOOK                   0

#define configMAX_PRIORITIES                  ( 5 )
#define configMINIMAL_STACK_SIZE              ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE                 ( ( size_t ) ( 10 * 1024 ) )
#define configMAX_TASK_NAME_LEN               ( 16 )

#define configUSE_16_BIT_TICKS                0
#define configIDLE_SHOULD_YIELD               1

/* 内存/同步 */
#define configSUPPORT_DYNAMIC_ALLOCATION      1
#define configSUPPORT_STATIC_ALLOCATION       0
#define configUSE_MUTEXES                     1
#define configUSE_RECURSIVE_MUTEXES           1
#define configUSE_COUNTING_SEMAPHORES         1
#define configQUEUE_REGISTRY_SIZE             8

/* 时间片 */
#define configUSE_TIME_SLICING                1

/* 软件定时器 */
#define configUSE_TIMERS                      1
#define configTIMER_TASK_PRIORITY             ( 3 )
#define configTIMER_QUEUE_LENGTH              10
#define configTIMER_TASK_STACK_DEPTH          ( configMINIMAL_STACK_SIZE * 2 )

/* 任务通知 */
#define configUSE_TASK_NOTIFICATIONS          1

/* =========================================================
 * 中断优先级配置（Cortex-M3 工程化关键）
 *
 * STM32F103 一般实现 4bit 优先级（__NVIC_PRIO_BITS=4）
 * StdPeriph 下建议配 NVIC_PriorityGroup_4（全抢占，无子优先级）
 *
 * 约定：
 *  - 数值越小，优先级越高
 *  - 只有 “优先级数值 >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY”
 *    的中断，才允许调用 FreeRTOS 的 FromISR API
 * ========================================================= */
#define configPRIO_BITS                               4

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY       15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY  5

#define configKERNEL_INTERRUPT_PRIORITY               ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY          ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* =========================================================
 * Hook & Debug
 * ========================================================= */
#define configUSE_MALLOC_FAILED_HOOK                  1
#define configCHECK_FOR_STACK_OVERFLOW                2

/* 开启 assert（强烈建议） */
void vAssertCalled(const char *file, int line);
#define configASSERT(x)  if ((x) == 0) { vAssertCalled(__FILE__, __LINE__); }

/* =========================================================
 * API 包含项
 * ========================================================= */
#define INCLUDE_vTaskPrioritySet       1
#define INCLUDE_uxTaskPriorityGet      1
#define INCLUDE_vTaskDelete            1
#define INCLUDE_vTaskSuspend           1
#define INCLUDE_vTaskDelayUntil        1
#define INCLUDE_vTaskDelay             1

/* =========================================================
 * Cortex-M3 端口中断入口映射
 * 说明：port.c 内实现的是 xPortSysTickHandler 等符号，
 * 通过宏映射到真实向量表函数名（SysTick_Handler 等）
 * ========================================================= */
#define xPortPendSVHandler             PendSV_Handler
#define vPortSVCHandler                SVC_Handler
#define xPortSysTickHandler            SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
