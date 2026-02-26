#pragma once
#include <stdio.h>
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_page.h"
#include "wifi.h"
#include "lcd.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi_default.h"
#include <string.h>
#include "esp_sntp.h"    
#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_tls.h"


//天气的数据结构体
typedef struct
{
    char weather[10];
    int8_t high_temperature;
    int8_t low_temperature;
}weather_t;


void wifi_init(char *wifi_ssid,char *wifi_passwd);
void wifi_scan();
void get_weather_init();
