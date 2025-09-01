#include "Usart_Idel_Config.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
// ESP01S UART2
#define U2_BUFF_SIZE (1024)      // 接收缓存大小
uint8_t rx_buffer[U2_BUFF_SIZE]; // 创建接收缓存,大小为BUFF_SIZE
volatile uint8_t Recv_OK = 0;

/* USER CODE BEGIN Private defines */
extern DMA_HandleTypeDef hdma_usart2_rx; // 手动外部声明
/* USER CODE END Private defines */

void Usart2_Idel_Config(void)
{
   HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buffer, U2_BUFF_SIZE); // 手动开启串口DMA模式接收数据
   __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);               // 手动关闭DMA_IT_HT中断
}

/* 串口接收完成回调函数 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
   if (huart->Instance == USART2)
   {
      HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buffer, U2_BUFF_SIZE); // 接收完毕后重启串口DMA模式接收数据

      // HAL_UART_Transmit(&huart1, rx_buffer, Size, 0xffff);         // 将接收到的数据再发出
      Recv_OK = 1;

      __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT); // 手动关闭DMA_IT_HT中断
                                                        // memset(rx_buffer, 0, U2_BUFF_SIZE);							// 清除接收缓存
   }
}
/* 串口错误回调函数 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
   if (huart->Instance == USART2)
   {
      HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buffer, U2_BUFF_SIZE); // 手动开启串口DMA模式接收数据
      __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);               // 手动关闭DMA_IT_HT中断
      memset(rx_buffer, 0, U2_BUFF_SIZE);                             // 清除接收缓存
   }
}

#define SECTOR_SIZE 4096         // 扇区大小4KB
#define RECEIVE_SIZE 256         // 每次接收256字节
uint32_t current_flash_addr = 0; // 当前Flash操作地址
uint32_t sector_byte_count = 0;  // 扇区内已写入字节计数
uint8_t EN = 0;

void Usart_to_W25q32()
{
   uint8_t Time_Out = 0;
   static uint32_t Addr = 0x00;
   static uint32_t Recv_Count = 0;
   HAL_StatusTypeDef temp;
   uint8_t U1_RX_CMD[50] = {0};
   printf_DMA("Send \"AT+Image\\r\\n to writ image bin\"\r\n");
   for (uint8_t i = 0; i < 20; i++)
   {

      temp = HAL_UART_Receive(&huart1, U1_RX_CMD, 50, 1000);
      // 匹配"AT+Image:地址\r\n"格式并提取地址
      if (strstr((char *)U1_RX_CMD, "AT+Image:") != NULL)
      {
         uint32_t addr; // 提取的地址变量
         char *cmd = strstr((char *)U1_RX_CMD, "AT+Image:");
         char *end = strstr(cmd, "\r\n"); // 定位结尾标记

         // 验证格式完整性
         if (!end)
         {
            printf_DMA("Error: Invalid command format\r\n");
            continue;
         }
         printf_DMA("Debug: Parsing address from '%s'\r\n", cmd + 10);
         // 提取地址（支持0x前缀和纯数字格式）
         if (sscanf(cmd + 10, "%lx", (unsigned long*)&addr) != 1)
         {
            printf_DMA("Error: Failed to parse address\r\n");
            continue;
         }

         // 地址有效性检查
         if (addr >= 0x400000)
         { // W25Q32最大地址4MB
            printf_DMA("Error: Address 0x%06X exceeds W25Q32 capacity\r\n", addr);
            continue;
         }
         if (addr % 0x1000 != 0)
         { // 扇区大小4KB(0x1000)
            printf_DMA("Error: Address 0x%06X is not sector aligned\r\n", addr);
            continue;
         }

         // 地址验证通过，更新当前Flash地址
         current_flash_addr = addr;
         EN = 1;
         W25Qxx_Sector_Erase(current_flash_addr);
         W25Qxx_Wait_Free();
         memset(rx_buffer, 0xFF, U2_BUFF_SIZE);
         printf_DMA("Erase sector 0x%06X OK!\r\n", current_flash_addr);
         printf_DMA("Wait for image data...\r\n");
         break;
      }

      //---------------------------------------

      //  temp = HAL_UART_Receive(&huart1, U1_RX_CMD, 20, 1000);
      //  if (strstr((char *)U1_RX_CMD, "AT+Image") != NULL)
      //  {

      //     EN = 1;
      //     // 初始擦除第一个扇区
      //     W25Qxx_Sector_Erase(current_flash_addr);
      //     W25Qxx_Wait_Free();
      //     memset(rx_buffer, 0xFF, U2_BUFF_SIZE);
      //     printf_DMA("Erase 0X%x OK!\r\n", current_flash_addr);
      //     printf_DMA("Wait Send Image bin \r\n");
      //     break;
      //  }
   }
   printf_DMA("Wait Time Out!\r\n");
   while (EN)
   {
      // 串口接收256字节数据
      temp = HAL_UART_Receive(&huart1, rx_buffer, RECEIVE_SIZE, 10 * 1000);
      if (temp == HAL_OK)
      {
         // 写入256字节到Flash
         W25Qxx_Write_Page(current_flash_addr, rx_buffer, RECEIVE_SIZE);
         W25Qxx_Wait_Free();
         memset(rx_buffer, 0XFF, RECEIVE_SIZE);

         // 更新地址和计数
         current_flash_addr += RECEIVE_SIZE;
         sector_byte_count += RECEIVE_SIZE;
         Recv_Count += RECEIVE_SIZE;
         // 检查是否达到扇区边界(4KB)
         if (sector_byte_count >= SECTOR_SIZE)
         {
            // 擦除下一个扇区
            W25Qxx_Sector_Erase(current_flash_addr);
            W25Qxx_Wait_Free();
            printf_DMA("Erase 0X%x OK!\r\n", current_flash_addr);

            sector_byte_count = 0; // 重置扇区字节计数
         }
         Time_Out = 0;
      }
      else if (temp == HAL_TIMEOUT)
      {
         Time_Out++;
         if (Time_Out == 1)
         {
            // 写入256字节到Flash
            W25Qxx_Write_Page(current_flash_addr, rx_buffer, huart1.RxXferSize - huart1.RxXferCount);
            W25Qxx_Wait_Free();
            memset(rx_buffer, 0XFF, huart1.RxXferSize - huart1.RxXferCount);

            // 更新地址和计数
            current_flash_addr += huart1.RxXferSize - huart1.RxXferCount;
            sector_byte_count += huart1.RxXferSize - huart1.RxXferCount;
            Recv_Count += huart1.RxXferSize - huart1.RxXferCount;
            // 检查是否达到扇区边界(4KB)
            if (sector_byte_count >= SECTOR_SIZE)
            {
               // 擦除下一个扇区
               W25Qxx_Sector_Erase(current_flash_addr);
               W25Qxx_Wait_Free();
               printf_DMA("Erase 0X%x OK!\r\n", current_flash_addr);

               sector_byte_count = 0; // 重置扇区字节计数
            }
         }
         if (Time_Out == 2) // 第二次超时时退出
         {
            EN = 0;

            printf_DMA("Recv_Count = %d Last_Addr = %x\r\n", Recv_Count, current_flash_addr);
            // W25Qxx_Print_Sector(0x000000);
            break;
         }
      }
   }
}
