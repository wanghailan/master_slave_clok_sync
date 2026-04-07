/*
 * Copyright (c) 2020 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "board.h"

//*****************************************************************************
//
// Board Configurations
// Initializes the rest of the modules. 
// Call this function in your application if you wish to do all module 
// initialization.
// If you wish to not use some of the initializations, instead of the 
// Board_init use the individual Module_inits
//
//*****************************************************************************
void Board_init()
{
	EALLOW;

	PinMux_init();
	CLB_init();

	EDIS;
}

//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************
void PinMux_init()
{
	//
	// PinMux for modules assigned to CPU1
	//
	

}

//*****************************************************************************
//
// CLB Configurations
//
//*****************************************************************************
void CLB_init(){
	myCLB5_init();
	myCLB6_init();
	myCLB7_init();
	myCLB8_init();
}

void myCLB5_init(){
	CLB_setOutputMask(myCLB5_BASE,
				(0UL << 0UL) | 
				(1UL << 0UL) | 
				(1UL << 2UL), true);
	CLB_disableOutputMaskUpdates(myCLB5_BASE);
	//
	// myCLB5 CLB_IN0 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB5_BASE, CLB_IN0, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB5_BASE, CLB_IN0, CLB_GLOBAL_IN_MUX_EPWM5A);
	CLB_configGPInputMux(myCLB5_BASE, CLB_IN0, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB5_BASE, CLB_IN0);
	CLB_selectInputFilter(myCLB5_BASE, CLB_IN0, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB5_BASE, CLB_IN0);
	//
	// myCLB5 CLB_IN1 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB5_BASE, CLB_IN1, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB5_BASE, CLB_IN1, CLB_GLOBAL_IN_MUX_EPWM5B);
	CLB_configGPInputMux(myCLB5_BASE, CLB_IN1, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB5_BASE, CLB_IN1);
	CLB_selectInputFilter(myCLB5_BASE, CLB_IN1, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB5_BASE, CLB_IN1);
	//
	// myCLB5 CLB_IN2 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB5_BASE, CLB_IN2, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB5_BASE, CLB_IN2, CLB_GLOBAL_IN_MUX_EPWM1A);
	CLB_configGPInputMux(myCLB5_BASE, CLB_IN2, CLB_GP_IN_MUX_GP_REG);
	CLB_selectInputFilter(myCLB5_BASE, CLB_IN2, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB5_BASE, CLB_IN2);
	//
	// myCLB5 CLB_IN3 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB5_BASE, CLB_IN3, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB5_BASE, CLB_IN3, CLB_GLOBAL_IN_MUX_EPWM6B);
	CLB_configGPInputMux(myCLB5_BASE, CLB_IN3, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB5_BASE, CLB_IN3);
	CLB_selectInputFilter(myCLB5_BASE, CLB_IN3, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB5_BASE, CLB_IN3);
	CLB_setGPREG(myCLB5_BASE,0);

	CLB_disableCLB(myCLB5_BASE);
}
void myCLB6_init(){
	CLB_setOutputMask(myCLB6_BASE,
				(0UL << 0UL) | 
				(1UL << 0UL) | 
				(1UL << 2UL), true);
	CLB_disableOutputMaskUpdates(myCLB6_BASE);
	//
	// myCLB6 CLB_IN0 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB6_BASE, CLB_IN0, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB6_BASE, CLB_IN0, CLB_GLOBAL_IN_MUX_EPWM6A);
	CLB_configGPInputMux(myCLB6_BASE, CLB_IN0, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB6_BASE, CLB_IN0);
	CLB_selectInputFilter(myCLB6_BASE, CLB_IN0, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB6_BASE, CLB_IN0);
	//
	// myCLB6 CLB_IN1 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB6_BASE, CLB_IN1, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB6_BASE, CLB_IN1, CLB_GLOBAL_IN_MUX_EPWM6B);
	CLB_configGPInputMux(myCLB6_BASE, CLB_IN1, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB6_BASE, CLB_IN1);
	CLB_selectInputFilter(myCLB6_BASE, CLB_IN1, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB6_BASE, CLB_IN1);
	//
	// myCLB6 CLB_IN2 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB6_BASE, CLB_IN2, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB6_BASE, CLB_IN2, CLB_GLOBAL_IN_MUX_EPWM1A);
	CLB_configGPInputMux(myCLB6_BASE, CLB_IN2, CLB_GP_IN_MUX_GP_REG);
	CLB_selectInputFilter(myCLB6_BASE, CLB_IN2, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB6_BASE, CLB_IN2);
	//
	// myCLB6 CLB_IN3 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB6_BASE, CLB_IN3, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB6_BASE, CLB_IN3, CLB_GLOBAL_IN_MUX_EPWM5A);
	CLB_configGPInputMux(myCLB6_BASE, CLB_IN3, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB6_BASE, CLB_IN3);
	CLB_selectInputFilter(myCLB6_BASE, CLB_IN3, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB6_BASE, CLB_IN3);
	CLB_setGPREG(myCLB6_BASE,0);

	CLB_disableCLB(myCLB6_BASE);
}
void myCLB7_init(){
	CLB_setOutputMask(myCLB7_BASE,
				(0UL << 0UL) | 
				(1UL << 0UL) | 
				(1UL << 2UL), true);
	CLB_disableOutputMaskUpdates(myCLB7_BASE);
	//
	// myCLB7 CLB_IN0 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB7_BASE, CLB_IN0, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB7_BASE, CLB_IN0, CLB_GLOBAL_IN_MUX_EPWM7A);
	CLB_configGPInputMux(myCLB7_BASE, CLB_IN0, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB7_BASE, CLB_IN0);
	CLB_selectInputFilter(myCLB7_BASE, CLB_IN0, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB7_BASE, CLB_IN0);
	//
	// myCLB7 CLB_IN1 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB7_BASE, CLB_IN1, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB7_BASE, CLB_IN1, CLB_GLOBAL_IN_MUX_EPWM7B);
	CLB_configGPInputMux(myCLB7_BASE, CLB_IN1, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB7_BASE, CLB_IN1);
	CLB_selectInputFilter(myCLB7_BASE, CLB_IN1, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB7_BASE, CLB_IN1);
	//
	// myCLB7 CLB_IN2 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB7_BASE, CLB_IN2, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB7_BASE, CLB_IN2, CLB_GLOBAL_IN_MUX_EPWM1A);
	CLB_configGPInputMux(myCLB7_BASE, CLB_IN2, CLB_GP_IN_MUX_GP_REG);
	CLB_selectInputFilter(myCLB7_BASE, CLB_IN2, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB7_BASE, CLB_IN2);
	//
	// myCLB7 CLB_IN3 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB7_BASE, CLB_IN3, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB7_BASE, CLB_IN3, CLB_GLOBAL_IN_MUX_EPWM8B);
	CLB_configGPInputMux(myCLB7_BASE, CLB_IN3, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB7_BASE, CLB_IN3);
	CLB_selectInputFilter(myCLB7_BASE, CLB_IN3, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB7_BASE, CLB_IN3);
	CLB_setGPREG(myCLB7_BASE,0);

	CLB_disableCLB(myCLB7_BASE);
}
void myCLB8_init(){
	CLB_setOutputMask(myCLB8_BASE,
				(0UL << 0UL) | 
				(1UL << 0UL) | 
				(1UL << 2UL), true);
	CLB_disableOutputMaskUpdates(myCLB8_BASE);
	//
	// myCLB8 CLB_IN0 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB8_BASE, CLB_IN0, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB8_BASE, CLB_IN0, CLB_GLOBAL_IN_MUX_EPWM8A);
	CLB_configGPInputMux(myCLB8_BASE, CLB_IN0, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB8_BASE, CLB_IN0);
	CLB_selectInputFilter(myCLB8_BASE, CLB_IN0, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB8_BASE, CLB_IN0);
	//
	// myCLB8 CLB_IN1 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB8_BASE, CLB_IN1, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB8_BASE, CLB_IN1, CLB_GLOBAL_IN_MUX_EPWM8B);
	CLB_configGPInputMux(myCLB8_BASE, CLB_IN1, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB8_BASE, CLB_IN1);
	CLB_selectInputFilter(myCLB8_BASE, CLB_IN1, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB8_BASE, CLB_IN1);
	//
	// myCLB8 CLB_IN2 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB8_BASE, CLB_IN2, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB8_BASE, CLB_IN2, CLB_GLOBAL_IN_MUX_EPWM1A);
	CLB_configGPInputMux(myCLB8_BASE, CLB_IN2, CLB_GP_IN_MUX_GP_REG);
	CLB_selectInputFilter(myCLB8_BASE, CLB_IN2, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB8_BASE, CLB_IN2);
	//
	// myCLB8 CLB_IN3 initialization
	//
	// The following functions configure the CLB input mux and whether the inputs
	// have synchronization or pipeline enabled; check the device manual for more
	// information on when a signal needs to be synchronized or go through a
	// pipeline filter
	//
	CLB_configLocalInputMux(myCLB8_BASE, CLB_IN3, CLB_LOCAL_IN_MUX_GLOBAL_IN);
	CLB_configGlobalInputMux(myCLB8_BASE, CLB_IN3, CLB_GLOBAL_IN_MUX_EPWM7A);
	CLB_configGPInputMux(myCLB8_BASE, CLB_IN3, CLB_GP_IN_MUX_EXTERNAL);
	CLB_enableSynchronization(myCLB8_BASE, CLB_IN3);
	CLB_selectInputFilter(myCLB8_BASE, CLB_IN3, CLB_FILTER_NONE);
	CLB_disableInputPipelineMode(myCLB8_BASE, CLB_IN3);
	CLB_setGPREG(myCLB8_BASE,0);

	CLB_disableCLB(myCLB8_BASE);
}

