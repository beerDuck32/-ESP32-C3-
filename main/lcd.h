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
#include "esp_lcd_touch.h"
#include "key.h"
#include "driver/timer.h"
#include "esp_attr.h"  // 包含 IRAM_ATTR
#include "freertos/FreeRTOS.h"
#include "wife.h"

//event
typedef enum{
    EVENT_NO_TOUCH,
    EVENT_TOUCH_CLICK,    // 点击事件
}event_type_t;

typedef enum{
    NanNing=0,
    ShenZhen,    
    GuangZhou 
}weather_city;

typedef enum {
    PAGE_NO,
    PAGE_UNLOCK,  //解锁界面
    PAGE_MAIN,    // 主界面
    PAGE_WEATHER,  //天气界面
    PAGE_TEMP,      //温湿度
    PAGE_SET,        //设置
    PAGE_WIFI_SET,
    PAGE_WIFI_CONNCET
} page_type_t;

typedef enum{
    SET_UP_TOLOCK,
    SET_NUM_LOCK,
}set_lock;

typedef struct 
{
    event_type_t event;
    page_type_t page;
    set_lock set;
    uint8_t touch_x;  //触摸的x轴
    uint16_t touch_y;  //触摸的y轴
    weather_city city;
    
}type_struct;


// Using SPI2 in the example
#define LCD_HOST  SPI2_HOST

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  0
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_SCLK           3
#define EXAMPLE_PIN_NUM_MOSI           5
#define EXAMPLE_PIN_NUM_MISO           -1
#define EXAMPLE_PIN_NUM_LCD_DC         6
#define EXAMPLE_PIN_NUM_LCD_RST        -1
#define EXAMPLE_PIN_NUM_LCD_CS         4
#define EXAMPLE_PIN_NUM_BK_LIGHT       2
#define EXAMPLE_PIN_NUM_TOUCH_CS       -1

// The pixel number in horizontal and vertical
#if CONFIG_EXAMPLE_LCD_CONTROLLER_ILI9341
#define EXAMPLE_LCD_H_RES              240
#define EXAMPLE_LCD_V_RES              320
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
#define EXAMPLE_LCD_H_RES              240
#define EXAMPLE_LCD_V_RES              240
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_ST7789
#define EXAMPLE_LCD_H_RES              240
#define EXAMPLE_LCD_V_RES              320
#endif
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

#define EXAMPLE_LVGL_TICK_PERIOD_MS    2


#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
#endif


void Init_lcd(void);
void init_time();
void create_binary_semaphore(void);