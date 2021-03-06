#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED

#include "stm32f407xx.h"                  // Device header
#include <string.h>
#include <stdbool.h>

#define UART_RX_BUF_SIZE 81         // in bytes
#define UART_TX_BUF_SIZE 81         

void UART_Init (void);
void UART_TX   (char data[]);

#endif
