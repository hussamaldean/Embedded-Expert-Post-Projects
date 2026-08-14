/* ============================ INA219.h ============================ */
#ifndef __INA219_H
#define __INA219_H

#include "main.h"
#include "i2c.h"
#include <stdint.h>
#include <stdbool.h>

/* INA219 I2C address (7-bit). Default = 0x40. HAL expects (addr << 1). */
#define INA219_ADDRESS_GND_GND   (0x40)   /* A0=GND, A1=GND */
#define INA219_ADDRESS_GND_VS    (0x41)
#define INA219_ADDRESS_GND_SDA   (0x42)
#define INA219_ADDRESS_GND_SCL   (0x43)
#define INA219_ADDRESS_VS_GND    (0x44)
#define INA219_ADDRESS_VS_VS     (0x45)
#define INA219_ADDRESS_VS_SDA    (0x46)
#define INA219_ADDRESS_VS_SCL    (0x47)
#define INA219_ADDRESS_SDA_GND   (0x48)
#define INA219_ADDRESS_SDA_VS    (0x49)
#define INA219_ADDRESS_SDA_SDA   (0x4A)
#define INA219_ADDRESS_SDA_SCL   (0x4B)
#define INA219_ADDRESS_SCL_GND   (0x4C)
#define INA219_ADDRESS_SCL_VS    (0x4D)
#define INA219_ADDRESS_SCL_SDA   (0x4E)
#define INA219_ADDRESS_SCL_SCL   (0x4F)

/* Registers */
#define INA219_REG_CONFIGURATION   0x00
#define INA219_REG_SHUNT_VOLTAGE   0x01
#define INA219_REG_BUS_VOLTAGE     0x02
#define INA219_REG_POWER           0x03
#define INA219_REG_CURRENT         0x04
#define INA219_REG_CALIBRATION     0x05

/* PGA (shunt voltage range) */
typedef enum {
    INA219_PGA_40MV = 0x00,   /* +/- 40 mV, gain 1/8  */
    INA219_PGA_80MV = 0x01,   /* +/- 80 mV, gain 1/4  */
    INA219_PGA_160MV = 0x02,  /* +/- 160 mV, gain 1/2 */
    INA219_PGA_320MV = 0x03   /* +/- 320 mV, gain 1   */
} INA219_PGA_t;

/* Bus ADC resolution / averaging */
typedef enum {
    INA219_ADC_9BIT   = 0x00,
    INA219_ADC_10BIT  = 0x01,
    INA219_ADC_11BIT  = 0x02,
    INA219_ADC_12BIT  = 0x03,
    INA219_ADC_2SAMP  = 0x09,
    INA219_ADC_4SAMP  = 0x0A,
    INA219_ADC_8SAMP  = 0x0B,
    INA219_ADC_16SAMP = 0x0C,
    INA219_ADC_32SAMP = 0x0D,
    INA219_ADC_64SAMP = 0x0E,
    INA219_ADC_128SAMP = 0x0F
} INA219_ADC_t;

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t  address;       /* 7-bit address (e.g. 0x40) */
    float    shunt_ohm;     /* shunt resistor value in ohms */
    float    current_lsb;   /* A per LSB (computed from calibration) */
    float    power_lsb;     /* W per LSB (20 * current_lsb) */
} INA219_t;

/* API */
HAL_StatusTypeDef INA219_Init(INA219_t *dev,
                              I2C_HandleTypeDef *hi2c,
                              uint8_t address_7bit,
                              float shunt_resistor_ohm,
                              float max_current_amps);

HAL_StatusTypeDef INA219_Reset(INA219_t *dev);
HAL_StatusTypeDef INA219_SetCalibration(INA219_t *dev, float max_current);

HAL_StatusTypeDef INA219_Read_bus_voltage_mv(INA219_t *dev, float *voltage);
HAL_StatusTypeDef INA219_Read_shunt_voltage_mv(INA219_t *dev, float *voltage);
HAL_StatusTypeDef INA219_Read_current_mA(INA219_t *dev, float *current);
HAL_StatusTypeDef INA219_Read_power_mW(INA219_t *dev, float *power);
HAL_StatusTypeDef INA219_Read_bus_voltage_raw(INA219_t *dev, int16_t *raw);
bool               INA219_ConversionReady(INA219_t *dev);

#endif /* __INA219_H */
