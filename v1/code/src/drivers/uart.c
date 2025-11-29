#include "uart.h"
#include "stm_defines.h"
#include "sys_init.h"

void uart_init(uint32_t baudrate)
{
    const uint32_t usartdiv_mul = 16;
    // Set baud rate
    const uint32_t frac_qty = (1 << USART_BRR_DIV_Mantissa_Pos);
    const uint32_t uart_clk_hz = cc_dbg_uart_clk_hz_get();    
    const uint32_t mantissa = uart_clk_hz / (baudrate * 16);
    uint32_t fraction = ((uart_clk_hz % (usartdiv_mul * baudrate)) * frac_qty) / (usartdiv_mul * baudrate);

    USART1->BRR = (fraction & USART_BRR_DIV_Fraction_Msk) | (mantissa << USART_BRR_DIV_Mantissa_Pos);
    
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

