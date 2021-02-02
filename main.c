#include "stm32f4xx.h"                  // Device header
#include "uart.h"
#include "i2c.h"
#include "uartcmds.h"

extern char uart_rx_data[UART_RX_BUF_SIZE];
extern bool uart_cmd_received;
extern uint8_t i2c_rx_data[I2C_RX_BUF_SIZE];

int main(void)
{
	UART_Init();
	I2C_Init ();
	
	while(1)
	{
		if (uart_cmd_received) {
			UART_parse_cmd(uart_rx_data);
			uart_cmd_received = false;
		}
		else __nop(); // An essential kludge to prevent ARMCLANG from breaking the code by "optimization"
	}
	
	//uint8_t ds3231_start_addr = 0;
	//volatile uint8_t hours, mins, secs;
	
	/*

	*/
}
