/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "myi2c.h"
#include "lcd.h"
#include "esp_lcd_touch_ft5x06.h"
#include "key.h"
#include "temp.h"
#include "wifi.h"

//声明标志位
extern bool switch_city_flag;
extern bool wifi_open;
extern SemaphoreHandle_t temp_flag;
//声明外部变量
extern type_struct all_type;

void read_gxhtc3_task(void *arg)
{
    while(1)
    {
        xSemaphoreTake(temp_flag, portMAX_DELAY);//等待信号量
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        gxhtc3_main();
        size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
        printf("剩余内存:%d\n",free_heap);
        

    }
}

void lvgl_show_task(void *arg)
{
    while (1)
    {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();//执行各种回调函数
    }
}

void get_weather_task(void *arg)//切换城市
{
    while(1)
    {
        if(switch_city_flag==1)
        {
            printf("获取天气中\n");
            get_weather_init();
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void wifi_scan_task(void *arg)
{
    while(1)
    {
        if(all_type.page==PAGE_WIFI_SET&&wifi_open==1)
        {
            printf("扫描wifi\n");
            wifi_scan();
            
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void Init()
{
    //ESP_ERROR_CHECK(i2c_master_init());
    Init_lcd();//iiC初始化也在里
    key_init();
    wifi_init("ChinaNet-Y24f","xkca9eec");
    create_binary_semaphore();
}
void app_main(void)
{
    Init();
    xTaskCreate(lvgl_show_task,"lvgl_show_task",10240,NULL,1,NULL);
    xTaskCreate(read_gxhtc3_task,"read_gxhtc3_task",1800,NULL,5,NULL);
    xTaskCreate(get_weather_task,"get_weather_task",4096,NULL,5,NULL);
    xTaskCreate(wifi_scan_task,"wifi_scan_task",4096,NULL,5,NULL);
}
