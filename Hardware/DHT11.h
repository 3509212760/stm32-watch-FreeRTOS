#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"
#include <stdint.h>

void DHT11_Init(void);
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);

/* ===== 新增：RTOS 共享缓存接口 ===== */
void DHT11_CacheInit(void);
uint8_t DHT11_CacheUpdate(void);                 /* 读一次传感器并更新缓存，返回0成功/1失败 */
uint8_t DHT11_CacheGet(uint8_t *temp, uint8_t *humi); /* 从缓存取数据，返回1表示数据有效 */

#endif
