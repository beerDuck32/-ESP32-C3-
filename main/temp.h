#pragma once
#include <stdio.h>
#include "esp_err.h"
#include "myi2c.h"
#include "driver/i2c.h"

uint8_t gxhtc3_calc_crc(uint8_t *crcdata, uint8_t len);
esp_err_t gxhtc3_read_id(void);
esp_err_t gxhtc3_wake_up();
void gxhtc3_main();