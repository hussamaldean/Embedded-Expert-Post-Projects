#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

#include "config_protocol.h"

/* Persisted configuration of the emulated sensor */
typedef struct {
    uint8_t  sample_rate;   /* Hz */
    uint8_t  threshold;
    uint16_t reserved;
} sensor_config_t;

/* Called once at boot */
void sensor_config_init(void);

/* Feed raw bytes received on USART1.
   Returns the byte to send back as ACK/NACK (CFG_ACK or CFG_NACK). */
uint8_t sensor_config_handle_rx(const uint8_t *buf, uint16_t size);

/* Accessor for other emulation code */
const sensor_config_t *sensor_config_get(void);

#endif /* SENSOR_CONFIG_H */
