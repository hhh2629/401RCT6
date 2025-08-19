#include "Usart_Idel_Config.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
//ESP01S UART2
#define U2_BUFF_SIZE	(1024)			   //���ջ����С
uint8_t rx_buffer[U2_BUFF_SIZE];      // �������ջ���,��СΪBUFF_SIZE


/* USER CODE BEGIN Private defines */
extern DMA_HandleTypeDef hdma_usart2_rx;        //�ֶ��ⲿ����
/* USER CODE END Private defines */


void Usart2_Idel_Config(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2,rx_buffer,U2_BUFF_SIZE);	//�ֶ���������DMAģʽ��������
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);		   	//�ֶ��ر�DMA_IT_HT�ж�

}

/* ���ڽ�����ɻص����� */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
   if (huart->Instance == USART2)
   {
   	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buffer, U2_BUFF_SIZE); // ������Ϻ���������DMAģʽ��������
   	HAL_UART_Transmit(&huart1, rx_buffer, Size, 0xffff);         // �����յ��������ٷ���
   	__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);		   	// �ֶ��ر�DMA_IT_HT�ж�
   	memset(rx_buffer, 0, U2_BUFF_SIZE);							// ������ջ���
   }
}	
/* ���ڴ���ص����� */
void HAL_UART_ErrorCallback(UART_HandleTypeDef * huart)
{
   if(huart->Instance == USART2)
   {
   	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buffer, U2_BUFF_SIZE); //�ֶ���������DMAģʽ��������
   	__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);		   // �ֶ��ر�DMA_IT_HT�ж�
   	memset(rx_buffer, 0, U2_BUFF_SIZE);							   // ������ջ���
   }
}

