/*
 * ILI9341.h
 *
 *  Created on: Mar 26, 2026
 *      Author: hussamaldean
 */

#ifndef INC_ILI9341_H_
#define INC_ILI9341_H_

#include "main.h"

#include "spi.h"

typedef enum {
	ROTATE_0,
	ROTATE_90,
	ROTATE_180,
	ROTATE_270
} LCD_Horizontal_t;

void ILI9341_Init(LCD_Horizontal_t rot);

void ILI9341_WritePixel(uint16_t x, uint16_t y, uint16_t color);

void ILI9341_Draw_Bitmap(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, const uint8_t *Image);


void ILI9341_Draw_Bitmap_DMA(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, const uint8_t *Image);

#endif /* INC_ILI9341_H_ */
