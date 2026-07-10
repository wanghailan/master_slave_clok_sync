/*
 * Drv_I2C_RTC.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_I2C_RTC_H_
#define DRIVER_DRV_I2C_RTC_H_

typedef struct{
    uint16_t     seconds;
    uint16_t     minutes;
    uint16_t     hours;
    uint16_t     day;
    uint16_t     month;
    uint16_t     year;
    uint16_t     dweek;
}RtcTime;

extern RtcTime Str_RtcTime;

void rtc_init(void);
void rtc_write_time(int16_t regAddr,int16_t regData);
void rtc_read_time(void);

#endif /* DRIVER_DRV_I2C_RTC_H_ */
