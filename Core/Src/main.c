/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "Usart_Idel_Config.h"
#include "W25Qxx.h"
//#include "ssd1306.h"
#include "st7789.h"



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//内存
//$K\keil5_disp_size_bar.exe
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



#define len 512
uint8_t temp[len]={0};
void W25Q32_Show_On_LCD_DMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t Address)
{
    
    uint32_t current_addr = Address;
   
    uint32_t total_bytes = w * h * 2;  // RGB565格式总字节数
    uint32_t remaining = total_bytes;

    ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    W25Qxx_CS_Low();
    W25Qxx_SPI_RW_Byte(0x03); // 发送读取命令
    
    if (W25Qxx_Address_Len == 32)
        W25Qxx_SPI_RW_Byte((Address & 0xFF000000) >> 24); // 32位地址处理

    W25Qxx_SPI_RW_Byte((Address & 0x00FF0000) >> 16); // 发送地址高位
    W25Qxx_SPI_RW_Byte((Address & 0x0000FF00) >> 8);  // 发送地址中位
    W25Qxx_SPI_RW_Byte((Address & 0x000000FF) >> 0);  // 发送地址低位

    while (remaining > 0)
    {
        uint16_t read_size = (remaining > len) ? len : remaining;
        HAL_SPI_Receive_DMA(&hspi3,temp,read_size);
        // 等待DMA传输完成
        while (!dma_recv_complete);
        dma_recv_complete = 0;

        ST7789_Write_Datas(temp, read_size);
        // 更新地址和剩余长度
        current_addr += read_size;
        remaining -= read_size;

        // 等待DMA传输完成
        while (!dma_transfer_complete);
        dma_transfer_complete = 0;
    }

    W25Qxx_CS_Hight();
}

/**
 * @brief 从W25Q32读取指定区域数据并显示到ST7789 LCD指定位置
 * @param x: LCD显示起始X坐标
 * @param y: LCD显示起始Y坐标
 * @param w: 显示宽度
 * @param h: 显示高度
 * @param addr: W25Q32数据起始地址
 * @note RGB565格式(2字节/像素),每次读取256字节(128像素)
 */
void W25Q32_Show_On_LCD(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t addr)
{
  #define  READ_SIZE 256
    uint8_t read_buf[READ_SIZE];
    uint32_t total_bytes = w * h * 2;  // RGB565格式总字节数
    uint32_t remaining = total_bytes;
    uint32_t current_addr = addr;

    // 参数有效性检查
    if (w == 0 || h == 0 || 
        x + w > ST7789_WIDTH || 
        y + h > ST7789_HEIGHT || 
        current_addr + total_bytes > 0x400000)  // W25Q32总容量4MB
        return;

    // 设置LCD显示窗口
    ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    while (remaining > 0)
    {
        uint16_t read_size = (remaining > READ_SIZE) ? READ_SIZE : remaining;

        // 从W25Q32读取数据
        W25Qxx_Read_Data_P256(current_addr, read_buf, read_size);

        // DMA传输到LCD
        ST7789_Write_Datas(read_buf, read_size);

        // 更新地址和剩余长度
        current_addr += read_size;
        remaining -= read_size;

        // 等待DMA传输完成
        while (!dma_transfer_complete);
        dma_transfer_complete = 0;
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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_SPI1_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */
  Usart2_Idel_Config();
  HAL_TIM_Base_Start_IT(&htim11);
//    ssd1306_Init();
//    ssd1306_FlipScreenVertically();
//    ssd1306_Clear();
//    ssd1306_SetColor(White);
//    
//    ssd1306_SetCursor(0, 0);
//    ssd1306_WriteString("HELLO", Font_16x26);
//    ssd1306_UpdateScreen();
  
 
//  W25Qxx_Sector_Erase(0x000000);
//  W25Qxx_Wait_Free();
//  for(uint16_t i=0;i<256;i++)
//        p[i]=i;
//    
//  W25Qxx_Write_Page(0x000000,p,256);
//  W25Qxx_Wait_Free();
//  W25Qxx_Write_Page(0x000100,p,256);
//  W25Qxx_Wait_Free();
//  
//  W25Qxx_Print_Sector(0x000000);
//  printf_DMA("id:%x\r\n",W25Qxx_Read_ID());
     
    ST7789_Init();


    uint32_t cnt=0;
    printf_DMA("Star\r\n");
    
//    //Recv_Count = 153608 Last_Addr = 25808
//    W25Q32_Show_On_LCD(0,0,320,240,0x0);
      W25Q32_Show_On_LCD_DMA(0,0,320,240,0x0);
      HAL_Delay(2000);
//    
//    //Recv_Count = 85928 Last_Addr = 3afa8
//    W25Q32_Show_On_LCD(0,0,179,240,0x26000);
      W25Q32_Show_On_LCD_DMA(0,0,179,240,0x26000);
      HAL_Delay(2000);
//    
//    //Recv_Count = 131208 Last_Addr = 5b088
//    W25Q32_Show_On_LCD(0,0,320,205,0x3b000); 
      W25Q32_Show_On_LCD_DMA(0,0,320,205,0x3B000);

 
  //Usart_to_W25q32();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
        
        HAL_Delay(1);
        
        cnt++;
        if(cnt%5==0)
        {
            
        }
        if(cnt%200==0)
        {
            HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_14);
            cnt=0;
        }
                

        
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
