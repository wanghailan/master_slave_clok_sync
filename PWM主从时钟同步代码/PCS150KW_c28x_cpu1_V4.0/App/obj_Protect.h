/*
 * obj_Protect.h
 *
 *  Created on: 2025年02月14日
 *      Author: guowei680
 */

#ifndef OBJ_PROTECT_H_
#define OBJ_PROTECT_H_

#include "obj_setting.h"


extern uint64_t  epwm1TZIntCount;

void PCS_HAL_setupCBCProtection();
void EPWM_CBCTripZoneInt_Counter(void);
void FaultMonitor_PhaseSequence(void);
int16_t App_AllFault_Deal(void);


#endif /* obj_Protect.h */
