#include "temp.h"

#define POLYNOMIAL  0x31 // P(x) = x^8 + x^5 + x^4 + 1 = 00110001

//CRC校验
uint8_t gxhtc3_calc_crc(uint8_t *crcdata, uint8_t len)
{
    uint8_t crc = 0xFF;

    for(uint8_t i = 0; i < len; i++)
    {
        crc ^= (crcdata[i]);
        for(uint8_t j = 8; j > 0; --j)
        {
            if(crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
            else           crc = (crc << 1);
        }
    }
    return crc;
}

//软复位gxhtc3
esp_err_t gxhtc3_reset(void)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    
    i2c_master_write_byte(cmd, 0x70 << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x80, true);
    i2c_master_write_byte(cmd, 0x5D, true);

    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}


// 读取ID
esp_err_t gxhtc3_read_id(void)
{
    esp_err_t ret;
    uint8_t data[3];
    //创建一个cmd链表(本质是一个结构体)
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    //调用start函数，表头为iic start命令
    i2c_master_start(cmd);
    //调用write命令，链表的第二个数据就是 写命令和写入的数据
    
    i2c_master_write_byte(cmd, 0x70 << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0xEF, true);
    i2c_master_write_byte(cmd, 0xC8, true);
    //调用stop命令，链表的第5个数据就是 stop命令
    i2c_master_stop(cmd);

    // 7. 提交第一个指令链表，让I2C外设批量执行上述所有写操作
    // I2C_MASTER_NUM：指定使用的I2C端口（如I2C_NUM_0）
    // 1000 / portTICK_PERIOD_MS：超时时间（1000ms，超时则返回通信失败）
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    //如果失败直接释放
    if (ret != ESP_OK) {
        i2c_cmd_link_delete(cmd);
        printf("第一次通讯失败\n");
        return ret;
    }
    i2c_cmd_link_delete(cmd);
    //重新创建指令链表
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | I2C_MASTER_READ, true);
    // 3. 读取3字节数据到data数组中
    // data：存储读取数据的数组（data[0]、data[1]是ID，data[2]是CRC）
    // 3：读取的字节数（2字节ID+1字节CRC）
    // I2C_MASTER_LAST_NACK：表示读取最后1字节时，主机发送NACK（告诉从机“数据读完了”）
    i2c_master_read(cmd, data, 3, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    //提交第二个指令链表，执行以上的读操作
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);

    //查看CRC校验是否成功
    if(data[2]!=gxhtc3_calc_crc(data,2)){
        ret = ESP_FAIL;
        printf("校验失败");
    }
    i2c_cmd_link_delete(cmd);
    return ret;
}



esp_err_t gxhtc3_wake_up()
{
    esp_err_t res;
    i2c_cmd_handle_t cmd=i2c_cmd_link_create();
    i2c_master_start(cmd);
    //首先找到器件地址，并且发出写命令
    i2c_master_write_byte(cmd,0x70<<1|I2C_MASTER_WRITE,true);
    //发出唤醒MSB命令（参考数据手册）
    i2c_master_write_byte(cmd,0x35,true);
    //发出唤醒LSB命令（参考数据手册）
    i2c_master_write_byte(cmd,0x17,true);
    i2c_master_stop(cmd);
    res=i2c_master_cmd_begin(I2C_MASTER_NUM,cmd,1000/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return res;
}

esp_err_t gxhtc3_measure()
{
    esp_err_t res;
    i2c_cmd_handle_t cmd=i2c_cmd_link_create();
    i2c_master_start(cmd);
    //首先找到器件地址，并且发出写命令
    i2c_master_write_byte(cmd,0x70<<1|I2C_MASTER_WRITE,true);
    //发出测量MSB命令（参考数据手册）
    i2c_master_write_byte(cmd,0x7C,true);
    //发出测量MSB命令（参考数据手册）
    i2c_master_write_byte(cmd,0xA2,true);
    i2c_master_stop(cmd);
    res=i2c_master_cmd_begin(I2C_MASTER_NUM,cmd,1000/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return res;
}

//创建一个接收数据的数组，一共6个字节（参考数据手册）
uint8_t  gxhtc3_Date[6];
esp_err_t gxhtc3_readDate()
{
    esp_err_t res;
    i2c_cmd_handle_t cmd=i2c_cmd_link_create();
    i2c_master_start(cmd);
    //首先找到器件地址，并且发出读命令
    i2c_master_write_byte(cmd,0x70<<1|I2C_MASTER_READ,true);
    // I2C_MASTER_LAST_NACK：表示读取最后1字节时，主机发送NACK（告诉从机“数据读完了”）
    i2c_master_read(cmd,gxhtc3_Date,6,I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    res=i2c_master_cmd_begin(I2C_MASTER_NUM,cmd,1000/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return res;
}
float temp;
float humidity;
void gxhtc3_DisposeDate()
{   
    //CRC校验
    if(gxhtc3_Date[2]!=gxhtc3_calc_crc(gxhtc3_Date,2))
    {
        printf("湿度CRC校验失败\n");
        printf("%d\n",gxhtc3_Date[2]);
    }
    else
    {
        //printf("湿度CRC校验成功\n");
    }
    if(gxhtc3_Date[5]!=gxhtc3_calc_crc(&gxhtc3_Date[3],2))
    {
        printf("温度CRC校验失败\n");
        printf("%d\n",gxhtc3_Date[5]);
    }
    else
    {
        //printf("温度CRC校验成功\n");
    }

    //校验成功后处理数据
    uint16_t original_date_temp=(gxhtc3_Date[0]<<8)|gxhtc3_Date[1];
    uint16_t original_date_humidity=(gxhtc3_Date[3]<<8)|gxhtc3_Date[4];
    temp=(175.0*(float)original_date_temp)/65535.0-45.0;
    humidity=(100.0*(float)original_date_humidity)/65535.0;
    // printf("temp=%1f\n",temp);
    // printf("humidity=%1f\n",humidity);

}

bool read_id_flag=false;
void gxhtc3_main()
{   
    //1.软复位
    gxhtc3_reset();

    //2.读取从机id(仅需读取一次)
    if(read_id_flag==false)
    {
        esp_err_t id_ret = gxhtc3_read_id();
        uint8_t read_id_time=2;
        while(id_ret != ESP_OK)
        {
            id_ret = gxhtc3_read_id();
            printf("GXHTC3 READ ID NO.%d\n",read_id_time);
            read_id_time++;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        printf("GXHTC3 OK\n");
        read_id_flag=true;
    }
    
    //3.唤醒从机
    if(gxhtc3_wake_up()==0)
    {
        //printf("GXHTC3 wake OK\n");
    }
    else
    {
        printf("GXHTC3 wake err\n");
    }
    
    //4.器件测量数据
    if(gxhtc3_measure()==0)
    {
        // printf("GXHTC3 measureing\n");
    }
    else
    {
        printf("GXHTC3 measureing err\n");
    }

    //读取数据
    while(gxhtc3_readDate()!=0)
    {
        printf("读取失败，重新读取\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    
    //进行数据的处理及输出
    gxhtc3_DisposeDate();
}

