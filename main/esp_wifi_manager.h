/*
 * esp_wifi_manager.h
 *
 *  Created on: 31-Jan-2024
 *      Author: Yash
 */

#ifndef MAIN_ESP_WIFI_MANAGER_H_
#define MAIN_ESP_WIFI_MANAGER_H_

#include <stdio.h>
#include <string.h>

#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/ip4_addr.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <esp_mac.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <freertos/event_groups.h>
#include "FreeRTOSConfig.h"

#include "driver/gpio.h"
#include "driver/i2c.h"

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_types.h"

#include "esp_log.h"
#include "esp_system.h"

#include "esp_wifi.h"
#include <esp_wifi_types.h>
#include "esp_wifi_default.h"
#include <nvs_flash.h>
#include "sdkconfig.h"


#define MY_WIFI_DEFAULT_STA		"MyWiFI"
#define MY_WIFI_DEFAULT_PASS	"MyWiFI1234"

/*
 * DO NOT MODIFY BELOW VALUES
 * Values are defined as per wifi_config_t
 * */
#define SSID_MAX_LEN		32
#define PASSWORD_MAX_LEN	64

#define AP_DEFAULT_IP		"192.168.0.1"
#define AP_DEFAULT_GATEWAY	"192.168.0.1"
#define AP_DEFAULT_NETMASK	"255.255.255.0"

/*
 * Maximum number of available APs to be scanned
 * */
#define MAX_AP_SCAN_COUNT	5

#define WPA2_MINIMUM_PASSWORD_LENGTH 8

#define AP_DEFAULT_NAME	"espWiMan"
#define AP_DEFAULT_PASSWORD "espWiMan123"

#define AP_BEACON_DEFAULT_INTERVAL	100

#define AP_SSID_VISIBLE	0
#define AP_SSID_HIDDEN	1

#define AP_DEFAULT_CHANNEL	6
#define AP_DEFAULT_BANDWIDTH	WIFI_BW_HT20

#define WIFI_STA_ONLY_MODE	0

#define AP_DEFAULT_MAX_CONNECTIONS	2	

#define WIFI_MAN_HEARTBEAT_BROADCAST_PORT	6969
#define HEARTBEAT_BROADCAST_INTERVAL_MS		5000

#define MAC_LENGTH	6

//
//	WiFi Manager Advertisement
//
#define WIFI_MAN_ADVERTISE_FUNC		0xFF
#define WIFI_MAN_DEFAULT_DEV_TYPE	0xED
#define WIFI_MAN_VERSION			0U
#define WIFI_MAN_ESP_WIFI_MODE		0x1

typedef struct ESP_WIFI_MAN_UDP_ADVERTISE_FORMAT
{
	uint8_t func_num;		// 0xff for advertise
	uint8_t device_type;	// device type - different device types TBD
	uint8_t wifi_man_ver;	// wifi manager version
	uint8_t esp_wifi_mode;	// 0 - STA	1 - STA+AP	2 - AP
	uint32_t device_id;
}
wifi_manager_advrtse_t;
#define WIFI_MAN_ADVERT_MSG_LEN		sizeof(wifi_manager_advrtse_t)


//
//	wifi settings
//
typedef struct ESP_WIFI_MAN_SETTINGS
{
	uint8_t ap_ssid[SSID_MAX_LEN];
	uint8_t ap_password[PASSWORD_MAX_LEN];
	uint8_t ap_channel;
	uint8_t ap_hidden;
	wifi_bandwidth_t ap_bandwidth;

	uint8_t is_static_ip;
	esp_netif_ip_info_t static_ip_info;

	bool sta_only;
	wifi_ps_type_t sta_power_save;
}
esp_wifi_settings_t;

//
//	gllobal vairables
//
extern esp_wifi_settings_t wifi_settings;

//
//	global functions
//
extern esp_err_t Start_WiFi_Manager();

#endif /* MAIN_ESP_WIFI_MANAGER_H_ */
