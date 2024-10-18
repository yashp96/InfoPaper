#include "includes.h"

/* Macros */
#define TAG "E-PAPER"
// #define LOG_EPAPER       /* Uncomment to enable serial logging for epaper functions */
// #define COMPACT_COLORS   /* Uncomment to use alternate ePaperGetImgColor function*/

/* Function Declarations */

static void ePaperSPIPreTxCallback(spi_transaction_t *t);
static void ePaperReset();

/* Variables */
spi_bus_config_t EPaperBusConfig =
{
    .miso_io_num =  -1,
    .mosi_io_num = GPIO_EPAPER_MOSI,
    .sclk_io_num = GPIO_EPAPER_SCK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = PARALLEL_LINES*320*2+8
};

spi_device_interface_config_t ePaperDevCfg =
{
    .clock_speed_hz = 20*1000*1000,             //Clock out at 20 MHz
    .mode = 0,                                  //SPI mode 0
    .spics_io_num = GPIO_EPAPER_CS,             //CS pin
    .queue_size = 7,                            //We want to be able to queue 7 transactions at a time
    .pre_cb=ePaperSPIPreTxCallback,             //Specify pre-transfer callback to handle D/C line
};

spi_device_handle_t ePaperSPI;

/* Function Definitions */

/*
@brief callback function called by SPI task before tranmission begins

@param *t Pointer to SPI transmission onject

@return void
*/
static void ePaperSPIPreTxCallback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(GPIO_EPAPER_DC, dc);
}

/*
@brief Perform reset on ePaper

@param na

@return void
*/
static void ePaperReset()
{
    gpio_set_level(GPIO_EPAPER_RESET, false);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(GPIO_EPAPER_RESET, true);
    vTaskDelay(pdMS_TO_TICKS(100));
}

/*
@brief Write a byte on to the SPI bus.

@param spi SPI handle for the SPI bus being used.
@param cmd epaper commanf to be executed
@paran keep_cs_active flag to keep CS ping active after transmission is complete

@return void
*/
void ePaperWriteCmd(spi_device_handle_t spi, const uint8_t cmd, bool keep_cs_active)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    if (keep_cs_active)
    {
        t.flags = SPI_TRANS_CS_KEEP_ACTIVE;   //Keep CS active after data transfer
    }
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

/*
@brief Write a byte on to the SPI bus.

@param spi SPI handle for the SPI bus being used.
@param data data to be transmitted
@paran keep_cs_active flag to keep CS ping active after transmission is complete

@return void
*/
void ePaperWriteData(spi_device_handle_t spi, const uint8_t data, bool keep_cs_active)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8;
    t.tx_buffer = &data;
    if (keep_cs_active)
    {
        t.flags = SPI_TRANS_CS_KEEP_ACTIVE;   //Keep CS active after data transfer
    }
    t.user = (void*)1;
    ret = spi_device_polling_transmit(spi, &t); //Transmit!
    assert(ret == ESP_OK);          //Should have had no issues.
}

/*
@brief Write data from buffer to the SPI bus.

@param spi SPI handle for the SPI bus being used.
@param data pointer to data to be transmitted
@paran len number of bytes to be transmitted

@return void
*/
void ePaperWriteDataBuffer(spi_device_handle_t spi, const uint8_t* data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8 * len;
    t.tx_buffer = data;
    t.user = (void*)1;
    ret = spi_device_polling_transmit(spi, &t); //Transmit!
    assert(ret == ESP_OK);          //Should have had no issues.
}

/*
@brief Waits while epaper is busy processing already transmitted commands.

@param na

@return void
*/
void ePaperCheckBusyStatus()
{
#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "Checking ePaper Busy Status");
#endif
    while(!gpio_get_level(GPIO_EPAPER_BUSY))
    {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "ePaper opr complete");
#endif
}

/*
@brief Refresh the epaper to display latest image that was transferred.

@param na

@return void
*/
void ePaperRefresh()
{
#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "Refreshing ePaper");
#endif
    ePaperWriteCmd(ePaperSPI, 0x12, false);
    vTaskDelay(pdMS_TO_TICKS(100));
    ePaperCheckBusyStatus();
}

/*
@brief Puts epaper to deep sleep to reduce power consumptions.

@param na

@return void
*/
void ePaperSleep()
{
    spi_device_acquire_bus(ePaperSPI, portMAX_DELAY);
    ePaperWriteCmd(ePaperSPI, 0x02, false);
    ePaperCheckBusyStatus();
    ePaperWriteCmd(ePaperSPI, 0x07, true);
    ePaperWriteData(ePaperSPI, 0xA5, false);
    spi_device_release_bus(ePaperSPI);
}

/*
@brief Resets and initializes the ePaper so that we can set the image.
better to refer epaper datasheet for understanding the registers and their values.
Note: Needed to be called epaper is put to sleep in current power cycle.

@param na

@return void
*/
void ePaperInit()
{
    esp_err_t err;

    gpio_config_t paper_control_io = {};
    paper_control_io.pin_bit_mask = ((1ULL << GPIO_EPAPER_DC) | (1ULL << GPIO_EPAPER_RESET));
    paper_control_io.mode = GPIO_MODE_OUTPUT;
    paper_control_io.pull_up_en = true;
    gpio_config(&paper_control_io);

    gpio_config_t paper_busy = {};
    paper_busy.pin_bit_mask = 1ULL<<GPIO_EPAPER_BUSY;
    paper_busy.mode = GPIO_MODE_INPUT;
    paper_busy.pull_up_en = true;
    gpio_config(&paper_busy);

    err = spi_bus_initialize(SPI2_HOST, &EPaperBusConfig, SPI_DMA_DISABLED);
    err = spi_bus_add_device(SPI2_HOST, &ePaperDevCfg, &ePaperSPI);

    // reset ePaper
    ePaperReset();

    spi_device_acquire_bus(ePaperSPI, portMAX_DELAY);

    ePaperWriteCmd(ePaperSPI, 0x01, true); // power
    ePaperWriteData(ePaperSPI, 0x37, true);
    ePaperWriteData(ePaperSPI, 0x00, true);
    ePaperWriteData(ePaperSPI, 0x23, true);
    ePaperWriteData(ePaperSPI, 0x23, false);

    ePaperWriteCmd(ePaperSPI, 0x00, true); // panel setings
    ePaperWriteData(ePaperSPI, 0xEF, true);
    ePaperWriteData(ePaperSPI, 0x08, false);

    ePaperWriteCmd(ePaperSPI, 0x03, true);  // pfs
    ePaperWriteData(ePaperSPI, 0x00, false);

    ePaperWriteCmd(ePaperSPI, 0x06, true); // boost
    ePaperWriteData(ePaperSPI, 0xC7, true);
    ePaperWriteData(ePaperSPI, 0xC7, true);
    ePaperWriteData(ePaperSPI, 0x1D, false);

    ePaperWriteCmd(ePaperSPI, 0x30, true); //PLL
    ePaperWriteData(ePaperSPI, 0x3C, false);

    ePaperWriteCmd(ePaperSPI, 0x41, true); // tse
    ePaperWriteData(ePaperSPI, 0x00, false);

    ePaperWriteCmd(ePaperSPI, 0x50, true); // vcom
    ePaperWriteData(ePaperSPI, 0x37, false); // 0x77

    ePaperWriteCmd(ePaperSPI, 0x60, true); // tcon
    ePaperWriteData(ePaperSPI, 0x22, false);

    ePaperWriteCmd(ePaperSPI, 0x60, true); // tcon
    ePaperWriteData(ePaperSPI, 0x22, false);

    ePaperWriteCmd(ePaperSPI, 0x61, true); // tres
    ePaperWriteData(ePaperSPI, 0x02, true);
    ePaperWriteData(ePaperSPI, 0x58, true);
    ePaperWriteData(ePaperSPI, 0x01, true);
    ePaperWriteData(ePaperSPI, 0xC0, false);

    ePaperWriteCmd(ePaperSPI, 0xE3, true);
    ePaperWriteData(ePaperSPI, 0xAA, false);

    ePaperWriteCmd(ePaperSPI, 0x04, false);

    ePaperCheckBusyStatus();
    spi_device_release_bus(ePaperSPI);
    
#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "ePaper init. complete");
#endif
}

/*
@brief Sets complete epaper to a specific color. Internally it uses same commands that are used to set
an image.

@param color Pass one of the colors supported by ePaper

@return void
*/
void ePaperSetACEP(unsigned char color)
{
    unsigned int i = 0, j = 0;

    ESP_LOGI(TAG,"ePaper ACEP started");

    ePaperWriteCmd(ePaperSPI, 0x10, true);

    for (i = 0; i < 448; i++)
    {
        for (j = 0; j < 300; j++)
        {
            ePaperWriteData(ePaperSPI, color, true);
        }      
    }

    ePaperWriteCmd(ePaperSPI, 0x12, false);
    vTaskDelay(pdMS_TO_TICKS(100));
    ePaperCheckBusyStatus();
#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "ePaper ACEP opr complete");
#endif
}

/*
@brief Clears epaper display

@param na

@return void
*/
void ePaperClear()
{
    uint16_t i = 0, j = 0;
    
    spi_device_acquire_bus(ePaperSPI, portMAX_DELAY);
    // ePaperSetACEP(CLEAN_8BIT);
    ePaperWriteCmd(ePaperSPI, 0x10, true);
    
#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "ePaper clear opr. started");
#endif

    for (i = 0; i < 448; i++)
    {
        for (j = 0; j < 300; j++)
        {
            ePaperWriteData(ePaperSPI, WHITE_8BIT, true);
        }    
    }

    ePaperRefresh();
    spi_device_release_bus(ePaperSPI);
#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "ePaper cleared");
#endif
}

#ifdef COMPACT_COLORS
/*
@brief Use this function to map the colors of image to the colors supported
by epaper. This function uses switch case for mapping color which is kind of ineffecient
if the colors in the image are don't match.
Uncomment COMPACT_COLORS to use alternate version of this function.

@param color pixel data of image that is to mapped as per epaper colors

@return uint8_t 4 bit color code compatible with epaper
*/
uint8_t ePaperGetImgColor(unsigned char color)
{
  uint8_t ecolor = 0;
  switch(color)
  {
    case 0xFF:
      ecolor = WHITE_4BIT;
    break;
    case 0xFC:
      ecolor = YELLOW_4BIT;
    break;  
    case 0xEC:
      ecolor = ORANGE_4BIT;
    break;      
    case 0xE0:
      ecolor = RED_4BIT;
    break;  
    case 0x35:
      ecolor = GREEN_4BIT;
    break;  
    case 0x2B:
      ecolor = BLUE_4BIT;
    break;  
    case 0x00:
      ecolor = BLACK_4BIT;
    break;    
    default:
    break;      
  }
   return ecolor;
}
#else
/*
@brief Use this function to map the colors of image to the colors supported
by epaper. This function uses range for mapping color which is not great but gets the
job done. May be you can modify if you have better ideas.
Uncomment macro COMPACT_COLORS to use alternate version of this function.

@param color pixel data of image that is to mapped as per epaper colors

@return uint8_t 4 bit color code compatible with epaper
*/
uint8_t ePaperGetImgColor(unsigned char color)
{
  uint8_t ecolor = 0;

    if(color == 0xFF)
    {
        ecolor = WHITE_4BIT;
    }
    else if(color >= 0xF8 && color <= 0xFE)
    {
        ecolor = YELLOW_4BIT;
    }
    else if(color >= 0xEC && color <= 0xF5)
    {
        ecolor = ORANGE_4BIT;
    }
    else if(color >= 0x80 && color <= 0xEB)
    {
        ecolor = RED_4BIT;
    }
    else if(color >= 0x19 && color <= 0x5F) // 0x19
    {
        ecolor = GREEN_4BIT;
    }
    else if(color >= 0x03 && color <= 0x2B) 
    {
        ecolor = BLUE_4BIT;
    }
    else
    {
        ecolor = BLACK_4BIT;
    }

   return ecolor;
}
#endif

/*
@brief Sets ePaper to image transfer mode and transmits
image data over SPI

@param img points to image data buffer

@return void
*/
void ePaperDisplayImg(const uint8_t* img)
{
    uint16_t row, col, plane;
    uint8_t temp_c1, temp_c2;
    uint8_t data_H, data_L, data;

#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "img tx. to epaper");
#endif

    spi_device_acquire_bus(ePaperSPI, portMAX_DELAY);
    // ePaperSetACEP(CLEAN_8BIT);
    ePaperWriteCmd(ePaperSPI, 0x10, true);

    for (row = 0; row < 448;row++)
    { 
        plane = 0;
        for (col = 0; col < 300; col++)
        {
            temp_c1 = img[row*600+plane++]; 
            temp_c2 = img[row*600+plane++];
            data_H = ePaperGetImgColor(temp_c1)<<4;
            data_L = ePaperGetImgColor(temp_c2);
            data = data_H|data_L;
            ePaperWriteData(ePaperSPI, data, true);
        }
    }

    ePaperRefresh();
    spi_device_release_bus(ePaperSPI);
#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "img tx. done");
#endif
}

/*
@brief Commands ePaper to enter image transfer mode

@param none

@return void
*/
void ePaperStartImgTxMode()
{
    spi_device_acquire_bus(ePaperSPI, portMAX_DELAY);
    ePaperWriteCmd(ePaperSPI, 0x10, false);
    spi_device_release_bus(ePaperSPI);
#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "epaper in img tx mode");
#endif
}

/*
@brief Transmits image to ePaper over SPI bus.

@param img points to image data buffer

@return void
*/
void ePaperTxImg(const uint8_t* img)
{   uint16_t cell = 0;
    spi_device_acquire_bus(ePaperSPI, portMAX_DELAY);

#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "txing imgdata");
#endif

    for(cell = 0; cell < IMG_BIN_TX_BYTES; cell++)
    {
        ePaperWriteData(ePaperSPI, img[cell], false);
    }
#ifdef LOG_EPAPER
    ESP_LOGI(TAG, "imgdata tx done.");
#endif

    spi_device_release_bus(ePaperSPI);
}