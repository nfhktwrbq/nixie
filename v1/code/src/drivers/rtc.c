#include "stm_defines.h"
#include "sys_init.h"
#include "consts.h"


void rtc_init(void) 
{
    // Wait for last write operation to complete
    while (!(RTC->CRL & RTC_CRL_RTOFF)); // Wait for RTOFF bit to set

    // Disable write protection
    RTC->CRL |= RTC_CRL_CNF;

    const uint32_t prescaler = cc_rtc_clk_hz_get() - 1;

    const uint32_t cur_prescaler = ((RTC->DIVH & 0xF) << 16) | RTC->DIVL;
    if (cur_prescaler != prescaler)
    {
        RTC->PRLL = (prescaler & C_TWO_BYTE_MASK);
        RTC->PRLH = ((prescaler >> 16) & 0xF);
    }

    // Enable write protection
    RTC->CRL &= ~RTC_CRL_CNF;

    // Wait for last write operation to complete
    while (!(RTC->CRL & RTC_CRL_RTOFF)); // Wait for RTOFF bit to set
}

void rtc_datetime_set(uint32_t timestamp) 
{
    // Wait for last write operation to complete
    while (!(RTC->CRL & RTC_CRL_RTOFF)); // Wait for RTOFF bit to set

    // Disable write protection
    RTC->CRL |= RTC_CRL_CNF;

    // Set the RTC counter value
    RTC->CNTH = (timestamp >> 16) & 0xFFFF;
    RTC->CNTL = timestamp & 0xFFFF;

    // Enable write protection
    RTC->CRL &= ~RTC_CRL_CNF;

    // Wait for last write operation to complete
    while (!(RTC->CRL & RTC_CRL_RTOFF)); // Wait for RTOFF bit to set
}

uint32_t rtc_datetime_get(void) 
{
    uint32_t time;

    // Wait for RTC registers synchronization
    RTC->CRL &= ~RTC_CRL_RSF;
    while (!(RTC->CRL & RTC_CRL_RSF)); // Wait for RSF bit to set

    // Read the RTC counter value
    time = (RTC->CNTH << 16) | RTC->CNTL;

    return time;
}


