/*
 * wifi.c - part of ice-v_wifimgr. Adds TCP socket for updating FPGA bitstream
 * 06-21-22 E. Brombaugh
 */

#include <string.h>
#include "wifi.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "rom/crc.h"
#include "ice.h"
#include "spiffs.h"
#include "phy.h"
#include "adc_c3.h"
#include "wifi_manager.h"
#include "socket.h"
#include "mdns.h"

static const char *TAG = "wifi";

/******************************************************************************/
/* API                                                                        */
/******************************************************************************/
/* wifi connection state */
uint8_t wifi_connected = 0;

/**
 * @brief things we do when the connection comes up.
 */
void cb_connected(void *pvParameter){
	ip_event_got_ip_t* param = (ip_event_got_ip_t*)pvParameter;

	/* transform IP to human readable string */
	char str_ip[16];
	esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

	ESP_LOGI(TAG, "Connected - IP = %s", str_ip);
	
	wifi_connected = 1;
}

/**
 * @brief things we do when the connection drops.
 */
void cb_disconnected(void *pvParameter){
	ESP_LOGI(TAG, "Disconnected");
	
	wifi_connected = 0;
}

/*
 * initialize the WiFi manager and set up a socket
 */
esp_err_t wifi_init(void)
{
	esp_err_t ret = ESP_OK;
		
	/* prevent USB shutdown */
	ESP_LOGI(TAG, "Preventing USB disable.");
	phy_bbpll_en_usb(true);

	/* start the wifi manager */
	ESP_LOGI(TAG, "Starting WiFi Manager.");
	wifi_manager_start();

	/* register callback for the connection status */
	/* Note - for some reason the log prints don't work here */
	ESP_LOGI(TAG, "Registering Callbacks.");
	wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &cb_connected);
	wifi_manager_set_callback(WM_EVENT_STA_DISCONNECTED, &cb_disconnected);
	ESP_LOGI(TAG, "Registered Callbacks.");
	
	/* wait for connection */
	ESP_LOGI(TAG, "Waiting for Connection.");
	while(!wifi_connected)
	{
		vTaskDelay(1);
	}
	
#if 1
	/* initialize mDNS service */
    ESP_ERROR_CHECK( mdns_init() );
	ESP_ERROR_CHECK( mdns_hostname_set("ICE-V") );
	ESP_ERROR_CHECK( mdns_instance_name_set("ESP32C3 + FPGA") );
    ESP_ERROR_CHECK( mdns_service_add(NULL, "_FPGA", "_tcp", 3333, NULL, 0)  );
#endif
	
	/* whatever else you want running on top of WiFi */
	ESP_LOGI(TAG, "Setting up TCP socket server.");
	xTaskCreate(socket_task, "socket", 4096, (void*)AF_INET, 5, NULL);

	return ret;
}

/*
 * get RSSI
 */
int8_t wifi_get_rssi(void)
{
	wifi_ap_record_t ap;
	esp_wifi_sta_get_ap_info(&ap);
	return ap.rssi;
}