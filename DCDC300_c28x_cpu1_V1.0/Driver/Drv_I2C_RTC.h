/*
 * Drv_I2C_RTC.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_I2C_RTC_H_
#define DRIVER_DRV_I2C_RTC_H_

typedef struct{
    uint8_t     seconds;
    uint8_t     minutes;
    uint8_t     hours;
    uint8_t     day;
    uint8_t     month;
    uint8_t     year;
    uint8_t     dweek;
}RtcTime;

extern RtcTime Str_RtcTime;

void rtc_init(void);
void rtc_write_time(int16_t regAddr,int16_t regData);
void rtc_read_time(void);

#endif /* DRIVER_DRV_I2C_RTC_H_ */
