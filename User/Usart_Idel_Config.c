#include "Usart_Idel_Config.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
//ESP01S UART2
#define U2_BUFF_SIZE	(1024)			   //接收缓存大小
uint8_t rx_buffer[U2_BUFF_SIZE];      // 创建接收缓存,大小为BUFF_SIZE


/* USER CODE BEGIN Private defines */
extern DMA_HandleTypeDef hdma_usart2_rx;        //手动外部声明
/* USER CODE END Private defines */


void Usart2_Idel_Config(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2,rx_buffer,U2_BUFF_SIZE);	//手动开启串口DMA模式接收数据
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);		   	//手动关闭DMA_IT_HT中断

}

/* 串口接收完成回调函数 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
   if (huart->Instance == USART2)
   {
   	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buffer, U2_BUFF_SIZE); // 接收完毕后重启串口DMA模式接收数据
   	HAL_UART_Transmit(&huart1, rx_buffer, Size, 0xffff);         // 将接收到的数据再发出
   	__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);		   	// 手动关闭DMA_IT_HT中断
   	memset(rx_buffer, 0, U2_BUFF_SIZE);							// 清除接收缓存
   }
}	
/* 串口错误回调函数 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef * huart)
{
   if(huart->Instance == USART2)
   {
   	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buffer, U2_BUFF_SIZE); //手动开启串口DMA模式接收数据
   	__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);		   // 手动关闭DMA_IT_HT中断
   	memset(rx_buffer, 0, U2_BUFF_SIZE);							   // 清除接收缓存
   }
}

