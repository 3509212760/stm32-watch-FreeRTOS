#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "MyRTC.h"                                                                                                                                                                                                     
#include "Key.h"
#include "LED.h"
#include "SetTime.h"
#include "menu.h"
#include "MPU6050.h"
#include "Delay.h"
#include <math.h>
#include "dino.h"
#include "AD.h"
#include "Store.h"
#include "DHT11.h"  //新增：引入DHT11头文件

uint8_t KeyNum;

void Peripheral_Init(void)
{
    MyRTC_Init();
    Key_Init();
    LED_Init();
    MPU6050_Init();
    AD_Init();
    DHT11_Init(); // 新增：初始化DHT11对应的GPIO
}



/*----------------------------------首页时钟-------------------------------------*/

uint16_t ADValue;
float VBAT;
int Battery_Capacity;

void Show_Battery(void)
{
	int sum;
	for(int i=0;i<3000;i++)
	{
		ADValue=AD_GetValue();
		sum+=ADValue;
		
	}
	ADValue=sum/3000;
	VBAT=(float)ADValue/4095*3.3;
	Battery_Capacity=(ADValue-3276)*100/819;
	if(Battery_Capacity<0)Battery_Capacity=0;
	
	//OLED_ShowNum(64,0,ADValue,4,OLED_6X8);
	//OLED_Printf(64,8,OLED_6X8,"VBAT:%.2f",VBAT);
	OLED_ShowNum(85,4,Battery_Capacity,3,OLED_6X8);
	OLED_ShowChar(103,4,'%',OLED_6X8);
	
	if(Battery_Capacity==100)OLED_ShowImage(110,0,16,16,Battery);
	else if(Battery_Capacity>=10&&Battery_Capacity<100)
	{
		OLED_ShowImage(110,0,16,16,Battery);
		OLED_ClearArea((112+Battery_Capacity/10),5,(10-Battery_Capacity/10),6);
		OLED_ClearArea(85,4,6,8);
	}
	
	else
	{
		OLED_ShowImage(110,0,16,16,Battery);
		OLED_ClearArea(112,5,10,6);
		OLED_ClearArea(85,4,12,8);
	}
};

void Show_Clock_UI(void)
{
	Show_Battery();
	MyRTC_ReadTime();
	OLED_Printf(0,0,OLED_6X8,"%d-%d-%d",MyRTC_Time[0],MyRTC_Time[1],MyRTC_Time[2]);
	OLED_Printf(16,16,OLED_12X24,"%02d:%02d:%02d",MyRTC_Time[3],MyRTC_Time[4],MyRTC_Time[5]);
	OLED_ShowString(0,48,"菜单",OLED_8X16);
	OLED_ShowString(96,48,"设置",OLED_8X16);
}

int clkflag=1;

int First_Page_Clock(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();

		if(KeyNum==1)//上一项
		{
			clkflag--;
			if(clkflag<=0)clkflag=2;
		}
		else if(KeyNum==2)//下一项
		{
			clkflag++;
			if(clkflag>=3)clkflag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			return clkflag;
		}
		
		else if(KeyNum==4)
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_13);
			GPIO_SetBits(GPIOB, GPIO_Pin_12);
		};
		switch(clkflag)
		{
			case 1:
				Show_Clock_UI();
				OLED_ReverseArea(0,48,32,16);
				OLED_Update();
				break;
			
			case 2:
				Show_Clock_UI();
				OLED_ReverseArea(96,48,32,16);
				OLED_Update();
				break;
		}
	}
}

/*----------------------------------设置界面-------------------------------------*/

void Show_SettingPage_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(0,16,"日期时间设置",OLED_8X16);
}

int setflag=1;
int SettingPage(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t setflag_temp=0;
		if(KeyNum==1)//上一项
		{
			setflag--;
			if(setflag<=0)setflag=2;
		}
		else if(KeyNum==2)//下一项
		{
			setflag++;
			if(setflag>=3)setflag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			setflag_temp=setflag;
		}
		
		if(setflag_temp==1){return 0;}
		else if(setflag_temp==2){SetTime();}
		
		switch(setflag)
		{
			case 1:
				Show_SettingPage_UI();
				OLED_ReverseArea(0,0,16,16);
				OLED_Update();
				break;
			
			case 2:
				Show_SettingPage_UI();
				OLED_ReverseArea(0,16,96,16);
				OLED_Update();
				break;
		}
	}
}

/*----------------------------------滑动菜单界面-------------------------------------*/

uint8_t pre_selection;//上次选择的选项
uint8_t target_selection;//目标选项
uint8_t x_pre=48;//上次选项的x坐标
uint8_t Speed=4;//速度
uint8_t move_flag;//开始移动的标志位，1表示开始移动，0表示停止移动

void Menu_Animation(void)
{
	OLED_Clear();
	OLED_ShowImage(42,10,44,44,Frame);
	
	if(pre_selection<target_selection)
	{
		x_pre-=Speed;
		if(x_pre==0)
		{
			pre_selection++;
			move_flag=0;
			x_pre=48;
		}
	}
	
	if(pre_selection>target_selection)
	{
		x_pre+=Speed;
		if(x_pre==96)
		{
			pre_selection--;
			move_flag=0;
			x_pre=48;
		}
	}
	
	if(pre_selection>=1)
	{
		OLED_ShowImage(x_pre-48,16,32,32,Menu_Graph[pre_selection-1]);
	}
	
	if(pre_selection>=2)
	{
		OLED_ShowImage(x_pre-96,16,32,32,Menu_Graph[pre_selection-2]);
	}
	
	OLED_ShowImage(x_pre,16,32,32,Menu_Graph[pre_selection]);
	OLED_ShowImage(x_pre+48,16,32,32,Menu_Graph[pre_selection+1]);
	OLED_ShowImage(x_pre+96,16,32,32,Menu_Graph[pre_selection+2]);
	
	OLED_Update();
}

void Set_Selection(uint8_t move_flag,uint8_t Pre_Selection,uint8_t Target_Selection)
{
	if(move_flag==1)
	{
		pre_selection=Pre_Selection;
		target_selection=Target_Selection;
		
	}
	Menu_Animation();
}

void MenuToFunction(void)
{
	for(uint8_t i=0;i<=6;i++)
	{
		OLED_Clear();
			if(pre_selection>=1)
		{
			OLED_ShowImage(x_pre-48,16+8*i,32,32,Menu_Graph[pre_selection-1]);
		}
		
		
		OLED_ShowImage(x_pre,16+8*i,32,32,Menu_Graph[pre_selection]);
		OLED_ShowImage(x_pre+48,16+8*i,32,32,Menu_Graph[pre_selection+1]);
		
		OLED_Update();
	}
	
}


uint8_t menu_flag=1;
int Menu(void)
{
	move_flag=1;
	uint8_t DirectFlag=2;//置1：移动到上一项；置2：移动到下一项
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t menu_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			DirectFlag=1;
			move_flag=1;
			menu_flag--;
			if(menu_flag<=0)menu_flag=9;
		}
		else if(KeyNum==2)//下一项
		{
			DirectFlag=2;
			move_flag=1;
			menu_flag++;
			if(menu_flag>=10)menu_flag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			menu_flag_temp=menu_flag;
		}
		
		if(menu_flag_temp==1){return 0;}
		else if(menu_flag_temp==2){MenuToFunction();StopWatch();}
		else if(menu_flag_temp==3){MenuToFunction();LED();}
	    else if(menu_flag_temp==4){MenuToFunction();MPU6050();}
		else if(menu_flag_temp==5){MenuToFunction();Game();}
		else if(menu_flag_temp==6){MenuToFunction();Emoji();}
		else if(menu_flag_temp==7){MenuToFunction();Gradienter();}
		else if(menu_flag_temp==8){MenuToFunction();Steps();}
		else if(menu_flag_temp==9){MenuToFunction();DHT11_Page();}
			

			if(menu_flag==1)
			{
				if(DirectFlag==1)Set_Selection(move_flag,1,0);
				else if(DirectFlag==2)Set_Selection(move_flag,0,0);
			}
			
			else
			{
				if(DirectFlag==1)Set_Selection(move_flag,menu_flag,menu_flag-1);
				else if(DirectFlag==2)Set_Selection(move_flag,menu_flag-2,menu_flag-1);
			}
	}
}

/*----------------------------------秒表-------------------------------------*/

uint8_t hour,min,sec;
void Show_StopWatch_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_Printf(32,20,OLED_8X16,"%02d:%02d:%02d",hour,min,sec);
	OLED_ShowString(8,44,"开始",OLED_8X16);
	OLED_ShowString(48,44,"停止",OLED_8X16);
	OLED_ShowString(88,44,"清除",OLED_8X16);
}

uint8_t start_timing_flag;//1：开始，0：停止
void StopWatch_Tick(void)
{
	static uint16_t Count;
	Count++;
	if(Count>=1000)
	{
		Count=0;
			if(start_timing_flag==1)
		{
			sec++;
			if(sec>=60)
			{
				sec=0;
				min++;
				if(min>=60)
				{
					min=0;
					hour++;
					if(hour>99)hour=0;
				}
			}
		}
	}
	
}


uint8_t stopwatch_flag=1;
int StopWatch(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t stopwatch_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			stopwatch_flag--;
			if(stopwatch_flag<=0)stopwatch_flag=4;
		}
		else if(KeyNum==2)//下一项
		{
			stopwatch_flag++;
			if(stopwatch_flag>=5)stopwatch_flag=1;
		}
		else if(KeyNum==3)//确认
		{
			switch(stopwatch_flag)
			{
				case 1: // 确认返回
					OLED_Clear();
					OLED_Update();
					return 0; // 退出秒表
					
				case 2: // 确认开始计时
					start_timing_flag=1;
					break;
					
				case 3: // 确认停止计时
					start_timing_flag=0;
					break;
					
				case 4: // 确认清除计时
					start_timing_flag=0;
					hour=min=sec=0;
					break;
			}
		}
		// 执行功能后刷新界面
		Show_StopWatch_UI();
		switch(stopwatch_flag)
		{
			case 1: OLED_ReverseArea(0,0,16,16); break;
			case 2: OLED_ReverseArea(8,44,32,16); break;
			case 3: OLED_ReverseArea(48,44,32,16); break;
			case 4: OLED_ReverseArea(88,44,32,16); break;
		}
		
		OLED_Update();
		
	}
}
/*----------------------------------手电筒-------------------------------------*/
void Show_LED_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(20,20,"OFF",OLED_12X24);
	OLED_ShowString(72,20,"ON",OLED_12X24);
}
uint8_t led_flag = 1;
int LED(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t led_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			led_flag--;
			if(led_flag<=0)led_flag=3;
		}
		else if(KeyNum==2)//下一项
		{
			led_flag++;
			if(led_flag>=4)led_flag=1;
		}
		else if(KeyNum==3)//确认
		{
			switch(led_flag)
			{
				case 1: // 确认返回
					OLED_Clear();
					OLED_Update();
					return 0; // 退出手电
					
				case 2: // 确认关闭
					LED_OFF();
					break;
					
				case 3: // 确认开灯
					LED_ON();
					break;
					
			}
		}
		// 执行功能后刷新界面
		Show_LED_UI();
		switch(led_flag)
		{
			case 1: OLED_ReverseArea(0,0,16,16); break;
			case 2: OLED_ReverseArea(20,20,36,24); break;
			case 3: OLED_ReverseArea(72,20,24,24); break;
		}
		
		OLED_Update();
		
	}
}

/*----------------------------------MPU6050-------------------------------------*/

int16_t ax,ay,az,gx,gy,gz;//MPU6050测得的三轴加速度和角速度
float roll_g,pitch_g,yaw_g;//陀螺仪解算的欧拉角
float roll_a,pitch_a;//加速度计解算的欧拉角
float Roll,Pitch,Yaw;//互补滤波后的欧拉角
float a=0.9;//互补滤波器系数
float Delta_t=0.005;//采样周期
double pi=3.1415927;

void MPU6050_Calculation(void)
{
	Delay_ms(5);
	MPU6050_GetData(&ax,&ay,&az,&gx,&gy,&gz);
	
	//通过陀螺仪解算欧拉角
	roll_g=Roll+(float)gx*Delta_t;
	pitch_g=Pitch+(float)gy*Delta_t;
	yaw_g=Yaw+(float)gz*Delta_t;
	
	//通过加速度计解算欧拉角
	pitch_a=atan2((-1)*ax,az)*180/pi;
	roll_a=atan2(ay,az)*180/pi;
	
	//通过互补滤波器进行数据融合
	Roll=a*roll_g+(1-a)*roll_a;
	Pitch=a*pitch_g+(1-a)*pitch_a;
	Yaw=a*yaw_g;
	
}

void Show_MPU6050_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_Printf(0,16,OLED_8X16,"Roll: %.2f",Roll);
	OLED_Printf(0,32,OLED_8X16,"Pitch:%.2f",Pitch);
	OLED_Printf(0,48,OLED_8X16,"Yaw:  %.2f",Yaw);
}

int MPU6050(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		
		OLED_Clear();
		MPU6050_Calculation();
		Show_MPU6050_UI();
		OLED_ReverseArea(0,0,16,16);
		OLED_Update();
	}
}

/*----------------------------------游戏-------------------------------------*/

void Show_Game_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(0,16,"谷歌小恐龙",OLED_8X16);
}

uint8_t game_flag=1;
int Game(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t game_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			game_flag--;
			if(game_flag<=0)game_flag=2;
		}
		else if(KeyNum==2)//下一项
		{
			game_flag++;
			if(game_flag>=3)game_flag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			game_flag_temp=game_flag;
		}
		
		if(game_flag_temp==1){return 0;}
		else if(game_flag_temp==2){DinoGame_Pos_Init();DinoGame_Animation();}
		
		switch(game_flag)
		{
			case 1:
				Show_Game_UI();
				OLED_ReverseArea(0,0,16,16);
				OLED_Update();
				break;
			
			case 2:
				Show_Game_UI();
				LED_OFF();
				OLED_ReverseArea(0,16,80,16);
				OLED_Update();
				break;
			
			
		
		}
	}
}

/*----------------------------------动态表情包-------------------------------------*/

void Show_Emoji_UI(void)
{
	/*闭眼*/
	for(uint8_t i=0;i<3;i++)
	{
		OLED_Clear();
		OLED_ShowImage(30,10+i,16,16,Eyebrow[0]);//左眉毛
		OLED_ShowImage(82,10+i,16,16,Eyebrow[1]);//右眉毛
		OLED_DrawEllipse(40,32,6,6-i,1);//左眼
		OLED_DrawEllipse(88,32,6,6-i,1);//右眼
		OLED_ShowImage(54,40,20,20,Mouth);
		OLED_Update();
		Delay_ms(100);
	}
	
	/*睁眼*/
	for(uint8_t i=0;i<3;i++)
	{
		OLED_Clear();
		OLED_ShowImage(30,12-i,16,16,Eyebrow[0]);//左眉毛
		OLED_ShowImage(82,12-i,16,16,Eyebrow[1]);//右眉毛
		OLED_DrawEllipse(40,32,6,4+i,1);//左眼
		OLED_DrawEllipse(88,32,6,4+i,1);//右眼
		OLED_ShowImage(54,40,20,20,Mouth);
		OLED_Update();
		Delay_ms(100);
	}
	
	Delay_ms(500);
	
}

int Emoji(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		
		Show_Emoji_UI();
		
	}
}

/*----------------------------------水平仪-------------------------------------*/

void Show_Gradienter_UI(void)
{
	MPU6050_Calculation();
	OLED_DrawCircle(64,32,30,0);
	OLED_DrawCircle(64-Roll,32+Pitch,4,1);
}

int Gradienter(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		OLED_Clear();
		Show_Gradienter_UI();
		OLED_Update();
	}
}
/*----------------------------------步数-------------------------------------*/

void Show_Steps_UI()
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(0,16,"开始",OLED_8X16);
	OLED_ShowString(0,32,"历史记录",OLED_8X16);
}

uint8_t steps_flag=1;
int Steps()
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t steps_flag_temp=0;
		
		if(KeyNum==1)				//上一项
		{
			steps_flag--;
			if(steps_flag<=0)steps_flag=3;
		}
		else if(KeyNum==2)			//下一项
		{
			steps_flag++;
			if(steps_flag>=4)steps_flag=1;
		}
		else if(KeyNum==3)			//确认
		{
			OLED_Clear();
			OLED_Update();
			steps_flag_temp=steps_flag;
		}
		
		if(steps_flag_temp==1){return 0;}
		else if(steps_flag_temp==2){Start_Steps();}
		else if(steps_flag_temp==3){Show_Record();}
		
		switch(steps_flag)
		{
			case 1:
				Show_Steps_UI();
				OLED_ReverseArea(0,0,16,16);
				OLED_Update();
				break;
			
			case 2:
				Show_Steps_UI();
				OLED_ReverseArea(0,16,32,16);
				OLED_Update();
				break;
			
			case 3:
				Show_Steps_UI();
				OLED_ReverseArea(0,32,64,16);
				OLED_Update();
				break;
		}
	}
}

uint8_t left_step_flag,right_step_flag,steps_num_temp;
int Start_Steps()
{
	uint16_t steps_num = 0;
	while(1)
	{
		OLED_Clear();
		MPU6050_Calculation();
				
		if(Yaw > 20 && left_step_flag == 0)
		{
			steps_num_temp++;
			left_step_flag=1;
			right_step_flag=0;
		}
		if(Yaw < -20 && right_step_flag == 0)
		{
			steps_num_temp++;
			right_step_flag=1;
			left_step_flag=0;
		}
		
		if(steps_num_temp / 2 == 1)
		{
			steps_num_temp=0;
			steps_num++;
		}
		
		OLED_ShowNum(30,20,steps_num,5,OLED_12X24);
		OLED_Update();
		
		KeyNum=Key_GetNum();
		if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			
			for(uint8_t i = 12; i > 3; i -= 3)
			{
				Store_Data[i] = Store_Data[i-3];
				Store_Data[i-1] = Store_Data[i-4];
				Store_Data[i-2] = Store_Data[i-5];
			}
			
			MyRTC_ReadTime();			
			Store_Data[3] = steps_num;
			Store_Data[2] = MyRTC_Time[2];
			Store_Data[1] = MyRTC_Time[1];

			Store_Save();
			return 0;
		}
	}
}

int Show_Record()
{
	while(1)
	{
		OLED_Clear();
		
		for(uint8_t y = 0,i = 1; y<64 && i<12; y += 16, i +=3)
		{
			OLED_Printf(0,y,OLED_8X16,"%d月%d日:%d",Store_Data[i],Store_Data[i+1],Store_Data[i+2]);
		}
		
		OLED_Update();
		
		KeyNum=Key_GetNum();
		if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}
	}
}
/*----------------------------------温湿度计-------------------------------------*/
void Show_DHT11_UI(uint8_t T, uint8_t H)
{
    OLED_ShowImage(0,0,16,16,Return);
    OLED_ShowString(32, 0, "Weather", OLED_8X16);
    
    // 显示温度
    OLED_ShowString(20, 24, "Temp:", OLED_8X16);
    OLED_ShowNum(65, 24, T, 2, OLED_8X16);
    OLED_ShowChar(82, 24, 'C', OLED_8X16);
    
    // 显示湿度
    OLED_ShowString(20, 44, "Humi:", OLED_8X16);
    OLED_ShowNum(65, 44, H, 2, OLED_8X16);
    OLED_ShowChar(82, 44, '%', OLED_8X16);
}

int DHT11_Page(void) // 改个名字避免与头文件冲突
{
    uint8_t Temperature = 0, Humidity = 0;
    uint32_t LastUpdate = 0;
    
    // 初次读取
    DHT11_Read_Data(&Temperature, &Humidity);
    
    while(1)
    {
        KeyNum = Key_GetNum();
        if(KeyNum == 3) // 按确认键退出
        {
            OLED_Clear();
            OLED_Update();
            return 0;
        }

        // DHT11读取频率不宜过快，建议每1秒读取一次
        // 这里简单处理，如果项目中已有全局滴答定时器更好
        static int count = 0;
        if(++count > 100) { 
            DHT11_Read_Data(&Temperature, &Humidity);
            count = 0;
        }

        OLED_Clear();
        Show_DHT11_UI(Temperature, Humidity);
        OLED_ReverseArea(0,0,16,16); // 反显返回图标
        OLED_Update();
        Delay_ms(10); // 降低刷新频率，减少功耗
    }
}
