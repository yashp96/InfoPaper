#include "esp_config_app.h"

#define TAG "CONFIG APP"

/* @brief the HTTP server handle */
static httpd_handle_t HttpdHandle = NULL;

/* strings holding the URLs of the wifi manager */
static char* HttpRootUrl = NULL;
static char* HttpRedirectUrl = NULL;
static char* HttpJsUrl = NULL;
static char* HttpCssUrl = NULL;
static char* HttpConnectUrl = NULL;
static char* HttpApUrl = NULL;
static char* HttpStatusUrl = NULL;

/* Handler Functions Prototypes*/
static esp_err_t HttpServerGetHandler(httpd_req_t *req);
static esp_err_t HttpServerPostHandler(httpd_req_t *req);
static esp_err_t HttpServerDeleteHandler(httpd_req_t *req);

/* URI wild card for any GET request */
static const httpd_uri_t HttpServerGetRequest = {
    .uri       = "*",
    .method    = HTTP_GET,
    .handler   = HttpServerGetHandler
};

static const httpd_uri_t HttpServerPostRequest = {
	.uri	= "*",
	.method = HTTP_POST,
	.handler = HttpServerPostHandler
};

static const httpd_uri_t HttpServerDeleteRequest = {
	.uri	= "*",
	.method = HTTP_DELETE,
	.handler = HttpServerDeleteHandler
};


esp_err_t HttpServerGetHandler(httpd_req_t *req)
{
    return ESP_OK;
}

esp_err_t HttpServerPostHandler(httpd_req_t *req)
{
    return ESP_OK;
}

esp_err_t HttpServerDeleteHandler(httpd_req_t *req)
{
    return ESP_OK;
}

void StartConfigApp(bool lru_purge_enable)
{
    esp_err_t error;

    if(HttpdHandle == NULL)
    {
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        config.uri_match_fn = httpd_uri_match_wildcard;
		config.lru_purge_enable = lru_purge_enable;

        if(HttpRootUrl == NULL)
        {
            int rootLen = strlen(CONFIG_APP_LOCATION);

            // todo: generate links for all the page resources
            // e.g. script.js, style.css, etc.


        }

        error = httpd_start(&HttpdHandle, &config);

        if(error == ESP_OK)
        {
            httpd_register_uri_handler(HttpdHandle, &HttpServerGetRequest);
	        httpd_register_uri_handler(HttpdHandle, &HttpServerPostRequest);
	        httpd_register_uri_handler(HttpdHandle, &HttpServerDeleteRequest);

            ESP_LOGI(TAG, "HTTP Server Started.");
        }

    }
}

void StopConfigApp()
{

}

esp_err_t ConfigAppSetHandlerHook(httpd_method_t method,  esp_err_t (*handler)(httpd_req_t *r))
{
    return ESP_OK;
}