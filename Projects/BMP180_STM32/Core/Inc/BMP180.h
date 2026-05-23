/*
 * BMP180.h
 *
 *  Created on: May 2, 2026
 *      Author: hussamaldean
 */

#ifndef INC_BMP180_H_
#define INC_BMP180_H_


typedef enum
{
	OSS0=0,
	OSS1=1,
	OSS2=2,
	OSS3=3
}OSS;

void BMP180_init();

void update_BMP180(OSS OSS);
float get_temperature(void);
float get_pressure(void);


#endif /* INC_BMP180_H_ */
