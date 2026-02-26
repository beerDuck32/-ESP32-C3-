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

#define WIFI_SSID "ChinaNet-Y24f"
#define WIFI_PASSWORE "xkca9eec"


#define WEATHER_URL_NanNing "https://api.seniverse.com/v3/weather/daily.json?key=SAhaQLQiug4EYoEV-&location=nanning&language=zh-Hans&unit=c&start=0&days=3"
#define WEATHER_URL_ShenZhen "https://api.seniverse.com/v3/weather/daily.json?key=SAhaQLQiug4EYoEV-&location=shenzhen&language=zh-Hans&unit=c&start=0&days=3"
#define WEATHER_URL_GuangZhou "https://api.seniverse.com/v3/weather/daily.json?key=SAhaQLQiug4EYoEV-&location=guangzhou&language=zh-Hans&unit=c&start=0&days=3"
//时间结构体
struct tm timeinfo={
    .tm_hour=12,
    .tm_mday=1,
    .tm_min=0,
    .tm_sec=0,
    .tm_year=1900+2026
};   
char wifi_connected_name[30]="ChinaNet-Y24f";
//外部变量
extern type_struct all_type;

static const char *TAG = "WIFI&HTTP";
//标志位
bool get_time_flag=false;
extern bool first_get_time_flag;
extern bool switch_city_flag;
bool wifi_connected_flag=0;
bool wifi_init_flag=0;
// wifi一键扫描函数
wifi_ap_record_t wifi_list[10];
uint16_t ap_count = 0;
void wifi_scan()
{
    printf("=== 最小WiFi扫描示例 ===\n");
    
    // 1. 初始化NVS
    nvs_flash_init();
    
    // 2. 初始化网络栈和事件循环
    esp_netif_init();
    esp_event_loop_create_default();
    
    // 3. 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    
    // 4. 设置STA模式
    esp_wifi_set_mode(WIFI_MODE_STA);
    
    // 5. 启动WiFi
    esp_wifi_start();
    
    printf("WiFi初始化完成，等待1秒...\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 6. 执行扫描
    wifi_scan_config_t scan_cfg = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
    };
    
    printf("开始扫描...\n");
    esp_err_t ret = esp_wifi_scan_start(&scan_cfg, true);
    
    if (ret == ESP_OK) {
        printf("扫描成功！\n");
        
        esp_wifi_scan_get_ap_num(&ap_count);
        printf("发现 %d 个WiFi网络\n", ap_count);
        memset(wifi_list,0,sizeof(wifi_list));
        if (ap_count > 0) {
            // wifi_ap_record_t wifi_list[10];
            uint16_t count = ap_count > 10 ? 10 : ap_count;
            esp_wifi_scan_get_ap_records(&count, wifi_list);
            for (int i = 0; i < count; i++) {
                printf("%d. %s\n", i+1, wifi_list[i].ssid);
            }
        }
    } else {
        printf("扫描失败: %s (错误码: 0x%X)\n", 
               esp_err_to_name(ret), ret);
    }
    printf("=== 程序结束 ===\n");
}

// 获取时间的函数，在连接WiFi后调用
bool get_network_time(void) {
    // 1. 先检查WiFi是否连接
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) != ESP_OK) {
        ESP_LOGE(TAG, "WiFi未连接，无法获取时间");
        return false;
    }
    
    ESP_LOGI(TAG, "开始同步网络时间...");
    
    // 2. 设置时区
    setenv("TZ", "CST-8", 1);
    tzset();
    
    // 3. 停止之前的SNTP服务（如果已启动）
    esp_sntp_stop();
    
    // 4. 配置SNTP
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "ntp1.aliyun.com");  // 国内服务器更快
    esp_sntp_setservername(1, "cn.ntp.org.cn");
    esp_sntp_setservername(2, "pool.ntp.org");
    
    // 5. 初始化SNTP
    esp_sntp_init();
    
    // 6. 等待同步（带超时和检查）
    int retry = 0;
    const int max_retry = 15;
    
    while (retry < max_retry) {
        ESP_LOGI(TAG, "等待时间同步 %d/%d...", retry + 1, max_retry);
        
        // 获取当前时间
        time_t now;
        time(&now);
        localtime_r(&now, &timeinfo);
        
        // 检查时间是否有效（年份大于2025）
        if (timeinfo.tm_year > (2025 - 1900)) {
            ESP_LOGI(TAG, "时间同步成功！");
            get_time_flag=true;
            first_get_time_flag=true;
            // 显示时间
            const char* weekdays[] = {"日", "一", "二", "三", 
                                 "四", "五", "六"};

            printf("%d年%d月%d日星期%s,%d时%d分%d秒\n",timeinfo.tm_year+1900,timeinfo.tm_mon+1,timeinfo.tm_mday,weekdays[timeinfo.tm_wday],timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
            return true;
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
        retry++;
    }
    
    ESP_LOGE(TAG, "时间同步失败");
    esp_sntp_stop();  // 清理
    return false;
}




void wifi_event_handle(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data)
{
    printf("1");
    if(event_base==WIFI_EVENT)//如果为wifi事件
    {
        switch (event_id)//判断event_id
        {
            //启动了STA的工作模式事件
            case WIFI_EVENT_STA_START:
                //开始连接
                esp_wifi_connect();
                break;
            //已经连接上wifi事件
            case WIFI_EVENT_STA_CONNECTED:
                printf("esp32 connected to ap!\n");
                break;
            //断连事件
            case WIFI_EVENT_STA_DISCONNECTED:
                wifi_connected_flag=0;
                // esp_wifi_connect();
                printf("esp32 disconnected\n");
                break;
            default:break;
        }
    }
    else if(event_base==IP_EVENT)//ip事件
    {
        switch ((event_id))
        {
            //已经获取到了IP
            case IP_EVENT_STA_GOT_IP:
                printf("esp32 get ip successful%d\n",get_time_flag);
                if(get_time_flag==false)
                {
                    get_network_time();
                    get_weather_init();
                }

                wifi_connected_flag=1;
                break;
        default:
            break;
        }
    }
}

void wifi_init(char *wifi_ssid,char *wifi_passwd)
{   
    if(wifi_init_flag==0)
    {
        //1.初始化nvs，连接wife成功后，idf会把密码等信息存入nvs，下次上电后会默认使用这个密码
        ESP_ERROR_CHECK(nvs_flash_init());
        //初始化tcp ip 协议栈
        ESP_ERROR_CHECK(esp_netif_init());
        //初始化wife事件回调
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        //创建STA模式
        esp_netif_create_default_wifi_sta();
        //wifi初始化，这里是直接赋默认值
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        //注册回调函数  参数1：WIFI事件  参数2：监听所有事件  参数3：回调函数  参数4：自定义参数
        esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_event_handle,NULL);
        //注册回调函数  参数1：IP事件   参数2：获取到ip事件   参数3：回调函数  参数4：自定义参数
        esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,wifi_event_handle,NULL);
        wifi_init_flag=1;
    }
    

    //配置wifi结构体
    wifi_config_t wifi_cfg={
        .sta.threshold.authmode=WIFI_AUTH_WPA_PSK,//加密方式
        .sta.pmf_cfg.capable=true,  //启动保护管理帧，增加安全性
        .sta.pmf_cfg.required=false    //是否只和有保护管理帧功能的设备通信
    };
    if (wifi_ssid != NULL) {
        // strncpy：安全拷贝，最多拷贝31个字符（留1字节给'\0'）
        strncpy((char *)wifi_cfg.sta.ssid, wifi_ssid, sizeof(wifi_cfg.sta.ssid) - 1);
    }
    // 强制补'\0'，确保是合法字符串（ESP32 WiFi驱动要求）
    wifi_cfg.sta.ssid[sizeof(wifi_cfg.sta.ssid) - 1] = '\0';

    // 7.2 拷贝密码：同理，最多拷贝63个字符
    if (wifi_passwd != NULL) {
        strncpy((char *)wifi_cfg.sta.password, wifi_passwd, sizeof(wifi_cfg.sta.password) - 1);
    }
    wifi_cfg.sta.password[sizeof(wifi_cfg.sta.password) - 1] = '\0';

    // 1. 先停止WiFi（避免更新配置时出错）
    esp_wifi_stop();
    //设置wifi模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    //配置wifi
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA,&wifi_cfg));
    //启用wifi
    ESP_ERROR_CHECK(esp_wifi_start());
}

// 定义缓冲区：存储服务器返回的完整数据（根据API返回大小调整，天气API返回约200字节）
#define MAX_RECV_BUFFER 2048
static char recv_buffer[MAX_RECV_BUFFER] = {0}; // 静态缓冲区，避免回调函数多次调用丢失数据
static int recv_len = 0; // 记录已接收的数据长度



weather_t today_weather_data={0};
weather_t tomorrow_weather_data={0};
weather_t after_tomorrow_weather_data={0};
weather_t weather_data3={0};
weather_t weather_data4={0};
//处理拿到的天气数据
void analysis_weaher_data()
{
    //*****今天*****//
    char *weather_text = strstr(recv_buffer, "\"text_day\":\"");//在recv_buffer中查找 "text_day":"(今日天气)
    char *end=NULL;
    if (weather_text) //查找到
    {
        weather_text += 12; // 跳过"text_day":"
        end = strchr(weather_text, '"');//查找" 
        if (end) *end = '\0';//查找到，则结束字符串
        strcpy(today_weather_data.weather, weather_text); 
        ESP_LOGI(TAG, "当前天气：%s", today_weather_data.weather);
        *end='"';//延续字符串
    }

    weather_text=strstr(weather_text,"\"high\":\"");//查找 "high":" (今日最高气温)
    if(weather_text)//查找到
    {
        weather_text+= 8; //跳过"high":"
        end = strchr(weather_text, '"');//查找" 
        if (end) *end = '\0';//查找到，则结束字符串
        today_weather_data.high_temperature=atoi(weather_text);
        ESP_LOGI(TAG, "今日最高气温：%d", today_weather_data.high_temperature);
        *end='"';//延续字符串
    }

    weather_text=strstr(weather_text,"\"low\":\"");//查找 "low":" (今日最低气温)
    if(weather_text)//查找到
    {
        weather_text+= 7; //跳过"high":"
        end = strchr(weather_text, '"');//查找" 
        if (end) *end = '\0';//查找到，则结束字符串
        today_weather_data.low_temperature=atoi(weather_text);
        ESP_LOGI(TAG, "今日最低气温：%d", today_weather_data.low_temperature);
        *end='"';//延续字符串
    }


    //*****明天*****//
    weather_text = strstr(weather_text, "\"text_day\":\"");//查找 "text_day":"(明日天气，现在的weather_text已经去掉了前面的"text_day":")
    if (weather_text) //查找到
    {
        weather_text += 12; // 跳过"text_day":"
        end = strchr(weather_text, '"');//查找" 
        if (end) *end = '\0';//查找到，则结束字符串
        strcpy(tomorrow_weather_data.weather, weather_text); 
        ESP_LOGI(TAG, "明日天气：%s", tomorrow_weather_data.weather);
        *end='"';//延续字符串
    }
    
    weather_text=strstr(weather_text,"\"high\":\"");//查找 "high":" (明日最高气温)
    if(weather_text)//查找到
    {
        weather_text+= 8; //跳过"high":"
        end = strchr(weather_text, '"');//查找" 
        if (end) *end = '\0';//查找到，则结束字符串
        tomorrow_weather_data.high_temperature=atoi(weather_text);
        ESP_LOGI(TAG, "明日最高气温：%d", tomorrow_weather_data.high_temperature);
        *end='"';//延续字符串
    }

    weather_text=strstr(weather_text,"\"low\":\"");//查找 "low":" (明日最低气温)
    if(weather_text)//查找到
    {
        weather_text+= 7; //跳过"high":"
        end = strchr(weather_text, '"');//查找" 
        if (end) *end = '\0';//查找到，则结束字符串
        tomorrow_weather_data.low_temperature=atoi(weather_text);
        ESP_LOGI(TAG, "明日最低气温：%d", tomorrow_weather_data.low_temperature);
        *end='"';//延续字符串
    }


    //*****后天*****//
    weather_text = strstr(weather_text, "\"text_day\":\"");//查找 "text_day":"(后天天气，现在的weather_text已经去掉了前面的"text_day":")
    if (weather_text) //查找到
    {
        weather_text += 12; // 跳过"text_day":"
        end = strchr(weather_text, '"');//查找" 
        if (end) *end = '\0';//查找到，则结束字符串
        strcpy(after_tomorrow_weather_data.weather, weather_text); 
        ESP_LOGI(TAG, "后天天气：%s", after_tomorrow_weather_data.weather);
        *end='"';//延续字符串
    }
    
    weather_text=strstr(weather_text,"\"high\":\"");//查找 "high":" (后天最高气温)
    if(weather_text)//查找到
    {
        weather_text+= 8; //跳过"high":"
        end = strchr(weather_text, '"');//查找" 
        if (end) *end = '\0';//查找到，则结束字符串
        after_tomorrow_weather_data.high_temperature=atoi(weather_text);
        ESP_LOGI(TAG, "后天最高气温：%d", after_tomorrow_weather_data.high_temperature);
        *end='"';//延续字符串
    }

    weather_text=strstr(weather_text,"\"low\":\"");//查找 "low":" (后天最低气温)
    if(weather_text)//查找到
    {
        weather_text+= 7; //跳过"high":"
        end = strchr(weather_text, '"');//查找" 
        if (end) *end = '\0';//查找到，则结束字符串
        after_tomorrow_weather_data.low_temperature=atoi(weather_text);
        ESP_LOGI(TAG, "后天最低气温：%d", after_tomorrow_weather_data.low_temperature);
        *end='"';//延续字符串
    }
    switch_city_flag=0;
}

//https回调函数
esp_err_t  https_event_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_CONNECTED:
        printf("连接http成功\n");
        break;
    
    case HTTP_EVENT_ON_DATA:
        // 确保数据不超出缓冲区
        if (recv_len + evt->data_len < MAX_RECV_BUFFER) {
            // 把当前分块数据复制到缓冲区
            memcpy(recv_buffer + recv_len, evt->data, evt->data_len);
            recv_len += evt->data_len;
            ESP_LOGI(TAG, "接收数据：%.*s（已接收%d字节）", evt->data_len, (char*)evt->data, recv_len);
        } else {
            ESP_LOGE(TAG, "缓冲区不足，数据溢出！");
        }
        break;

        // 3. 数据接收完成（解析完整数据）
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "📊 数据接收完成，完整内容：\n%s", recv_buffer);
            analysis_weaher_data();
            break;
    default:
        break;
    }
    return ESP_OK;
}

void get_weather_init()
{
    recv_len=0;
    esp_http_client_config_t config = {0}; // 初始化所有字段为0/NULL
    config.event_handler = https_event_handler;// 回调函数
    config.transport_type = HTTP_TRANSPORT_OVER_SSL; // 你的结构体：transport_type字段
    config.skip_cert_common_name_check = true; // 跳过证书校验！
    config.timeout_ms = 10000;                  // 超时时间
    config.use_global_ca_store = false;        // 你的结构体：use_global_ca_store字段

    if(all_type.city==NanNing)
    {
        config.url = WEATHER_URL_NanNing;
    }
    if(all_type.city==ShenZhen)
    {
        config.url = WEATHER_URL_ShenZhen;
    }
        if(all_type.city==GuangZhou)
    {
        config.url = WEATHER_URL_GuangZhou;
    }

    // 创建客户端
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "创建HTTP客户端失败");

        switch_city_flag=0;
        get_weather_init();
        return; // 失败直接返回，避免空指针
    }

    //执行请求
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, " HTTPS请求成功,状态码：%d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGW(TAG, "HTTPS请求失败:%s", esp_err_to_name(err));
    }

    // 释放资源
    esp_http_client_cleanup(client);
}

