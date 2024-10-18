/*
 * esp_wifi_manager.c
 *
 *  Created on: 31-Jan-2024
 *      Author: Yash
 */

#include "esp_wifi_manager.h"
#include "esp_config_app.h"

/************ DEFINES *************/
#define TAG "WiMan"	// short for WiFi Manager

/************ VARIABLES ***********/
static esp_netif_t* ESPStaObj = NULL;
static esp_netif_t* EspAPObj = NULL;

esp_wifi_settings_t WiFiSettings = {
	.ap_ssid = AP_DEFAULT_NAME,
	.ap_password = AP_DEFAULT_PASSWORD,
	.ap_hidden = AP_SSID_VISIBLE,
	.ap_channel = AP_DEFAULT_CHANNEL,
	.ap_bandwidth = AP_DEFAULT_BANDWIDTH,
	.sta_only = WIFI_STA_ONLY_MODE,
	.sta_power_save = WIFI_PS_NONE,
	.is_static_ip = 0,
};

wifi_manager_advrtse_t AdvertiseMsg = { WIFI_MAN_ADVERTISE_FUNC,
										WIFI_MAN_DEFAULT_DEV_TYPE,
										WIFI_MAN_VERSION,
										WIFI_MAN_ESP_WIFI_MODE, 0 }; 

wifi_config_t* WiFiManApStaConfig;
const char DefaultStationSSID[SSID_MAX_LEN] = MY_WIFI_DEFAULT_STA;
const char DefaultStationPass[PASSWORD_MAX_LEN] = MY_WIFI_DEFAULT_PASS;
const char* DefaultHostName = "ESP-PROTO_DEV";

wifi_config_t* GetStationDefaultConfig()
{
	if(NULL == WiFiManApStaConfig)
	{
		WiFiManApStaConfig = (wifi_config_t*)malloc(sizeof(wifi_config_t));
	}

	memset(WiFiManApStaConfig, 0x0, sizeof(wifi_config_t));

	memcpy(WiFiManApStaConfig->sta.ssid, DefaultStationSSID, SSID_MAX_LEN);
	memcpy(WiFiManApStaConfig->sta.password, DefaultStationPass, PASSWORD_MAX_LEN);

	return WiFiManApStaConfig;
}

uint8_t* GetWifiManagerAdvertPkt()
{
	return (uint8_t*) &AdvertiseMsg;
}

static void WiFiManagerHeartbeatTask(void* pvParameters)
{
	//char rx_buffer[128];
    char host_ip[] = "255.255.255.255";
    int addr_family = 0;
    int ip_protocol = 0;

	struct sockaddr_in dest_addr;
	dest_addr.sin_addr.s_addr = inet_addr(host_ip);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(WIFI_MAN_HEARTBEAT_BROADCAST_PORT);
	addr_family = AF_INET;
	ip_protocol = IPPROTO_IP;

	int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
	if (sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
	}
	
	// Set timeout
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	setsockopt (sock, SOL_SOCKET, SO_BROADCAST, &timeout, sizeof timeout);

	ESP_LOGI(TAG, "Socket created, sending to %s:%d", host_ip, WIFI_MAN_HEARTBEAT_BROADCAST_PORT);
	
	uint8_t mac[MAC_LENGTH];
	esp_read_mac(mac, ESP_MAC_EFUSE_FACTORY);
	memcpy(&AdvertiseMsg.device_id, mac, sizeof(uint32_t));

	while(1)
	{
		int err = sendto(sock, GetWifiManagerAdvertPkt(), WIFI_MAN_ADVERT_MSG_LEN, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
		if (err < 0) {
			ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
			break;
		}
		ESP_LOGI(TAG, "Message sent");

		vTaskDelay(pdMS_TO_TICKS(HEARTBEAT_BROADCAST_INTERVAL_MS));
	}

	// should not reach here
	vTaskDelete(NULL);
}

/*
*	WIFI_EVENT_WIFI_READY - will never be generated:
	https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/wifi.html
*/
static void WiFiManagerEventHandler(void* arg, esp_event_base_t event_base,
										int32_t event_id, void* event_data)
{
	if (WIFI_EVENT == event_base)
	{
		ESP_LOGD(TAG, "WiFi Event Detected");
	}
	else if (IP_EVENT == event_base)
	{
		ESP_LOGD(TAG, "IP Event Detected");
	}
	else
	{
		ESP_LOGE(TAG, "Unknown event type");
	}
}

esp_err_t Start_WiFi_Manager()
{
	esp_err_t ret = ESP_OK;
	size_t sz;

	// init non-volatile storage
	nvs_flash_init();

	// initialize esp32 tcp/ip LwIP stack
	if(ESP_OK != esp_netif_init())
	{
		ESP_LOGE(TAG, "LwIP Stack Failed");
	}

	if(ESP_OK != esp_event_loop_create_default())
	{
		ESP_LOGE(TAG, "WiFi Event Loop Creation Failed");
	}

	ESPStaObj = esp_netif_create_default_wifi_sta();
	EspAPObj = esp_netif_create_default_wifi_ap();

	esp_netif_set_hostname(ESPStaObj, DefaultHostName);

	wifi_init_config_t wifiInitConfig = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifiInitConfig));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	/* setup event handlers */
	esp_event_handler_instance_t wifiEventObj;
	esp_event_handler_instance_t ipEventObj;

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
										&WiFiManagerEventHandler, NULL, &wifiEventObj));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID,
										&WiFiManagerEventHandler, NULL, &ipEventObj));

	wifi_config_t apConfig = 
	{
		.ap = {
			.ssid_len = 0,
			.channel = WiFiSettings.ap_channel,
			.ssid_hidden = WiFiSettings.ap_hidden,
			.max_connection = AP_DEFAULT_MAX_CONNECTIONS,
			.beacon_interval = AP_BEACON_DEFAULT_INTERVAL
		},
	};

	memcpy(apConfig.ap.ssid, WiFiSettings.ap_ssid , sizeof(WiFiSettings.ap_ssid));

	if(strlen( (char*)WiFiSettings.ap_password) < WPA2_MINIMUM_PASSWORD_LENGTH)
	{
		apConfig.ap.authmode = WIFI_AUTH_OPEN;
		memset( apConfig.ap.password, 0x00, sizeof(apConfig.ap.password) );
	}
	else
	{
		apConfig.ap.authmode = WIFI_AUTH_WPA2_PSK;
		memcpy(apConfig.ap.password, WiFiSettings.ap_password, sizeof(WiFiSettings.ap_password));
	}

	//
	// init netif for AP
	//
	esp_netif_dhcps_stop(EspAPObj);
	esp_netif_ip_info_t apipInfo;
	memset(&apipInfo, 0x00, sizeof(apipInfo));
	inet_pton(AF_INET, AP_DEFAULT_IP, &apipInfo.ip);
	inet_pton(AF_INET, AP_DEFAULT_GATEWAY, &apipInfo.gw);
	inet_pton(AF_INET, AP_DEFAULT_NETMASK, &apipInfo.netmask);
	ESP_ERROR_CHECK(esp_netif_set_ip_info(EspAPObj, &apipInfo));
	ESP_ERROR_CHECK(esp_netif_dhcps_start(EspAPObj));

	//
	//	init ESP32 Radio as AP + STA mode
	//
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &apConfig));
	ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WiFiSettings.ap_bandwidth));
	ESP_ERROR_CHECK(esp_wifi_set_ps(WiFiSettings.sta_power_save));

	// IMPORTANT*** TODO by default to be started in STA mode for WiFi Manager purpose
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, GetStationDefaultConfig()));
	ESP_ERROR_CHECK(esp_wifi_connect());

	vTaskDelay(pdMS_TO_TICKS(5000));

	// xTaskCreate(WiFiManagerHeartbeatTask, "Heartbeat", 2*configMINIMAL_STACK_SIZE, NULL, 5, NULL);

	// StartConfigApp(false);

	return ret;
}
