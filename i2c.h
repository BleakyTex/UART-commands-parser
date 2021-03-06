#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

#include <stdint.h>
#include "stm32f4xx.h"                  // Device header
#include <stdbool.h>

#define I2C_RX_BUF_SIZE 16          // in bytes
#define I2C_TX_BUF_SIZE 16

void I2C_Init  (void);
void I2C_Write (uint8_t  addr, 
	              uint8_t  *txdata,
                uint16_t amount,
                bool     isRegAddress);
void I2C_Read  (uint8_t  addr, 
	              uint8_t  *rxdata,
                uint16_t amount,
                uint32_t timeout);

#endif
