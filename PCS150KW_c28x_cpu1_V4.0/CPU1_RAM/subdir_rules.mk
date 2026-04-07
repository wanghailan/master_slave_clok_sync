################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-1829974015: ../c28x.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs1280/ccs/utils/sysconfig_1.21.0/sysconfig_cli.bat" --script "D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/c28x.syscfg" -o "syscfg" --compiler ccs
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/error.h: build-1829974015 ../c28x.syscfg
syscfg: build-1829974015

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu64 --idiv_support=idiv0 --tmu_support=tmu0 --vcu_support=vcrc -Ooff --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/device" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/driverlib/f2838x/driverlib" --include_path="C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --define=DEBUG --define=CPU1 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/CPU1_RAM/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


