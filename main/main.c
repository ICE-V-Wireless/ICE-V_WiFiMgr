/*
 * main.c - top level
 * part of ICE-V_WiFiMgr
 * 06-21-22 E. Brombaugh
*/

#include "main.h"
#include "ice.h"
#include "driver/gpio.h"
#include "rom/crc.h"
#include "spiffs.h"
#include "wifi.h"
#include "adc_c3.h"
#include "phy.h"
#include <esp_wifi.h>
#include <esp_netif.h>
#include "wifi_manager.h"

#define BOOT_PIN 9
#define LED_PIN 10

static const char* TAG = "main";

/* build version in simple format */
const char *fwVersionStr = "V0.1";
const char *cfg_file = "/spiffs/bitstream.bin";

/* build time */
const char *bdate = __DATE__;
const char *btime = __TIME__;

/*
 * Boot checker
 */
void boot_task(void *pvParameters)
{
	uint8_t boot_state, curr_boot_state;
	uint32_t boot_time = 0;
	
	/* set up BOOT button GPIO */
	gpio_set_direction(BOOT_PIN, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BOOT_PIN, GPIO_PULLUP_ONLY);
	
	/* wait for boot button released */
	vTaskDelay(1);
	while((boot_state = gpio_get_level(BOOT_PIN))==0)
	{
		vTaskDelay(1);
	}
	
	/* loop while checking */
	while(1)
	{
		/* check for changes */
		curr_boot_state = gpio_get_level(BOOT_PIN);
		if(curr_boot_state != boot_state)
		{
			boot_state = curr_boot_state;
			boot_time = 0;
			ESP_LOGI(TAG, "Boot State: %d", boot_state);
		}
		
		/* check for 3 second press */
		if((boot_state == 0) && (boot_time > 3000))
		{
			ESP_LOGI(TAG, "Boot Button Held for >= 3sec: Clearing WiFi credentials.");
			boot_time = 0;
			wifi_reset_credentials();
			
			ESP_LOGI(TAG, "Restarting...");
			esp_restart();
		}
		
		/* count time */
		boot_time += portTICK_PERIOD_MS;
		vTaskDelay(1);
	}
}

/*
 * Main!
 */
void app_main(void)
{
	/* Startup */
    ESP_LOGI(TAG, "-----------------------------");
    ESP_LOGI(TAG, "ICE-V WiFimgr starting...");
    ESP_LOGI(TAG, "Version: %s", fwVersionStr);
    ESP_LOGI(TAG, "Build Date: %s", bdate);
    ESP_LOGI(TAG, "Build Time: %s", btime);

    ESP_LOGI(TAG, "Initializing SPIFFS");
	spiffs_init();

	/* init FPGA SPI port */
	ICE_Init();
    ESP_LOGI(TAG, "FPGA SPI port initialized");
	
	/* configure FPGA from SPIFFS file */
    ESP_LOGI(TAG, "Reading file %s", cfg_file);
	uint8_t *bin = NULL;
	uint32_t sz;
	if(!spiffs_read((char *)cfg_file, &bin, &sz))
	{
		uint8_t cfg_stat;
		
		/* loop on config failure */
		while((cfg_stat = ICE_FPGA_Config(bin, sz)))
			ESP_LOGW(TAG, "FPGA configured ERROR - status = %d", cfg_stat);
		ESP_LOGI(TAG, "FPGA configured OK - status = %d", cfg_stat);
		free(bin);
	}

    /* init ADC for Vbat readings */
    if(!adc_c3_init())
        ESP_LOGI(TAG, "ADC Initialized");
    else
        ESP_LOGW(TAG, "ADC Init Failed");
    
	/* init WiFi & socket */
	if(!wifi_init())
		ESP_LOGI(TAG, "WiFi Running");
	else
		ESP_LOGE(TAG, "WiFi Init Failed");
	
	/* start checking BOOT pin for 3 sec push */
	xTaskCreate(boot_task, "boot", 2048, NULL, 4, NULL);
	
	/* wait here forever blinking */
    ESP_LOGI(TAG, "Looping...");
	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
	int8_t i=0;
	while(1)
	{
		gpio_set_level(LED_PIN, (i++)&1);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
