/*!
 * @file    I2C.h
 * @author  Tyler Holmes
 *
 * @date    20-May-2017
 * @brief   Wrapper functions around the stm32f3xx HAL I2C functions.
 *
 */
#pragma once

#include "common.h"
#include <stdbool.h>
#include <stdint.h>

ret_t I2C_init(void);
bool I2C_isBusy(void);
ret_t I2C_readData(uint8_t address, uint8_t mem_address, uint8_t *data_ptr, uint8_t data_len);
ret_t I2C_writeData(uint8_t dev_address, uint8_t mem_address, uint8_t *data_ptr, uint8_t data_len,
                    bool blocking);
ret_t I2C_writeByte(uint8_t dev_address, uint8_t mem_address, uint8_t data, bool blocking);
