/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define fieldWidth 15
#define fieldHeight 25
#define guideTextLine 22
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t input[1];
uint8_t clr3[] = "\033[3J";
uint8_t clr0[] = "\033[0J";
uint8_t resetCursor[] = "\033[1;1H";
uint8_t newLine[] = "\r\n";
uint8_t close[] = { 27, '[', '?', '2', '5', 'l' };
uint8_t field[fieldHeight][fieldWidth];
uint8_t tetrominos[7][16];
uint8_t curTeTro[16];
uint8_t infoText[] = "    Tetris by TeamTamoad      Score: ";
uint8_t gameOverText1[] = "    Game Over! Noooob!!!\r\n\n    Your score is ";
uint8_t gameOverText2[] = "    \nPress SPACE BAR to continue.";
uint8_t scoreText[6];
uint8_t numText[10] = "0123456789";
uint8_t leftSpace[] = "    ";
uint8_t guideText[guideTextLine][40];
uint16_t score = 0;
uint8_t gameOver = 0;
uint16_t curX = fieldWidth/2 - 2;
uint16_t curY = 0;
uint8_t speed = 15;
uint8_t speedCount = 0;
uint8_t pieceCount = 1;
//uint8_t redColor[] = "\033[0;31m";
//uint8_t defaultColor[] = "\033[0m";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void recursor() {
	HAL_UART_Transmit(&huart2, resetCursor, sizeof(resetCursor), 50);
}

void clearScreen() {
	HAL_UART_Transmit(&huart2, resetCursor, sizeof(resetCursor), 50);
	HAL_UART_Transmit(&huart2, clr0, sizeof(clr0), 50);
	HAL_UART_Transmit(&huart2, clr3, sizeof(clr3), 50);
	HAL_UART_Transmit(&huart2, resetCursor, sizeof(resetCursor), 50);
}

void display() { //Display field + current Tetromino
	uint8_t temp[fieldHeight][fieldWidth];
	memcpy(temp, field, fieldWidth*fieldHeight);
	for(int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			if (curTeTro[i*4 + j] != '.') {
				temp[curY+i][curX+j] = curTeTro[i*4 + j];
			}
		}
	}
	HAL_UART_Transmit(&huart2, newLine, sizeof(newLine), 50);
	HAL_UART_Transmit(&huart2, infoText, sizeof(infoText), 50);
	HAL_UART_Transmit(&huart2, scoreText, sizeof(scoreText), 50);
	HAL_UART_Transmit(&huart2, newLine, sizeof(newLine), 50);
	for(uint8_t i=0; i<fieldHeight; i++) {
		HAL_UART_Transmit(&huart2, leftSpace, sizeof(leftSpace), 50);
		HAL_UART_Transmit(&huart2, temp[i], sizeof(temp[i]), 50);
		if (i<guideTextLine) HAL_UART_Transmit(&huart2, guideText[i], sizeof(guideText[i]), 50);
		HAL_UART_Transmit(&huart2, newLine, sizeof(newLine), 50);
	}
}

void rotate(uint8_t tetro[16], uint8_t clockwise) {
	uint8_t newTetro[16];
	if (clockwise == 1) {
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				//ni = j, nj = 3-i;
				newTetro[j*4 + 3 - i] = tetro[i*4 + j];
			}
		}
	} else {
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				newTetro[i + 12 - 4*j] = tetro[i*4 + j];
			}
		}
	}
	memcpy(tetro, newTetro, 16);
}

uint8_t canMove(int direction) {
	//Check move down
	if (direction == 0) {
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				if (curTeTro[i*4 + j] != '.' && field[curY+i+1][curX + j] != '.') return 0;
			}
		}
	//Check move right
	} else if (direction == 1) {
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				if (curTeTro[i*4 + j] != '.' && field[curY+i][curX + j + 1] != '.') return 0;
			}
		}
	//Check move left
	} else if (direction == 2){
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				if (curTeTro[i*4 + j] != '.' && field[curY+i][curX + j - 1] != '.') return 0;
			}
		}
	}
	return 1;
}

uint8_t canRotate(int direction) {
	//Check clockwise
	if (direction == 1) {
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				//ni = j, nj = 3-i;
				if(curTeTro[i*4 + j] != '.' && field[curY+j][curX+3-i] != '.') return 0;
			}
		}
	//Check counter-clockwise
	} else {
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				//ni = 3-j, nj = i
				if(curTeTro[i*4 + j] != '.' && field[3-j+curY][curX+i] != '.') return 0;
			}
		}
	}
	return 1;
}

uint8_t gotLine(uint8_t y) {
	uint8_t line = 1;
	for (int i=1; i<fieldWidth-1; i++) {
		if (field[y][i] == '.') {
			line = 0;
			break;
		}
	}
	return line;
}

void pushAboveDown(uint8_t y) {
	for(int i=y; i>= 1; i--) {
		for (int j=1; j<fieldWidth-1; j++) {
			field[i][j] = field[i-1][j];
		}
	}
	for (int i=1; i<fieldWidth-1; i++) {
		field[0][i] = '.';
	}
}

uint8_t isGameOver() {
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			if (curTeTro[i*4 + j] != '.' && field[curY+i][curX + j - 1] != '.') return 1;
		}
	}
	return 0;
}

void setScoreText() {
	scoreText[0] = numText[(score/1000)%10];
	scoreText[1] = numText[(score/100)%10];
	scoreText[2] = numText[(score/10)%10];
	scoreText[3] = numText[score%10];
	scoreText[4] = '\r';
	scoreText[5] = '\n';
}

void initGame() {
	for (int i=0; i<fieldHeight; i++) {
		for (int j=0; j<fieldWidth; j++) {
			if (j== 0 || j == fieldWidth-1) {
				field[i][j] = '|';
			} else if (i == fieldHeight-1) {
				field[i][j] = '#';
			} else {
				field[i][j] = '.';
			}
		}

	}
	score = 0;
	setScoreText();
	gameOver = 0;
	curX = fieldWidth/2 - 2;
	curY = 0;
	speed = 20;
	speedCount = 0;
	pieceCount = 1;
	HAL_UART_Transmit(&huart2, close, sizeof(close), 50);
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM4) {
    	if (!gameOver) {
    		//Move tetromino down
    		if (canMove(0)) {
    			curY++;
    		} else {
    			//Set tetromino to Map
    			for(int i=0; i<4; i++) {
    				for (int j=0; j<4; j++) {
    					if (curTeTro[i*4 + j] != '.') {
    			  			field[curY+i][curX+j] = curTeTro[i*4 + j];
    			  		 }
    			  	}
    			}
    			//Check for the line and update score
    			for (uint8_t i=0; i<fieldHeight-1; i++) {
    			  	if (gotLine(i)) {
    			  		score += 100;
    			  		setScoreText();
    			  		pushAboveDown(i);
    			  	}
    			}
    			//Get the new tetromino
    			curY = 0;
    			curX = fieldWidth/2 - 2;
    			memcpy(curTeTro, tetrominos[rand()%7], 16);
    			//Update piece count and speed
    			pieceCount++;
    			//CheckGameOver
    			if (isGameOver()) {
    				gameOver = 1;
    				clearScreen();
    				HAL_UART_Transmit(&huart2, newLine, sizeof(newLine), 50);
    				HAL_UART_Transmit(&huart2, gameOverText1, sizeof(gameOverText1), 50);
    				HAL_UART_Transmit(&huart2, scoreText, sizeof(scoreText), 50);
    				HAL_UART_Transmit(&huart2, gameOverText2, sizeof(gameOverText2), 50);
    			}
    		}
    		if (!gameOver) {
    			recursor();
    			display();
    		}
    	} else {
    		recursor();
    		HAL_UART_Transmit(&huart2, newLine, sizeof(newLine), 50);
    		HAL_UART_Transmit(&huart2, gameOverText1, sizeof(gameOverText1), 50);
    		HAL_UART_Transmit(&huart2, scoreText, sizeof(scoreText), 50);
    		HAL_UART_Transmit(&huart2, gameOverText2, sizeof(gameOverText2), 50);
    	}

    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//Tetromino
	memcpy(tetrominos[0], ".....OO..OO.....", 16);
	memcpy(tetrominos[1], ".I...I...I...I..", 16);
	memcpy(tetrominos[2], ".....SS.SS......", 16);
	memcpy(tetrominos[3], "....ZZ...ZZ.....", 16);
	memcpy(tetrominos[4], ".L...L...LL.....", 16);
	memcpy(tetrominos[5], "..J...J..JJ.....", 16);
	memcpy(tetrominos[6], "....TTT..T......", 16);
	memcpy(curTeTro, tetrominos[rand()%7], 16);
	memcpy(guideText[0], "    There are 7 types of Tetrominoes.   ", 40);
	memcpy(guideText[1], "                                        ", 40);
	memcpy(guideText[2], "         SS   ZZ    TTT   OO            ", 40);
	memcpy(guideText[3], "        SS     ZZ    T    OO            ", 40);
	memcpy(guideText[4], "                                        ", 40);
	memcpy(guideText[5], "           I                            ", 40);
	memcpy(guideText[6], "           I      J     L               ", 40);
	memcpy(guideText[7], "           I      J     L               ", 40);
	memcpy(guideText[8], "           I     JJ     LL              ", 40);
	memcpy(guideText[9], "                                        ", 40);
	memcpy(guideText[10], "                                        ", 40);
	memcpy(guideText[11], "    Control                             ", 40);
	memcpy(guideText[12], "                                        ", 40);
	memcpy(guideText[13], "    A - Move left                       ", 40);
	memcpy(guideText[14], "                                        ", 40);
	memcpy(guideText[15], "    D - Move right                      ", 40);
	memcpy(guideText[16], "                                        ", 40);
	memcpy(guideText[17], "    S - Move down                       ", 40);
	memcpy(guideText[18], "                                        ", 40);
	memcpy(guideText[19], "    J - Rotate counter-clockwise        ", 40);
	memcpy(guideText[20], "                                        ", 40);
	memcpy(guideText[21], "    K - Rotate clockwise                ", 40);
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
  MX_TIM4_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  initGame();
  recursor();
  clearScreen();
  recursor();
  HAL_UART_Receive_IT(&huart2, input, sizeof(input));
  HAL_TIM_Base_Start_IT(&htim4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 60000;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 460800;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	//IF press right arrow
	if (!gameOver) {
		if (input[0] == 'd' && canMove(1)) {
			curX++;
			recursor();
			display();
		//If press left arrow
		} else if (input[0] == 'a' && canMove(2)){
			curX--;
			recursor();
			display();
		//If press down arrow
		} else if (input[0] == 's' && canMove(0)){
			curY++;
			recursor();
			display();
		//If press j (rotate counter-clockwise)
		} else if (input[0] == 'j' && canRotate(0)) {
			rotate(curTeTro, 0);
			recursor();
			display();
		//If press k (rotate clockwise)
		} else if (input[0] == 'k' && canRotate(1)) {
			rotate(curTeTro, 1);
			recursor();
			display();
		}
	} else if (input[0] == ' ') {
		initGame();
	}
	HAL_UART_Receive_IT(&huart2, input, sizeof(input));
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
