/*
 * CS43L22.h
 *
 *  Created on: Jun 20, 2026
 *      Author: hussamaldean
 */

#ifndef INC_CS43L22_H_
#define INC_CS43L22_H_


#include "stdint.h"


#define DAC_I2C_ADDR 				(0x94)

#define POWER_CONTROL1				0x02
#define POWER_CONTROL2				0x04
#define CLOCKING_CONTROL 	  		0x05
#define INTERFACE_CONTROL1			0x06
#define INTERFACE_CONTROL2			0x07
#define PASSTHROUGH_A				0x08
#define PASSTHROUGH_B				0x09
#define MISCELLANEOUS_CONTRLS		0x0E
#define PLAYBACK_CONTROL			0x0F
#define PASSTHROUGH_VOLUME_A		0x14
#define PASSTHROUGH_VOLUME_B		0x15
#define PCM_VOLUME_A				0x1A
#define PCM_VOLUME_B				0x1B
#define CONFIG_00					0x00
#define CONFIG_47					0x47
#define CONFIG_32					0x32

#define CS43L22_REG_MASTER_A_VOL    0x20
#define CS43L22_REG_MASTER_B_VOL    0x21

#define CS43_MUTE				 	0x00

#define CS43_RIGHT					0x01
#define CS43_LEFT				 	0x02
#define CS43_RIGHT_LEFT	 			0x03

#define VOLUME_CONVERT_A(Volume)    (((Volume) > 100)? 255:((uint8_t)(((Volume) * 255) / 100)))
#define VOLUME_CONVERT_D(Volume)    (((Volume) > 100)? 24:((uint8_t)((((Volume) * 48) / 100) - 24)))

typedef enum
{
	CS43L22_MODE_I2S = 0,
	CS43L22_MODE_ANALOG,
}CS43_MODE;

void CS43_Init(CS43_MODE outputMode);
void CS43_Enable_RightLeft(uint8_t side);
void CS43_SetVolume(uint8_t volume);
void CS43_Start(void);
void CS43_Stop(void);
void CS43L22_RST_Pin_Low(void);
void CS43L22_RST_Pin_High(void);





#endif /* INC_CS43L22_H_ */
