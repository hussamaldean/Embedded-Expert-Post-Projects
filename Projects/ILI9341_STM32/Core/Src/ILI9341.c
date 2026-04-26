/*
 * ILI9341.c
 *
 *  Created on: Mar 26, 2026
 *      Author: hussamaldean
 */


#include "ILI9341.h"





static void CS_Deselect()
{
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, SET);
}

static void CS_Select()
{
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, RESET);
}

static void RST_HIGH()
{
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, SET);
}

static void RST_LOW()
{
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, RESET);
}

static void DC_HIGH()
{
	HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, SET);
}

static void DC_LOW()
{
	HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, RESET);
}

static void Reset(void)
{
	RST_HIGH();
    HAL_Delay(100);
    RST_LOW();
    HAL_Delay(100);
    RST_HIGH();
    HAL_Delay(100);
}

static void WriteData(uint8_t data)
{
	CS_Select();
	DC_HIGH();

	HAL_SPI_Transmit(&hspi5, &data, 1, 10);

	CS_Deselect();

}

static void WriteCommand(uint8_t Command)
{
	CS_Select();
	DC_LOW();

	HAL_SPI_Transmit(&hspi5, &Command, 1, 10);

	CS_Deselect();

}

static void WriteData16Bit(uint16_t data)
{
	CS_Select();
	DC_HIGH();

	uint8_t data16_bit[2]={(data>>8)&0xFF, data&0xFF};

	HAL_SPI_Transmit(&hspi5, data16_bit, 2, 10);

	CS_Deselect();

}




static void LCD_direction(LCD_Horizontal_t direction)
{
	switch (direction) {
	case ROTATE_0:
		WriteCommand(0x36);
		WriteData(0x48);
		break;
	case ROTATE_90:
		WriteCommand(0x36);
		WriteData(0x28);
		break;
	case ROTATE_180:
		WriteCommand(0x36);
		WriteData(0x88);
		break;
	case ROTATE_270:
		WriteCommand(0x36);
		WriteData(0xE8);
		break;
	}
}


void ILI9341_Init(LCD_Horizontal_t rot)
{
	Reset();
	HAL_Delay(800);

	/* Power Control A */
	WriteCommand(0xCB);
	WriteData(0x39);
	WriteData(0x2C);
	WriteData(0x00);
	WriteData(0x34);
	WriteData(0x02);
	/* Power Control B */
	WriteCommand(0xCF);
	WriteData(0x00);
	WriteData(0xC1);
	WriteData(0x30);
	/* Driver timing control A */
	WriteCommand(0xE8);
	WriteData(0x85);
	WriteData(0x00);
	WriteData(0x78);
	/* Driver timing control B */
	WriteCommand(0xEA);
	WriteData(0x00);
	WriteData(0x00);
	/* Power on Sequence control */
	WriteCommand(0xED);
	WriteData(0x64);
	WriteData(0x03);
	WriteData(0x12);
	WriteData(0x81);
	/* Pump ratio control */
	WriteCommand(0xF7);
	WriteData(0x20);
	/* Power Control 1 */
	WriteCommand(0xC0);
	WriteData(0x10);
	/* Power Control 2 */
	WriteCommand(0xC1);
	WriteData(0x10);
	/* VCOM Control 1 */
	WriteCommand(0xC5);
	WriteData(0x3E);
	WriteData(0x28);
	/* VCOM Control 2 */
	WriteCommand(0xC7);
	WriteData(0x86);
	/* VCOM Control 2 */
	WriteCommand(0x36);
	WriteData(0x48);
	/* Pixel Format Set */
	WriteCommand(0x3A);
	WriteData(0x55);    //16bit
	WriteCommand(0xB1);
	WriteData(0x00);
	WriteData(0x18);

	/* Display Function Control */
	WriteCommand(0xB6);
	WriteData(0x08);
	WriteData(0x82);
	WriteData(0x27);
	/* 3GAMMA FUNCTION DISABLE */
	WriteCommand(0xF2);
	WriteData(0x00);
	/* GAMMA CURVE SELECTED */
	WriteCommand(0x26); //Gamma set
	WriteData(0x01); 	//Gamma Curve (G2.2)
	//Positive Gamma  Correction
	WriteCommand(0xE0);
	WriteData(0x0F);
	WriteData(0x31);
	WriteData(0x2B);
	WriteData(0x0C);
	WriteData(0x0E);
	WriteData(0x08);
	WriteData(0x4E);
	WriteData(0xF1);
	WriteData(0x37);
	WriteData(0x07);
	WriteData(0x10);
	WriteData(0x03);
	WriteData(0x0E);
	WriteData(0x09);
	WriteData(0x00);
	//Negative Gamma  Correction
	WriteCommand(0xE1);
	WriteData(0x00);
	WriteData(0x0E);
	WriteData(0x14);
	WriteData(0x03);
	WriteData(0x11);
	WriteData(0x07);
	WriteData(0x31);
	WriteData(0xC1);
	WriteData(0x48);
	WriteData(0x08);
	WriteData(0x0F);
	WriteData(0x0C);
	WriteData(0x31);
	WriteData(0x36);
	WriteData(0x0F);





	//EXIT SLEEP
	WriteCommand(0x11);

	HAL_Delay(120);

	//TURN ON DISPLAY
	WriteCommand(0x29);
	HAL_Delay(10);
	WriteData(0x2C);
	HAL_Delay(10);
	LCD_direction(rot);

}

static void ILI9341_SetWindow(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y)
{
	// Set Window
	WriteCommand(0x2a);
	WriteData(start_x >> 8);
	WriteData(0xFF & start_x);
	WriteData(end_x >> 8);
	WriteData(0xFF & end_x);

	WriteCommand(0x2b);
	WriteData(start_y >> 8);
	WriteData(0xFF & start_y);
	WriteData(end_y >> 8);
	WriteData(0xFF & end_y);

}

void ILI9341_WritePixel(uint16_t x, uint16_t y, uint16_t color)
{
	ILI9341_SetWindow(x, y, x, y);

	WriteCommand(0x2c);

	WriteData16Bit(color);

}

void ILI9341_Draw_Bitmap(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, const uint8_t *Image)
{


	uint16_t width  = Xend - Xstart;
	uint16_t height = Yend - Ystart;

	uint32_t idx = 0;
	uint8_t  pixel[2];

	ILI9341_SetWindow(Xstart,Ystart,Xend-1,Yend-1);



	WriteCommand(0x2c);

	CS_Select();

	DC_HIGH();

	for (uint16_t y = 0; y < height; y++)
		{
			for (uint16_t x = 0; x < width; x++)
			{
				pixel[0] = Image[idx++];  // High byte
				pixel[1] = Image[idx++];  // Low byte

				HAL_SPI_Transmit(&hspi5, pixel, 2, HAL_MAX_DELAY);
			}
		}

	CS_Deselect();

}

void ILI9341_Draw_Bitmap_DMA(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, const uint8_t *Image)
{


	uint32_t size = (Xend - Xstart + 1) * (Yend - Ystart + 1) * 2;


	ILI9341_SetWindow(Xstart,Ystart,Xend,Yend);



	WriteCommand(0x2c);

	CS_Select();

	DC_HIGH();
	HAL_SPI_Transmit_DMA(&hspi5, Image, size);


}

__weak void ILI9341_Transfer_Completed(void);

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	CS_Deselect();
	ILI9341_Transfer_Completed();


}

