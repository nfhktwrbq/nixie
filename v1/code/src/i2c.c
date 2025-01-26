#include <stdbool.h>
#include "i2c.h"
#include "stm_defines.h"
#include "stm32f103xb.h"
#include "consts.h"
#include "sys_init.h"

#define TWSR_MASK 0xfc
#define I2C_100KHZ_COEFF (5u)
#define I2C_IE ((I2C_CR2_ITERREN * 1) | (I2C_CR2_ITEVTEN * 1) | (I2C_CR2_ITBUFEN * 1))
#define DEFAULT_ADDR (0x11)

#define I2C_FLAG_OVR ((I2C_SR2_MSL << 16) | I2C_SR1_OVR)
#define I2C_FLAG_AF ((I2C_SR2_MSL << 16) | I2C_SR1_AF)
#define I2C_FLAG_ARLO ((I2C_SR2_MSL << 16) | I2C_SR1_ARLO)
#define I2C_FLAG_BERR ((I2C_SR2_MSL << 16) | I2C_SR1_BERR)
#define I2C_FLAG_TXE ((I2C_SR2_MSL << 16) | I2C_SR1_TXE)
#define I2C_FLAG_RXNE ((I2C_SR2_MSL << 16) | I2C_SR1_RXNE)
#define I2C_FLAG_STOPF ((I2C_SR2_MSL << 16) | I2C_SR1_STOPF)
#define I2C_FLAG_ADD10 ((I2C_SR2_MSL << 16) | I2C_SR1_ADD10)
#define I2C_FLAG_BTF ((I2C_SR2_MSL << 16) | I2C_SR1_BTF)
#define I2C_FLAG_ADDR ((I2C_SR2_MSL << 16) | I2C_SR1_ADDR)
#define I2C_FLAG_SB ((I2C_SR2_MSL << 16) | I2C_SR1_SB)
#define I2C_FLAG_DUALF (0x00100000 | I2C_SR2_DUALF)
#define I2C_FLAG_GENCALL (0x00100000 | I2C_SR2_GENCALL)
#define I2C_FLAG_TRA (0x00100000 | I2C_SR2_TRA)
#define I2C_FLAG_BUSY (0x00100000 | I2C_SR2_BUSY)
#define I2C_FLAG_MSL (0x00100000 | I2C_SR2_MSL)

typedef enum i2s_state_e
{
    I2C_STATE_NONE,
    I2C_STATE_RESET,
    I2C_STATE_READY,
    I2C_STATE_BUSY,
    I2C_STATE_BUSY_TX,
    I2C_STATE_BUSY_RX,
    I2C_STATE_LISTEN,
    I2C_STATE_BUSY_TX_LISTEN,
    I2C_STATE_BUSY_RX_LISTEN,
    I2C_STATE_ABORT,
    I2C_STATE_TIMEOUT,
    I2C_STATE_ERROR,
} i2s_state_e;

static volatile uint8_t i2c_buf[I2C_BUFFER_SIZE];
static volatile uint8_t i2c_msg_size;
static volatile i2s_state_e i2c_state = 0;
static volatile i2s_state_e i2c_prev_state = 0;

static volatile bool is_tx = false;

/*предделители для установки скорости обмена twi модуля*/
uint8_t pre[4] = {2, 8, 32, 128};

/****************************************************************************
 Init
****************************************************************************/
uint8_t i2c_master_init(uint16_t freq)
{
    I2C1->CR1 &= ~I2C_CR1_PE;

    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    I2C1->CR2 = ((freq << I2C_CR2_FREQ_Pos) & I2C_CR2_FREQ_Msk) |
                (I2C_CR2_DMAEN * 0) |
                (I2C_CR2_LAST * 0);

    I2C1->CCR = (I2C_CCR_FS * 0) |
                (I2C_CCR_DUTY * 0) |
                (((freq * I2C_100KHZ_COEFF) << I2C_CCR_CCR_Pos) & I2C_CCR_CCR_Msk);

    I2C1->TRISE = freq + 1;

    I2C1->OAR1 = 0x11;
    I2C1->OAR2 = DEFAULT_ADDR;

    I2C1->CR1 |= I2C_CR1_PE;

    return 0;
}

/****************************************************************************
 Проверка - не занят ли I2C модуль. Используется внутри модуля
****************************************************************************/
static uint8_t i2c_transceiver_busy(void)
{
    return i2c_state == I2C_STATE_READY;
}

/****************************************************************************
 Взять статус I2C модуля
****************************************************************************/
uint8_t i2c_get_state(void)
{
    while (i2c_transceiver_busy())
        ;
    return i2c_state;
}

/****************************************************************************
 Передать сообщение msg из msg_size байтов на I2C шину
****************************************************************************/
void i2c_send_data(uint8_t *msg, uint8_t msg_size)
{
    uint8_t i;

    while (i2c_transceiver_busy())
        ; // ждем, когда I2C модуль освободится

    i2c_msg_size = msg_size; // сохряняем кол. байт для передачи
    i2c_buf[0] = msg[0];     // и первый байт сообщения

    if (!(msg[0] & (1 << I2C_READ_BIT)))
    { // если первый байт типа SLA+W
        for (i = 1; i < msg_size; i++)
        { // то сохряняем остальную часть сообщения
            i2c_buf[i] = msg[i];
        }
        is_tx = true;
        i2c_state = I2C_STATE_BUSY_TX;
    }
    else
    {
        is_tx = false;
        I2C1->CR1 |= I2C_CR1_ACK;
        i2c_state = I2C_STATE_BUSY_RX;
    }

    // разрешаем прерывание и формируем состояние старт
    I2C1->CR1 &= ~I2C_CR1_POS;
    I2C1->CR2 |= I2C_IE;
    I2C1->CR1 |= I2C_CR1_START;
}

/****************************************************************************
 Переписать полученные данные в буфер msg в количестве msg_size байт.
****************************************************************************/
uint8_t i2c_get_data(uint8_t *msg, uint8_t msg_size)
{
    uint8_t i;

    while (i2c_transceiver_busy())
        ; // ждем, когда I2C модуль освободится

    if (i2c_state == I2C_SUCCESS) // если сообщение успешно принято,
    {
        for (i = 0; i < msg_size; i++) // то переписываем его из внутреннего буфера в переданный
        {
            msg[i] = i2c_buf[i];
        }
    }

    return i2c_state;
}

static void master_addr(void)
{
    I2C1->SR1 &= ~I2C_SR1_ADDR;
    if (is_tx)
    {
    }
    else
    {
        if (i2c_msg_size == 1)
        {
            I2C1->CR1 |= I2C_CR1_STOP;
        }
        else if (i2c_msg_size == 2)
        {
            I2C1->CR1 &= ~I2C_CR1_ACK;
            I2C1->CR1 |= I2C_CR1_STOP;
        }
        else if (i2c_msg_size == 3)
        {
            I2C1->CR1 |= I2C_CR1_POS;
            I2C1->CR1 &= ~I2C_CR1_ACK;
        }
        else
        {
            I2C1->CR1 |= I2C_CR1_ACK;
        }
    }
}

static void master_tx_txe(uint32_t *ptr)
{
    const i2s_state_e cur_state = i2c_state;

    if ((i2c_msg_size == 1u) && (cur_state == I2C_STATE_BUSY_TX))
    {
        /* Disable EVT, BUF and ERR interrupt */
        I2C1->CR2 &= ~I2C_IE;

        /* Generate Stop */
        I2C1->CR1 |= I2C_CR1_STOP;

        i2c_prev_state = I2C_STATE_NONE;
        i2c_state = I2C_STATE_READY;
    }
    else if (cur_state == I2C_STATE_BUSY_TX)
    {
        if (*ptr == (i2c_msg_size - 1u))
        {
            /* Disable BUF interrupt */
            I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
        }
        else
        {
            /* Write data to DR */
            I2C1->DR = i2c_buf[*ptr];

            /* Update counter */
            (*ptr)++;
        }
    }
    else
    {
        /* Do nothing */
    }
}

static void master_tx_btf(uint32_t *ptr)
{
    if (i2c_state == I2C_STATE_BUSY_TX)
    {
        if (*ptr < i2c_msg_size)
        {
            /* Write data to DR */
            I2C1->DR = i2c_buf[*ptr];

            /* Increment Buffer pointer */
            (*ptr)++;
        }
        else
        {
            I2C1->CR2 &= ~I2C_IE;
            I2C1->CR1 |= I2C_CR1_STOP;
            i2c_prev_state = I2C_STATE_NONE;
            i2c_state = I2C_STATE_READY;
        }
    }
    else
    {
        /* Do nothing */
    }
}

static void master_rx_rxne(uint32_t * ptr)
{
    if (i2c_state == I2C_STATE_BUSY_RX)
    {
        uint32_t tmp = i2c_msg_size - *ptr;

        if (tmp > 3u)
        {
            /* Read data from DR */
            i2c_buf[*ptr] = I2C1->DR & C_BYTE_MASK;

            /* Increment Buffer pointer */
            (*ptr)++;

            if ((i2c_msg_size - *ptr) == 3u)
            {
                /* Disable BUF interrupt, this help to treat correctly the last 4 bytes
                on BTF subroutine */
                /* Disable BUF interrupt */
                I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
            }
        }
        else if ((tmp == 1u) || (tmp == 0u))
        {
            // if (I2C_WaitOnSTOPRequestThroughIT(hi2c) == HAL_OK)
            // {
            //     /* Disable Acknowledge */
            //     I2C1->CR1 &= ~I2C_CR1_ACK;

            //     /* Disable EVT, BUF and ERR interrupt */
            //     I2C1->CR2 &= ~I2C_IE;

            //     /* Read data from DR */
            //     i2c_buf[*ptr] = I2C1->DR & C_BYTE_MASK;

            //     /* Increment Buffer pointer */
            //     (*ptr)++;

            //     i2c_state = I2C_STATE_READY;
            //     i2c_prev_state = I2C_STATE_BUSY_RX;
            // }
            // else
            {
                /* Disable EVT, BUF and ERR interrupt */
                I2C1->CR2 &= ~I2C_IE;

                //* Read data from DR */
                i2c_buf[*ptr] = I2C1->DR & C_BYTE_MASK;

                /* Increment Buffer pointer */
                (*ptr)++;

                i2c_state = I2C_STATE_READY;
            }
        }
        else
        {
            /* Disable BUF interrupt, this help to treat correctly the last 2 bytes
               on BTF subroutine if there is a reception delay between N-1 and N byte */
            I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
        }
    }
}

static void master_rx_btf(uint32_t * ptr)
{
  if ((i2c_msg_size - *ptr) == 4U)
  {
    /* Disable BUF interrupt, this help to treat correctly the last 2 bytes
       on BTF subroutine if there is a reception delay between N-1 and N byte */
    I2C1->CR2 &= ~I2C_CR2_ITBUFEN;

    //* Read data from DR */
    i2c_buf[*ptr] = I2C1->DR & C_BYTE_MASK;

    /* Increment Buffer pointer */
    (*ptr)++;
  }
  else if ((i2c_msg_size - *ptr) == 3U)
  {
    /* Disable BUF interrupt, this help to treat correctly the last 2 bytes
       on BTF subroutine if there is a reception delay between N-1 and N byte */
    I2C1->CR2 &= ~I2C_CR2_ITBUFEN;

    /* Disable Acknowledge */
    I2C1->CR1 &= I2C_CR1_ACK;

    //* Read data from DR */
    i2c_buf[*ptr] = I2C1->DR & C_BYTE_MASK;

    /* Increment Buffer pointer */
    (*ptr)++;
  }
  else if ((i2c_msg_size - *ptr) == 2U)
  {

    /* Generate Stop */
    I2C1->CR1 |= I2C_CR1_STOP;

    //* Read data from DR */
    i2c_buf[*ptr] = I2C1->DR & C_BYTE_MASK;

    /* Increment Buffer pointer */
    (*ptr)++;

    //* Read data from DR */
    i2c_buf[*ptr] = I2C1->DR & C_BYTE_MASK;

    /* Increment Buffer pointer */
    (*ptr)++;

    /* Disable EVT and ERR interrupt */
    I2C1->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);

    i2c_state = I2C_STATE_READY;
    i2c_prev_state = I2C_STATE_BUSY_RX;
  }
  else
  {
    //* Read data from DR */
    i2c_buf[*ptr] = I2C1->DR & C_BYTE_MASK;

    /* Increment Buffer pointer */
    (*ptr)++;
  }
}

/****************************************************************************
 Обработчик прерывания I2C модуля
****************************************************************************/
void i2c_event_handler(void)
{
    static uint32_t ptr;
    const uint32_t stat_1 = I2C1->SR1;
    const uint32_t stat_2 = I2C1->SR2;
    const i2s_state_e cur_state = i2c_state;

    /* Exit IRQ event until Start Bit detected in case of Other frame requested */
    // if (((stat_1 & I2C_FLAG_SB) == 0) && (IS_I2C_TRANSFER_OTHER_OPTIONS_REQUEST(CurrentXferOptions) == 1U))
    // {
    //   return;
    // }

    /* SB Set ----------------------------------------------------------------*/
    if (stat_1 & I2C_FLAG_SB)
    {
        ptr = 0;
        I2C1->DR = i2c_buf[ptr];
        ptr++;
        uart_send_char('s');
    }
    /* ADDR Set --------------------------------------------------------------*/
    else if (stat_1 & I2C_FLAG_ADDR)
    {
        uart_send_char('a');
        master_addr();        
    }
    /* I2C in mode Transmitter -----------------------------------------------*/
    else if (stat_2 & I2C_FLAG_TRA)
    {
        uart_send_char('t');
        /* TXE set and BTF reset -----------------------------------------------*/
        if ((stat_1 & I2C_FLAG_TXE) && ((stat_1 & I2C_FLAG_BTF) == 0))
        {
            uart_send_char('e');
            master_tx_txe(&ptr);
        }
        /* BTF set -------------------------------------------------------------*/
        else if (stat_1 & I2C_FLAG_BTF)
        {
            if (cur_state == I2C_STATE_BUSY_TX)
            {
                uart_send_char('b');
                master_tx_btf(&ptr);
            }
        }
        else
        {
            /* Do nothing */
        }
    }
    else
    {
        uart_send_char('r');
        /* RXNE set and BTF reset -----------------------------------------------*/
        if ((stat_1 & I2C_FLAG_RXNE) && ((stat_1 & I2C_FLAG_BTF) == 0))
        {
            master_rx_rxne(&ptr);
        }
        /* BTF set -------------------------------------------------------------*/
        else if (stat_1 & I2C_FLAG_BTF)
        {
            master_rx_btf(&ptr);
        }
        else
        {
            /* Do nothing */
        }
    }
}

void i2c_error_handler(void)
{
    const uint32_t stat = I2C1->SR1;

    if (stat & I2C_FLAG_BERR)
    {
        I2C1->SR1 &= ~(I2C_SR1_BERR);
        I2C1->CR1 |= I2C_CR1_SWRST;
        uart_send_char('B');
    }

    if (stat & I2C_FLAG_ARLO)
    {
        I2C1->SR1 &= ~(I2C_SR1_ARLO);
        I2C1->CR1 |= I2C_CR1_STOP;
        uart_send_char('L');
    }

    if (stat & I2C_FLAG_AF)
    {
        I2C1->SR1 &= ~(I2C_SR1_AF);
        I2C1->CR1 |= I2C_CR1_STOP;
        uart_send_char('A');
    }

    if (stat & I2C_FLAG_OVR)
    {
        I2C1->SR1 &= ~(I2C_SR1_OVR);
        uart_send_char('O');
    }

    I2C1->CR1 &= ~I2C_CR1_POS;
    I2C1->CR2 &= ~I2C_IE;
    (void)I2C1->DR;
}
