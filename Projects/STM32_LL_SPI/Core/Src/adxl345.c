#include "adxl345.h"
#include "spi.h"


#define DATA_FORMAT_R   		(0x31) /*data format register*/
#define POWER_CTL_R 			(0x2D) /*Power control register*/
#define DATA_START_ADDR			(0x32) /*Start address to read the values*/

#define	RESET					(0x00) /*Reset value*/
#define SET_MEASURE_B		    (0x08) /*Put adxl345 into measurement mode*/

/*Variables which hold some variable*/
uint8_t data_rec[6];
uint8_t acc_range;
int16_t x,y,z;


#define   MULTI_BYTE_EN			0x40
#define	  READ_OPERATION		0x80

#define CS_LOW()    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_0)
#define CS_HIGH()   LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_0)


static void adxl345_write(uint8_t address, uint8_t value)
{
	uint8_t data[2];

	/*Enable multi-byte, place address into buffer*/
	data[0] = address|MULTI_BYTE_EN;

	/*Place data into buffer*/
	data[1] = value;

	/*Pull cs line low to enable slave*/
	CS_LOW();

	/*Transmit data and address*/
	SPI_TransmitBytes(SPI1,data, 2);

	/*Pull cs line high to disable slave*/
	CS_HIGH();


}

static void adxl345_read(uint8_t address, uint8_t * rxdata)
{

	/*Set read operation*/
	address |= READ_OPERATION;

	/*Enable multi-byte*/
	address |= MULTI_BYTE_EN;

	/*Pull cs line low to enable slave*/
	CS_LOW();

	/*Send address*/
	SPI_TransmitBytes(SPI1,&address,1);

	/*Read 6 bytes */
	SPI_ReceiveBytes(SPI1,rxdata,6);

	/*Pull cs line high to disable slave*/
	CS_HIGH();

}

void adxl345_init(adxl345Parameters param)
	{
		acc_range=param;

		/*Set data format range to +-4g*/
		adxl345_write (DATA_FORMAT_R, param);

		/*Reset all bits*/
		adxl345_write (POWER_CTL_R, RESET);

		/*Configure power control measure bit*/
		adxl345_write (POWER_CTL_R, SET_MEASURE_B);

	}


void adxl345_update()
	{



		adxl345_read(DATA_START_ADDR,data_rec);

		x = ((data_rec[1]<<8)|data_rec[0]);
		y = ((data_rec[3]<<8)|data_rec[2]);
		z = ((data_rec[5]<<8)|data_rec[4]);

	}

void adxl345_get_values(accleration_values_t * values)
	{
	float divider;
	switch(acc_range)
	{
		case accl_2g: divider=0.003906; /*1/256*/	break;
		case accl_4g: divider=0.0078125;/*1/128*/	break;
		case accl_8g: divider=0.01563;	/*1/64*/	break;
		case accl_16g: divider=0.03125;	/*1/32*/	break;
	}

		values->ax=x*divider;
		values->ay=y*divider;
		values->az=z*divider;
	}
