/*
 * wifi.h - part of ice-v_wifimgr. Adds TCP socket for updating FPGA bitstream
 * 06-21-22 E. Brombaugh
 */

#ifndef __WIFI__
#define __WIFI__

#include "main.h"

esp_err_t wifi_init(void);
int8_t wifi_get_rssi(void);
void wifi_reset_credentials(void);

#endif
