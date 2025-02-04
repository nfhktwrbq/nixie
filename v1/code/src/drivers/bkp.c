
/// \brief
/// Explanation of BKP Data Registers
/// BKP->DR[registerNumber]: The BKP data registers are accessed as an array of 16-bit values. 
/// On the STM32F103C8T6, there are 10 BKP data registers (BKP_DR1 to BKP_DR10).
/// 
/// Backup Domain: The BKP registers are part of the backup domain, which means they 
/// retain their values when the main power is lost (if VBAT is connected to a battery).
/// 
/// Notes
/// Battery Backup: To retain BKP data during a power loss, ensure that the VBAT pin is 
/// connected to a backup battery (e.g., a coin cell battery).
/// 
/// Tamper Detection: The BKP registers can also be used in conjunction with the tamper 
/// detection feature. If tamper detection is enabled, the BKP registers are cleared 
/// when a tamper event occurs.
/// 
/// Register Range: On the STM32F103C8T6, only BKP_DR1 to BKP_DR10 are available. 
/// Other STM32 microcontrollers may have more BKP data registers.*/

#include "stm_defines.h"


void bkp_data_write(uint8_t registerNumber, uint16_t data) 
{
    // Ensure the register number is valid (BKP_DR1 to BKP_DR10 on STM32F103C8T6)
    if (registerNumber < 1 || registerNumber > 10) 
    {
        return; // Invalid register number
    }

    // Write data to the specified BKP data register
    volatile uint32_t * dr = &(BKP->DR1);
    dr[registerNumber] = data;
}

uint16_t bkp_data_read(uint8_t registerNumber) 
{
    // Ensure the register number is valid (BKP_DR1 to BKP_DR10 on STM32F103C8T6)
    if (registerNumber < 1 || registerNumber > 10) 
    {
        return 0; // Invalid register number
    }

    // Read data from the specified BKP data register
    volatile uint32_t * dr = &(BKP->DR1);
    return (uint16_t)dr[registerNumber];
}


