#include "stm32f4xx.h"                  // Device header
#include "uart.h"
#include "i2c.h"
#include "uartcmds.h"

extern char uart_rx_data[UART_RX_BUF_SIZE];
extern bool uart_cmd_received;
extern uint8_t i2c_rx_data[I2C_RX_BUF_SIZE];

int main(void)
{
	//uint8_t ds3231_start_addr = 0;
	//volatile uint8_t hours, mins, secs;
	
	UART_Init();
	I2C_Init ();
	
	while(1)
	{
		if (uart_cmd_received) {
			UART_parse_cmd(uart_rx_data);
			uart_cmd_received = false;
		}
	}
	
	/*
	I2C_Write (0x68, &ds3231_start_addr, 1, true);
	I2C_Read  (0x68, i2c_rx_data, 3, 1000000);
	
	secs  = 10*((i2c_rx_data[0] >> 4) & 0x7) + (i2c_rx_data[0] & 0xF);
	mins  = 10*((i2c_rx_data[1] >> 4) & 0x7) + (i2c_rx_data[1] & 0xF);
	hours = 10*((i2c_rx_data[2] >> 4) & 0x1) + (i2c_rx_data[2] & 0xF);
	*/
}