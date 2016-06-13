/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/main.h
  * @author  MCD Application Team
  * @version V1.6.0
  * @date    04-September-2015
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
/*======================================
=            Include module            =
======================================*/
/*----------  ST Library  ----------*/
#include "stm32f4xx.h"
#include "integer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tm_stm32f4_disco.h"
#include "tm_stm32f4_usart.h"
#include <string.h>
/*----------  FreeRTOS  ----------*/
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "croutine.h"
/*----------  RTC  ----------*/
#include "tm_stm32f4_ds1307.h"
/*----------  RF  ----------*/
#include "esp8266.h"
/*----------  LCD  ----------*/
#include "SSD1289.h"
/*----------  Fuzzy  ----------*/
#include "Fuzzy.h"
/*=====  End of Include module  ======*/

TM_DS1307_Time_t time;
NRF_ThoiTiet NRF;
uint8_t Time_Start = 0;
int Time_Write = 0;
uint8_t count24;
int temp;
char sbuff[300];

char* date[] = {"",         "Monday", "Tuesday",  "Wednesday",
                "Thursday", "Friday", "Saturday", "Sunday"};
char* Month[] = {"",        "January",  "February", "March",  "April",
                 "May",     "June",     "July",     "August", "September",
                 "October", "November", "December"};
char* Mua[] = {"Heavy", "Rain", "Sunny"};
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

int timeDelay_lwf = 0;
FUZ_SYS_LWF fuzzy_system_lwf;
int readRf(void);
int showTime(void);
int resetLcd(void);
int sendThSp(void);
/*declarece*/
int initMain(int flag);
int initTask(void);
int showDefaultLCD(void);
int ResetWeather(WeatherForecast weatherForecast);
float pushPresDownArray(float pushValue);

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
