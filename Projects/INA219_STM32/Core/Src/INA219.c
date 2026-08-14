/* ============================ INA219.c ============================ */
#include "INA219.h"

/* ---------- low level helpers ---------- */
static HAL_StatusTypeDef INA219_WriteReg(INA219_t *dev, uint8_t reg, uint16_t value)
{
    uint8_t buf[2];
    buf[0] = (value >> 8) & 0xFF;
    buf[1] = value & 0xFF;

    return HAL_I2C_Mem_Write(dev->hi2c,
                             (dev->address << 1),
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             buf, 2, 100);
}

static HAL_StatusTypeDef INA219_ReadReg(INA219_t *dev, uint8_t reg, uint16_t *value)
{
    uint8_t buf[2];
    HAL_StatusTypeDef st;

    st = HAL_I2C_Mem_Read(dev->hi2c,
                          (dev->address << 1),
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          buf, 2, 100);

    if (st != HAL_OK) return st;

    *value = ((uint16_t)buf[0] << 8) | buf[1];
    return HAL_OK;
}

/* ---------- public API ---------- */
HAL_StatusTypeDef INA219_Reset(INA219_t *dev)
{
    /* Bit 15 = reset */
    return INA219_WriteReg(dev, INA219_REG_CONFIGURATION, 0x8000);
}

HAL_StatusTypeDef INA219_SetCalibration(INA219_t *dev, float max_current)
{
    /* CAL = 0.04096 / (current_lsb * Rshunt)
       current_lsb chosen so that max_current fits in 15 bits (signed). */
    float current_lsb = max_current / 32767.0f;
    if (current_lsb <= 0.0f) current_lsb = 0.0001f;

    float cal = 0.04096f / (current_lsb * dev->shunt_ohm);
    if (cal > 65535.0f) cal = 65535.0f;
    if (cal < 1.0f)     cal = 1.0f;

    uint16_t cal_reg = (uint16_t)cal;
    /* Recompute actual current_lsb from rounded cal value */
    dev->current_lsb = 0.04096f / (cal_reg * dev->shunt_ohm);
    dev->power_lsb   = 20.0f * dev->current_lsb;

    HAL_StatusTypeDef st = INA219_WriteReg(dev, INA219_REG_CALIBRATION, cal_reg);
    return st;
}

HAL_StatusTypeDef INA219_Init(INA219_t *dev,
                              I2C_HandleTypeDef *hi2c,
                              uint8_t address_7bit,
                              float shunt_resistor_ohm,
                              float max_current_amps)
{
    if (dev == NULL || hi2c == NULL) return HAL_ERROR;

    dev->hi2c       = hi2c;
    dev->address    = address_7bit & 0x7F;
    dev->shunt_ohm  = shunt_resistor_ohm;
    dev->current_lsb = 0.0f;
    dev->power_lsb   = 0.0f;

    /* Reset */
    HAL_StatusTypeDef st = INA219_Reset(dev);
    if (st != HAL_OK) return st;
    HAL_Delay(2);

    /* Configuration:
       BRNG   = 1  (32V bus range)
       PG     = INA219_PGA_320MV (11:12)
       BADC   = 12-bit (7:10)
       SADC   = 12-bit (3:6)
       MODE   = 0x07 (shunt & bus, continuous) (0:2)
    */
    uint16_t config = (1u << 13)                              /* BRNG = 32V */
                    | ((uint16_t)INA219_PGA_320MV << 11)     /* PG   */
                    | ((uint16_t)INA219_ADC_12BIT  << 7)     /* BADC */
                    | ((uint16_t)INA219_ADC_12BIT  << 3)     /* SADC */
                    | 0x07;                                   /* MODE */

    st = INA219_WriteReg(dev, INA219_REG_CONFIGURATION, config);
    if (st != HAL_OK) return st;

    /* Apply calibration so current/power registers produce valid data */
    st = INA219_SetCalibration(dev, max_current_amps);
    return st;
}

HAL_StatusTypeDef INA219_Read_bus_voltage_raw(INA219_t *dev, int16_t *raw)
{
    uint16_t v;
    HAL_StatusTypeDef st = INA219_ReadReg(dev, INA219_REG_BUS_VOLTAGE, &v);
    if (st != HAL_OK) return st;

    /* Clear all flags by masking only the 13 bits that matter, then shift */
    *raw = (int16_t)((v & 0xFFF8) >> 3);

    return HAL_OK;
}

bool INA219_ConversionReady(INA219_t *dev)
{
    uint16_t v;
    if (INA219_ReadReg(dev, INA219_REG_BUS_VOLTAGE, &v) != HAL_OK) return false;
    /* Bit 1 = Conversion Ready (CNVR) */
    return (v & 0x02) ? true : false;
}

HAL_StatusTypeDef INA219_Read_bus_voltage_mv(INA219_t *dev, float *voltage)
{
    int16_t raw;
    HAL_StatusTypeDef st = INA219_Read_bus_voltage_raw(dev, &raw);
    if (st != HAL_OK) return st;
    *voltage = raw * 4.0f;     /* LSB = 4 mV */
    return HAL_OK;
}

HAL_StatusTypeDef INA219_Read_shunt_voltage_mv(INA219_t *dev, float *voltage)
{
    uint16_t v;
    HAL_StatusTypeDef st = INA219_ReadReg(dev, INA219_REG_SHUNT_VOLTAGE, &v);
    if (st != HAL_OK) return st;
    int16_t sv = (int16_t)v;
    *voltage = sv * 0.01f;     /* LSB = 10 uV = 0.01 mV */
    return HAL_OK;
}

HAL_StatusTypeDef INA219_Read_current_mA(INA219_t *dev, float *current)
{
    uint16_t v;
    HAL_StatusTypeDef st = INA219_ReadReg(dev, INA219_REG_CURRENT, &v);
    if (st != HAL_OK) return st;
    int16_t raw = (int16_t)v;
    *current = raw * dev->current_lsb * 1000.0f;  /* A -> mA */
    return HAL_OK;
}

HAL_StatusTypeDef INA219_Read_power_mW(INA219_t *dev, float *power)
{
    uint16_t v;
    HAL_StatusTypeDef st = INA219_ReadReg(dev, INA219_REG_POWER, &v);
    if (st != HAL_OK) return st;
    /* power register is unsigned 16-bit, LSB = 20 * current_lsb (in W) */
    *power = v * dev->power_lsb * 1000.0f;  /* W -> mW */
    return HAL_OK;
}
