#ifndef __MENU_H
#define __MENU_H

void Peripheral_Init(void);
void Show_Clock_UI(void);
int First_Page_Clock(void);
int SettingPage(void);
int Menu(void);
void StopWatch_Tick(void);
int StopWatch(void);
int LED(void);
int MPU6050(void);
int Game(void);
int Emoji(void);
int Gradienter(void);
void Show_Steps_UI();
int Steps();
int Start_Steps();
int Show_Record();
int DHT11(void);
int DHT11_Page(void);

/* ===== Steps background task interface (ÐÂÔö) ===== */
void Steps_BackgroundInit(void);
void Steps_SetRunning(uint8_t en);
void Steps_ResetSession(void);
uint16_t Steps_GetSession(void);
void Steps_BackgroundTick(void);

#endif