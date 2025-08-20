#ifndef ST7789_H_
#define ST7789_H_

#include "spi.h"
#include "fonts1.h"

#define ST7789_SPI_PORT hspi1
// Pin
#define ST7789_RST_PORT  
#define ST7789_RST_PIN  

#define ST7789_DC_PORT  GPIOA
#define ST7789_DC_PIN   GPIO_PIN_6

#define ST7789_CS_PORT  GPIOA
#define ST7789_CS_PIN   GPIO_PIN_4

#define ST7789_WIDTH  240
#define ST7789_HEIGHT 320

#define ABS(x) ((x) > 0 ? (x) : -(x))

void ST7789_Init();

void ST7789_Clear(uint16_t color);
void ST7789_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,uint16_t color);
void ST7789_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color,uint8_t isFilled);
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);


void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);

void ST7789_ShowChar(uint16_t x, uint16_t y, uint16_t color, uint16_t bgcolor,char ch, FontDef font);
void ST7789_ShowString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor);
void ST7789_ShowChineses(int16_t X, int16_t Y, uint16_t color,uint16_t bgcolor,char *Chinese);
void ST7789_ShowChinese(uint16_t x, uint16_t y, uint16_t color,uint16_t bgcolor,const uint8_t *Chinese);
void ST7789_ShowBinNum(int16_t X, int16_t Y,uint16_t color,uint16_t bgcolor, uint32_t Number, uint8_t Length, FontDef font);
void ST7789_Printf(int16_t X, int16_t Y, uint16_t color,uint16_t bgcolor,FontDef font, char *format, ...);
#endif