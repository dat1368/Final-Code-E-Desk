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
    


//Weather WeatherArray[24];
float PresDownArray[24];
double prePresure = 0;
static uint8_t pipe_check=0xff;

WeatherForecast WthFor;

uint32_t multiplier;
uint8_t pipeflag[6]= {0,0,0,0,0,0};


xTaskHandle ptr_readRf;
xTaskHandle ptr_Fuzzy;
xTaskHandle ptr_SendThgSpk;
xTaskHandle ptr_showLCD;
xTaskHandle ptr_readRTC;

static void vDA2_readRf(void *pvParameters); /*Nhận dữ liệu từ RF*/
static void vDA2_Fuzzy(void *pvParameters);
//void vDA2_showLCD(void *pvParameters);
static void vDA2_readRTC(void *pvParameters);
static void vDA2_sendThgSpk(void *pvParameters);

int main() 
{
    int i =0;
    initMain(1);
    if(initTask())
    {
    vTaskStartScheduler();
    }
    for( ;; );
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
        resetLcd();
        showDefaultLCD();
        
        ResetWeather(WthFor);
        
        if (TM_DS1307_Init() != TM_DS1307_Result_Ok)
        {
            /*Warning that RTC error*/
            TM_DISCO_LedOn(LED_ORANGE);
        }
    }
    return 1;
}

static void vDA2_sendThgSpk(void *pvParameters)
{
    for(;;)
    {
         Time_Start = time.minutes;
         setEsp(USART, PINPACK, BAUDRATE);
         vTaskDelay( 50 / portTICK_RATE_MS );
         connect();
         senddata(NRF);
         TM_DS1307_GetDateTime(&time);
         Time_Write = time.minutes - Time_Start;
         if (   (time.minutes - Time_Start >= 5)
                ||((time.minutes - Time_Start >= -55)
                && (time.minutes - Time_Start < 0))) 
         {
         Time_Start = time.minutes;
         if ((temp != count24)) 
             {
              senddata(NRF);
              temp = count24;
            }
         }
         vTaskSuspend(NULL);
         vTaskResume(ptr_readRf);
    }
}

int ResetWeather(WeatherForecast weatherForecast)
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
    
    weatherForecast.output = 50;
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
    int i =0;

    while(1)
    {
        vTaskDelay( 50 / portTICK_RATE_MS );
        TM_DISCO_LedToggle(LED_ORANGE);
        if (TM_NRF24L01_DataReady())
        {
            TM_DISCO_LedToggle(LED_GREEN);
            RF_receive(&NRF);
            WthFor.currentWeather[4].Temp = NRF.NhietDo;
            WthFor.currentWeather[4].Humi = NRF.DoAm;
            WthFor.currentWeather[4].Pres = NRF.ApSuat;
            WthFor.currentWeather[4].Rain = NRF.Mua;
            if(WthFor.currentWeather[4].Rain == 2)
            {
                WthFor.strForWeather = "Dry";
            }
            else if(WthFor.currentWeather[4].Rain == 1)
            {
                WthFor.strForWeather = "Rain";
            }
            else
            {
                WthFor.strForWeather ="Heavy";
            }
            LCD_showUpdate(0);
            vTaskResume(ptr_Fuzzy);
            vTaskSuspend(ptr_readRf);
            }
    }
}
/*Function*/
static void vDA2_readRTC(void *pvParameters)
{
    while(1)
    {
    vTaskDelay( 50/portTICK_RATE_MS );
    TM_DS1307_GetDateTime(&time);
    LCD_showDateTime();
    }
}

int vDA2_showLCD(void *pvParameters)
{
    LCD_showUpdate(0);
    return 1;
}
/*Function*/
int resetLcd()
{

  TM_DS1307_GetDateTime(&time);
  LCD_CharSize(16);
  LCD_SetTextColor(RED);
  sprintf(sbuff, "%s,%02d,%s,%04d", date[time.day], time.date,Month[time.month], time.year + 2000);
  LCD_StringLine(86, 230, (uint8_t *)sbuff);
  TM_DS1307_GetDateTime(&time);
  LCD_CharSize(16);
  sprintf(sbuff, "Temperature:%02d", NRF.NhietDo);
  LCD_StringLine(134, 306, (uint8_t *)sbuff);
  sprintf(sbuff, "Humidity:%02d", NRF.DoAm);
  LCD_StringLine(134, 120, (uint8_t *)sbuff);
  sprintf(sbuff, "Pressure: %ld", NRF.ApSuat);
  LCD_StringLine(168, 306, (uint8_t *)sbuff);
  sprintf(sbuff, "Current:%s", Mua[NRF.Mua]);
  LCD_StringLine(168, 120, (uint8_t *)sbuff);
  sprintf(sbuff, "Total Decrease:%g", WthFor.totalDecrease);
  LCD_StringLine(200, 306, (uint8_t *)sbuff);
  LCD_CharSize(24);
  LCD_SetTextColor(GREEN);
  sprintf(sbuff, "%02d:%02d:%02d", time.hours, time.minutes, time.seconds);
  LCD_StringLine(62, 235, (uint8_t *)sbuff);
  LCD_CharSize(16);
  sprintf(sbuff, "Forecast:%s After %d hours(%2g%%)", "Rain", 2, WthFor.output * 100);
  LCD_StringLine(110, 300, (uint8_t *)sbuff);
  return 1;
}
/*Function*/
void vDA2_Fuzzy(void *pvParameters)
{
	while(1)
	{
        vTaskDelay( 50 / portTICK_RATE_MS );
        WthFor.currentDateTime.month = time.month;
        WthFor.currentDateTime.hour = time.hours;
        /*
        *Cho nay gan vao trung binh cong cua 4 node ( WthFor.currentWeather[0]
        */
        WthFor.rateInYear = percentMonth(WthFor.currentDateTime.month);
        WthFor.rateInDay = percentDay(WthFor.currentDateTime.hour);
        /* Tranh chia cho 0 */
        if (prePresure == 0)
        {
            WthFor.presDown = 0;
        }
        else
        {
            WthFor.presDown = (((WthFor.currentWeather[4].Pres)-prePresure) / prePresure) * 100000;
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
        vTaskResume(ptr_readRf);
        vTaskSuspend(ptr_Fuzzy);
        //vTaskResume(ptr_SendThgSpk);
    }
}
/*Function*/
int initTask()
{
    xTaskCreate(vDA2_readRf,(const signed char *)"readRf", STACK_SIZE_MIN,NULL, (unsigned)1,&ptr_readRf);
    xTaskCreate(vDA2_Fuzzy,(const signed char*)"Fuzzy",STACK_SIZE_MIN,NULL,tskIDLE_PRIORITY,&ptr_Fuzzy);
    // xTaskCreate(vDA2_showLCD, (const signed char *)"showLCD", STACK_SIZE_MIN,ptr_showLCD, tskIDLE_PRIORITY, NULL);
    //xTaskCreate(vDA2_sendThgSpk,(const signed char*)"sendThgSpk",STACK_SIZE_MIN, ptr_SendThgSpk,2, NULL );
    xTaskCreate(vDA2_readRTC,(const signed char*)"readRTC",STACK_SIZE_MIN,NULL,tskIDLE_PRIORITY,&ptr_readRTC);
    return 1;
}

float pushPresDownArray(float pushValue)
{
	/*push to PresDownArray pushvalue and return total presdown*/
	int j =0,i =0;
    float totalPresDown = 0;
    int array_size = sizeof(PresDownArray) / sizeof(PresDownArray[0]);
	
	for (j = array_size - 1; j > 0; j--)
	{
		PresDownArray[j] = PresDownArray[j - 1];
	}
	PresDownArray[0] = pushValue;
	for (i = 0; i < 24; i++)
	{
		totalPresDown += PresDownArray[i];
	}
	return totalPresDown;
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

int LCD_showDateTime()
{
    LCD_Clear_P(BLACK, 62, 0, 4000);
    LCD_CharSize(24);
    sprintf(sbuff, "%02d:%02d:%02d", time.hours, time.minutes, time.seconds);
    LCD_StringLine(62, 235, (uint8_t *)sbuff);
    if (time.hours == 0 && time.minutes == 00 && time.seconds == 0)
        {
            LCD_Clear_P(BLACK, 86, 0, 4000);
            LCD_CharSize(16);
            sprintf(sbuff, "%s,%d,%s,%d", date[time.day], time.date, Month[time.month],time.year + 2000);
            LCD_StringLine(86, 306, (uint8_t *)sbuff);
        }
}

int LCD_showUpdate(int mode)
{
    switch(mode)
    {
        case 0:
        {
            LCD_Clear_P(BLACK, 134, 0, 32436);
            LCD_CharSize(16);
            LCD_SetTextColor(RED);
            sprintf(sbuff, "Temperature:%d", WthFor.currentWeather[4].Temp);
            LCD_StringLine(134, 306, (uint8_t *)sbuff);
            sprintf(sbuff, "Humidity:%d", WthFor.currentWeather[4].Humi);
            LCD_StringLine(134, 120, (uint8_t *)sbuff);
            sprintf(sbuff, "Pressure:%ld", WthFor.currentWeather[4].Pres);
            LCD_StringLine(168, 306, (uint8_t *)sbuff);
            sprintf(sbuff, "Current:%s", WthFor.strForWeather);
            LCD_StringLine(168, 120, (uint8_t *)sbuff);
            sprintf(sbuff, "Total Decrease:%f", WthFor.totalDecrease);
            LCD_StringLine(200, 306, (uint8_t *)sbuff);
            LCD_Clear_P(BLACK, 110, 0, 4000);
            LCD_SetTextColor(GREEN);
            LCD_CharSize(16);
            sprintf(sbuff, "%s after %d hours[%3.3f%%]",(WthFor.strWeather), 4 , WthFor.output*100);
            LCD_StringLine(110, 300, (uint8_t *)sbuff);
            showDefaultLCD();
            break;
        }
        case 1:
        {
            /*LCD show history*/
            break;
        }
        default:
        {
            break;
        }
    }
    return 1;
}

int averageWth(WeatherForecast _wthFor[5])
{
    
}

//double round(double soCanLamTron, int chuSo)
//{
//    int temp;
//    int i, result = 1;
//   
//    //Pow
//    for(i = 1; i <= chuSo; i++)
//    {
//        result *= 10;
//    }
//   
//    temp = soCanLamTron*result;
//    if(temp%10 >= 5)
//    {
//        temp += 1;
//    }
//    soCanLamTron = (double)temp/result;
//    return soCanLamTron;
//}

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
