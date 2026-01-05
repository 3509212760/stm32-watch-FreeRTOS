#include "DHT11.h"
#include "Delay.h"


#define DHT11_IO_OUT() {GPIOA->CRL&=0xFFFFF0FF; GPIOA->CRL|=0x00000300;} // PA2 推挽输出
#define DHT11_IO_IN()  {GPIOA->CRL&=0xFFFFF0FF; GPIOA->CRL|=0x00000800;} // PA2 上拉输入
#define DHT11_DQ_OUT(x) GPIO_WriteBit(GPIOA, GPIO_Pin_2, (BitAction)x)
#define DHT11_DQ_IN    GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2)

void DHT11_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    DHT11_IO_OUT();
    DHT11_DQ_OUT(1);
}

// 复位DHT11
void DHT11_Rst(void) {                 
    DHT11_IO_OUT();
    DHT11_DQ_OUT(0);
    Delay_ms(20);    // 拉低至少18ms
    DHT11_DQ_OUT(1);
    Delay_us(30);    // 主机拉高20~40us
}

// 等待DHT11回应
uint8_t DHT11_Check(void) {   
    uint8_t retry=0;
    DHT11_IO_IN();
    while (DHT11_DQ_IN && retry<100) {retry++; Delay_us(1);}
    if(retry>=100) return 1; else retry=0;
    while (!DHT11_DQ_IN && retry<100) {retry++; Delay_us(1);}
    if(retry>=100) return 1;
    return 0;
}

uint8_t DHT11_Read_Bit(void) {
    uint8_t retry=0;
    while(DHT11_DQ_IN && retry<100) {retry++; Delay_us(1);}
    retry=0;
    while(!DHT11_DQ_IN && retry<100) {retry++; Delay_us(1);}
    Delay_us(40); // 等待40us后判断电平
    if(DHT11_DQ_IN) return 1; else return 0;
}

uint8_t DHT11_Read_Byte(void) {        
    uint8_t i, dat=0;
    for (i=0; i<8; i++) {
        dat <<= 1; 
        dat |= DHT11_Read_Bit();
    }						    
    return dat;
}

uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi) {        
    uint8_t buf[5];
    uint8_t i;
    DHT11_Rst();
    if(DHT11_Check()==0) {
        for(i=0; i<5; i++) buf[i]=DHT11_Read_Byte();
        if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4]) {
            *humi=buf[0];
            *temp=buf[2];
        }
    } else return 1;
    return 0;
}
