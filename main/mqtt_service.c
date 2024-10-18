#include "includes.h"

#define TAG "mqtt"
#define TASK_QUEUE_LEN  5

// #define DEBUG_MQTT_SVC

uint8_t Ready2Subscribe = 0;
uint8_t ImageDataBuffer[IMG_BIN_TX_BYTES] = { 0 };
QueueHandle_t mqtt2paper;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id = 0;
    epaper_cmd_t rx_cmd;

    uint8_t* dataptr = (uint8_t*)event->data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
#ifdef DEBUG_MQTT_SVC
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        ESP_LOGI(TAG, "subscribing...");
#endif
        Ready2Subscribe = 1;
        break;
    case MQTT_EVENT_DISCONNECTED:
#ifdef DEBUG_MQTT_SVC
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
#endif
        break;

    case MQTT_EVENT_SUBSCRIBED:
#ifdef DEBUG_MQTT_SVC
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
#endif
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
#ifdef DEBUG_MQTT_SVC
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
#endif
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
#ifdef DEBUG_MQTT_SVC
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
#endif
        break;
    case MQTT_EVENT_PUBLISHED:
#ifdef DEBUG_MQTT_SVC
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
#endif
        break;
    case MQTT_EVENT_DATA:
        rx_cmd.sof = dataptr[EPAPER_SOF_INDX];
        rx_cmd.cmd = dataptr[EPAPER_CMD_INDX];
        rx_cmd.data_seq_num = (dataptr[2] << 8 | dataptr[3]);
        if(rx_cmd.cmd == EPAPER_TX_IMG)
        {
            memcpy(ImageDataBuffer, &dataptr[EPAPER_DATA_INDX], IMG_BIN_TX_BYTES);
        }
        xQueueSend(mqtt2paper, &rx_cmd, pdMS_TO_TICKS(10));

#ifdef DEBUG_MQTT_SVC
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
#endif
        break;
    case MQTT_EVENT_ERROR:
#ifdef DEBUG_MQTT_SVC
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
#endif
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}


void vTaskMqtt2Epaper(void* pvParameters)
{
    epaper_cmd_t cmd;
    int msg_id = 0;
    uint8_t opr_success = 0;
    esp_mqtt_client_config_t mqtt_cfg = 
    {
        .broker.address.uri = "mqtt://test.mosquitto.org",
        .broker.address.port = 1883,
    };  
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    
    mqtt2paper = xQueueCreate(TASK_QUEUE_LEN, sizeof(epaper_cmd_t));

    while(1)
    {
        if(Ready2Subscribe)
        {
            msg_id = esp_mqtt_client_subscribe(client, "esp32EpaperDiscount/test", 0);
            ESP_LOGI(TAG, "connected to broker and now subscribing");
            Ready2Subscribe = 0;
        }

        if(xQueueReceive(mqtt2paper, &cmd, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            ESP_LOGI(TAG, "MQTT_EVENT_DATA %x %x %x", cmd.sof, cmd.cmd, cmd.data_seq_num);
            switch(cmd.cmd)
            {
                case EPAPER_INIT:
                    ePaperInit();
                    cmd.cmd_exec_flag = EPAPER_INIT;
                    opr_success = 1;
                break;

                case EPAPER_CLEAR:
                    ePaperClear();
                    cmd.cmd_exec_flag = EPAPER_CLEAR;
                    opr_success = 1;
                break;

                case EPAPER_REFRESH:
                    ePaperRefresh();
                    cmd.cmd_exec_flag = EPAPER_INIT_IMG;
                    opr_success = 1;
                break;

                case EPAPER_SLEEP:
                    ePaperSleep();
                    cmd.cmd_exec_flag = EPAPER_INIT_IMG;
                    opr_success = 1;
                break;

                case EPAPER_INIT_IMG:
                    ePaperStartImgTxMode();
                    cmd.cmd_exec_flag = EPAPER_INIT_IMG;
                    opr_success = 1;
                break;

                case EPAPER_TX_IMG:
                    ePaperTxImg(ImageDataBuffer);
                    cmd.cmd_exec_flag = EPAPER_TX_IMG;
                    opr_success = 1;
                break;

                case EPAPER_TEST:
                cmd.cmd_exec_flag = EPAPER_TEST;
                opr_success = 1;
                break;
                
                default:
                break;
            }

            if(opr_success)
            {
                msg_id = esp_mqtt_client_publish(client, "esp32EpaperDiscount/testAck",
                (const char*)&cmd, sizeof(epaper_cmd_t), 0, 0);
                opr_success = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void mqtt_app_start()
{
    xTaskCreate(vTaskMqtt2Epaper, "mqpaper", 4*configMINIMAL_STACK_SIZE, NULL,
    5, NULL);
}