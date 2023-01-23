/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Responsible for scheduling events and initializing system components.
  * @author 		: Brandon Thibeaux
  * @date 			: Jan 1, 2023
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include <stdbool.h>
#include "Globals.h"
#include "Structures.h"
#include "App_Config.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define DEBUG_BUFFER_SIZE 100
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Tempo_Button(uint16_t GPIO_Pin);
void print();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t DEBUG_PRINT_BUFFER[DEBUG_BUFFER_SIZE];

uint32_t last_tick = 0;
uint32_t last_tick_debouce = 0;
uint8_t tempo_debounce_interval = 100;
TempoButtonDebouce = 0;
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
  // init variables
  uint8_t bpmButton = 0;
  unsigned char toggleButton = 0 ; // make sure code executes on falling edge
  unsigned long timeoutCount ; // in miliseconds to reset tap count
  bool startTimeout = false;
  uint32_t tap_average_period = 0;

  // init clock objects
  last_tick = HAL_GetTick();
  last_tick_debouce = HAL_GetTick();
  time1 = HAL_GetTick();
  time2 = HAL_GetTick();

  clock1.pin = Clock_Pulse_Pin;
  clock1.port = GPIOB; // assign port number

  clock1.period = 250; // BPM 120

  clock2.pin = Gate_Pulse_Pin;
  clock2.port = GPIOB;

  clock2.period = 250;

  // init rotary encoder

  // init tempo button
  tempoButton.button.buttonState = RELEASED;
  tempoButton.button.port = Tempo_Button_GPIO_Port;
  tempoButton.button.pin = Tempo_Button_Pin;
  tempoButton.tap_count = 0;
  tempoButton.tap_interval_buffer_size = 4;
  tempoButton.tap_intervals[tempoButton.tap_interval_buffer_size];
  memset(tempoButton.tap_intervals,0,tempoButton.tap_interval_buffer_size);

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */


// Start Clock Pulse Timer ISR
  HAL_TIM_Base_Start_IT(&htim2);
//  HAL_GPIO_EXTI_IRQHandler(Tempo_Button_Pin);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t tap_array2[4]; // delete later
  while (1)
  {


// TEMPO BUTTON CODE

	  // Allow another button press to be captured
	  while(((last_tick_debouce + tempo_debounce_interval)< HAL_GetTick()) && (TempoButtonDebouce == 1))
	  {
		  TempoButtonDebouce = 0;
	  }

	  // Tempo Button Logic if pressed
	  GPIO_PinState buttonRead = HAL_GPIO_ReadPin(tempoButton.button.port, tempoButton.button.pin);
	  if(tempoButton.button.buttonState == PRESSED && buttonRead ==  GPIO_PIN_SET )
	  {
		  //clean up
		  tempoButton.button.buttonState = RELEASED;


#ifdef TempoISR_DEBUG
		  sprintf(DEBUG_PRINT_BUFFER,"* Tempo Button released. tap count: %d\r\n",tempoButton.tap_count);
		  print(DEBUG_PRINT_BUFFER,DEBUG_BUFFER_SIZE);
#endif

		  // if we have more than 1 sample, do this
		  if(tempoButton.tap_count > 0)
		  {
			  // REMEMBER we start at 1 and don't get to 0 until we get to 4
			  tempoButton.tap_intervals[(tempoButton.tap_count % tempoButton.tap_interval_buffer_size)] = (HAL_GetTick() - last_tick);
			  volatile uint32_t temp =  tempoButton.tap_intervals[(tempoButton.tap_count % tempoButton.tap_interval_buffer_size)];
			  tap_array2[(tempoButton.tap_count % tempoButton.tap_interval_buffer_size)]= temp;

#ifdef TempoISR_DEBUG
			  sprintf(DEBUG_PRINT_BUFFER,"* Time interval: %d\r\n",tempoButton.tap_intervals[(tempoButton.tap_count % tempoButton.tap_interval_buffer_size)]);
			  print(DEBUG_PRINT_BUFFER,DEBUG_BUFFER_SIZE);
#endif
		  }
		  tempoButton.tap_count++;
		  last_tick = HAL_GetTick();
		  volatile uint32_t debug = tap_array2; // TODO this works, figure out why tempoButton.tap_intervals does not work


	  }
	  if((HAL_GetTick()>(last_tick+timeoutValue)) && (tempoButton.tap_count>0))
	  {
		  tap_average_period = 0; // temp variable

		  // Get Average
		  for(int i = 1; i<tempoButton.tap_interval_buffer_size+1; i++)
		  {
			  // TODO Simplify this
			  volatile uint32_t temp = tempoButton.tap_intervals[i%tempoButton.tap_interval_buffer_size];
			  tap_average_period = tap_average_period + temp;
		  }
		  // TODO Use bit shifting for division
		  tap_average_period  = (tap_average_period/tempoButton.tap_interval_buffer_size);


		  // clean up
		  tempoButton.tap_count = 0;

#ifdef TempoISR_DEBUG
		  	  sprintf(DEBUG_PRINT_BUFFER,"Average Period (ms): %d\r\n",tap_average_period);
		  	  print(DEBUG_PRINT_BUFFER,DEBUG_BUFFER_SIZE);
			  sprintf(DEBUG_PRINT_BUFFER,"****************************\r\n");
			  print(DEBUG_PRINT_BUFFER,DEBUG_BUFFER_SIZE);
#endif
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//void Tempo_Button(uint16_t GPIO_Pin)
//{}
//void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
//{
//	GPIO_PinState buttonRead = HAL_GPIO_ReadPin(tempoButton.button.port, tempoButton.button.port);
//	if(GPIO_Pin == Tempo_Button_Pin && tempoButton.button.buttonState == PRESSED)
//	{
//		tempoButton.tap_count++;
//		tempoButton.button.buttonState = RELEASED;
//
//#ifdef TempoISR_DEBUG
//		sprintf(DEBUG_PRINT_BUFFER,"Tempo Button released. tap count: %d, button state:, %d\r",tempoButton.tap_count,buttonRead);
//		print(DEBUG_PRINT_BUFFER,DEBUG_BUFFER_SIZE);
//#endif
//	}
//}
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	// if we have already entered this state raise this flag. It will later be released in the main loop
	if(TempoButtonDebouce)
	{
		return;
	}
	GPIO_PinState buttonRead = HAL_GPIO_ReadPin(tempoButton.button.port, tempoButton.button.port);

	if(GPIO_Pin == Tempo_Button_Pin && tempoButton.button.buttonState == RELEASED && buttonRead == GPIO_PIN_RESET)
	{
		// load data
		tempoButton.button.buttonState = PRESSED;
	}
	TempoButtonDebouce = 1;
	last_tick_debouce = HAL_GetTick();
}

// This ISR is in charge of pulsing our output clock pins. It schedules when each pin needs to be turn on or off depending
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim2)
	{

		if (divider < 0)
		{
			divider = 0;
		}
		else if(divider > 4)
		{
			divider = 4;
		}

		uint8_t offset = 0; // We are trying to compensate for the delay in scheduling due to the constant calculations and condition testing that needs to be performed in this ISR, it dirty but it works good enough.
		if(divider == 3)
		{
			offset = 1;
		}
		else if(divider == 4)
		{
			offset = 1;
		}
		else
		{
			offset = 0;
		}
		calculatedEnd = (1<<divider);
		calculatedWaitPeriod = ((clock1.period  + (delaybuffer))>>divider) + offset;
		clock2.period = clock1.period; // we want clock2 to be some kind of ratio dervied from clock 1.

		//delaybuffer = delaybuffer/divider;
		// Note: I do not want to rework the whole program to control clock 2. Rather What if we executed this task a
			//fraction of the time so we can toggle clock2 on and off and safe gaurd clock1 with an if statment. This way the tempo will keep its integrity and we still get a controllable gate clock.

			// We want to be able to say clock1.period = 250 ms and clock2. period = clock1.period/4. But to do this we need to execute, toggle and test these conditions more times than we toggle toggle our tempo clock (clock1).
			// so we may need to redefine out schedule condition, take the period of clock 1 and execute task a fraction of that time period. Enabling us to sync and schedule both clock pulses with some ease.

		if(HAL_GetTick() - time1 >= calculatedWaitPeriod)// pulse // Adding an offset of 10 to realign and compensate for delay caused by division, approx 10 ms.
		{
		  #ifdef debug
		  Serial.print("wait period after processing: ");
		  Serial.println(calculatedWaitPeriod);
		  #endif

		  // update time variables
		  time1 = HAL_GetTick();
		  // hard code pin high and pin low using counter%2 ==  0
		  // so we can add small delay to the high if or the low else to adjust duty cycle
		  switch(pulseToggle)
		  {
			case(0):
			{
			  HAL_GPIO_WritePin(clock2.port, clock2.pin, 1) ;
			  pulseToggle = 1;
			  delaybuffer = clock1.posDutyCycleDelay;
			  dividerCount++;
			  switch(dividerCount >= calculatedEnd )
			  {
				case(1):
				{
					HAL_GPIO_WritePin(clock1.port, clock1.pin, 1) ;
				  dividerCount = 0;
				  break;
				}default:
				{break;}
			  }
			  break;
			}

			case(1):
			{
				HAL_GPIO_WritePin(clock2.port, clock2.pin, 0) ;
				pulseToggle = 0;
				delaybuffer = clock1.negDutyCycleDelay;
				dividerCount2++;
				switch(dividerCount2 >= calculatedEnd )
				{
				  case(1):
				  {
					  HAL_GPIO_WritePin(clock1.port, clock1.pin, 0) ;
					dividerCount2 = 0;
					break;
				  }
				  default:
				  {break;}
				}

				break;
			}
			default:
			{
			  pulseToggle = 0;
			  break;
			}
		  }

		}

	}
}
void print(uint8_t* data, size_t size)
{
	HAL_UART_Transmit(&huart2, data,size, 100);

}
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
