#include "uart.h"

bool uart_cmd_received = false;
char uart_rx_data[UART_RX_BUF_SIZE];
char uart_tx_data[UART_TX_BUF_SIZE];

/* UART init */
void UART_Init (void)
{
	/* GPIO setup */
	RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->AFR[0] |= (7U << GPIO_AFRL_AFSEL2_Pos)     // USART2_TX for PB2
	                |(7U << GPIO_AFRL_AFSEL3_Pos);    // USART2_RX for PB3
	GPIOA->MODER  |= (0b10 << GPIO_MODER_MODER2_Pos)  // GPIO alt. function for PB2
	                |(0b10 << GPIO_MODER_MODER3_Pos); // GPIO alt. function for PB3
	/* UART setup */
	RCC->APB1ENR  |= RCC_APB1ENR_USART2EN;          // Enable USART2 clocking
	USART2->CR1   |= USART_CR1_UE                   // Enable USART2 module
	               | USART_CR1_M                    // 9 bits word length (8b data + 1b parity)
	               | USART_CR1_PS                   // Odd parity
	               | USART_CR1_PCE;                 // Enable parity control
	USART2->CR2   |= 0b10 << USART_CR2_STOP_Pos;    // 2 stop-bits
	USART2->BRR    = 0x8B;                          // 115200 baud (115108 actual)
																							    
	USART2->CR1   |= USART_CR1_TE;                  // Enable UART transmitter
	while (!(USART2->SR & USART_SR_TXE)) {}         // Wait while it sends an idle frame
																							    
	USART2->CR1   |= USART_CR1_RE                   // Enable UART receiver
	               | USART_CR1_RXNEIE;              // Enable interrupt on data reception
	NVIC_EnableIRQ(USART2_IRQn);                    // Allow interrupts from USART2
}

/* UART byte transmit function */
void UART_TX (char tx_data[])
{
	uint8_t i = 0;
	
	while ((tx_data[i] != '\0') && (i < UART_TX_BUF_SIZE))
	{
		USART2->DR = tx_data[i];
		++i;
		while (!(USART2->SR & USART_SR_TXE)) {}  
	}
}

/* UART received data handler */
void USART2_IRQHandler (void)
{
	static volatile uint8_t position = 0;
	
	if (USART2->SR & USART_SR_RXNE)
	{
		/* Reset the RX string if new command is being received 
		   or if rx string overflow is detected                 */
		if (uart_cmd_received || (position > (UART_TX_BUF_SIZE - 1))) {
			uart_cmd_received = false;
			position = 0;
			memset(uart_rx_data, 0, UART_RX_BUF_SIZE);
		}
		
		/* Read the command and check for end symbol */
		uart_rx_data[position] = USART2->DR;
		
		if (uart_rx_data[position] == '\r') {
			uart_cmd_received = true;
			position = 0;
			return;
		}
		
		++position;
	}
}
