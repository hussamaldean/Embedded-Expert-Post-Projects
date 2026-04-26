/*
 * STMPE811.c
 *
 *  Created on: Apr 8, 2026
 *      Author: hussamaldean
 */


#include "STMPE811.h"

#include "i2c.h"




static void STMPE811_WriteReg(uint8_t reg, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c3,deviceAddress,reg,I2C_MEMADD_SIZE_8BIT,&data,1,HAL_MAX_DELAY);
}

static void STMPE811_ReadReg(uint8_t reg, uint8_t *data)
{
    HAL_I2C_Mem_Read(&hi2c3,deviceAddress,reg,I2C_MEMADD_SIZE_8BIT,data,1,HAL_MAX_DELAY);
}

static void STMPE811_ReadMulti(uint8_t reg, uint8_t *data, uint16_t len)
{
	HAL_I2C_Mem_Read(&hi2c3,deviceAddress ,reg,I2C_MEMADD_SIZE_8BIT,data,len,HAL_MAX_DELAY);
}



static uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
	  return (x - in_min) * (out_max - out_min + 1) / (in_max - in_min + 1) + out_min;
}


void STMPE811_Touch_Enable(void)
{
    uint8_t mode;

    /* Power Down the STMPE811 */
    STMPE811_WriteReg(STMPE811_REG_SYS_CTRL1, 0x02);
    HAL_Delay(10);

    /* Power On (reset registers) */
    STMPE811_WriteReg(STMPE811_REG_SYS_CTRL1, 0x00);
    HAL_Delay(2);

    /* Disable GPIO clock */
    STMPE811_ReadReg(STMPE811_REG_SYS_CTRL2, &mode);
    mode &= ~(STMPE811_IO_FCT);
    STMPE811_WriteReg(STMPE811_REG_SYS_CTRL2, mode);

    /* Select TSC pins in alternate mode */
    STMPE811_WriteReg(STMPE811_REG_IO_AF, STMPE811_TOUCH_IO_ALL);

    /* Enable ADC and Touch */
    mode &= ~(STMPE811_TS_FCT | STMPE811_ADC_FCT);
    STMPE811_WriteReg(STMPE811_REG_SYS_CTRL2, mode);

    /* ADC configuration */
    STMPE811_WriteReg(STMPE811_REG_ADC_CTRL1, 0x49);
    HAL_Delay(2);

    STMPE811_WriteReg(STMPE811_REG_ADC_CTRL2, 0x01);

    /* Touch configuration
       AVG = 8 samples
       Delay = 500us
       Panel driver = 500us
    */
    STMPE811_WriteReg(STMPE811_REG_TSC_CFG, 0xDA);

    /* FIFO threshold = 1 */
    STMPE811_WriteReg(STMPE811_REG_FIFO_TH, 0x01);

    /* Reset FIFO */
    STMPE811_WriteReg(STMPE811_REG_FIFO_STA, 0x01);
    STMPE811_WriteReg(STMPE811_REG_FIFO_STA, 0x00);

    /* Pressure measurement configuration */
    STMPE811_WriteReg(STMPE811_REG_TSC_FRACT_XYZ, 0x07);

    /* Drive current = 50mA */
    STMPE811_WriteReg(STMPE811_REG_TSC_I_DRIVE, 0x01);

    /* Enable Touch Screen Controller */
    STMPE811_WriteReg(STMPE811_REG_TSC_CTRL, 0x01);

    /* Clear interrupts */
    STMPE811_WriteReg(STMPE811_REG_INT_STA, 0xFF);

    HAL_Delay(5);
}

TouchDetect_t isTouched(void)
{
    uint8_t value = 0;

    STMPE811_ReadReg(STMPE811_REG_TSC_CTRL, &value);

    value &= STMPE811_TS_CTRL_STATUS;

    if(value == STMPE811_TS_CTRL_STATUS)
    {
        STMPE811_ReadReg(STMPE811_REG_FIFO_SIZE, &value);

        if(value > 0)
        {
            return touched;
        }
    }
    else
    {
        /* Reset FIFO */
        STMPE811_WriteReg(STMPE811_REG_FIFO_STA, 0x01);
        STMPE811_WriteReg(STMPE811_REG_FIFO_STA, 0x00);
    }

    return no_touch;
}

void getTouchValue(uint16_t *X, uint16_t *Y)
{
    uint32_t uldataXYZ;
    uint8_t dataXYZ[4];

    uint16_t x,y;

    STMPE811_ReadMulti(STMPE811_REG_TSC_DATA_NON_INC, dataXYZ, 4);

    /* Combine data */
    uldataXYZ = (dataXYZ[0] << 24) |
                (dataXYZ[1] << 16) |
                (dataXYZ[2] << 8)  |
                (dataXYZ[3]);

    x = (uldataXYZ >> 20) & 0x0FFF;
    y = (uldataXYZ >> 8)  & 0x0FFF;

    /* Reset FIFO */
    STMPE811_WriteReg(STMPE811_REG_FIFO_STA, 0x01);

    /* Re-enable FIFO */
    STMPE811_WriteReg(STMPE811_REG_FIFO_STA, 0x00);

    *X=map(x,Touch_XMIN,Touch_XMAX,0,240);
    *Y=map(y,Touch_YMIN,Touch_YMAX,0,320);


}

uint16_t getID(void)
{
    uint8_t data[2];

    STMPE811_ReadMulti(STMPE811_REG_CHIP_ID, data, 2);

    return (data[0] << 8) | data[1];
}
