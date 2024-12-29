/*

SPDX-License-Identifier: MIT

MIT License

Copyright (c) 2019-2020 Mika Tuupola

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <driver/i2c_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <stdint.h>
#include <string.h>

#include "i2c_helper.h"

static const char* TAG = "i2c_helper";
//static const uint8_t ACK_CHECK_EN = 1;

int32_t i2c_init(i2c_port_t port) {
    ESP_LOGI(TAG, "Starting I2C master at port %d.", port);

    i2c_master_bus_config_t bus_config;
    memset((void *)&bus_config, 0, sizeof(i2c_master_bus_config_t));
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.i2c_port = port;
    bus_config.scl_io_num = (port == I2C_NUM_0) ? I2C_HELPER_MASTER_0_SCL : I2C_HELPER_MASTER_1_SCL;
    bus_config.sda_io_num = (port == I2C_NUM_0) ? I2C_HELPER_MASTER_0_SDA : I2C_HELPER_MASTER_1_SDA;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = false;
    
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    return ESP_OK;
}

int32_t i2c_read(void *handle, uint8_t reg, uint8_t *buffer, uint16_t length) {

    esp_err_t result;
    i2c_master_dev_handle_t dev_handle = *(i2c_master_dev_handle_t *)&handle;
    
    if(reg)
    {
        result = i2c_master_transmit_receive(
            dev_handle, &reg, 1, buffer, length, 1000 / portTICK_PERIOD_MS);
    }
    else
    {
        result = i2c_master_receive(dev_handle, buffer, length, 1000 / portTICK_PERIOD_MS);    } 

    ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, length, ESP_LOG_DEBUG);
    ESP_ERROR_CHECK_WITHOUT_ABORT(result);

    return result;
}

int32_t i2c_write(void *handle, const uint8_t *buffer, uint16_t size)
{
    esp_err_t result;
    i2c_master_dev_handle_t dev_handle = *(i2c_master_dev_handle_t *)&handle;
    result = i2c_master_transmit(dev_handle, buffer, size, 1000 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK_WITHOUT_ABORT(result);

    return result;
}

int32_t i2c_close(i2c_port_t port) {
    i2c_master_bus_handle_t bus_handle;
    ESP_LOGI(TAG, "Closing I2C master at port %d", port);
    ESP_ERROR_CHECK(i2c_master_get_bus_handle(port, &bus_handle));
    return i2c_del_master_bus(bus_handle);
}
