/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

// This demo UI is adapted from LVGL official example: https://docs.lvgl.io/master/widgets/extra/meter.html#simple-meter

#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_page.h"
#include "wifi.h"
//声明图片对象指针
lv_obj_t * obj_img_lock=NULL;
lv_obj_t * obj_img_main=NULL;

lv_obj_t * obj_img_weather=NULL;
lv_obj_t * obj_img_temp=NULL;
lv_obj_t * obj_img_set=NULL;
lv_obj_t * obj_img_inlock=NULL;
lv_obj_t * obj_img_wifi=NULL;

//声明文字对象指针
lv_obj_t * obj_font_top=NULL;
lv_obj_t * obj_font_bottom=NULL;
lv_obj_t * obj_font_mid=NULL;
lv_obj_t * obj_font_mid_sec=NULL;
//天气文字对象
lv_obj_t * obj_weather_font_top=NULL;
lv_obj_t * obj_weather_font_top1=NULL;
lv_obj_t * obj_weather_font_mid=NULL;
lv_obj_t * obj_weather_font_mid_low=NULL;
lv_obj_t * obj_weather_font_bottom1=NULL;
lv_obj_t * obj_weather_font_bottom2=NULL;
lv_obj_t * obj_weather_font_bottom3=NULL;
lv_obj_t * obj_weather_font_switch=NULL;


//温湿度文字对象
lv_obj_t * obj_cb_font=NULL;
lv_obj_t * obj_humidity_font=NULL;
lv_obj_t * obj_temp_font=NULL;

//设置页面文字对象
lv_obj_t * obj_set_wifi_font=NULL;
lv_obj_t * obj_set_pwm_font=NULL;

//wifi设置对象
lv_obj_t * obj_wifi_state_font=NULL;
lv_obj_t * obj_wifi_state1_font=NULL;
lv_obj_t * obj_wifi_top_font=NULL;
lv_obj_t * obj_wifi_top1_font=NULL;
lv_obj_t * obj_wifi_name1_font=NULL;
lv_obj_t * obj_wifi_name2_font=NULL;
lv_obj_t * obj_wifi_name3_font=NULL;

lv_obj_t * obj_wifi_passwd_font=NULL;
lv_obj_t * obj_wifi_passwd_font1=NULL;
lv_obj_t * obj_wifi_passwd_font2=NULL;
lv_obj_t * obj_wifi_passwd_font3=NULL;
lv_obj_t * obj_wifi_passwd_font4=NULL;

//声明外部变量
extern type_struct all_type;
extern struct tm timeinfo;
extern weather_t today_weather_data;
extern weather_t tomorrow_weather_data;
extern weather_t after_tomorrow_weather_data;
extern float temp;
extern float humidity;
extern wifi_ap_record_t wifi_list[10];
extern char wifi_connected_name[30];
//声明标志位
extern bool get_time_flag;
bool first_get_time_flag=false;
extern bool switch_city_flag;
extern bool wifi_connected_flag;
bool wifi_open=1;
bool wifi_big_write=0;
extern uint16_t ap_count;
//创建全局变量
uint8_t hour=0;
uint8_t min=0;
uint8_t sec=0;
char passwd[32];

//声明函数
void page_main();
/**
 * @brief  清屏
 * @param  clear_lvgl: 是否清除LVGL的对象
 * @param  clear_screen: 是否关闭屏幕
 */
void lvgl_clear_screen(bool clear_lvgl,bool clear_screen)
{
    if(clear_screen)
    {
        gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, 1);//灭屏
    }
    if(clear_lvgl)
    {
        lv_obj_clean(lv_scr_act()); // 清空屏幕所有LVGL对象
        lv_refr_now(lv_disp_get_default()); // 立即刷新
        obj_img_lock=NULL;
        obj_img_main=NULL;
        obj_img_weather=NULL;
        obj_img_temp=NULL;
        obj_img_set=NULL;
        obj_img_inlock=NULL;
        obj_img_wifi=NULL;

        obj_font_top=NULL;
        obj_font_bottom=NULL;
        obj_font_mid=NULL;
        obj_font_mid_sec=NULL;

        obj_weather_font_top=NULL;
        obj_weather_font_top1=NULL;
        obj_weather_font_mid=NULL;
        obj_weather_font_mid_low=NULL;
        obj_weather_font_bottom1=NULL;
        obj_weather_font_bottom2=NULL;
        obj_weather_font_bottom3=NULL;
        obj_weather_font_switch=NULL;

        obj_cb_font=NULL;
        obj_humidity_font=NULL;
        obj_temp_font=NULL;

        obj_set_pwm_font=NULL;
        obj_set_wifi_font=NULL;
        obj_wifi_state_font=NULL;
        obj_wifi_state1_font=NULL;
        obj_wifi_top_font=NULL;
        obj_wifi_top1_font=NULL;
        obj_wifi_name1_font=NULL;
        obj_wifi_name2_font=NULL;
        obj_wifi_name3_font=NULL;

        obj_wifi_passwd_font=NULL;
        obj_wifi_passwd_font1=NULL;
        obj_wifi_passwd_font2=NULL;
        obj_wifi_passwd_font3=NULL;
        obj_wifi_passwd_font4=NULL;
    }
    
    
}


bool create_img(lv_obj_t **img, const void *img_src)
{
    // 第一步：先校验「指针的指针」和「图片资源」的合法性（入参校验）
    // 顺序：先检查 img 是否为NULL，避免解引用崩溃
    if(img == NULL || img_src == NULL) {
        return false; // 入参无效，直接返回失败
    }

    // 第二步：如果旧对象存在，先删除（避免重复创建造成内存泄漏）
    if(*img != NULL) {
        lv_obj_del(*img);
        *img = NULL;
    }

    // 第三步：创建图片对象并设置属性
    *img = lv_img_create(lv_scr_act()); // 父对象默认设为屏幕，也可改为参数
    if(*img == NULL) {
        return false; // 创建失败
    }
    lv_img_set_src(*img, img_src);
    

    return true; // 创建成功
}


void create_main_text()
{   
    //声明文字资源
    LV_FONT_DECLARE(font_alipuhui);
    LV_FONT_DECLARE(font_Led);
    LV_FONT_DECLARE(font_Led_sec);
    
    
    //底部文字
    if(obj_font_bottom==NULL)
    {   
        obj_font_bottom = lv_label_create(lv_scr_act());
    }
    char font_text[30];
    sprintf(font_text,"%d年%d月%d日",timeinfo.tm_year+1900,(uint8_t)timeinfo.tm_mon+1,(uint8_t)timeinfo.tm_mday);
    lv_label_set_text(obj_font_bottom,font_text);
    lv_obj_set_style_text_font(obj_font_bottom, &font_alipuhui, 0); 
    lv_obj_align(obj_font_bottom, LV_ALIGN_BOTTOM_LEFT, 0, -180); // 先回正中心，避免偏移出屏

    //中间文字
    if(obj_font_mid==NULL)
    {
        obj_font_mid=lv_label_create(lv_scr_act());
        obj_font_mid_sec=lv_label_create(lv_scr_act());
    }
    //时分文字
    if(hour>=10&&min>=10)
    {
        sprintf(font_text,"%d%d",hour,min);
        lv_label_set_text(obj_font_mid,font_text);
        lv_obj_set_style_text_font(obj_font_mid, &font_Led, 0);
        lv_obj_align(obj_font_mid, LV_ALIGN_TOP_LEFT, 0, 40);
    }
    else if(hour>=10&&min<10)
    {
        sprintf(font_text,"%d0%d",hour,min);
        lv_label_set_text(obj_font_mid,font_text);
        lv_obj_set_style_text_font(obj_font_mid, &font_Led, 0);
        lv_obj_align(obj_font_mid, LV_ALIGN_TOP_LEFT, 0, 40);
    }
    else if(hour<10&&min>=10)
    {
        sprintf(font_text,"0%d%d",hour,min);
        lv_label_set_text(obj_font_mid,font_text);
        lv_obj_set_style_text_font(obj_font_mid, &font_Led, 0);
        lv_obj_align(obj_font_mid, LV_ALIGN_TOP_LEFT, 0, 40);
    }
    else if(hour<10&&min<10)
    {
        sprintf(font_text,"0%d0%d",hour,min);
        lv_label_set_text(obj_font_mid,font_text);
        lv_obj_set_style_text_font(obj_font_mid, &font_Led, 0);
        lv_obj_align(obj_font_mid, LV_ALIGN_TOP_LEFT, 0, 40);
    }

    
    //秒文字
    if(sec>=10)
    {
        sprintf(font_text,"%d",sec);
        lv_label_set_text(obj_font_mid_sec,font_text);
        lv_obj_set_style_text_font(obj_font_mid_sec, &font_Led_sec, 0);
        lv_obj_align(obj_font_mid_sec, LV_ALIGN_TOP_LEFT, 180, 56);
    }
    else if(sec<10)
    {
        sprintf(font_text,"0%d",sec);
        lv_label_set_text(obj_font_mid_sec,font_text);
        lv_obj_set_style_text_font(obj_font_mid_sec, &font_Led_sec, 0);
        lv_obj_align(obj_font_mid_sec, LV_ALIGN_TOP_LEFT, 180, 56);
    }
    


    //顶部文字
    if(obj_font_top==NULL)
    {
        obj_font_top = lv_label_create(lv_scr_act());
    }
    const char* weekdays[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
    sprintf(font_text,"%s%d:%d",weekdays[timeinfo.tm_wday],hour,min);
    lv_label_set_text(obj_font_top,font_text);
    lv_obj_set_style_text_font(obj_font_top, &font_alipuhui, 0);
    lv_obj_align(obj_font_top, LV_ALIGN_TOP_LEFT, 0, 0);

    

}
void create_main_img()
{
    //先声明图片资源
    LV_IMG_DECLARE(img_weather);
    LV_IMG_DECLARE(img_temp);
    LV_IMG_DECLARE(img_main);
    LV_IMG_DECLARE(img_set);
    LV_IMG_DECLARE(img_inlock);

    //壁纸最先创建，让其被覆盖
    create_img(&obj_img_main,&img_main); 

    //其次为应用图标
    create_img(&obj_img_weather,&img_weather);
    create_img(&obj_img_temp,&img_temp);
    create_img(&obj_img_set,&img_set);
    create_img(&obj_img_inlock,&img_inlock);
    

    //调整图标位置
    lv_obj_align(obj_img_weather,LV_ALIGN_BOTTOM_MID,-80,-100);  //天气图标
    lv_obj_align(obj_img_temp,LV_ALIGN_BOTTOM_MID, 0,-104);      //温度图标
    lv_obj_align(obj_img_set,LV_ALIGN_BOTTOM_MID, 81,-104);       //设置图标
    lv_obj_align(obj_img_inlock,LV_ALIGN_BOTTOM_MID, -80,-20);      //上锁图标
   
}

void create_lock_img()
{
    LV_IMG_DECLARE(lock); 
    create_img(&obj_img_lock,&lock);
    lv_obj_align(obj_img_lock, LV_ALIGN_CENTER, 0, 0); 
}

//上划解锁界面  返回值为解锁是否成功
bool start_y_flag=true;
uint16_t start_y=0,end_y=0;
int16_t up_value=0;
bool page_unlock() 
{
    //如果锁屏图片为空，创建锁屏图片
    if(obj_img_lock==NULL)
    {
        LV_IMG_DECLARE(lock); 
        create_img(&obj_img_lock,&lock);
    }

    //如果主页面资源为空，创建主页面资源
    if(obj_img_main==NULL)
    {
        create_main_img();//
    }

    //如果文字资源为空，创建文字资源
    if(get_time_flag==true&&obj_font_top==NULL&&all_type.page==PAGE_MAIN)
    {
        create_main_text();
    }



    if(start_y_flag&&all_type.event==EVENT_TOUCH_CLICK)//记录刚开始时触摸的y轴
    {
        start_y=all_type.touch_y;
        start_y_flag=false;
    }
    
    end_y=all_type.touch_y;//事实更新最后触摸的y轴
    up_value=start_y-end_y;//更新计算差值
    if(all_type.event==EVENT_NO_TOUCH) //放手后更新标志位并且复位图片
    {
        start_y_flag=true; //更新标志位，重新赋值开始值

        if(up_value<-100)    //滑动到一定的距离就解锁成功，返回true
        {
            return true;
        }
        else
        {
            up_value=0;
        }
    }

    if(up_value<=0)//禁止下滑
    {
        lv_obj_align(obj_img_lock, LV_ALIGN_CENTER, 0, up_value);
    }

    return false;
}

void icon_wifi()
{
    LV_IMG_DECLARE(img_wifi);
    LV_IMG_DECLARE(img_wifi_off);
    // if(obj_img_wifi==NULL)
    // {
    //     if(wifi_connected_flag)//连接上
    //     {
    //         lv_img_set_src(&obj_img_wifi,&img_wifi);
    //     }
    //     else
    //     {
    //         lv_img_set_src(&obj_img_wifi,&img_wifi_off);
    //     }
    // }
    if(wifi_connected_flag)
    {
        create_img(&obj_img_wifi,&img_wifi);
    }
    else
    {
        create_img(&obj_img_wifi,&img_wifi_off);
    }
    lv_obj_align(obj_img_wifi,LV_ALIGN_TOP_RIGHT, 0,0);
    
}

void page_main()
{   
    //到了主页面，删除锁屏图片资源
    if(obj_img_lock!=NULL)
    {
        lv_obj_del(obj_img_lock);
        obj_img_lock=NULL;
    }
    icon_wifi();
    //如果主页面资源为空，创建主页面资源
    if(obj_img_main==NULL)
    {
        create_main_img();//
    }
    if(get_time_flag==true&&all_type.page==PAGE_MAIN)
    {
        create_main_text();
    }
    
}


void page_weather()
{

    //声明文字
    LV_FONT_DECLARE(font_alipuhui);
    
    //顶部文字
    if(obj_weather_font_top==NULL)
    {   
        //设置背景
        lv_obj_t * scr = lv_scr_act();
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);   // 纯黑背景
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);              // 不透明

        obj_weather_font_top = lv_label_create(lv_scr_act());
    }
    char font_text[60];
    sprintf(font_text,"返回               切换城市");
    lv_label_set_text(obj_weather_font_top,font_text);
    lv_obj_set_style_text_font(obj_weather_font_top, &font_alipuhui, 0);
    lv_obj_align(obj_weather_font_top, LV_ALIGN_TOP_LEFT, 0, 5);

    if(switch_city_flag==0) //切换完成
    {
        if(obj_weather_font_switch!=NULL)
        {
            lv_obj_del(obj_weather_font_switch);
            obj_weather_font_switch=NULL;
        }

        if(obj_weather_font_top1==NULL)
        {   
            obj_weather_font_top1 = lv_label_create(lv_scr_act());
        }
        if(all_type.city==NanNing)
        {
            sprintf(font_text,"%d月%d日  今天  南宁市",(uint8_t)timeinfo.tm_mon+1,(uint8_t)timeinfo.tm_mday);
        }
        if(all_type.city==ShenZhen)
        {
            sprintf(font_text,"%d月%d日  今天  深圳市",(uint8_t)timeinfo.tm_mon+1,(uint8_t)timeinfo.tm_mday);
        }
        if(all_type.city==GuangZhou)
        {
            sprintf(font_text,"%d月%d日  今天  广州市",(uint8_t)timeinfo.tm_mon+1,(uint8_t)timeinfo.tm_mday);
        }

        lv_label_set_text(obj_weather_font_top1,font_text);
        lv_obj_set_style_text_font(obj_weather_font_top1, &font_alipuhui, 0);
        lv_obj_align(obj_weather_font_top1, LV_ALIGN_TOP_LEFT, 0, 40);

        LV_FONT_DECLARE(font_weather_big);
        //中间大字
        if(obj_weather_font_mid==NULL)
        {
            obj_weather_font_mid=lv_label_create(lv_scr_act());
        }

        sprintf(font_text,"%s",today_weather_data.weather);
        lv_label_set_text(obj_weather_font_mid,font_text);
        lv_obj_set_style_text_font(obj_weather_font_mid, &font_weather_big, 0);
        lv_obj_align(obj_weather_font_mid, LV_ALIGN_TOP_MID, 0, 80);

        //中间小字
        if(obj_weather_font_mid_low==NULL)
        {
            obj_weather_font_mid_low=lv_label_create(lv_scr_act());
        }
        sprintf(font_text,"%d~%d℃",today_weather_data.low_temperature,today_weather_data.high_temperature);
        lv_label_set_text(obj_weather_font_mid_low,font_text);
        lv_obj_set_style_text_font(obj_weather_font_mid_low, &font_alipuhui, 0);
        lv_obj_align(obj_weather_font_mid_low, LV_ALIGN_TOP_MID, 0, 150);

        //明日天气
        if(obj_weather_font_bottom1==NULL)
        {
            obj_weather_font_bottom1=lv_label_create(lv_scr_act());
        }
        if(strlen(tomorrow_weather_data.weather)==3)
        {
            sprintf(font_text,"%d月%d日  %s  %d~%d℃",(uint8_t)timeinfo.tm_mon+1,(uint8_t)timeinfo.tm_mday+1,tomorrow_weather_data.weather,tomorrow_weather_data.low_temperature,tomorrow_weather_data.high_temperature);
        }
        if(strlen(tomorrow_weather_data.weather)==6)
        {
            sprintf(font_text,"%d月%d日%s%d~%d℃",(uint8_t)timeinfo.tm_mon+1,(uint8_t)timeinfo.tm_mday+1,tomorrow_weather_data.weather,tomorrow_weather_data.low_temperature,tomorrow_weather_data.high_temperature);
        }
        lv_label_set_text(obj_weather_font_bottom1,font_text);
        lv_obj_set_style_text_font(obj_weather_font_bottom1, &font_alipuhui, 0);
        lv_obj_align(obj_weather_font_bottom1, LV_ALIGN_TOP_LEFT, 0, 185);

        //后天天气
        if(obj_weather_font_bottom2==NULL)
        {
            obj_weather_font_bottom2=lv_label_create(lv_scr_act());
        }
        if(strlen(after_tomorrow_weather_data.weather)==3)
        {
            sprintf(font_text,"%d月%d日  %s  %d~%d℃",(uint8_t)timeinfo.tm_mon+1,(uint8_t)timeinfo.tm_mday+2,after_tomorrow_weather_data.weather,after_tomorrow_weather_data.low_temperature,after_tomorrow_weather_data.high_temperature);

        }
        if(strlen(after_tomorrow_weather_data.weather)==6)
        {
            sprintf(font_text,"%d月%d日%s%d~%d℃",(uint8_t)timeinfo.tm_mon+1,(uint8_t)timeinfo.tm_mday+2,after_tomorrow_weather_data.weather,after_tomorrow_weather_data.low_temperature,after_tomorrow_weather_data.high_temperature);

        }

        lv_label_set_text(obj_weather_font_bottom2,font_text);
        lv_obj_set_style_text_font(obj_weather_font_bottom2, &font_alipuhui, 0);
        lv_obj_align(obj_weather_font_bottom2, LV_ALIGN_TOP_LEFT, 0, 225);
    }
    if(switch_city_flag==1)//切换中
    {

        if(obj_weather_font_top==NULL)
        {   
            //设置背景
            lv_obj_t * scr = lv_scr_act();
            lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);   // 纯黑背景
            lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);              // 不透明

            obj_weather_font_top = lv_label_create(lv_scr_act());
        }
        char font_text[35];
        sprintf(font_text,"返回               切换城市");
        lv_label_set_text(obj_weather_font_top,font_text);
        lv_obj_set_style_text_font(obj_weather_font_top, &font_alipuhui, 0);
        lv_obj_align(obj_weather_font_top, LV_ALIGN_TOP_LEFT, 0, 5);

        if(obj_weather_font_switch==NULL)
        {   
            obj_weather_font_switch = lv_label_create(lv_scr_act());
        }
        if(all_type.city==NanNing)
        {
            sprintf(font_text,"切换到南宁市~~");
        }
        if(all_type.city==ShenZhen)
        {
            sprintf(font_text,"切换到深圳市~~");
        }
        if(all_type.city==GuangZhou)
        {
            sprintf(font_text,"切换到广州市~~");
        }
        lv_label_set_text(obj_weather_font_switch,font_text);
        lv_obj_set_style_text_font(obj_weather_font_switch, &font_alipuhui, 0);
        lv_obj_align(obj_weather_font_switch, LV_ALIGN_CENTER, 0, -10);
    }
 
}

void page_temp()
{
    LV_FONT_DECLARE(font_alipuhui);
    //温度文字
    if(obj_temp_font==NULL)
    {   
        //设置背景
        lv_obj_t * scr = lv_scr_act();
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);   // 纯黑背景
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);              // 不透明

        obj_temp_font = lv_label_create(lv_scr_act());
    }
    char font_text[50];
    sprintf(font_text,"当前温度:%.1f",temp);
    lv_label_set_text(obj_temp_font,font_text);
    lv_obj_set_style_text_font(obj_temp_font, &font_alipuhui, 0);
    lv_obj_align(obj_temp_font, LV_ALIGN_CENTER, 0, 20);

    //湿度文字
    if(obj_humidity_font==NULL)
    {   
        obj_humidity_font = lv_label_create(lv_scr_act());
    }
    sprintf(font_text,"当前湿度:%.1f",humidity);
    lv_label_set_text(obj_humidity_font,font_text);
    lv_obj_set_style_text_font(obj_humidity_font, &font_alipuhui, 0);
    lv_obj_align(obj_humidity_font, LV_ALIGN_CENTER, 0, -10);

    //返回文字
    if(obj_cb_font==NULL)
    {   
        obj_cb_font = lv_label_create(lv_scr_act());
    }
    sprintf(font_text,"返回");
    lv_label_set_text(obj_cb_font,font_text);
    lv_obj_set_style_text_font(obj_cb_font, &font_alipuhui, 0);
    lv_obj_align(obj_cb_font, LV_ALIGN_TOP_LEFT, 0, 5);

}


void page_set()
{
    LV_FONT_DECLARE(font_alipuhui);
    //返回文字
    if(obj_cb_font==NULL)
    {   
        //设置背景
        lv_obj_t * scr = lv_scr_act();
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);   // 纯黑背景
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);              // 不透明

        obj_cb_font = lv_label_create(lv_scr_act());
    }
    char font_text[20];
    sprintf(font_text,"返回");
    lv_label_set_text(obj_cb_font,font_text);
    lv_obj_set_style_text_font(obj_cb_font, &font_alipuhui, 0);
    lv_obj_align(obj_cb_font, LV_ALIGN_TOP_LEFT, 0, 5);

    if(obj_set_wifi_font==NULL)
    {
        obj_set_wifi_font=lv_label_create(lv_scr_act());
    }
    sprintf(font_text,"WIFI设置");
    lv_label_set_text(obj_set_wifi_font,font_text);
    lv_obj_set_style_text_font(obj_set_wifi_font, &font_alipuhui, 0);
    lv_obj_align(obj_set_wifi_font, LV_ALIGN_CENTER, 0, -50);

    if(obj_set_pwm_font==NULL)
    {
        obj_set_pwm_font=lv_label_create(lv_scr_act());
    }
    sprintf(font_text,"亮度设置");
    lv_label_set_text(obj_set_pwm_font,font_text);
    lv_obj_set_style_text_font(obj_set_pwm_font, &font_alipuhui, 0);
    lv_obj_align(obj_set_pwm_font, LV_ALIGN_CENTER, 0, 50);

}

void page_wifi_set()
{
    LV_FONT_DECLARE(font_alipuhui);
    //返回文字
    if(obj_cb_font==NULL)
    {   
        //设置背景
        lv_obj_t * scr = lv_scr_act();
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);   // 纯黑背景
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);              // 不透明

        obj_cb_font = lv_label_create(lv_scr_act());
    }
    char font_text[50];
    sprintf(font_text,"返回");
    lv_label_set_text(obj_cb_font,font_text);
    lv_obj_set_style_text_font(obj_cb_font, &font_alipuhui, 0);
    lv_obj_align(obj_cb_font, LV_ALIGN_TOP_LEFT, 0, 5);

    if(obj_wifi_top_font==NULL)
    {
        obj_wifi_top_font=lv_label_create(lv_scr_act());
    }
    sprintf(font_text,"WIFI状态:");
    lv_label_set_text(obj_wifi_top_font,font_text);
    lv_obj_set_style_text_font(obj_wifi_top_font, &font_alipuhui, 0);
    lv_obj_align(obj_wifi_top_font, LV_ALIGN_TOP_LEFT, 0, 40);

    if(obj_wifi_state_font==NULL)
    {
        obj_wifi_state_font=lv_label_create(lv_scr_act());
    }
    if(wifi_open==1)
    {
        sprintf(font_text,"已开启      关闭WIFI");
        lv_label_set_text(obj_wifi_state_font,font_text);
        lv_obj_set_style_text_font(obj_wifi_state_font, &font_alipuhui, 0);
        lv_obj_align(obj_wifi_state_font, LV_ALIGN_TOP_LEFT, 0, 70);

        if(obj_wifi_state1_font==NULL)
        {
            obj_wifi_state1_font=lv_label_create(lv_scr_act());
        }
        wifi_config_t wifi_cfg;
        esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg);
        if(wifi_connected_flag)
        {
            sprintf(font_text,"已连接:%s",wifi_cfg.sta.ssid);
        }
        else
        {
            sprintf(font_text,"未连接wifi");
        }
        lv_label_set_text(obj_wifi_state1_font,font_text);
        lv_obj_set_style_text_font(obj_wifi_state1_font, &font_alipuhui, 0);
        lv_obj_align(obj_wifi_state1_font, LV_ALIGN_TOP_LEFT, 0, 100);

        if(obj_wifi_top1_font==NULL)
        {
            obj_wifi_top1_font=lv_label_create(lv_scr_act());
        }
        sprintf(font_text,"可用网络:");
        lv_label_set_text(obj_wifi_top1_font,font_text);
        lv_obj_set_style_text_font(obj_wifi_top1_font, &font_alipuhui, 0);
        lv_obj_align(obj_wifi_top1_font, LV_ALIGN_TOP_LEFT, 0, 140);


        if(obj_wifi_name1_font==NULL)
        {
            obj_wifi_name1_font=lv_label_create(lv_scr_act());
        }
        sprintf(font_text,"%s",wifi_list[0].ssid);
        lv_label_set_text(obj_wifi_name1_font,font_text);
        lv_obj_set_style_text_font(obj_wifi_name1_font, &font_alipuhui, 0);
        lv_obj_align(obj_wifi_name1_font, LV_ALIGN_TOP_LEFT, 0, 180);



        if(obj_wifi_name2_font==NULL)
        {
            obj_wifi_name2_font=lv_label_create(lv_scr_act());
        }

        sprintf(font_text,"%s",wifi_list[1].ssid);
        lv_label_set_text(obj_wifi_name2_font,font_text);
        lv_obj_set_style_text_font(obj_wifi_name2_font, &font_alipuhui, 0);
        lv_obj_align(obj_wifi_name2_font, LV_ALIGN_TOP_LEFT, 0, 220);


        if(obj_wifi_name3_font==NULL)
        {
            obj_wifi_name3_font=lv_label_create(lv_scr_act());
        }
        sprintf(font_text,"%s",wifi_list[2].ssid);
        lv_label_set_text(obj_wifi_name3_font,font_text);
        lv_obj_set_style_text_font(obj_wifi_name3_font, &font_alipuhui, 0);
        lv_obj_align(obj_wifi_name3_font, LV_ALIGN_TOP_LEFT, 0, 250);

    }
    else
    {
        sprintf(font_text,"未开启      开启WIFI");
        lv_label_set_text(obj_wifi_state_font,font_text);
        lv_obj_set_style_text_font(obj_wifi_state_font, &font_alipuhui, 0);
        lv_obj_align(obj_wifi_state_font, LV_ALIGN_TOP_LEFT, 0, 70);
    }

}

//num为用户点击的第num个wifi
void page_wifi_connect(uint8_t num)
{
    LV_FONT_DECLARE(font_alipuhui);
    LV_FONT_DECLARE(font_passwd);
    //返回文字
    if(obj_cb_font==NULL)
    {   
        //设置背景
        lv_obj_t * scr = lv_scr_act();
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);   // 纯黑背景
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);              // 不透明

        obj_cb_font = lv_label_create(lv_scr_act());
    }
    char font_text[40];
    sprintf(font_text,"返回                   连接");
    lv_label_set_text(obj_cb_font,font_text);
    lv_obj_set_style_text_font(obj_cb_font, &font_alipuhui, 0);
    lv_obj_align(obj_cb_font, LV_ALIGN_TOP_LEFT, 0, 5);

    if(obj_wifi_top_font==NULL)
    {
        obj_wifi_top_font=lv_label_create(lv_scr_act());
    }
    sprintf(font_text,"连接:%s",wifi_list[num].ssid);
    // printf("%d",num);
    lv_label_set_text(obj_wifi_top_font,font_text);
    lv_obj_set_style_text_font(obj_wifi_top_font, &font_alipuhui, 0);
    lv_obj_align(obj_wifi_top_font, LV_ALIGN_TOP_LEFT, 0, 40);

    if(obj_wifi_top1_font==NULL)
    {
        obj_wifi_top1_font=lv_label_create(lv_scr_act());
    }
    sprintf(font_text,"密码:%s",passwd);
    lv_label_set_text(obj_wifi_top1_font,font_text);
    lv_obj_set_style_text_font(obj_wifi_top1_font, &font_alipuhui, 0);
    lv_obj_align(obj_wifi_top1_font, LV_ALIGN_TOP_LEFT, 0, 70);

    if(obj_wifi_passwd_font==NULL)
    {
        obj_wifi_passwd_font=lv_label_create(lv_scr_act());
    }
    sprintf(font_text,"0123456789");
    lv_label_set_text(obj_wifi_passwd_font,font_text);
    lv_obj_set_style_text_font(obj_wifi_passwd_font, &font_passwd, 0);
    lv_obj_align(obj_wifi_passwd_font, LV_ALIGN_TOP_LEFT, 0, 100);
    if(wifi_big_write==0)
    {
        if(obj_wifi_passwd_font1==NULL)
        {
            obj_wifi_passwd_font1=lv_label_create(lv_scr_act());
        }
        sprintf(font_text," qwertyuiop");
        lv_label_set_text(obj_wifi_passwd_font1,font_text);
        lv_obj_set_style_text_font(obj_wifi_passwd_font1, &font_passwd, 0);
        lv_obj_align(obj_wifi_passwd_font1, LV_ALIGN_TOP_LEFT, 0, 140);
        if(obj_wifi_passwd_font2==NULL)
        {
            obj_wifi_passwd_font2=lv_label_create(lv_scr_act());
        }
        sprintf(font_text,"  asdfghjkl");
        lv_label_set_text(obj_wifi_passwd_font2,font_text);
        lv_obj_set_style_text_font(obj_wifi_passwd_font2, &font_passwd, 0);
        lv_obj_align(obj_wifi_passwd_font2, LV_ALIGN_TOP_LEFT, 0, 180);
        if(obj_wifi_passwd_font3==NULL)
        {
            obj_wifi_passwd_font3=lv_label_create(lv_scr_act());
        }
        sprintf(font_text,"  zxcvbnm");
        lv_label_set_text(obj_wifi_passwd_font3,font_text);
        lv_obj_set_style_text_font(obj_wifi_passwd_font3, &font_passwd, 0);
        lv_obj_align(obj_wifi_passwd_font3, LV_ALIGN_TOP_LEFT, 0, 220);

        if(obj_wifi_passwd_font4==NULL)
        {
            obj_wifi_passwd_font4=lv_label_create(lv_scr_act());
        }
        sprintf(font_text,"切换大小写           退格");
        lv_label_set_text(obj_wifi_passwd_font4,font_text);
        lv_obj_set_style_text_font(obj_wifi_passwd_font4, &font_alipuhui, 0);
        lv_obj_align(obj_wifi_passwd_font4, LV_ALIGN_TOP_LEFT, 0, 270);

    }
    if(wifi_big_write==1)
    {
        if(obj_wifi_passwd_font1==NULL)
        {
            obj_wifi_passwd_font1=lv_label_create(lv_scr_act());
        }
        sprintf(font_text,"QWERTYUIO");
        lv_label_set_text(obj_wifi_passwd_font1,font_text);
        lv_obj_set_style_text_font(obj_wifi_passwd_font1, &font_passwd, 0);
        lv_obj_align(obj_wifi_passwd_font1, LV_ALIGN_TOP_LEFT, 0, 140);
        if(obj_wifi_passwd_font2==NULL)
        {
            obj_wifi_passwd_font2=lv_label_create(lv_scr_act());
        }
        sprintf(font_text,"ASDFGHJKL");
        lv_label_set_text(obj_wifi_passwd_font2,font_text);
        lv_obj_set_style_text_font(obj_wifi_passwd_font2, &font_passwd, 0);
        lv_obj_align(obj_wifi_passwd_font2, LV_ALIGN_TOP_LEFT, 0, 180);
        if(obj_wifi_passwd_font3==NULL)
        {
            obj_wifi_passwd_font3=lv_label_create(lv_scr_act());
        }
        sprintf(font_text,"ZXCVBNMP");
        lv_label_set_text(obj_wifi_passwd_font3,font_text);
        lv_obj_set_style_text_font(obj_wifi_passwd_font3, &font_passwd, 0);
        lv_obj_align(obj_wifi_passwd_font3, LV_ALIGN_TOP_LEFT, 0, 220);
        if(obj_wifi_passwd_font4==NULL)
        {
            obj_wifi_passwd_font4=lv_label_create(lv_scr_act());
        }
        sprintf(font_text,"切换大小写        退格");
        lv_label_set_text(obj_wifi_passwd_font4,font_text);
        lv_obj_set_style_text_font(obj_wifi_passwd_font4, &font_alipuhui, 0);
        lv_obj_align(obj_wifi_passwd_font4, LV_ALIGN_TOP_LEFT, 0, 270);

    }

}
void example_lvgl_demo_ui()
{   
    if(obj_img_main==NULL)
    {
        create_main_img();
    }
    if(obj_img_lock==NULL)
    {
        create_lock_img();
    }
    if(get_time_flag==true&&obj_font_top==NULL&&all_type.page==PAGE_MAIN)
    {
        create_main_text();
    }
}
