/**
 *	Keil project for USART using strings
 *
 *  Before you start, select your target, on the right of the "Load" button
 *
 *	@author		Tilen Majerle
 *	@email		tilen@majerle.eu
 *	@website	http://stm32f4-discovery.com
 *	@ide		Keil uVision 5
 *	@packs		STM32F4xx Keil packs version 2.2.0 or greater required
 *	@stdperiph	STM32F4xx Standard peripheral drivers version 1.4.0 or
 *greater required
 */
#include "main.h"
/*
RTOS TASK

*/
#define STACK_SIZE_MIN 128 /* usStackDepth	- the stack size DEFINED IN \
                              WORDS.*/
/*define struct of weather forecast*/
    
typedef struct struct_MyDateTime
{
	int day;
	int month;
	int year;

	int hour;
	int minute;

}MyDateTime;

typedef struct 
{
    uint8_t Temp; 
    uint8_t Humi; 
    long Pres;   
    uint8_t Rain;    
} WeatherRoot;

typedef struct
{
    MyDateTime currentDateTime;
    WeatherRoot currentWeather[5];
    float presDown;
    float rateInYear;
    float rateInDay;
    float output;
    double totalDecrease;
    char* strWeather;
    float ratio;
}WeatherForecast;

//Weather WeatherArray[24];
float PresDownArray[24];
double prePresure = 0;
static uint8_t pipe_check=0xff;

WeatherForecast WthFor;

uint32_t multiplier;
uint8_t pipeflag[6]= {0,0,0,0,0,0};
uint8_t dataOut[32], dataIn0[32],dataIn1[32],dataIn2[32];

xTaskHandle ptr_readRf;
xTaskHandle ptr_Fuzzy;
xTaskHandle ptr_SendThgSpk;
xTaskHandle ptr_showLCD;
xTaskHandle ptr_readRTC;

//static int vDA2_readRf(void *pvParameters); /*Nhận dữ liệu từ RF*/
static void vDA2_Fuzzy(void *pvParameters);
//void vDA2_showLCD(void *pvParameters);
static void vDA2_readRTC(void *pvParameters);
//static int vDA2_sendThgSpk(void *pvParameters);
//static int vDA2_readRTC(void *pvParameters);

/*declarece*/
int initMain(int flag);
int initTask(void);
int showDefaultLCD(void);
int ResetWeather(WeatherForecast weatherForecast);
float pushPresDownArray(float pushValue);
int main() 
{
    initMain(1);
    /* Init ESP module */
    if (TM_DS1307_Init() != TM_DS1307_Result_Ok)
    {
        /*Warning that RTC error*/
        TM_DISCO_LedOn(LED_RED);
    }
    resetLcd();
    while (TM_NRF24L01_DataReady() != 1);
    RF_receive(&NRF);
    resetLcd();
    ResetWeather(WthFor);
    Time_Start = time.minutes;
    setEsp(USART, PINPACK, BAUDRATE);
    connect();
    senddata(NRF);

    if(initTask())
    {
    vTaskStartScheduler();
    }
}

//int vDA2_sendThgSpk()
//{
//  TM_DS1307_GetDateTime(&time);
//  Time_Write = time.minutes - Time_Start;
//  if ((time.minutes - Time_Start >= 5) ||
//      ((time.minutes - Time_Start >= -55) && (time.minutes - Time_Start < 0))) {
//    Time_Start = time.minutes;
//    if ((temp != count24)) {
//      senddata(NRF);
//      temp = count24;
//      LCD_Clear_P(BLACK, 134, 0, 32436);
//      LCD_CharSize(16);
//      LCD_SetTextColor(RED);
//      sprintf(sbuff, "Temperature:%d", NRF.NhietDo);
//      LCD_StringLine(134, 306, (uint8_t *)sbuff);
//      sprintf(sbuff, "Humidity:%d", NRF.DoAm);
//      LCD_StringLine(134, 120, (uint8_t *)sbuff);
//      sprintf(sbuff, "Pressure:%ld", NRF.ApSuat);
//      LCD_StringLine(168, 306, (uint8_t *)sbuff);
//      sprintf(sbuff, "Current:%s", Mua[NRF.Mua]);
//      LCD_StringLine(168, 120, (uint8_t *)sbuff);
//      sprintf(sbuff, "Total Decrease:%f", mainForecast.totalPe);
//      LCD_StringLine(200, 306, (uint8_t *)sbuff);
//      LCD_Clear_P(BLACK, 110, 0, 4000);
//      LCD_SetTextColor(GREEN);
//      LCD_CharSize(16);
//      sprintf(sbuff, "Forecast:%s After %d hours(%2g%%)", "Rain", 2, mainForecast.u * 100);
//      LCD_StringLine(110, 300, (uint8_t *)sbuff);
//    }
//  }
//  return 1;
//}

int resetWeather(WeatherForecast weatherForecast)
{
    int arrayTemp=0;
    weatherForecast.currentDateTime.hour = 0;
    weatherForecast.currentDateTime.day = 0;
    weatherForecast.currentDateTime.minute = 0;
    weatherForecast.currentDateTime.month =0;
    weatherForecast.currentDateTime.year =0;
    for(arrayTemp =0; arrayTemp <5;arrayTemp++)
    {
    weatherForecast.currentWeather[arrayTemp].Humi=0;
    weatherForecast.currentWeather[arrayTemp].Temp=0;
    weatherForecast.currentWeather[arrayTemp].Pres=0;
    weatherForecast.currentWeather[arrayTemp].Rain=0;
    }
    
    weatherForecast.output =0;
    weatherForecast.presDown=0;
    weatherForecast.rateInDay=0;
    weatherForecast.rateInYear=0;
    weatherForecast.totalDecrease=0;
    weatherForecast.ratio=0;
    weatherForecast.strWeather = "NULL";
    return 1;
}
    
static void vDA2_readRf(void *pvParameters) 
{
  while(1)
  {
    if (TM_NRF24L01_DataReady())
      {
                    /*
          *If node 1
          *     receiver WthFor.currentWeather[1];
                
          If node 2
          *     receiver WthFor.currentWeather[2];
          If node 3
          *     receiver WthFor.currentWeather[3];
          If node 4
          *     receiver WthFor.currentWeather[4];
          */
          pipe_check = TM_NRF24L01_CHECK_PIPE();
          RF_receive(&NRF);
          switch(pipe_check)
          {
          case 0:
                WthFor.currentWeather[0].Temp = NRF.NhietDo;
                WthFor.currentWeather[0].Humi = NRF.DoAm;
                WthFor.currentWeather[0].Pres = NRF.ApSuat;
                WthFor.currentWeather[0].Rain = NRF.Mua;
              break;
          case 1:
                WthFor.currentWeather[1].Temp = NRF.NhietDo;
                WthFor.currentWeather[1].Humi = NRF.DoAm;
                WthFor.currentWeather[1].Pres = NRF.ApSuat;
                WthFor.currentWeather[1].Rain = NRF.Mua;
              break;
          case 2:
                WthFor.currentWeather[2].Temp = NRF.NhietDo;
                WthFor.currentWeather[2].Humi = NRF.DoAm;
                WthFor.currentWeather[2].Pres = NRF.ApSuat;
                WthFor.currentWeather[2].Rain = NRF.Mua;
              break;
          case 3:
                WthFor.currentWeather[3].Temp = NRF.NhietDo;
                WthFor.currentWeather[3].Humi = NRF.DoAm;
                WthFor.currentWeather[3].Pres = NRF.ApSuat;
                WthFor.currentWeather[3].Rain = NRF.Mua;
              break;
          default:
                WthFor.currentWeather[0].Temp = NRF.NhietDo;
                WthFor.currentWeather[0].Humi = NRF.DoAm;
                WthFor.currentWeather[0].Pres = NRF.ApSuat;
                WthFor.currentWeather[0].Rain = NRF.Mua;
          }
          for(int i = 0 ; i < 4 ; i++)
          {
            WthFor.currentWeather[4].Temp = ;
            WthFor.currentWeather[4].Humi = NRF.DoAm;
            WthFor.currentWeather[4].Pres = NRF.ApSuat;
            WthFor.currentWeather[4].Rain = NRF.Mua;
          }
          
          
      }
  }
}
/*Function*/
//static void vDA2_readRTC(void *pvParameters)
//{
//    while(1)
//    {
//    TM_DS1307_GetDateTime(&time);
//    LCD_Clear_P(BLACK, 62, 0, 4000);
//    LCD_CharSize(24);
//    sprintf(sbuff, "%02d:%02d:%02d", time.hours, time.minutes, time.seconds);
//    LCD_StringLine(62, 235, (uint8_t *)sbuff);
//    if (time.hours == 0 && time.minutes == 00 && time.seconds == 0)
//        {
//            LCD_Clear_P(BLACK, 86, 0, 4000);
//            LCD_CharSize(16);
//            sprintf(sbuff, "%s,%d,%s,%d", date[time.day], time.date, Month[time.month],time.year + 2000);
//            LCD_StringLine(86, 306, (uint8_t *)sbuff);
//        }
//    }
//}

//int vDA2_showLCD(void *pvParameters)
//{
//    showTime();
//}
/*Function*/
//int resetLcd()
//{

//  TM_DS1307_GetDateTime(&time);
//  LCD_CharSize(16);
//  LCD_SetTextColor(RED);
//  sprintf(sbuff, "%s,%02d,%s,%04d", date[time.day], time.date,Month[time.month], time.year + 2000);
//  LCD_StringLine(86, 230, (uint8_t *)sbuff);
//  TM_DS1307_GetDateTime(&time);
//  LCD_CharSize(16);
//  sprintf(sbuff, "Temperature:%02d", NRF.NhietDo);
//  LCD_StringLine(134, 306, (uint8_t *)sbuff);
//  sprintf(sbuff, "Humidity:%02d", NRF.DoAm);
//  LCD_StringLine(134, 120, (uint8_t *)sbuff);
//  sprintf(sbuff, "Pressure: %ld", NRF.ApSuat);
//  LCD_StringLine(168, 306, (uint8_t *)sbuff);
//  sprintf(sbuff, "Current:%s", Mua[NRF.Mua]);
//  LCD_StringLine(168, 120, (uint8_t *)sbuff);
//  sprintf(sbuff, "Total Decrease:%g", WthFor.totalDecrease);
//  LCD_StringLine(200, 306, (uint8_t *)sbuff);
//  LCD_CharSize(24);
//  LCD_SetTextColor(GREEN);
//  sprintf(sbuff, "%02d:%02d:%02d", time.hours, time.minutes, time.seconds);
//  LCD_StringLine(62, 235, (uint8_t *)sbuff);
//  LCD_CharSize(16);
//  sprintf(sbuff, "Forecast:%s After %d hours(%2g%%)", "Rain", 2, WthFor.output * 100);
//  LCD_StringLine(110, 300, (uint8_t *)sbuff);
//  return 1;
//}
/*Function*/
void vDA2_Fuzzy(void *pvParameters)
{
	while(1)
	{
        WthFor.currentDateTime.month = time.month;
        WthFor.currentDateTime.hour = time.hours;
        /*
        *Cho nay gan vao trung binh cong cua 4 node ( WthFor.currentWeather[0]
        */
        WthFor.rateInYear = percentMonth(WthFor.currentDateTime.month);
        WthFor.rateInDay = percentDay(WthFor.currentDateTime.hour);
        if (prePresure == 0)
        {
            WthFor.presDown = 0;
        }
        else
        {
            WthFor.presDown = ((WthFor.currentWeather[4].Pres-prePresure) / prePresure) * 100000;
        }
        WthFor.totalDecrease = pushPresDownArray(WthFor.presDown);
        prePresure = WthFor.currentWeather[4].Pres;
        WthFor.output = fuzzy_control_lwf(
                        WthFor.currentWeather[4].Temp,
                        WthFor.currentWeather[4].Humi,
                        WthFor.totalDecrease,
                        WthFor.rateInYear,
                        WthFor.rateInDay,
                        &fuzzy_system_lwf);
        if (WthFor.output <= 0.575)
        {
            WthFor.strWeather = "NOT_RAIN";
        }
        else if(WthFor.output <=0.605)
        {
            WthFor.strWeather = "LITTLE_RAIN";
        }
        else
        {
            WthFor.strWeather = "HEAVY_RAIN";
        }
    }
}
/*Function*/
int initTask()
{
    xTaskCreate(vDA2_readRf, (const signed char *)"readRf", STACK_SIZE_MIN,NULL, (unsigned)1,&ptr_readRf);
    xTaskCreate(vDA2_Fuzzy,(const signed char*)"Fuzzy",STACK_SIZE_MIN,NULL,tskIDLE_PRIORITY,&ptr_Fuzzy);
    // xTaskCreate(vDA2_showLCD, (const signed char *)"showLCD", STACK_SIZE_MIN,ptr_showLCD, tskIDLE_PRIORITY, NULL);
    // xTaskCreate(vDA2_SendThgSpk,(const signed char*)"sendThgSpk",STACK_SIZE_MIN, ptr_SendThgSpk,2, NULL );
    xTaskCreate(vDA2_readRTC,(const signed char*)"readRTC",STACK_SIZE_MIN,NULL,tskIDLE_PRIORITY,&ptr_readRTC);
    return 1;
}

float pushPresDownArray(float pushValue)
{
	/*push to PresDownArray pushvalue and return total presdown*/
	int array_size = sizeof(PresDownArray) / sizeof(PresDownArray[0]);
	float totalPresDown = 0;
	for (int j = array_size - 1; j > 0; j--)
	{
		PresDownArray[j] = PresDownArray[j - 1];
	}
	PresDownArray[0] = pushValue;
	for (int i = 0; i < 24; i++)
	{
		totalPresDown += PresDownArray[i];
	}
	return totalPresDown;
}

int initMain(int flag)
{
    if(flag)
    {
        SystemInit();
        TM_DISCO_LedInit();
        TM_DELAY_Init();
        LCD_Init();
        setRF();
        fuzzy_init_lwf(&fuzzy_system_lwf);
        LCD_Clear(BLACK);
    }
    return 1;
}

int showDefaultLCD()
{
    LCD_SetBackColor(BLACK);
    LCD_SetTextColor(BLUE);
    LCD_CharSize(16);
    LCD_StringLine(14,290,(unsigned char*)"UNIVERSITY OF INFOMATION TECHNOLOGY");
    LCD_StringLine(28,275,(unsigned char*)"FACULTY OF COMPUTER ENGINEERING");
    LCD_SetTextColor(RED);
    LCD_StringLine(42,240,(unsigned char*)"LOCAL WEATHER FORECAST");
    return 1;
}

int averageWth(WeatherForecast _wthFor[5])
{
    
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line) {
  /* User can add his own implementation to report the file name and line
     number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1) {
  }
}
#endif
