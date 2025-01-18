#include "i2c.h"
#include "stm_defines.h"

#define TWSR_MASK          0xfc  
#define I2C_100KHZ_COEFF    (5u)
#define I2C_IE      ((I2C_CR2_ITERREN * 1) | (I2C_CR2_ITEVTEN * 1) | (I2C_CR2_ITBUFEN * 1))

volatile static uint8_t i2c_buf[TWI_BUFFER_SIZE];
volatile static uint8_t i2c_state = TWI_NO_STATE;      
volatile static uint8_t i2c_msg_size;       

/*предделители для установки скорости обмена twi модуля*/
uint8_t pre[4] = {2, 8, 32, 128};

/****************************************************************************
 Init
****************************************************************************/
uint8_t i2c_master_init(uint16_t fr)
{
  const uint32_t freq = 36; //36 MHz
  I2C1->CR2 = ((freq << I2C_CR2_FREQ_Pos) & I2C_CR2_FREQ_Msk)|   
              I2C_IE |
              (I2C_CR2_DMAEN   * 0) |
              (I2C_CR2_LAST    * 0);


  I2C1->CCR = (I2C_CCR_FS   * 0) |
              (I2C_CCR_DUTY * 0) |
              (((freq * I2C_100KHZ_COEFF) << I2C_CCR_CCR_Pos)  & I2C_CCR_CCR_Msk);

  I2C1->TRISE = freq + 1;

  // I2C1->CR1 |= I2C_CR1_PE;
  
  return 0;  
}    

/****************************************************************************
 Проверка - не занят ли TWI модуль. Используется внутри модуля
****************************************************************************/
static uint8_t i2c_transceiver_busy(void)
{
  return (I2C1->CR2 & (I2C_IE));                 
}

/****************************************************************************
 Взять статус TWI модуля
****************************************************************************/
uint8_t i2c_get_state(void)
{
  while (i2c_transceiver_busy());             
  return i2c_state;                        
}

/****************************************************************************
 Передать сообщение msg из msg_size байтов на TWI шину
****************************************************************************/
void i2c_send_data(uint8_t * msg, uint8_t msg_size)
{
  uint8_t i;

  while(i2c_transceiver_busy());   //ждем, когда TWI модуль освободится             

  i2c_msg_size = msg_size;           //сохряняем кол. байт для передачи             
  i2c_buf[0]  = msg[0];            //и первый байт сообщения 
  
  if (!(msg[0] & (TRUE<<TWI_READ_BIT)))
  {   //если первый байт типа SLA+W
    for (i = 1; i < msg_size; i++)
    {         //то сохряняем остальную часть сообщения
      i2c_buf[i] = msg[i];
    }
  }

  //разрешаем прерывание и формируем состояние старт                                          
  i2c_state = TWI_NO_STATE;
  I2C1->CR2 |= I2C_IE;
  I2C1->CR1 |= I2C_CR1_PE;
}

/****************************************************************************
 Переписать полученные данные в буфер msg в количестве msg_size байт. 
****************************************************************************/
uint8_t i2c_get_data(uint8_t *msg, uint8_t msg_size)
{
  uint8_t i;

  while(i2c_transceiver_busy());    //ждем, когда TWI модуль освободится 

  if(i2c_state == TWI_SUCCESS) //если сообщение успешно принято, 
  {                             
    for(i = 0; i < msg_size; i++)  //то переписываем его из внутреннего буфера в переданный
    {
      msg[i] = i2c_buf[i];
    }
  }
  
  return i2c_state;                                   
}

/****************************************************************************
 Обработчик прерывания TWI модуля
****************************************************************************/
ISR(TWI_vect)
{
  static uint8_t ptr;
  uint8_t stat = TWSR & TWSR_MASK;
  
  switch (stat){
    
    case TWI_START:                   // состояние START сформировано 
    case TWI_REP_START:               // состояние повторный START сформировано        
       ptr = 0;      

    case TWI_MTX_ADR_ACK:             // был передан пакет SLA+W и получено подтверждение
    case TWI_MTX_DATA_ACK:            // был передан байт данных и получено подтверждение  
       if (ptr < i2c_msg_size){
          TWDR = i2c_buf[ptr];                    //загружаем в регистр данных следующий байт
          TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT); //сбрасываем флаг TWINT    
          ptr++;
       }
       else{
          i2c_state = TWI_SUCCESS;  
          TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO)|(0<<TWIE); //формируем состояние СТОП, сбрасываем флаг, запрещаем прерывания
       }
       break;
     
    case TWI_MRX_DATA_ACK:          //байт данных принят и передано подтверждение  
       i2c_buf[ptr] = TWDR;
       ptr++;
    
    case TWI_MRX_ADR_ACK:           //был передан пакет SLA+R и получено подтвеждение  
      if (ptr < (i2c_msg_size-1)){
        TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);  //если это не предпоследний принятый байт, формируем подтверждение                             
      }
      else {
        TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);            //если приняли предпоследний байт, подтверждение не формируем
      }    
      break; 
      
    case TWI_MRX_DATA_NACK:       //был принят байт данных без подтверждения      
      i2c_buf[ptr] = TWDR;
      i2c_state = TWI_SUCCESS;  
      TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO); //формируем состояние стоп
      break; 
     
    case TWI_ARB_LOST:          //был потерян приоритет 
      TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA); // сбрасываем флаг TWINT, формируем повторный СТАРТ
      break;
      
    case TWI_MTX_ADR_NACK:      // был передан пает SLA+W и не получено подтверждение
    case TWI_MRX_ADR_NACK:      // был передан пакет SLA+R и не получено подтверждение    
    case TWI_MTX_DATA_NACK:     // был передан байт данных и не получено подтверждение
    case TWI_BUS_ERROR:         // ошибка на шине из-за некоректных состояний СТАРТ или СТОП
    default:     
      i2c_state = stat;                                                                                    
      TWCR = (1<<TWEN)|(0<<TWIE)|(0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC); //запретить прерывание                              
  }
}
