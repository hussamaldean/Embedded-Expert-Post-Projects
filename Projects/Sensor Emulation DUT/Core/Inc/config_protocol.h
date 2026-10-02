/*
 * config_protocol.h
 *
 *  Created on: Sep 30, 2026
 *      Author: hussamaldean
 */

#ifndef INC_CONFIG_PROTOCOL_H_
#define INC_CONFIG_PROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>

/* ---- Frame constants ---- */
#define CFG_START_BYTE      0xAA
#define CFG_CMD_WRITE       0x01
#define CFG_CMD_READ        0x02
#define CFG_ACK             0x06
#define CFG_NACK            0x15
#define CFG_MAX_PAYLOAD     32

/* Total frame = START(1) + CMD(1) + LEN(1) + PAYLOAD(N) + CRC(1) */
#define CFG_FRAME_OVERHEAD  4
#define CFG_MAX_FRAME       (CFG_MAX_PAYLOAD + CFG_FRAME_OVERHEAD)

/* ---- Result codes ---- */
typedef enum {
    CFG_OK = 0,
    CFG_ERR_TOO_SHORT,
    CFG_ERR_BAD_START,
    CFG_ERR_TOO_LONG,
    CFG_ERR_BAD_CRC,
    CFG_ERR_UNKNOWN_CMD,
    CFG_ERR_BAD_PAYLOAD
} cfg_status_t;

/* ---- Parsed frame view ---- */
typedef struct {
    uint8_t  cmd;
    uint8_t  length;
    uint8_t  payload[CFG_MAX_PAYLOAD];
} cfg_frame_t;

/* ---- Checksum ---- */
uint8_t cfg_checksum(const uint8_t *data, uint16_t len);

/* ---- Frame builder: returns total bytes written (0 on error) ---- */
uint16_t cfg_build_frame(uint8_t *out,
                         uint8_t cmd,
                         const uint8_t *payload,
                         uint8_t payload_len);

/* ---- Frame parser: fills `frame` on success ---- */
cfg_status_t cfg_parse_frame(const uint8_t *buf,
                             uint16_t size,
                             cfg_frame_t *frame);


#endif /* INC_CONFIG_PROTOCOL_H_ */
