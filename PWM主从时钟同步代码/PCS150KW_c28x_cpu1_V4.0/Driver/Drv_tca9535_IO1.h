/*
 * Drv_TCA9535_IO1.h
 *
 *  Created on: 2024年9月11日
 *      Author: guowei680
 */

#ifndef DRV_TCA9535_IO1_H_
#define DRV_TCA9535_IO1_H_


int16_t  tca9535_io1_init(void);
int16_t  tca9535_io1_p0Set(int16_t _reg,int16_t _set);
int16_t  tca9535_io1_p1Set(int16_t _reg,int16_t _set);
int16_t  tca9535_io1_p0Get(int16_t _reg);
int16_t  tca9535_io1_p1Get(int16_t _reg);

#endif /* DRV_TCA9535_IO1_H_ */
