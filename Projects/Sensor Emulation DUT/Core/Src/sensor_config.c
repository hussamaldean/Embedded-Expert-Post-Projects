#include "sensor_config.h"
#include <string.h>

static sensor_config_t g_cfg;
static cfg_frame_t     g_frame;

void sensor_config_init(void)
{
    g_cfg.sample_rate = 10;
    g_cfg.threshold   = 50;
    g_cfg.reserved    = 0;
}

const sensor_config_t *sensor_config_get(void)
{
    return &g_cfg;
}

/* --- Payload layouts ---
 *   CMD_WRITE: [sample_rate, threshold]
 *   CMD_READ : (none)
 */
uint8_t sensor_config_handle_rx(const uint8_t *buf, uint16_t size)
{
    cfg_status_t st = cfg_parse_frame(buf, size, &g_frame);
    if (st != CFG_OK) {
        return CFG_NACK;
    }

    switch (g_frame.cmd) {
        case CFG_CMD_WRITE:
            if (g_frame.length < 2) return CFG_NACK;
            g_cfg.sample_rate = g_frame.payload[0];
            g_cfg.threshold   = g_frame.payload[1];
            return CFG_ACK;

        case CFG_CMD_READ:
            /* Read-back will be implemented next step */
            return CFG_ACK;

        default:
            return CFG_NACK;
    }
}
