#include "uartcmds.h"
#include "uart.h"
#include "i2c.h"

#include "stm32f4xx.h"                  // Device header
#include <string.h>

/* List of functions, called by UART commands */
static const UART_cmd_func UART_func_list[] = {
	{"HELP", &UART_send_help},
	{"DSRT", &DS3231_read_time},
	{"DSRD", &DS3231_read_date},
	{"DSRC", &DS3231_read_temp},
	{"DSWT", &DS3231_set_time},
	{"DSWD", &DS3231_set_date},
	{"DSSQ", &DS3231_set_clk_out},
	{"ATWD", &AT24C32_write_data},
	{"ATRD", &AT24C32_read_data}
};

extern char uart_tx_data[UART_TX_BUF_SIZE];
const uint8_t UART_func_count = sizeof(UART_func_list) / sizeof(UART_func_list[0]);  // Get the amount of functions in UART_func_list

/* Parser for UART commands.                                          *
 * Checks if called function exists in UART_func_list[] and calls it. */
void UART_parse_cmd(char *UART_data)
{
	char func_name[UART_CMD_LENGTH + 1];
	char args[UART_RX_BUF_SIZE - UART_CMD_LENGTH + 1];
	uint8_t i, j;
	
	if (UART_data[0] != '@') {  // If key symbol @ is not present, just print the received data 
		UART_TX("Message: ");
		UART_TX(UART_data);
		UART_TX("\r");
	}
	
	else {
		for (i = 0; i < UART_CMD_LENGTH; i++)
			func_name[i] = UART_data[i+1];
		func_name[UART_CMD_LENGTH] = '\0';
		
		
		for (i = 0; i < UART_func_count; i++)  
		{
			if (strcmp(func_name, UART_func_list[i].cmd_name) == 0) {
				for (j = UART_CMD_LENGTH+1; j < UART_RX_BUF_SIZE; j++) {
					args[j - (UART_CMD_LENGTH+1)] = UART_data[j];
					
					if (UART_data[j] == '\r')  // Exit if end of command is reached
						break;
				}
				
				UART_func_list[i].func_ptr(args);  // Call the respective function
				return;
			}
		}
		
		UART_TX("Error: no such function! \r");
	}
}

void UART_send_help(char *args)
{
	if (args[0] != '\r') {
		UART_TX("Error: HELP doesn't take any arguments! \r");
		return;
	}
	
	else {
		UART_TX("***** COMMAND HELP *****\r");
		UART_TX("DS3231 commands:\r");
		UART_TX("@DSRT - read time from DS3231\r");
		UART_TX("@DSRD - read date from DS3231\r");
		UART_TX("@DSRC - read temperature from DS3231\r");
		UART_TX("@DSWT=hh,mm,ss - set time on DS3231\r");
		UART_TX("@DSWD=yy,mm,dd - set date on DS3231\r");
		UART_TX("@DSSQ=a - set square wave output on DS3231, a = 0..5\r");
		UART_TX("AT24C32 commands:\r");
		UART_TX("@ATWD=addr,data - write data (1 byte) on AT24C32, max addr = 0xFFF\r");
		UART_TX("@ATRD=addr,amount - read data (amount bytes) on AT24C32, max addr = 0xFFF\r");
		UART_TX("\r");
	}
}

void DS3231_read_time(char *args)
{
	UART_TX("DS3231_read_time not implemented\r");
}

void DS3231_read_date(char *args)
{
	UART_TX("DS3231_read_date not implemented\r");
}

void DS3231_read_temp(char *args)
{
	UART_TX("DS3231_read_temp not implemented\r");
}

void DS3231_set_time(char *args)
{
	UART_TX("DS3231_set_time not implemented\r");
}

void DS3231_set_date(char *args)
{
	UART_TX("DS3231_set_date not implemented\r");
}

void DS3231_set_clk_out(char *args)
{
	UART_TX("DS3231_set_clk_out not implemented\r");
}

void AT24C32_write_data(char *args)
{
	UART_TX("AT24C32_write_data not implemented\r");
}

void AT24C32_read_data(char *args)
{
	UART_TX("AT24C32_read_data not implemented\r");
}
