/*
 * project: InfoPaper
 * 1. Demonstration of simple library for ePaper using ESP32.
 * 2. Use of MQTT to transmit image from companion application
 * Author - yash
*/

#include "includes.h"

// #define SAMPLE_CODE

#ifdef SAMPLE_CODE
#include "image.h"

void vTaskEpaper(void* pvParameter)
{
	ePaperInit();
	ePaperClear();
	ePaperDisplayImg(gImage_img);
	ePaperSleep();
	vTaskDelete(NULL);
}
#endif

/*************** DEFINES ***************/
#define LED_GPIO	2

#define TAG "APP"

/*************** GLOBAL VARIABLES ***************/


/*************** FUNCTIONS ***************/
static void Setup_System_LED()
{
    gpio_config_t led = {};
    led.pin_bit_mask = (1ULL << LED_GPIO);
    led.mode = GPIO_MODE_OUTPUT;
    led.pull_up_en = true;
    gpio_config(&led);
}

/*************** MAIN APP ***************/
void app_main(void)
{

#ifdef SAMPLE_CODE
	xTaskCreate(vTaskEpaper, "ePaper", 4*configMINIMAL_STACK_SIZE, NULL, 5, NULL);
#else
	Start_WiFi_Manager();
	vTaskDelay(pdMS_TO_TICKS(5000));
	mqtt_app_start();
#endif 	 

    /* Configure GPIO for LED */
	Setup_System_LED();

    while (1)
    {
    	gpio_set_level(LED_GPIO, pdFALSE);
        vTaskDelay(pdMS_TO_TICKS(1000));

        gpio_set_level(LED_GPIO, pdTRUE);
		vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
