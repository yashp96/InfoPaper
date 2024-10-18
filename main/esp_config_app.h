#ifndef _ESP_CONFIG_APP_H
#define _ESP_CONFIG_APP_H

#include <stdbool.h>
#include <esp_http_server.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include "esp_netif.h"
#include <esp_http_server.h>

/** @brief base url of configuration app on webserver.
    basicaly location of index.html
*/
#define CONFIG_APP_LOCATION "/"

/** @brief launch web server for Configuration App
*/
void StartConfigApp(bool lru_purge_enable);

/** @brief stop web server for configuration app
*/
void StopConfigApp();

/** 
 * @brief sets a hook into the wifi manager URI handlers. Setting the handler to NULL disables the hook.
 * @return ESP_OK in case of success, ESP_ERR_INVALID_ARG if the method is unsupported.
 */
esp_err_t ConfigAppSetHandlerHook(httpd_method_t method,  esp_err_t (*handler)(httpd_req_t *r));

#endif