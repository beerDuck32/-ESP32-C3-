#include "key.h"
volatile extern type_struct all_type;
volatile extern uint16_t start_y,end_y;
volatile extern bool clear_screen_flag;    //黑屏的标志位，为了只让清屏函数执行一次
void gpio_isr_handler(void* arg)
{
    if(all_type.page!=PAGE_NO)//判断是否为黑屏界面，不是则进入黑屏界面
    {
        clear_screen_flag=1;   
        all_type.page=PAGE_NO;
        start_y=end_y;  //这一步是为了第一次调用PAGE_UNLOCK时，让差值为0
    }
    else
    {
        clear_screen_flag=0;
        start_y=end_y;  //这一步是为了第一次调用PAGE_UNLOCK时，让差值为0
         gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, 0);   //亮屏
        all_type.page=PAGE_UNLOCK;
    }
    
    
}

void key_init()
{
       gpio_config_t io_conf = {
        .mode=GPIO_MODE_DEF_INPUT,//输入模式
        .intr_type=GPIO_INTR_POSEDGE,//上升沿中断
        .pull_down_en=0,//关闭下拉电阻
        .pull_up_en=1,//打开上拉电阻
        .pin_bit_mask = 1ULL<<GPIO_NUM_9//PIN9
   };
   gpio_config(&io_conf);

   //install gpio isr service 使能中断服务
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_NUM_9, gpio_isr_handler, (void*) GPIO_NUM_9);
    //参数 1.GPIO口 2.中断函数名 3.中断函数的参数   相当于将触发中断的gpio口与中断函数进行绑定，让系统知道这个IO口应该触发这个汇总孤单中断函数。
}