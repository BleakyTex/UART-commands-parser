#include "i2c.h"

uint8_t i2c_rx_data[I2C_RX_BUF_SIZE];
uint8_t i2c_tx_data[I2C_TX_BUF_SIZE];

/* I2C init*/
void I2C_Init (void)
{
	/* GPIO setup */
	RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->OTYPER |= GPIO_OTYPER_OT7 | GPIO_OTYPER_OT8; // Open drain I2C pins
	GPIOB->AFR[0] |= (4U << GPIO_AFRL_AFSEL7_Pos);      // AF4 - I2C1_SDA
	GPIOB->AFR[1] |= (4U << GPIO_AFRH_AFSEL8_Pos);      // AF4 - I2C1_SCL
	GPIOB->MODER  |= (0b10 << GPIO_MODER_MODE7_Pos)     // Enable alt. function for PB7
	               | (0b10 << GPIO_MODER_MODE8_Pos);    // Same for PB8
	
	/* I2C setup */
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;       // Enable I2C clocking
	I2C1->CR2     =  16U << I2C_CR2_FREQ_Pos; // Specify 16 MHz clock freq
	I2C1->CCR    |= (20U << I2C_CCR_CCR_Pos)  // 400 kHz SCL freq 
	                |I2C_CCR_FS;              // Enable fast mode
	I2C1->TRISE   = 5U;                       // 250 ns rise time
	I2C1->CR1    |= I2C_CR1_PE;               // Enable I2C module
}

/* I2C data transmit function */
void I2C_Write (uint8_t  addr, uint8_t  *txdata,
                uint16_t amount, bool send_stopBit)
{
	if (amount == 0)
		return;
	
	I2C1->CR1 |= I2C_CR1_ACK;                 // Enable ACK response
	
	I2C1->CR1 |= I2C_CR1_START;               // Send start bit
	while (!(I2C1->SR1 & I2C_SR1_SB));        // Wait while its sent
	(void) I2C1->SR1;                         // Read SR1 register to reset SB flag
																			      
	I2C1->DR = addr << 1;                     // Send the address with R/W = 0
	while (!(I2C1->SR1 & I2C_SR1_ADDR));      // Wait while its sent
	(void) I2C1->SR1;                         // Read SR1 register...
	(void) I2C1->SR2;                         // ...then read SR2 register to reset ADDR flag
	
	for (uint16_t i = 0; i < amount; i++)     // Send [amount] of bytes
	{
		I2C1->DR = txdata[i];                   // Send byte
		while (!(I2C1->SR1 & I2C_SR1_TXE));     // Wait until byte is transmitted
	}
	
	while (!(I2C1->SR1 & I2C_SR1_BTF));       // Wait while last byte is transmitted
	
	// Send the stop bit if we're writing data, not requesting read from registers 
	if (send_stopBit) {                      
		I2C1->CR1 |= I2C_CR1_STOP;              // Send stop-bit
		while (!(I2C1->SR2 & I2C_SR2_MSL));     // Wait for I2C module to turn off
	}
}

/* I2C data receive function */
void I2C_Read (uint8_t addr,  uint8_t *rxdata,
               uint16_t amount, uint32_t timeout)
{
	I2C1->CR1 &= ~I2C_CR1_ACK; // Disable ACK response
	I2C1->CR1 &= ~I2C_CR1_POS; // Disable POS bit
	
	if (amount == 0)
		return;
	
	else if (amount == 1) {
		I2C1->CR1 |= I2C_CR1_ACK;            // Enable ACK response
																			   
		I2C1->CR1 |= I2C_CR1_START;          // Send start bit
		while (!(I2C1->SR1 & I2C_SR1_SB));   // Wait while its sent
		(void) I2C1->SR1;                    // Read SR1 register to reset SB flag
		
		I2C1->DR = (addr << 1) | (1U);       // Send the address with R/W = 1
		while (!(I2C1->SR1 & I2C_SR1_ADDR)); // Wait while its sent
		
		I2C1->CR1 &= ~I2C_CR1_ACK;           // Disable ACK response
		
		(void) I2C1->SR1;                    // Read SR1 register...
		(void) I2C1->SR2;                    // ...then read SR2 register to reset ADDR flag
		
		while (!(I2C1->SR1 & I2C_SR1_RXNE)); // Wait for data to be received
		rxdata[0] = I2C1->DR;                // Read the byte
		
		I2C1->CR1 |= I2C_CR1_STOP;           // Send stop-bit
		while (!(I2C1->SR2 & I2C_SR2_MSL));  // Wait for I2C module to turn off
	}
	
	else if (amount == 2) {
		I2C1->CR1 |= I2C_CR1_ACK;            // Enable ACK response
																			   
		I2C1->CR1 |= I2C_CR1_START;          // Send start bit
		while (!(I2C1->SR1 & I2C_SR1_SB));   // Wait while its sent
		(void) I2C1->SR1;                    // Read SR1 register to reset SB flag
		
		I2C1->DR = (addr << 1) | (1U);       // Send the address with R/W = 1
		while (!(I2C1->SR1 & I2C_SR1_ADDR)); // Wait while its sent
		
		I2C1->CR1 &= ~I2C_CR1_ACK;           // Disable ACK response
		I2C1->CR1 |=  I2C_CR1_POS;           // Enable POS bit
		
		(void) I2C1->SR1;                    // Read SR1 register...
		(void) I2C1->SR2;                    // ...then read SR2 register to reset ADDR flag
		
		while (!(I2C1->SR1 & I2C_SR1_BTF));  // Wait while BTF = 1
		
		I2C1->CR1 |= I2C_CR1_STOP;           // Send stop-bit
		while (!(I2C1->SR2 & I2C_SR2_MSL));  // Wait for I2C module to turn off
		
		rxdata[0] = I2C1->DR;                // Read first and second byte
		rxdata[1] = I2C1->DR; 
	}
	
	else {
		I2C1->CR1 |= I2C_CR1_ACK;               // Enable ACK response
		
		I2C1->CR1 |= I2C_CR1_START;             // Send start bit
		while (!(I2C1->SR1 & I2C_SR1_SB));      // Wait while its sent															      
		(void) I2C1->SR1;                       // Read SR1 register to reset SB flag
																			      
		I2C1->DR = (addr << 1) | (1U);          // Send the address with R/W = 1
		while (!(I2C1->SR1 & I2C_SR1_ADDR));    // Wait while its sent
		(void) I2C1->SR1;                       // Read SR1 register...
		(void) I2C1->SR2;                       // ...then read SR2 register to reset ADDR flag
		
		for (uint16_t i = 0; i < amount; i++)   // Receive [amount] of bytes
		{
			if (i < (amount - 1)) {             
			while (!(I2C1->SR1 & I2C_SR1_RXNE));  // Wait for data to be received
			rxdata[i] = I2C1->DR;                 // Store data into array  
			}		
			
			else {                                // If second last data is read...
				I2C1->CR1 &= ~I2C_CR1_ACK;          // ...send NACK
				I2C1->CR1 |= I2C_CR1_STOP;          // Send stop-bit
				
				while (!(I2C1->SR1 & I2C_SR1_RXNE));// Wait for the last byte
				rxdata[i] = I2C1->DR;               // Store it into array  
			}
		}
	}
}
