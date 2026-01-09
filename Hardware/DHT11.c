#include "DHT11.h"
#include "Delay.h"

/* ===== 新增：FreeRTOS 互斥与临界区 ===== */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define DHT11_IO_OUT() {GPIOA->CRL&=0xFFFFF0FF; GPIOA->CRL|=0x00000300;} // PA2 推挽输出
#define DHT11_IO_IN()  {GPIOA->CRL&=0xFFFFF0FF; GPIOA->CRL|=0x00000800;} // PA2 上拉输入
#define DHT11_DQ_OUT(x) GPIO_WriteBit(GPIOA, GPIO_Pin_2, (BitAction)x)
#define DHT11_DQ_IN    GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2)

void DHT11_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    DHT11_IO_OUT();
    DHT11_DQ_OUT(1);
}

// 复位DHT11
void DHT11_Rst(void)
{
    DHT11_IO_OUT();
    DHT11_DQ_OUT(0);
    Delay_ms(20);
    DHT11_DQ_OUT(1);
    Delay_us(30);
}

// 等待DHT11回应
uint8_t DHT11_Check(void)
{
    uint8_t retry=0;
    DHT11_IO_IN();
    while (DHT11_DQ_IN && retry<100) {retry++; Delay_us(1);}
    if(retry>=100) return 1; else retry=0;
    while (!DHT11_DQ_IN && retry<100) {retry++; Delay_us(1);}
    if(retry>=100) return 1;
    return 0;
}

uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry=0;
    while(DHT11_DQ_IN && retry<100) {retry++; Delay_us(1);}
    retry=0;
    while(!DHT11_DQ_IN && retry<100) {retry++; Delay_us(1);}
    Delay_us(40); // 等待40us后判断电平
    if(DHT11_DQ_IN) return 1; else return 0;
}

uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, dat=0;
    for (i=0; i<8; i++)
    {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}

uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi)
{
    uint8_t buf[5];
    uint8_t i;
    DHT11_Rst();
    if(DHT11_Check()==0)
    {
        for(i=0; i<5; i++) buf[i]=DHT11_Read_Byte();
        if((uint8_t)(buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
        {
            *humi=buf[0];
            *temp=buf[2];
        }
    }
    else return 1;

    return 0;
}

/* ===================================================================== */
/* ======================= 新增：RTOS 缓存层 ============================ */
/* ===================================================================== */

static SemaphoreHandle_t s_dht_mutex = NULL;
static uint8_t s_temp = 0;
static uint8_t s_humi = 0;
static uint8_t s_valid = 0;

void DHT11_CacheInit(void)
{
    if (s_dht_mutex == NULL)
    {
        s_dht_mutex = xSemaphoreCreateMutex();
    }
    s_valid = 0;
}

/* 读一次 DHT11 并更新缓存（0成功/1失败） */
uint8_t DHT11_CacheUpdate(void)
{
    uint8_t buf[5];
    uint8_t t = 0, h = 0;
    uint8_t ret = 1;

    /* 最多重试 3 次 */
    for (uint8_t attempt = 0; attempt < 3; attempt++)
    {
        /* 1) 复位阶段：不要关中断（这里有 Delay_ms(20) -> vTaskDelay） */
        DHT11_Rst();

        /* 2) 微秒读时序阶段：短暂全局关中断，避免任何中断打断时序 */
        __asm { CPSID I }

        if (DHT11_Check() == 0)
        {
            for (uint8_t i = 0; i < 5; i++)
            {
                buf[i] = DHT11_Read_Byte();
            }

            __asm { CPSIE I }

            /* 校验 */
            if ((uint8_t)(buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
            {
                h = buf[0];
                t = buf[2];
                ret = 0;
                break;
            }
        }
        else
        {
            __asm { CPSIE I }
        }

        /* 失败后给传感器一点恢复时间再试 */
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    if (ret == 0)
    {
        if (s_dht_mutex != NULL)
        {
            (void)xSemaphoreTake(s_dht_mutex, portMAX_DELAY);
            s_temp = t;
            s_humi = h;
            s_valid = 1;
            (void)xSemaphoreGive(s_dht_mutex);
        }
        else
        {
            s_temp = t;
            s_humi = h;
            s_valid = 1;
        }
    }
    /* 失败：不清 s_valid，不写 0，保持“最后一次有效值” */

    return ret;
}

/* 从缓存取数据：返回1表示数据有效，0表示还没成功采样过 */
uint8_t DHT11_CacheGet(uint8_t *temp, uint8_t *humi)
{
    if (temp == NULL || humi == NULL) return 0;

    if (s_dht_mutex != NULL)
    {
        if (xSemaphoreTake(s_dht_mutex, pdMS_TO_TICKS(5)) == pdPASS)
        {
            *temp = s_temp;
            *humi = s_humi;
            uint8_t v = s_valid;
            (void)xSemaphoreGive(s_dht_mutex);
            return v;
        }
        return 0;
    }
    else
    {
        *temp = s_temp;
        *humi = s_humi;
        return s_valid;
    }
}
