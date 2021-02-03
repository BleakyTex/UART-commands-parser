#include "uartcmds.h"
#include "uart.h"
#include "i2c.h"

#include "stm32f4xx.h"                  // Device header
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define AT24C32_ADDRESS 0x57

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

extern uint8_t i2c_rx_data[I2C_RX_BUF_SIZE];
extern uint8_t i2c_tx_data[I2C_TX_BUF_SIZE];
extern char    uart_tx_data[UART_TX_BUF_SIZE];
  // Get the amount of functions in UART_func_list
static const uint8_t  UART_func_count = sizeof(UART_func_list) / sizeof(UART_func_list[0]);  

/* Prints the syntax error message */
void UART_show_syntax_error(void)
{
	UART_TX("Error: Invalid syntax\r");
}

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
			func_name[i] = UART_data[i+1];                      // Extract function name from received command
		func_name[UART_CMD_LENGTH] = '\0';
		
		
		for (i = 0; i < UART_func_count; i++)  
		{                                                     // Look for the function name in UART_func_list[]
			if (strcmp(func_name, UART_func_list[i].cmd_name) == 0) {
				for (j = UART_CMD_LENGTH+1; j < UART_RX_BUF_SIZE; j++) {
					args[j - (UART_CMD_LENGTH+1)] = UART_data[j];   // If such function is found, extract function arguments from the command
					
					if (UART_data[j] == '\r')        // Exit when end of command is reached
						break;
				}
				
				UART_func_list[i].func_ptr(args);  // Call the respective function
				return;
			}
		}
		
		UART_TX("Error: no such function! \r");
	}
}

/* Prints the command list and syntax to UART terminal */
void UART_send_help(char *args)
{
	if (args[0] != '\r') {
		UART_TX("Error: HELP doesn't take any arguments! \r");
		return;
	}
	
	else {
		UART_TX("******** COMMAND HELP ********\r");
		UART_TX("DS3231 commands:\r");
		UART_TX("    @DSRT - read time\r");
		UART_TX("    @DSRD - read date\r");
		UART_TX("    @DSRC - read temperature\r");
		UART_TX("    @DSWT=hh,mm,ss - set time\r");
		UART_TX("    @DSWD=dd,mm,yy - set date\r");
		UART_TX("    @DSSQ=sqw,32k - set square wave output on SQW/32K, sqw = 0..3, 32k = 0..1\r");
		UART_TX("AT24C32 commands:\r");
		UART_TX("    @ATWD=addr,data - write 1 byte to AT24C32, max addr = 0xFFF\r");
		UART_TX("    @ATRD=addr,amount - read [amount] of bytes from AT24C32, max addr = 0xFFF\r");
		UART_TX("\r");
	}
}

/* Reads time from DS3231 and prints it to UART terminal */
void DS3231_read_time(char *args)
{
	i2c_tx_data[0] = 0x00;
	uint8_t secs, mins, hours;
	
	if (args[0] != '\r') {
		UART_TX("Error: DSRT doesn't take any arguments! \r");
		return;
	}
	
	else {
		I2C_Write (0x68, i2c_tx_data, 1, false);   // Set DS3231 address pointer on time register
		I2C_Read  (0x68, i2c_rx_data, 3, 1000000); // Read 3 time registers
		
		/* Convert bit fields to time */
		secs  = 10*((i2c_rx_data[0] >> 4) & 0x7) + (i2c_rx_data[0] & 0xF);
		mins  = 10*((i2c_rx_data[1] >> 4) & 0x7) + (i2c_rx_data[1] & 0xF);
		hours = 10*((i2c_rx_data[2] >> 4) & 0x3) + (i2c_rx_data[2] & 0xF);
		
		/* Print time to the terminal */
		sprintf(uart_tx_data, "Time: %02d:%02d:%02d\r", hours, mins, secs);
		UART_TX(uart_tx_data);
	}
}

/* Reads date from DS3231 and prints it to UART terminal */
void DS3231_read_date(char *args)
{
	uint8_t day, month, year;
	
	if (args[0] != '\r') {
		UART_TX("Error: DSRD doesn't take any arguments! \r");
		return;
	}
	
	else {
		i2c_tx_data[0] = 0x04;
		I2C_Write (0x68, i2c_tx_data, 1, false);   // Set DS3231 address pointer on time register
		I2C_Read  (0x68, i2c_rx_data, 3, 1000000); // Read 3 time registers
		
		/* Convert bit fields to time */
		day   = 10*((i2c_rx_data[0] >> 4) & 0x3) + (i2c_rx_data[0] & 0xF);
		month = 10*((i2c_rx_data[1] >> 4) & 0x1) + (i2c_rx_data[1] & 0xF);
		year  = 10*((i2c_rx_data[2] >> 4) & 0xF) + (i2c_rx_data[2] & 0xF);
		
		/* Print time to the terminal */
		sprintf(uart_tx_data, "Date: %02d/%02d/%02d\r", day, month, year);
		UART_TX(uart_tx_data);
	}
}

/* Reads temperature from DS3231 and prints it to UART terminal */
void DS3231_read_temp(char *args)
{
	float temperature;
	
	if (args[0] != '\r') {
	UART_TX("Error: DSRC doesn't take any arguments! \r");
	return;
	}
	else {
		i2c_tx_data[0] = 0x11;                     
		I2C_Write (0x68, i2c_tx_data, 1, false);   // Set DS3231 address pointer at temperature register
		I2C_Read  (0x68, i2c_rx_data, 2, 1000000); // Read 2 temperature registers
		
		/* Convert registers contents to temperature */
		temperature = ((int8_t) i2c_rx_data[0]) + ((i2c_rx_data[1] >> 6) * 0.25); 
		
			/* Print temperature to the UART terminal */
		sprintf(uart_tx_data, "Temperature: %.2f`C\r", temperature);
		UART_TX(uart_tx_data);
	}
}

/* Sets time on DS3231. Syntax: @DSWT=hh,mm,ss */
void DS3231_set_time(char *args)
{
	uint8_t secs, mins, hours;
	
	if (sscanf(args, "=%hhu,%hhu,%hhu", &hours, &mins, &secs) != 3) {  // %hhu is unsigned char
		UART_show_syntax_error();
		return;
	}
	
	if ((secs > 59) || (mins > 59) || (hours > 23)) {
		UART_TX("Error: invalid time format!\r");
		return;
	}
	else {
		i2c_tx_data[0] = 0x00; // Set DS3231 address pointer at time register
		i2c_tx_data[1] = (secs % 10) | ((secs / 10) << 4);
		i2c_tx_data[2] = (mins % 10) | ((mins / 10) << 4);
		i2c_tx_data[3] = (hours % 10) | ((hours / 10) << 4);
		I2C_Write (0x68, i2c_tx_data, 4, true);
		
		sprintf(uart_tx_data, "Time set: %02d:%02d:%02d\r", hours, mins, secs);
		UART_TX(uart_tx_data);
	}
}

/* Sets date on DS3231. Syntax: @DSWD=dd,mm,yy */
void DS3231_set_date(char *args)
{
	uint8_t days_in_months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	uint8_t day, month, year;
	
	if (sscanf(args, "=%hhu,%hhu,%hhu", &day, &month, &year) != 3) {  // %hhu is unsigned char
		UART_show_syntax_error();
		return;
	}
	else {
		if ((year == 0) || (year % 4 == 0)) // Check for leap year
			days_in_months[1] = 29;
		
		if ((day > days_in_months[month - 1]) || (day == 0) || (month == 0) || (month > 12)) {
			UART_TX("Error: invalid date!\r");
			return;
		}
		else {
			i2c_tx_data[0] = 0x04; // Set DS3231 address pointer at date register
			i2c_tx_data[1] = (day % 10) | ((day / 10) << 4);
			i2c_tx_data[2] = (month % 10) | ((month / 10) << 4);
			i2c_tx_data[3] = (year % 10) | ((year / 10) << 4);
			I2C_Write (0x68, i2c_tx_data, 4, true);
			
			sprintf(uart_tx_data, "Date set: %02d/%02d/%02d\r", day, month, year);
			UART_TX(uart_tx_data);
		}
	}
}

/* Set square wave output on pins SQW and/or 32K. Syntax: @DSSQ=a,b */
void DS3231_set_clk_out(char *args)
{
	uint8_t sqw_mode, en32k_mode;
	
	if (sscanf(args, "=%hhu,%hhu", &sqw_mode, &en32k_mode) != 2) {  // %hhu is unsigned char
		UART_show_syntax_error();
		return;
	}
	
	else {
		i2c_tx_data[0] = 0x0E; // Set DS3231 address pointer at control register
		
		switch (sqw_mode) {
			case 0:
				i2c_tx_data[1] = 0x00;
				I2C_Write (0x68, i2c_tx_data, 2, true);
				UART_TX("SQW output = 1 Hz\r");
				break;
			case 1:
				i2c_tx_data[1] = 0x08;
				I2C_Write (0x68, i2c_tx_data, 2, true);
				UART_TX("SQW output = 1.024 kHz\r");
				break;
			case 2:
				i2c_tx_data[1] = 0x10;
				I2C_Write (0x68, i2c_tx_data, 2, true);
				UART_TX("SQW output = 4.096 kHz\r");
				break;
			case 3:
				i2c_tx_data[1] = 0x18;
				I2C_Write (0x68, i2c_tx_data, 2, true);
				UART_TX("SQW output = 8.192 kHz\r");
				break;
			default:
				UART_TX("Error: SQW must be in range 0..3\r");
		}
		
		i2c_tx_data[0] = 0x0F; // Set DS3231 address pointer at control/status register
		if (en32k_mode == 0) {
			i2c_tx_data[1] = 0x00;
			I2C_Write (0x68, i2c_tx_data, 2, true);
			UART_TX("32K output disabled\r");
		}
		else if (en32k_mode == 1) {
			i2c_tx_data[1] = 0x08;
			I2C_Write (0x68, i2c_tx_data, 2, true);
			UART_TX("32K output enabled\r");
		}
		else {
			UART_TX("Error: 32K must be in range 0..1\r");
		}
	}
}

/* Write 1 byte to AT24C32. Syntax: @ATWD=addr,data */
void AT24C32_write_data(char *args)
{
	uint16_t address;
	uint8_t  data;
	
	if (sscanf(args, "=0x%hx,0x%hhx", &address, &data) != 2) {  
		UART_show_syntax_error();
		return;
	}
	
	if (address > 0xFFF) {
		UART_TX("Error: Address unavailable. Maximum address - 0xFFF\r");
		return;
	}
	else {
		i2c_tx_data[0] = (address >> 8) & 0xF;
		i2c_tx_data[1] = address & 0xFF;
		i2c_tx_data[2] = data;
		I2C_Write (AT24C32_ADDRESS, i2c_tx_data, 3, true);
		UART_TX("Byte written\r");
	}
}

/* Read bytes from AT24C32. Syntax: @ATRD=addr,amount */
void AT24C32_read_data(char *args)
{
	uint16_t address;
	uint16_t amount;
	
	 /* Syntax check */
	if (sscanf(args, "=0x%hx,%hu", &address, &amount) != 2) {  
		UART_show_syntax_error();
		return;
	}
	
	 /* Range check */
	if (address > 0xFFF) {      
		UART_TX("Error: Address unavailable. Maximum address - 0xFFF\r");
		return;
	}
	else if ((address + amount) > 0x1000) {
		UART_TX("Error: Cannot fetch this much data\r");
		return;
	}
	
	 /* Read data */
	else {                        
		i2c_tx_data[0] = (address >> 8) & 0xF;
		i2c_tx_data[1] = address & 0xFF;
		I2C_Write (AT24C32_ADDRESS, i2c_tx_data, 2, false);
		I2C_Read  (AT24C32_ADDRESS, i2c_rx_data, amount, 1000000);
		
		 /* Print fetched data */
		for (uint16_t i = 0; i < amount; i++) {
			sprintf(uart_tx_data, "0x%02X ", i2c_rx_data[i]);
			UART_TX(uart_tx_data);
		}
		UART_TX("\r");
	}
}
