#include <stdbool.h>
#include "i2c.h"
#include "stm_defines.h"
#include "stm32f103xb.h"
#include "consts.h"

#define TWSR_MASK 0xfc
#define I2C_100KHZ_COEFF (5u)
#define I2C_IE ((I2C_CR2_ITERREN * 1) | (I2C_CR2_ITEVTEN * 1) | (I2C_CR2_ITBUFEN * 1))

static volatile uint8_t i2c_buf[I2C_BUFFER_SIZE];
static volatile uint8_t i2c_state = 0;
static volatile uint8_t i2c_msg_size;

static volatile bool is_tx = false;

/*предделители для установки скорости обмена twi модуля*/
uint8_t pre[4] = {2, 8, 32, 128};

/****************************************************************************
 Init
****************************************************************************/
uint8_t i2c_master_init(uint16_t freq)
{
    I2C1->CR2 = ((freq << I2C_CR2_FREQ_Pos) & I2C_CR2_FREQ_Msk) |
                I2C_IE |
                (I2C_CR2_DMAEN * 0) |
                (I2C_CR2_LAST * 0);

    I2C1->CCR = (I2C_CCR_FS * 0) |
                (I2C_CCR_DUTY * 0) |
                (((freq * I2C_100KHZ_COEFF) << I2C_CCR_CCR_Pos) & I2C_CCR_CCR_Msk);

    I2C1->TRISE = freq + 1;

    // I2C1->CR1 |= I2C_CR1_PE;

    return 0;
}

/****************************************************************************
 Проверка - не занят ли I2C модуль. Используется внутри модуля
****************************************************************************/
static uint8_t i2c_transceiver_busy(void)
{
    return (I2C1->CR2 & I2C_IE) ? 0x01 : 0x00;
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
    }
    else
    {
        is_tx = false;
    }

    // разрешаем прерывание и формируем состояние старт
    i2c_state = I2C_NO_STATE;
    I2C1->CR2 |= I2C_IE;
    I2C1->CR1 |= I2C_CR1_PE;
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

/****************************************************************************
 Обработчик прерывания I2C модуля
****************************************************************************/
void irq_i2c_handler(void)
{
    static uint8_t ptr;
    const uint32_t stat_1 = I2C1->SR1;
    // const uint32_t stat_2 = I2C1->SR2;

    if (stat_1 & I2C_SR1_SB)
    {
        ptr = 0;
    } 
    else if ((stat_1 & I2C_SR1_ADDR) || (stat_1 & I2C_SR1_BTF))
    {
        if (is_tx)
        {
            if (ptr < i2c_msg_size)
            {
                I2C1->DR = i2c_buf[ptr];   // загружаем в регистр данных следующий байт
                ptr++;
            }
            else
            {
                i2c_state = I2C_SUCCESS;
                I2C1->CR1 |= I2C_CR1_STOP;
                I2C1->CR2 &= ~I2C_IE;
            }
        }
    }
    else if (stat_1 & I2C_SR1_RXNE)
    {
        i2c_buf[ptr] = I2C1->DR & C_BYTE_MASK;
        ptr++;
        if (ptr < i2c_msg_size)
        {
            I2C1->CR1 |= I2C_CR1_STOP;
        }
    }
    else if (stat_1 &  I2C_SR1_ARLO)
    {

    }
    else
    {
        I2C1->CR2 &= ~I2C_IE;
    }
}
