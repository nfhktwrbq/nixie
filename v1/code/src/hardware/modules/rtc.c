#include "rtc.h"

#include "stm_defines.h"
#include "sys_init.h"
#include "consts.h"
#include "debug.h"


void rtc_calibration_set(uint32_t cal)
{
    uint32_t bkp_rtcct = BKP->RTCCR;
    const uint32_t cur_cal = (bkp_rtcct & BKP_RTCCR_CAL_Msk) >> BKP_RTCCR_CAL_Pos;
    if (cur_cal != cal)
    {
        bkp_rtcct &= ~BKP_RTCCR_CAL_Msk;
        bkp_rtcct |= (cal << BKP_RTCCR_CAL_Pos) & BKP_RTCCR_CAL_Msk;
        BKP->RTCCR = bkp_rtcct;
    }
}

void rtc_init(void) 
{
    // enable access to the Backup registers and RTC
    PWR->CR |= PWR_CR_DBP;

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

    DBG_INFO("RTC cal = %u\n", (BKP->RTCCR & BKP_RTCCR_CAL_Msk) >> BKP_RTCCR_CAL_Pos);

    // Enable interrupt every second
    // RTC->CRH |= RTC_CRH_SECIE;

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

    DBG_INFO("RTC time is %u", time);

    return time;
}

bool rtc_second_flag_get(void)
{
    while (!(RTC->CRL & RTC_CRL_RTOFF)); // Wait for RTOFF bit to set
    bool flag = (bool)(RTC->CRL & RTC_CRL_SECF);
    if (flag)
    {
        RTC->CRL &= ~RTC_CRL_SECF;
    }
    return flag;
}


