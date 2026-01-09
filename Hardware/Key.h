#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"
#include <stdint.h>

void Key_Init(void);
uint8_t Key_GetNum(void);

void Key_Tick(void);
void Key3_Tick(void);

/* 新增：RTOS 事件驱动接口 */
uint8_t Key_WaitNum(uint32_t timeout_ms);   // 阻塞等待按键（timeout_ms=0 表示立刻返回）
uint8_t Key_PeekNum(void);                  // 非阻塞读取（队列/兼容裸机）

#endif
