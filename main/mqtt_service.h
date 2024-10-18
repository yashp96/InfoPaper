#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#define EPAPER_SOF  0xF1

#define IMG_BIN_SIZE_MAX        268800
#define IMG_BIN_TX_BYTES        900
#define IMG_TRANSFER_NUMBERS    (IMG_BIN_SIZE_MAX/IMG_BIN_TX_BYTES)

/*
 * FRAME FORMAT
 * | SOF (1) | COMMAND (1) | DATA LENGTH (2) | DATA (n Bytes <= 300) |
*/

typedef enum EPAPER_BUFFER_INDICES
{
    EPAPER_SOF_INDX = 0,
    EPAPER_CMD_INDX,
    EPAPER_DATA_SEQ_H_INDX,
    EPAPER_DATA_SEQ_L_INDX,
    EPAPER_DATA_INDX
}epaper_indices_e;

typedef enum EPAPER_COMMANDS
{
    EPAPER_INIT = 0x01,
    EPAPER_REFRESH = 0x02,
    EPAPER_SLEEP = 0x03,
    EPAPER_INIT_IMG = 0x04,
    EPAPER_TX_IMG = 0x05,
    EPAPER_TEST = 0x06,
    EPAPER_CLEAR = 0x07
}epaper_commands_e;

#pragma pack(1)
typedef struct EPAPER_COMMAND
{
    uint8_t sof;
    uint8_t cmd;
    uint16_t data_seq_num;
    uint8_t cmd_exec_flag;
}epaper_cmd_t;
#pragma pack(0)

void mqtt_app_start();

#endif