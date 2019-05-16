#ifndef __ST7735_H__
#define __ST7735_H__ 1

#include "Master.h"

enum initRFlags {
    none,
    INITR_GREENTAB,
    INITR_REDTAB,
    INITR_BLACKTAB
};

#define ST7735_BLACK 0x0000
#define ST7735_BLUE 0xF800
#define ST7735_RED 0x001F
#define ST7735_GREEN 0x07E0
#define ST7735_CYAN 0xFFE0
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW 0x07FF
#define ST7735_WHITE 0xFFFF

void ST7735_InitB(void);

void ST7735_InitR(enum initRFlags option);

void ST7735_FillScreen(unsigned short color);

void ST7735_FillRect(short x, short y, short w, short h, unsigned short color);

unsigned short ST7735_Color565(unsigned char r, unsigned char g, unsigned char b);

unsigned short ST7735_SwapColor(unsigned short x);

void ST7735_SetRotation(unsigned char m);

void ST7735_InvertDisplay(int i);

void ST7735_DrawPixel(short x, short y, unsigned short color);

void ST7735_DrawFastVLine(short x, short y, short h, unsigned short color);

void ST7735_DrawFastHLine(short x, short y, short w, unsigned short color);

void ST7735_DrawBitmap(short x, short y, const unsigned short *image, short w, short h);

void ST7735_DrawCharS(short x, short y, char c, short textColor, short bgColor, unsigned char size);

void ST7735_DrawChar(short x, short y, char c, short textColor, short bgColor, unsigned char size);

uint32_t ST7735_DrawString(uint16_t x, uint16_t y, char *pt, int16_t textColor);

void ST7735_SetTextColor(uint16_t color);

void ST7735_SetCursor(uint32_t newX, uint32_t newY);

void ST7735_OutChar(char ch);

#endif
