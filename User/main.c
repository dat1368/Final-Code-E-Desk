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
uint8_t light;
/*xTaskHandle*/
xTaskHandle ptr_ReadSensor;
xTaskHandle ptr_showLCD;
/**/
static void vDA2_ReadSensor(void *pvParameters);
static void vDA2_ShowLCD   (void *pvParameters);

/*LCD functions*/
int mLCD_resetLcd(void);
int mLCD_showTitle(void);
int mLCD_showDateTime(void);
int mLCD_showSensor(void);

float getDistance(uint16_t voltaValue);

Env InRoomEvn;
USERDATA_TYPE userData;

int main() 
{
    initMain(1);
    if(initTask())
    {
        vTaskStartScheduler();
    }
    for(;;);
}

int initMain(int flag)
{
    if(flag)
    {
        SystemInit();

        TM_DISCO_LedInit();
        TM_DISCO_ButtonInit();
        TM_DELAY_Init();
        
        LCD_Init();

        I2C1_Init();
        //BH1750_Init();

        DHT_GetTemHumi();
        
        TM_ADC_Init(ADC1,ADC_Channel_0);

        mLCD_resetLcd();
        LCD_Clear(BLACK);
        mLCD_showTitle();
        
        if (TM_DS1307_Init() != TM_DS1307_Result_Ok)
        {
            /*Warning that RTC error*/
            TM_DISCO_LedOn(LED_ALL);
        }
            
        if(0)
        {
            /* Set date and time */
            /* Day 7, 26th May 2014, 02:05:00 */
            time.hours = 16;
            time.minutes = 33;
            time.seconds = 20;
            time.date = 11;
            time.day = 8;
            time.month = 9;
            time.year = 16;
            TM_DS1307_SetDateTime(&time);

            /* Disable output first */
            TM_DS1307_DisableOutputPin();

            /* Set output pin to 4096 Hz */
            TM_DS1307_EnableOutputPin(TM_DS1307_OutputFrequency_4096Hz);
        }
        
        TM_DS1307_GetDateTime(&time);
            
        mLCD_resetLcd();
        
        InRoomEvn.AnhSang = 0;
        InRoomEvn.DoAmKhi = 0;
        InRoomEvn.DoAmKhi = 0;
    }
    return 1;
}

int initTask()
{
    xTaskCreate(vDA2_ReadSensor,(const signed char*)"vDA2_ReadSensor",STACK_SIZE_MIN,NULL,tskIDLE_PRIORITY+2,&ptr_ReadSensor);
    xTaskCreate(vDA2_ShowLCD, (const signed char *)"vDA2_ShowLCD", STACK_SIZE_MIN,NULL, tskIDLE_PRIORITY+1,&ptr_showLCD);
    return 1;
}

static void vDA2_ReadSensor(void *pvParameters)
{
    for(;;)
    {
        vTaskDelay( 300 / portTICK_RATE_MS );
        TM_DISCO_LedToggle(LED_ORANGE);
        //InRoomEvn.AnhSang = BH1750_Read();
        InRoomEvn.DoAmKhi = DHT_doam();
        InRoomEvn.NhietDo = DHT_nhietdo();
        
        userData.distance = getDistance(TM_ADC_Read(ADC1,ADC_Channel_0));
        
        TM_DS1307_GetDateTime(&time);
    }
}

static void vDA2_ShowLCD(void *pvParameters)
{
    for(;;)
    {
        vTaskDelay( 300 / portTICK_RATE_MS );
        mLCD_showSensor();
        mLCD_showDateTime();
    }
}
    
int mLCD_showSensor()
{
    LCD_CharSize(16);
    LCD_SetTextColor(RED);
    //LCD_Clear_P(WHITE, 110, 240, 5120);
    sprintf(sbuff, "%d*C , %02d lux , %02d humi   ",InRoomEvn.NhietDo,InRoomEvn.AnhSang,InRoomEvn.DoAmKhi);
    LCD_StringLine(110, 250, (uint8_t *)sbuff);
    
    sprintf(sbuff, "Distance : %0.2f",userData.distance);
    
    LCD_StringLine(150, 250, (uint8_t *)sbuff);
    return 1;
}

int mLCD_showDateTime()
{
    LCD_SetTextColor(GREEN);
    LCD_CharSize(24);
    sprintf(sbuff, "%02d:%02d:%02d", time.hours, time.minutes, time.seconds);
    LCD_StringLine(62, 235, (uint8_t *)sbuff);
    if (time.hours == 0 && time.minutes == 00 && time.seconds == 0)
    {
        LCD_Clear_P(BLACK, 86, 320, 5120);
        LCD_CharSize(16);
        sprintf(sbuff, "%s,%d %s %d", date[time.day], time.date, Month[time.month],time.year + 2000);
        LCD_StringLine(86, 230, (uint8_t *)sbuff);
    }
    return 1;
}

int mLCD_resetLcd()
{

    TM_DS1307_GetDateTime(&time);
    
    LCD_SetTextColor(GREEN);
    LCD_CharSize(24);
    sprintf(sbuff, "%02d:%02d:%02d", time.hours, time.minutes, time.seconds);
    LCD_StringLine(62, 235, (uint8_t *)sbuff);
    
    LCD_Clear_P(BLACK, 86, 320, 5120);
    LCD_CharSize(16);
    sprintf(sbuff, "%s,%d,%s,%d", date[time.day], time.date, Month[time.month],time.year + 2000);
    LCD_StringLine(86, 250, (uint8_t *)sbuff);

    
    LCD_CharSize(16);
    LCD_SetTextColor(RED);
    sprintf(sbuff, "%d*C , %02d lux , %02d humi",InRoomEvn.NhietDo,InRoomEvn.AnhSang,InRoomEvn.DoAmKhi);
    LCD_StringLine(110, 250, (uint8_t *)sbuff);
    /*Reset color*/
    LCD_SetTextColor(WHITE);
    return 1;
}

int mLCD_showTitle()
{
    LCD_Clear_P(BLACK,14,320,8960);
    LCD_SetBackColor(BLACK);
    LCD_SetTextColor(BLUE);
    LCD_CharSize(16);
    LCD_StringLine(14,290,(unsigned char*)"UNIVERSITY OF INFOMATION TECHNOLOGY");
    LCD_StringLine(28,275,(unsigned char*)"FACULTY OF COMPUTER ENGINEERING");
    /*LCD_SetTextColor(RED);
    LCD_StringLine(42,180,(unsigned char*)"E_DESK");*/
    return 1;
}

float getDistance(uint16_t voltaValue)
{
    float volts = voltaValue*0.0048828125;
    // value from sensor * (5/1024) - if running 3.3.volts then change 5 to 3.3
    float distance = 65*pow(volts, -1.10);
    return distance;
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
