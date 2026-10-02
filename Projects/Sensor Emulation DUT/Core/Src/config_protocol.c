#include "config_protocol.h"
#include <string.h>

uint8_t cfg_checksum(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
    }
    return crc;
}

uint16_t cfg_build_frame(uint8_t *out,
                         uint8_t cmd,
                         const uint8_t *payload,
                         uint8_t payload_len)
{
    if (out == NULL) return 0;
    if (payload_len > CFG_MAX_PAYLOAD) return 0;
    if ((payload_len > 0) && (payload == NULL)) return 0;

    uint16_t idx = 0;
    out[idx++] = CFG_START_BYTE;
    out[idx++] = cmd;
    out[idx++] = payload_len;

    for (uint8_t i = 0; i < payload_len; i++) {
        out[idx++] = payload[i];
    }

    out[idx] = cfg_checksum(out, idx);
    idx++;

    return idx;
}

cfg_status_t cfg_parse_frame(const uint8_t *buf,
                             uint16_t size,
                             cfg_frame_t *frame)
{
    if ((buf == NULL) || (frame == NULL)) return CFG_ERR_TOO_SHORT;
    if (size < CFG_FRAME_OVERHEAD)       return CFG_ERR_TOO_SHORT;
    if (buf[0] != CFG_START_BYTE)        return CFG_ERR_BAD_START;

    uint8_t cmd     = buf[1];
    uint8_t payload = buf[2];

    if (payload > CFG_MAX_PAYLOAD)       return CFG_ERR_TOO_LONG;
    if (size < (uint16_t)(payload + CFG_FRAME_OVERHEAD))
                                         return CFG_ERR_TOO_SHORT;

    /* Checksum covers START..last payload byte */
    uint8_t expected = cfg_checksum(buf, (uint16_t)(payload + 3));
    if (expected != buf[3 + payload])    return CFG_ERR_BAD_CRC;

    frame->cmd    = cmd;
    frame->length = payload;
    if (payload > 0) {
        memcpy(frame->payload, &buf[3], payload);
    }
    return CFG_OK;
}
