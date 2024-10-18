#ifndef SPI_EPAPER_DRVR
#define SPI_EPAPER_DRVR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

// IO Definitions
#define GPIO_EPAPER_BUSY    13
#define GPIO_EPAPER_RESET   12
#define GPIO_EPAPER_DC      14
#define GPIO_EPAPER_CS      27
#define GPIO_EPAPER_SCK     25
#define GPIO_EPAPER_MOSI    26

// eInk Color Definitions
#define BLACK_8BIT      0x00
#define WHITE_8BIT      0x11
#define GREEN_8BIT      0x22
#define BLUE_8BIT       0x33
#define RED_8BIT        0x44
#define YELLOW_8BIT     0x55
#define ORANGE_8BIT     0x66
#define CLEAN_8BIT      0x77

#define BLACK_4BIT      0x00
#define WHITE_4BIT      0x01
#define GREEN_4BIT      0x02
#define BLUE_4BIT       0x03
#define RED_4BIT        0x04
#define YELLOW_4BIT     0x05
#define ORANGE_4BIT     0x06
#define CLEAN_4BIT      0x07

//To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many. More means more memory use,
//but less overhead for setting up / finishing transfers. Make sure 240 is dividable by this.
#define PARALLEL_LINES 30

void ePaperInit();
void ePaperSleep();
void ePaperRefresh();
void ePaperClear();
void ePaperDisplayImg(const uint8_t* img);

void ePaperStartImgTxMode();
void ePaperTxImg(const uint8_t* img);

void ePaperWriteDataBuffer(spi_device_handle_t spi, const uint8_t* data, int len);
void ePaperWriteData(spi_device_handle_t spi, const uint8_t data, bool keep_cs_active);
void ePaperWriteCmd(spi_device_handle_t spi, const uint8_t cmd, bool keep_cs_active);

#endif