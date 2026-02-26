#pragma once
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
#include "esp_lcd_touch_ft5x06.h"
#include "lvgl_page.h"
#include "lcd.h"
#include "time.h"
#include "temp.h"
#include "wifi.h"

void example_lvgl_demo_ui();
bool page_unlock();
void page_main();
void lvgl_clear_screen(bool clear_lvgl,bool clear_screen);
void create_lock_img();
void page_weather();
void page_temp();
void page_set();
void page_wifi_set();
void page_wifi_connect(uint8_t num);
