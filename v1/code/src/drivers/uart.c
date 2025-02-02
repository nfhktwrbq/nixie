#include "uart.h"
#include "stm_defines.h"
#include "sys_init.h"

void uart_init(uint32_t baudrate)
{
   // Set baud rate
    USART1->BRR = SYSTEM_CORE_CLOCK_HZ / baudrate / 2; // Assuming SystemCoreClock is set to 72MHz
    /// todo
    // uint32_t mantissa = SYSTEM_CORE_CLOCK_HZ / (baudrate * 8);
    // uint32_t fraction = ((SYSTEM_CORE_CLOCK_HZ * 100 / 8 / baudrate ) % 100) * 16 / 100;
    // USART1->BRR = (fraction & 0xF) | (mantissa << 4);

    // Configure USART1
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE; // Enable Transmitter and Receiver
    USART1->CR1 |= USART_CR1_UE;               // Enable USART1
}

void uart_send_char(char c)
{
    // Wait until the transmit data register is empty
    while (!(USART1->SR & USART_SR_TXE))
        ;
    USART1->DR = c; // Send the character
}

#ifdef DEBUG
void _putchar(char character)
{
    uart_send_char(character);
}
#endif

