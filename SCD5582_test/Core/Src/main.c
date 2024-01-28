/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "font5x5.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define DS1307_ADDRESS 0x68 << 1

extern uint8_t menuCounter;
extern uint8_t b1Counter;
extern uint8_t b2Counter;

void clearDisplay(){
	uint8_t clear = 0xC0;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&clear, 1, 1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
}

void writeCharacter(uint8_t digit, char character){
	if(digit < 0 || digit >7){
		return;
	}
	uint8_t adress = 0b10100000 + digit;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&adress, 1, 1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);

	for(int row = 0; row <5; row++){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi1, (uint8_t*)&font[character - 0x20][row], 1, 1);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
	}
}

typedef struct{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t dayofweek;
	uint8_t dayofmonth;
	uint8_t month;
	uint8_t year;
}TIME;

TIME time;

uint8_t decToBcd(int val){
	return (uint8_t)( (val/10*16) + (val%10) );
}

int bcdToDec(uint8_t val){
	return (int)( (val/16*10) + (val%16) );
}

void Set_Time (uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month, uint8_t year){
	uint8_t set_time[7];
	set_time[0] = decToBcd(sec);
	set_time[1] = decToBcd(min);
	set_time[2] = decToBcd(hour);
	set_time[3] = decToBcd(dow);
	set_time[4] = decToBcd(dom);
	set_time[5] = decToBcd(month);
	set_time[6] = decToBcd(year);

	HAL_I2C_Mem_Write(&hi2c1, DS1307_ADDRESS, 0x00, 1, set_time, 7, 1);
}

void Get_Time (void)
{
	uint8_t get_time[7];
	HAL_I2C_Mem_Read(&hi2c1, DS1307_ADDRESS, 0x00, 1, get_time, 7, 1);
	time.seconds = bcdToDec(get_time[0]);
	time.minutes = bcdToDec(get_time[1]);
	time.hour = bcdToDec(get_time[2]);
	time.dayofweek = bcdToDec(get_time[3]);
	time.dayofmonth = bcdToDec(get_time[4]);
	time.month = bcdToDec(get_time[5]);
	time.year = bcdToDec(get_time[6]);
}

void writeTime(uint16_t millisNow){
	uint8_t seconds1, seconds2, minutes1, minutes2, hours1, hours2;
	seconds1 = (time.seconds/10) + '0';
	seconds2 = (time.seconds%10) + '0';
	minutes1 = (time.minutes/10) + '0';
	minutes2 = (time.minutes%10) + '0';
	hours1 = (time.hour/10) + '0';
	hours2 = (time.hour%10) + '0';

	writeCharacter(0,hours1);
	writeCharacter(1,hours2);
	if((millisNow / 500) % 2 == 0){
		writeCharacter(2,':');
	}
	else{
		writeCharacter(2,' ');
	}
	writeCharacter(3,minutes1);
	HAL_Delay(10);
	clearDisplay();
	writeCharacter(4,minutes2);
	if((millisNow / 500) % 2 == 0){
		writeCharacter(5,':');
	}
	else{
		writeCharacter(5,' ');
	}
	writeCharacter(6,seconds1);
	writeCharacter(7,seconds2);
	HAL_Delay(10);
	clearDisplay();
}

void writeDate(){
	uint8_t day1, day2, month1, month2, year1, year2;
	day1 = (time.dayofmonth/10) + '0';
	day2 = (time.dayofmonth%10) + '0';
	month1 = (time.month/10) + '0';
	month2 = (time.month%10) + '0';
	year1 = (time.year/10) + '0';
	year2 = (time.year%10) + '0';

	writeCharacter(0,day1);
	writeCharacter(1,day2);
	writeCharacter(2,'.');
	writeCharacter(3,month1);
	HAL_Delay(10);
	clearDisplay();
	writeCharacter(4,month2);
	writeCharacter(5,'.');
	writeCharacter(6,year1);
	writeCharacter(7,year2);
	HAL_Delay(10);
	clearDisplay();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim6);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
uint8_t brightness = 0xF2;
uint16_t currentTime;

HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
HAL_SPI_Transmit(&hspi1, (uint8_t*)&brightness, 1, 1);
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);

  while (1)
  {
	 Get_Time();

	 currentTime = __HAL_TIM_GET_COUNTER(&htim6);

	 if(currentTime <= 17000){
		  clearDisplay();
		  writeTime(currentTime);
	 }

	 if(currentTime > 17000 && currentTime <= 19999){
		  clearDisplay();
		  writeDate();
	 }

	 // Postavljanje sati:
	 while(menuCounter == 1){
		 uint8_t hours1, hours2;
		 time.hour = time.hour + b1Counter - b2Counter;
		 if(time.hour > 23){
			 time.hour = 0;
		 }
		 if(time.hour < 0){
			 time.hour = 23;
		 }
		 hours1 = (time.hour / 10) + '0';
		 hours2 = (time.hour % 10) + '0';
		 clearDisplay();
		 writeCharacter(0,hours1);
		 writeCharacter(1,hours2);
		 b1Counter = 0;
		 b2Counter = 0;
		 HAL_Delay(1);
	 }
	 // Postavljanje minuta:
	 while(menuCounter == 2){
		 uint8_t minutes1, minutes2;
		 time.minutes = time.minutes + b1Counter - b2Counter;
		 if(time.minutes > 59){
			 time.minutes = 0;
		 }
		 if(time.minutes < 0){
			 time.minutes = 59;
		 }
		 minutes1 = (time.minutes / 10) + '0';
		 minutes2 = (time.minutes % 10) + '0';
		 clearDisplay();
		 writeCharacter(3,minutes1);
		 writeCharacter(4,minutes2);
		 b1Counter = 0;
		 b2Counter = 0;
		 HAL_Delay(1);
	 }
	 // Postavljanje sekundi:
	 while(menuCounter == 3){
		 uint8_t seconds1, seconds2;
		 time.seconds = time.seconds + b1Counter - b2Counter;
		 if(time.seconds > 59){
			 time.seconds = 0;
		 }
		 if(time.seconds < 0){
			 time.seconds = 59;
		 }
		 seconds1 = (time.seconds / 10) + '0';
		 seconds2 = (time.seconds % 10) + '0';
		 clearDisplay();
		 writeCharacter(6,seconds1);
		 writeCharacter(7,seconds2);
		 b1Counter = 0;
		 b2Counter = 0;
		 HAL_Delay(1);
	 }
	 // Postavljanje dana u mjesecu:
	 while(menuCounter == 4){
		 uint8_t days1, days2;
		 time.dayofmonth = time.dayofmonth + b1Counter - b2Counter;
		 if(time.dayofmonth > 31){
			 time.dayofmonth = 1;
		 }
		 if(time.dayofmonth < 1){
			 time.dayofmonth = 31;
		 }
		 days1 = (time.dayofmonth / 10) + '0';
		 days2 = (time.dayofmonth % 10) + '0';
		 clearDisplay();
		 writeCharacter(0,days1);
		 writeCharacter(1,days2);
		 b1Counter = 0;
		 b2Counter = 0;
		 HAL_Delay(1);
	 }
	 // Postavljanje mjeseca:
	 while(menuCounter == 5){
		 uint8_t month1, month2;
		 time.month = time.month + b1Counter - b2Counter;
		 if(time.month > 12){
			 time.month = 1;
		 }
		 if(time.month < 1){
			 time.month = 12;
		 }
		 month1 = (time.month / 10) + '0';
		 month2 = (time.month % 10) + '0';
		 clearDisplay();
		 writeCharacter(3,month1);
		 writeCharacter(4,month2);
		 b1Counter = 0;
		 b2Counter = 0;
		 HAL_Delay(1);
	 }
	// Postavljanje godine:
	 while(menuCounter == 6){
		 uint8_t year1, year2;
		 time.year = time.year + b1Counter - b2Counter;
		 if(time.year < 23){
			 time.year = 23;
		 }
		 year1 = (time.year / 10) + '0';
		 year2 = (time.year % 10) + '0';
		 clearDisplay();
		 writeCharacter(6,year1);
		 writeCharacter(7,year2);
		 b1Counter = 0;
		 b2Counter = 0;
		 HAL_Delay(1);
	 }

	 if(menuCounter >= 7){
		 Set_Time(time.seconds,time.minutes,time.hour,time.dayofweek,time.dayofmonth,time.month,time.year);
		 menuCounter = 0;
	 }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
