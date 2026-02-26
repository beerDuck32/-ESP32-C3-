#include "lcd.h"
volatile extern uint16_t start_y,end_y;
static const char *TAG = "lcd Init";
esp_lcd_touch_handle_t tp = NULL;
type_struct all_type={
    .city=0,
    .page=PAGE_UNLOCK,  //开始时是解锁界面
    .set=SET_UP_TOLOCK, //开始时使用上划解锁
    .touch_x=0,
    .touch_y=0
};

//声明外部变量
extern volatile uint8_t sec;
extern volatile uint8_t min;
extern volatile uint8_t hour;
extern struct tm timeinfo;
extern uint16_t ap_count;
extern wifi_ap_record_t wifi_list[10];
extern char passwd[15];
//声明标志位
extern bool first_get_time_flag;
extern bool wifi_open;
extern bool wifi_big_write;
bool switch_city_flag=0; //为0则切换完成
SemaphoreHandle_t temp_flag;
static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

/* Rotate display and touch, when rotated screen in LVGL. Called when driver parameters are updated. */
static void lvgl_page_update_callback()
{

}


bool clear_screen_flag=0;//黑屏的标志位，为了只让清屏函数执行一次
extern lv_obj_t * obj_img_lock; //声明外部锁屏图片的对象变量，为了黑屏后立马创建这个图片，防止闪屏
static void lvgl_page_update()
{
    //获取时间
    if(first_get_time_flag==true)
    {
        hour=timeinfo.tm_hour;
        min=timeinfo.tm_min;
        sec=timeinfo.tm_sec;
        first_get_time_flag=false;
        //获取到了时间就开启定时器
        init_time();
    }
    //解锁界面的无密码设置（上划解锁界面）
    if(all_type.page==PAGE_UNLOCK&&all_type.set==SET_UP_TOLOCK)
    {
        //printf("当前为上划解锁界面");
        if(page_unlock())//如果解锁成功，回到主界面
        {
            all_type.page=PAGE_MAIN;
        }
    }

    //主界面
    if(all_type.page==PAGE_MAIN)
    {
        //printf("当前为主界面");
        page_main();
        if(all_type.touch_x<=70&&all_type.touch_y>20&&all_type.touch_y<90) //点击到锁屏
        {
            clear_screen_flag=1;
            all_type.page=PAGE_NO;
            start_y=end_y;  //这一步是为了第一次调用PAGE_UNLOCK时，让差值为0
        }
        if(all_type.touch_x<=70&&all_type.touch_y>100&&all_type.touch_y<170)//点击到天气
        {
            lvgl_clear_screen(1,0);//清屏
            all_type.page=PAGE_WEATHER;

        }
        if(all_type.touch_x<=140&&all_type.touch_x>70&&all_type.touch_y>100&&all_type.touch_y<170)//点击到温度
        {
            lvgl_clear_screen(1,0);
            all_type.page=PAGE_TEMP;
        }
        if(all_type.touch_x>140&&all_type.touch_y>100&&all_type.touch_y<170)//点击到设置
        {
            lvgl_clear_screen(1,0);
            all_type.page=PAGE_SET;
        }
    }

    if(all_type.page==PAGE_WEATHER)//天气界面
    {
        page_weather();
        if(all_type.touch_x<40&&all_type.touch_y>270)//返回
        {
            all_type.page=PAGE_MAIN;
        }
        if(all_type.touch_x>170&&all_type.touch_y>270&&all_type.event==EVENT_NO_TOUCH)//切换城市
        {
            all_type.touch_x=0;
            all_type.touch_y=0;
            printf("切换城市%d\n",all_type.city);
            switch_city_flag=1;
            all_type.city++;
            lvgl_clear_screen(1,0);
            if(all_type.city>2)
            {
                all_type.city=0;
            }

        }
    }

    if(all_type.page==PAGE_TEMP)//温湿度界面
    {
        page_temp();
        xSemaphoreGive(temp_flag);
        if(all_type.touch_x<40&&all_type.touch_y>270)//返回
        {

            all_type.page=PAGE_MAIN;
        }
    }

    if(all_type.page==PAGE_SET)//设置选择界面
    {
        page_set();
        if(all_type.touch_x<40&&all_type.touch_y>270)//返回
        {
            all_type.page=PAGE_MAIN;
        }
        if(all_type.touch_y<270&&all_type.touch_y>170&&all_type.event==EVENT_NO_TOUCH)//wifi设置
        {
            all_type.touch_y=0;
            all_type.page=PAGE_WIFI_SET;
            lvgl_clear_screen(1,0);
        }
    }
    static uint8_t wifi_num=3;//点击的第num个wifi
    if(all_type.page==PAGE_WIFI_SET)//wifi设置界面
    {
        
        page_wifi_set();
        if(all_type.touch_x<40&&all_type.touch_y>270&&all_type.event==EVENT_NO_TOUCH)//返回
        {
            all_type.touch_y=0;
            all_type.touch_x=0;
            lvgl_clear_screen(1,0);
            all_type.page=PAGE_SET;
        }
        if(all_type.touch_x>100&&all_type.touch_y>200&&all_type.touch_y<240&&all_type.event==EVENT_NO_TOUCH)//关闭|开启wifi
        {
            lvgl_clear_screen(1,0);
            all_type.touch_x=0;
            all_type.touch_y=0;
            wifi_open=!wifi_open;
            if(!wifi_open)
            {
                esp_wifi_stop();
                lv_refr_now(NULL);
            }
        }
        if(all_type.touch_y>120&&all_type.touch_y<140&&ap_count>0)//点击到第一个wifi
        {
            wifi_num=0;
            all_type.touch_x=0;
            all_type.touch_y=0;
            all_type.page=PAGE_WIFI_CONNCET;
            lvgl_clear_screen(1,0);
        }
        if(all_type.touch_y>60&&all_type.touch_y<120&&ap_count>1)//点击到第二个wifi
        {
            wifi_num=1;
            all_type.touch_x=0;
            all_type.touch_y=0;
            all_type.page=PAGE_WIFI_CONNCET;
            lvgl_clear_screen(1,0);
        }



    }
    if(all_type.page==PAGE_WIFI_CONNCET)//wifi连接密码界面
    {
        page_wifi_connect(wifi_num);
        if(all_type.touch_x<40&&all_type.touch_y>270&&all_type.event==EVENT_NO_TOUCH)//返回
        {
            all_type.touch_y=0;
            all_type.touch_x=0;
            lvgl_clear_screen(1,0);
            all_type.page=PAGE_WIFI_SET;
            memset(passwd,0,sizeof(passwd));
        }
        if(all_type.touch_x<120&&all_type.touch_y<40&&all_type.event==EVENT_NO_TOUCH)//切换大小写
        {
            all_type.touch_y=41;
            wifi_big_write=!wifi_big_write;
        }
        if(all_type.touch_y>80&&all_type.touch_y<200&&all_type.event==EVENT_NO_TOUCH)//点击密码
        {
            all_type.touch_y=79;
            strncat(passwd,"8",sizeof(passwd)-strlen(passwd)-1);
        }
        if(all_type.touch_x>120&&all_type.touch_y<40&&all_type.event==EVENT_NO_TOUCH)//退格
        {
            all_type.touch_y=41;
            passwd[strlen(passwd)-1]='\0';
        }
        if(all_type.touch_x>180&&all_type.touch_y>250&&all_type.event==EVENT_NO_TOUCH)//连接
        {
            printf("连接wifi");
            all_type.touch_x=179;
            wifi_init((char *)wifi_list[wifi_num].ssid,passwd);
        }
    }
    if(all_type.page==PAGE_NO)//黑屏
    {
        if(clear_screen_flag)
        {
            lvgl_clear_screen(1,1);
            clear_screen_flag=0;
        }
        example_lvgl_demo_ui();
    }
}

#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
// 获取被触摸的x和y轴 
uint8_t last_touch_pressed = 0; // 标志位记录触摸状态
static void example_lvgl_touch_cb(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    // 1. 定义匹配参数的变量（核心！）
    esp_lcd_touch_point_data_t touch_data[1]; // 参数2：触摸点数据数组（单点=长度1）
    uint8_t actual_point_cnt = 0;             // 参数3：实际触摸点数（输出）
    const uint8_t max_point_cnt = 1;          // 参数4：最大触摸点数（单点=1）
    esp_err_t ret = ESP_FAIL;                 // 函数返回值

    // 初始化触摸数据，避免脏值
    memset(touch_data, 0, sizeof(touch_data));
    actual_point_cnt = 0;

    /* 第一步：读取触摸控制器原始数据（FT6336必须先读） */
    esp_lcd_touch_read_data(drv->user_data);

    /* 第二步：调用精准匹配的 esp_lcd_touch_get_data（核心修正） */
    ret = esp_lcd_touch_get_data(
        drv->user_data,        // 参数1：FT6336触摸驱动句柄（tp）
        touch_data,            // 参数2：触摸点数据数组指针（类型完全匹配）
        &actual_point_cnt,     // 参数3：实际点数输出指针
        max_point_cnt          // 参数4：最大点数（1）
    );


    /* 第三步：判断是否触摸成功 */
    if (ret == ESP_OK && actual_point_cnt > 0) {
        // 取第一个触摸点的坐标（FT6336单点触摸）
        data->point.x = touch_data[0].x;
        data->point.y = touch_data[0].y;
        // 更新全局变量
        all_type.touch_x = touch_data[0].x;
        all_type.touch_y = touch_data[0].y;
        data->state = LV_INDEV_STATE_PRESSED;
        printf("x=%d,y=%d\n", data->point.x, data->point.y);
        //更新事件
        all_type.event=EVENT_TOUCH_CLICK;  
    } else {
        // 无触摸或读取失败
        data->state = LV_INDEV_STATE_RELEASED;
        //更新事件
        all_type.event=EVENT_NO_TOUCH;
        // 可选：打印失败原因（调试用）
        if (ret != ESP_OK) {
            printf("FT6336触摸数据读取失败，错误码：0x%x\n", ret);
        }
    }
    //更新界面互动
    lvgl_page_update();
}
#endif

static void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void Init_lcd()
{
     //1.初始化i2c
    ESP_ERROR_CHECK(i2c_master_init());
    //ESP_LOGI(TAG, "I2C initialized successfully");
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions
    
    //2.配置背光
    //ESP_LOGI(TAG, "Turn off LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT_OD,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    //3.初始化SPI引脚
    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,
        .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,
        .miso_io_num=EXAMPLE_PIN_NUM_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    //4.初始化LCD引脚
    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC,
        .cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = example_notify_lvgl_flush_ready,
        .user_ctx = &disp_drv,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    //5.配置屏幕参数
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,//复位引脚
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,//使用RGB像素分量顺序
        .bits_per_pixel = 16,//每个像素点占16个字节
    };


    ESP_LOGI(TAG, "Install ST7789 panel driver");
    
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle)); //给panel_handle赋值

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));//硬件复位 ST7789 屏幕
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));//软件初始化 ST7789
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));//开启 ST7789 的颜色反转功能

    //第一个false：水平镜像关闭（true则屏幕内容左右翻转）
    //第二个false：垂直镜像关闭（true则屏幕内容上下翻转）
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));

    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));//开启 ST7789 的显示功能

    //6.触摸 IO 句柄与 I2C 配置初始化
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();

    //7.触摸芯片核心参数配置
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,//触摸X轴最大坐标
        .y_max = EXAMPLE_LCD_V_RES,//触摸y轴最大坐标
        .rst_gpio_num = -1,        // 触摸复位引脚
        .int_gpio_num = -1,        // 触摸中断引脚
        .flags = {          
            .swap_xy = 0,          // 不交换X/Y坐标
            .mirror_x = 0,         // X轴不镜像
            .mirror_y = 1,         // Y轴不镜像
        },
    };

    //8.创建触摸驱动实例
    ESP_LOGI(TAG, "Initialize touch controller FT6336");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &tp));
 // CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_STMPE610
 // CONFIG_EXAMPLE_LCD_TOUCH_ENABLED

    ESP_LOGI(TAG, "Turn on LCD backlight");
    //9.设置背光
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

    //10.LVGL 核心初始化
    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    // alloc draw buffers used by LVGL
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    lv_color_t *buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 20);

    //11.注册 LCD 显示驱动到 LVGL
    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;//LCD水平分辨率
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;//LCD垂直分辨率
    disp_drv.flush_cb = example_lvgl_flush_cb;
    disp_drv.drv_update_cb = lvgl_page_update_callback;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    // 12.LVGL 心跳定时器初始化
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

    //13.触摸驱动注册到 LVGL（核心交互逻辑）
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
    static lv_indev_drv_t indev_drv;    // Input device driver (Touch)
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.disp = disp;
    indev_drv.read_cb = example_lvgl_touch_cb;//回调函数
    indev_drv.user_data = tp;

    lv_indev_drv_register(&indev_drv);
#endif
    //显示 LVGL 界面
    example_lvgl_demo_ui();
    ESP_LOGI(TAG, "Display LVGL Meter Widget");
}


//中断函数
bool IRAM_ATTR timer_isr_callback(void *args)
{
    // 1. 清除中断状态寄存器（必须！）
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    
    // 2. 重新使能报警器（必须！）
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
    
    // 3. 执行用户代码
    sec++;
    if(sec>=60)
    {
        min++;
        sec=0;
    }
    if(min>=60)
    {
        hour++;
        min=0;
    }

    // 4. 返回是否需要任务切换
    return pdFALSE;  // 不需要任务切换
}

//初始化定时器
void init_time()
{
    timer_config_t config = {
        .divider = 80,              // APB时钟80MHz，分频后为1MHz（1us计数）
        .counter_dir = TIMER_COUNT_UP, //向上计数
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = TIMER_AUTORELOAD_EN,  // 自动重载
        .intr_type = TIMER_INTR_LEVEL        // 中断类型
    };
    
    // 初始化定时器（组0，定时器0）
    timer_init(TIMER_GROUP_0, TIMER_0, &config);
    // 设置定时器计数值（从0开始）
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);

    // 设置报警值（1秒后触发：1,000,000 us）
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 1000000);

    // 使能中断
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);

    // 注册中断处理函数
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, timer_isr_callback, NULL, 0);

    // 启动定时器
    timer_start(TIMER_GROUP_0, TIMER_0);
}


void create_binary_semaphore(void)
{
    // 创建二值信号量
    temp_flag = xSemaphoreCreateBinary();
    if (temp_flag != NULL) {
        printf("信号量创建成功\n");
    }
}
