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
int mLCD_showTime(void);

Env InRoomEvn;

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
            BH1750_Init();

            DHT_GetTemHumi();

            mLCD_resetLcd();
            LCD_Clear(BLACK);
            mLCD_showTitle();
            if (TM_DS1307_Init() != TM_DS1307_Result_Ok)
            {
                /*Warning that RTC error*/
                TM_DISCO_LedOn(LED_ALL);
            }
            TM_DS1307_GetDateTime(&time);
            Time_Start=time.minutes;
            
            mLCD_showTime();
            
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
            vTaskDelay( 500 / portTICK_RATE_MS );
            TM_DISCO_LedToggle(LED_ORANGE);
            InRoomEvn.AnhSang = BH1750_Read();
            InRoomEvn.DoAmKhi = DHT_doam();
            InRoomEvn.NhietDo = DHT_nhietdo();
        }
    }

static void vDA2_ShowLCD(void *pvParameters)
    {
        for(;;)
        {
            vTaskDelay( 500 / portTICK_RATE_MS );
            LCD_CharSize(16);
            LCD_SetTextColor(RED);
            TM_DS1307_GetDateTime(&time);
            sprintf(sbuff, "%s,%02d,%s,%04d", date[time.day], time.date,Month[time.month], time.year + 2000);
            
            LCD_StringLine(86, 230, (uint8_t *)sbuff);
            LCD_CharSize(16);
            sprintf(sbuff, "Temperature:%d",InRoomEvn.NhietDo);
            LCD_StringLine(134, 306, (uint8_t *)sbuff);
            sprintf(sbuff, "Humidity:%02d", InRoomEvn.DoAmKhi);
            LCD_StringLine(134, 120, (uint8_t *)sbuff);
            sprintf(sbuff, "Light: %d",InRoomEvn.AnhSang);
            LCD_StringLine(168, 306, (uint8_t *)sbuff);
            mLCD_showTime();
        }
    }

int mLCD_resetLcd()
    {
        LCD_CharSize(16);
        LCD_SetTextColor(RED);
        TM_DS1307_GetDateTime(&time);
        sprintf(sbuff, "%s,%02d,%s,%04d", date[time.day], time.date,Month[time.month], time.year + 2000);
        LCD_StringLine(86, 230, (uint8_t *)sbuff);    
        LCD_CharSize(16);
        sprintf(sbuff, "Temperature:%d",InRoomEvn.NhietDo);
        LCD_StringLine(134, 306, (uint8_t *)sbuff);
        sprintf(sbuff, "Humidity:%02d", InRoomEvn.DoAmKhi);
        LCD_StringLine(134, 120, (uint8_t *)sbuff);
        sprintf(sbuff, "Light: %d",InRoomEvn.AnhSang);
        LCD_StringLine(168, 306, (uint8_t *)sbuff);
        sprintf(sbuff, "Current:%s", Mua[NRF.Mua]);
        LCD_StringLine(168, 120, (uint8_t *)sbuff);
        mLCD_showTime();
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
        LCD_SetTextColor(RED);
        LCD_StringLine(42,240,(unsigned char*)"E_DESK");
        return 1;
    }
    
int mLCD_showTime()
    {
        LCD_SetTextColor(GREEN);
        LCD_CharSize(24);
        sprintf(sbuff, "%02d:%02d:%02d", time.hours, time.minutes, time.seconds);
        LCD_StringLine(62, 235, (uint8_t *)sbuff);
        if (time.hours == 0 && time.minutes == 00 && time.seconds == 0)
        {
            LCD_Clear_P(BLACK, 86, 320, 5120);
            LCD_CharSize(16);
            sprintf(sbuff, "%s,%d,%s,%d", date[time.day], time.date, Month[time.month],time.year + 2000);
            LCD_StringLine(86, 230, (uint8_t *)sbuff);
        }
        return 1;
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
