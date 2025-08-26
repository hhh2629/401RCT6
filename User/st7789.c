#include "st7789.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>




uint8_t frame_buffer[BUFFER_SIZE];  // 字节缓冲区
volatile uint8_t dma_transfer_complete = 0;


void LCD_Writ_Bus(uint8_t *dat,uint16_t size) 
{	
    if( HAL_SPI_Transmit_DMA(&ST7789_SPI_PORT, dat, size) != HAL_OK)
    {
      Error_Handler();
    }
	// HAL_SPI_Transmit_DMA(&Spi1Handle,dat,size);
}


static void ST7789_Write_Cmd(uint8_t cmd)
{
    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);
    
    //HAL_SPI_Transmit(&ST7789_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
    LCD_Writ_Bus(&cmd,1);
    
    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_SET);
}
 void ST7789_Write_Data(uint8_t data)
{
    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    
    //HAL_SPI_Transmit(&ST7789_SPI_PORT, &data, sizeof(data), HAL_MAX_DELAY);
    LCD_Writ_Bus(&data,1);
    
    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_SET);
}
void ST7789_Write_Datas(uint8_t *buff, size_t buff_size)
{
    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
  
    // 这个循环用于解决发送超过2^16个数据
    while (buff_size > 0) {
        uint16_t chunk_size = buff_size > 65535 ? 65535 : buff_size;
        dma_transfer_complete = 0; // 重置传输标志

        //HAL_SPI_Transmit(&ST7789_SPI_PORT, buff, chunk_size, HAL_MAX_DELAY);

        // 使用DMA方式发送数据
        if (HAL_SPI_Transmit_DMA(&ST7789_SPI_PORT, buff, chunk_size) != HAL_OK)
        {
            // 处理DMA发送错误（根据需要添加错误处理代码）
        }
        // 等待DMA传输完成
        while (dma_transfer_complete == 0);
        
        buff += chunk_size;     // 记录当前发送的位置，用于下次发送
        buff_size -= chunk_size;
    }
    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_SET);
}


// SPI DMA传输完成回调函数
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &ST7789_SPI_PORT)
    {
        dma_transfer_complete = 1; // 设置传输完成标志
    }
}





void ST7789_Init()
{

    HAL_Delay(10);


    ST7789_Write_Cmd(0x36);		
    ST7789_Write_Data(0xA0);
    ST7789_Write_Cmd(0x3A);
    ST7789_Write_Data(0x05);
    ST7789_Write_Cmd(0xB2);		
    ST7789_Write_Data(0x0C);
    ST7789_Write_Data(0x0C);
    ST7789_Write_Data(0x00);
    ST7789_Write_Data(0x33);
    ST7789_Write_Data(0x33);

    ST7789_Write_Cmd(0XB7);		
    ST7789_Write_Data(0x35);	
    ST7789_Write_Cmd(0xBB);		
    ST7789_Write_Data(0x19);	
    ST7789_Write_Cmd(0xC0);		
    ST7789_Write_Data (0x2C);	
    ST7789_Write_Cmd (0xC2);	
    ST7789_Write_Data (0x01);	
    ST7789_Write_Cmd (0xC3);	
    ST7789_Write_Data (0x12);	
    ST7789_Write_Cmd (0xC4);	
    ST7789_Write_Data (0x20);	
    ST7789_Write_Cmd (0xC6);	
    ST7789_Write_Data (0x0F);	
    ST7789_Write_Cmd (0xD0);	
    ST7789_Write_Data (0xA4);	
    ST7789_Write_Data (0xA1);

    ST7789_Write_Cmd(0xE0);
    {
        uint8_t data[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
        ST7789_Write_Datas(data, sizeof(data));
    }

    ST7789_Write_Cmd(0xE1);
    {
        uint8_t data[] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
        ST7789_Write_Datas(data, sizeof(data));
    }
    ST7789_Write_Cmd (0x21);	
    ST7789_Write_Cmd (0x11);	   
    ST7789_Write_Cmd (0x29);	

    HAL_Delay(50);
    //ST7789_Clear(0xFFFF);
    ST7789_Clear(0x00);
}

void ST7789_SetRotation(uint8_t d)
{
   if(d==1)
   {
       ST7789_Write_Cmd(0x36);
       ST7789_Write_Data(0x00);
   }
   if(d==2)
   {
       ST7789_Write_Cmd(0x36);
       ST7789_Write_Data(0x60);      
   }
   if(d==3)
   {
       ST7789_Write_Cmd(0x36);
       ST7789_Write_Data(0xA0);
   }
}

void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t x_start = x0, x_end = x1;
    uint16_t y_start = y0, y_end = y1;

    /* Column Address set */
    ST7789_Write_Cmd(0x2A);
    {
        uint8_t data[] = {x_start >> 8, x_start & 0xFF, x_end >> 8, x_end & 0xFF};
        ST7789_Write_Datas(data, sizeof(data));
    }

    /* Row Address set */
    ST7789_Write_Cmd(0x2B);
    {
        uint8_t data[] = {y_start >> 8, y_start & 0xFF, y_end >> 8, y_end & 0xFF};
        ST7789_Write_Datas(data, sizeof(data));
    }
    /* Write to RAM */
    ST7789_Write_Cmd(0x2C);

}
void ST7789_Clear(uint16_t color)
{
    // 计算总像素数和总字节数
    const uint32_t total_pixels = ST7789_WIDTH * ST7789_HEIGHT;
    const uint32_t total_bytes = total_pixels * PIXEL_SIZE;
    // 填充1024字节颜色数据（512个像素）
    uint8_t color_high = (color >> 8) & 0xFF;
    uint8_t color_low = color & 0xFF;
    for (uint16_t i = 0; i < BUFFER_SIZE; i += 2)
    {
        frame_buffer[i] = color_high;
        frame_buffer[i+1] = color_low;
    }
    ST7789_SetAddressWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    // 分块DMA传输（每次1024字节）
    uint32_t remaining_bytes = total_bytes;
    uint8_t *buffer_ptr = frame_buffer;

    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);

     while (remaining_bytes > 0)
    {
        uint16_t transfer_size = remaining_bytes > BUFFER_SIZE ? BUFFER_SIZE : remaining_bytes;
        dma_transfer_complete = 0;
        
        // 启动DMA传输
        if (HAL_SPI_Transmit_DMA(&ST7789_SPI_PORT, buffer_ptr, transfer_size) != HAL_OK)
        {
            // 添加错误处理代码
        }
        
        // 等待传输完成
        while (dma_transfer_complete == 0);
        
        remaining_bytes -= transfer_size;
    }
    
    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_SET);
}

/**
 * @brief 填充指定矩形区域
 * @param x1 左上角X坐标
 * @param y1 左上角Y坐标
 * @param x2 右下角X坐标
 * @param y2 右下角Y坐标
 * @param color 填充颜色
 */
void ST7789_FillRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    // 计算矩形区域尺寸
    uint16_t width = x2 - x1 + 1;
    uint16_t height = y2 - y1 + 1;
    const uint32_t total_pixels = width * height;
    const uint32_t total_bytes = total_pixels * PIXEL_SIZE;

    // 填充颜色数据到缓冲区
    uint8_t color_high = (color >> 8) & 0xFF;
    uint8_t color_low = color & 0xFF;
    for (uint16_t i = 0; i < BUFFER_SIZE; i += 2)
    {
        frame_buffer[i] = color_high;
        frame_buffer[i+1] = color_low;
    }

    // 设置矩形区域地址窗口
    ST7789_SetAddressWindow(x1, y1, x2, y2);

    // 分块DMA传输
    uint32_t remaining_bytes = total_bytes;
    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);

    while (remaining_bytes > 0)
    {
        uint16_t transfer_size = remaining_bytes > BUFFER_SIZE ? BUFFER_SIZE : remaining_bytes;
        dma_transfer_complete = 0;

        if (HAL_SPI_Transmit_DMA(&ST7789_SPI_PORT, frame_buffer, transfer_size) != HAL_OK)
        {
            // 添加错误处理代码
        }

        while (dma_transfer_complete == 0);
        remaining_bytes -= transfer_size;
    }

    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_SET);
}


void OLED_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    if ((x < 0) || (x >= ST7789_WIDTH) ||(y < 0) || (y >= ST7789_HEIGHT))	return;

    ST7789_SetAddressWindow(x, y, x, y);
    uint8_t data[] = {color >> 8, color & 0xFF};    //楂樹綅鍜屼綆浣?
    ST7789_Write_Datas(data, sizeof(data));
}
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,uint16_t color)
{
    uint16_t swap;
    uint16_t steep = ABS(y1 - y0) > ABS(x1 - x0);
    if (steep) {
        swap = x0;
        x0 = y0;
        y0 = swap;

        swap = x1;
        x1 = y1;
        y1 = swap;
    }

    if (x0 > x1) {
        swap = x0;
        x0 = x1;
        x1 = swap;

        swap = y0;
        y0 = y1;
        y1 = swap;
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = ABS(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            OLED_DrawPoint(y0, x0, color);
        } else {
            OLED_DrawPoint(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}
void ST7789_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color,uint8_t isFilled)
{
    if (isFilled==0)
    {
        ST7789_DrawLine(x1, y1, x2, y1, color);
        ST7789_DrawLine(x1, y1, x1, y2, color);
        ST7789_DrawLine(x1, y2, x2, y2, color);
        ST7789_DrawLine(x2, y1, x2, y2, color);
    }else
    {
        uint16_t i, j;
        ST7789_SetAddressWindow(x1, y1, x2, y2);
        for (i = y1; i <= y2; i++)
            for (j = x1; j <= x2; j++) {
                uint8_t data[] = {color >> 8, color & 0xFF};
                ST7789_Write_Datas(data, sizeof(data));
            }
    }
    
}

void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
        return;
    if ((x + w - 1) >= ST7789_WIDTH)
        return;
    if ((y + h - 1) >= ST7789_HEIGHT)
        return;

    ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    ST7789_Write_Datas((uint8_t *)data, sizeof(uint16_t) * w * h);
}

void ST7789_DrawImage2(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const uint16_t *data)
{

    ST7789_SetAddressWindow(x1, y1, x2, y2);
    ST7789_Write_Datas((uint8_t *)data, (x2-x1+1)*(y2-y1+1)*2);
    
}


void ST7789_ShowChar(uint16_t x, uint16_t y, uint16_t color, uint16_t bgcolor,char ch, FontDef font)
{
    uint32_t i, b, j;
    ST7789_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);

    for (i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for (j = 0; j < font.width; j++) {
            if ((b << j) & 0x8000) {
                uint8_t data[] = {color >> 8, color & 0xFF};
                ST7789_Write_Datas(data, sizeof(data));
            }
            else {
                uint8_t data[] = {bgcolor >> 8, bgcolor & 0xFF};
                ST7789_Write_Datas(data, sizeof(data));
            }
        }
    }
}
void ST7789_ShowString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor)
{
    while (*str) {
        if (x + font.width >= ST7789_WIDTH) {
            x = 0;
            y += font.height;
            if (y + font.height >= ST7789_HEIGHT) {
                break;
            }

            if (*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }
        ST7789_ShowChar(x, y, color, bgcolor,*str, font);
        x += font.width;
        str++;
    }
}

void ST7789_ShowChinese(uint16_t x, uint16_t y, uint16_t color,uint16_t bgcolor,const uint8_t *Chinese)
{
    uint32_t i,j;
    uint8_t  byte;
    ST7789_SetAddressWindow(x, y, x + 16- 1, y + 16 - 1);
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 2; j++) {
            byte = Chinese[i*2+j];
            for(int b=0;b<8;b++)
            {
                if ((byte>>b)&0x01)
                {
                    uint8_t data[] = {color >> 8, color & 0xFF};
                    ST7789_Write_Datas(data, sizeof(data));
                }
                else {
                    uint8_t data[] = {bgcolor >> 8, bgcolor & 0xFF};
                    ST7789_Write_Datas(data, sizeof(data));
                }
            }
        }
    }
}
//void ST7789_ShowChineses(int16_t X, int16_t Y, uint16_t color,uint16_t bgcolor,char *Chinese)
//{
//    uint8_t pChinese = 0;
//    uint8_t pIndex;
//    uint8_t i=0;
//    char SingleChinese[OLED_CHN_CHAR_WIDTH + 1] = {0};

//    for (i = 0; Chinese[i] != '\0'; i ++)		//閬嶅巻姹夊瓧涓?
//    {
//        SingleChinese[pChinese] = Chinese[i];    //鎻愬彇姹夊瓧涓叉暟鎹?鍒板崟涓?姹夊瓧鏁扮粍
//        pChinese++;                            //璁℃?¤嚜澧?

//        /*褰撴彁鍙栨?℃暟鍒拌揪OLED_CHN_CHAR_WIDTH鏃讹紝鍗充唬琛ㄦ彁鍙栧埌浜嗕竴涓?瀹屾暣鐨勬眽瀛?*/
//        if (pChinese >= OLED_CHN_CHAR_WIDTH) {
//            pChinese = 0;        //璁℃?″綊闆?

//            /*閬嶅巻鏁翠釜姹夊瓧瀛楁ā搴擄紝瀵绘壘鍖归厤鐨勬眽瀛?*/
//            /*濡傛灉鎵惧埌鏈�鍚庝竴涓?姹夊瓧锛堝畾涔変负绌哄瓧绗︿覆锛夛紝鍒欒〃绀烘眽瀛楁湭鍦ㄥ瓧妯″簱瀹氫箟锛屽仠姝㈠?绘壘*/
//            for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex++) {
//                /*鎵惧埌鍖归厤鐨勬眽瀛?*/
//                if (strcmp(OLED_CF16x16[pIndex].Index, SingleChinese) == 0) {
//                    break;        //璺冲嚭寰?鐜?锛屾?ゆ椂pIndex鐨勫�间负鎸囧畾姹夊瓧鐨勭储寮?
//                }
//            }

//            /*灏嗘眽瀛楀瓧妯″簱OLED_CF16x16鐨勬寚瀹氭暟鎹?浠?16*16鐨勫浘鍍忔牸寮忔樉绀?*/
//            // ST7789_ShowChar(X + ((i + 1) / OLED_CHN_CHAR_WIDTH - 1) * 16, Y, 16, 16, OLED_CF16x16[pIndex].Data);
//            ST7789_ShowChinese(X + ((i + 1) / OLED_CHN_CHAR_WIDTH - 1) * 16, Y,color,bgcolor,OLED_CF16x16[pIndex].Data);
//        }
//    }
//}
static uint32_t ST7789_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;	//缁撴灉榛樿?や负1
    while (Y --)			//绱?涔榊娆?
    {
        Result *= X;		//姣忔?℃妸X绱?涔樺埌缁撴灉涓?
    }
    return Result;
}
void ST7789_ShowBinNum(int16_t X, int16_t Y,uint16_t color,uint16_t bgcolor, uint32_t Number, uint8_t Length, FontDef font)
{
    uint8_t i;
    for (i = 0; i < Length; i++)		//閬嶅巻鏁板瓧鐨勬瘡涓�浣?
    {
        /*璋冪敤OLED_ShowChar鍑芥暟锛屼緷娆℃樉绀烘瘡涓?鏁板瓧*/
        /*Number / OLED_Pow(2, Length - i - 1) % 2 鍙?浠ヤ簩杩涘埗鎻愬彇鏁板瓧鐨勬瘡涓�浣?*/
        /*+ '0' 鍙?灏嗘暟瀛楄浆鎹?涓哄瓧绗︽牸寮?*/
        ST7789_ShowChar(X + i * font.width, Y, color,bgcolor,Number / ST7789_Pow(2, Length - i - 1) % 2 + '0',font);
    }
}void ST7789_Printf(int16_t X, int16_t Y, uint16_t color,uint16_t bgcolor,FontDef font, char *format, ...)
{
    char String[256];						//瀹氫箟瀛楃?︽暟缁?
    va_list arg;							//瀹氫箟鍙?鍙樺弬鏁板垪琛ㄦ暟鎹?绫诲瀷鐨勫彉閲廰rg
    va_start(arg, format);					//浠巉ormat寮�濮嬶紝鎺ユ敹鍙傛暟鍒楄〃鍒癮rg鍙橀噺
    vsprintf(String, format, arg);			//浣跨敤vsprintf鎵撳嵃鏍煎紡鍖栧瓧绗︿覆鍜屽弬鏁板垪琛ㄥ埌瀛楃?︽暟缁勪腑
    va_end(arg);							//缁撴潫鍙橀噺arg
    ST7789_ShowString(X, Y, String, font,color,bgcolor);//OLED鏄剧ず瀛楃?︽暟缁勶紙瀛楃?︿覆锛?
}

