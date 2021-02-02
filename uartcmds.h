#ifndef UARTCMDS_H_INCLUDED
#define UARTCMDS_H_INCLUDED

#define UART_CMD_LENGTH 4

void UART_parse_cmd(char *UART_data);
void UART_show_syntax_error(void);

void UART_send_help(char *args);
void DS3231_read_time(char *args);
void DS3231_read_date(char *args);
void DS3231_read_temp(char *args);
void DS3231_set_time(char *args);
void DS3231_set_date(char *args);
void DS3231_set_clk_out(char *args);
void AT24C32_write_data(char *args);
void AT24C32_read_data(char *args);

typedef struct {
	char cmd_name[UART_CMD_LENGTH + 1];
	void (*func_ptr)(char *args);
} UART_cmd_func;

#endif
