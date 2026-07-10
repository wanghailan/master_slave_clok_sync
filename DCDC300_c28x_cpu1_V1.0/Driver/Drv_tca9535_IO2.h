/*
 * Drv_TCA9535_IO2.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRV_TCA9535_IO2_H_
#define DRV_TCA9535_IO2_H_


int16_t  tca9535_io2_init(void);
int16_t  tca9535_io2_p0Set(int16_t _reg,int16_t _set);
int16_t  tca9535_io2_p1Set(int16_t _reg,int16_t _set);
int16_t  tca9535_io2_p0Get(int16_t _reg);
int16_t  tca9535_io2_p1Get(int16_t _reg);

#endif /* DRV_TCA9535_IO2_H_ */
