################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-1829974015: ../c28x.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs1280/ccs/utils/sysconfig_1.21.0/sysconfig_cli.bat" --script "D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/c28x.syscfg" -o "syscfg" -s "C:/ti/c2000/C2000Ware_5_01_00_00/.metadata/sdk.json" -d "F2838x" --package 337bga --part F2838x_337bga --compiler ccs
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/board.c: build-1829974015 ../c28x.syscfg
syscfg/board.h: build-1829974015
syscfg/board.cmd.genlibs: build-1829974015
syscfg/board.opt: build-1829974015
syscfg/pinmux.csv: build-1829974015
syscfg/clb_config.h: build-1829974015
syscfg/clb_config.c: build-1829974015
syscfg/clb.dot: build-1829974015
syscfg/clb_sim.cpp: build-1829974015
syscfg/clb_simulation.bat: build-1829974015
syscfg/c2000ware_libraries.cmd.genlibs: build-1829974015
syscfg/c2000ware_libraries.opt: build-1829974015
syscfg/c2000ware_libraries.c: build-1829974015
syscfg/c2000ware_libraries.h: build-1829974015
syscfg/clocktree.h: build-1829974015
syscfg: build-1829974015

syscfg/%.obj: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 --vcu_support=vcrc -O3 --opt_for_speed=4 --fp_mode=relaxed --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/utilities/rampgen" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/device_support/f2838x/common/include" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/device_support/f2838x/headers/include" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/App" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/Driver/device" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/Driver" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/CLAmath" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/CLA" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/DCL" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/FPUfastRTS" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/power_measurement" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/sfra" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/spll" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/transforms" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/driverlib/f2838x/driverlib" --include_path="C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --advice:performance=all --define=_FLASH --define=DEBUG --define=CPU1 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="syscfg/$(basename $(<F)).d_raw" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/CPU1_FLASH/syscfg" --obj_directory="syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/%.obj: ./syscfg/%.cpp $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 --vcu_support=vcrc -O3 --opt_for_speed=4 --fp_mode=relaxed --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/utilities/rampgen" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/device_support/f2838x/common/include" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/device_support/f2838x/headers/include" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/App" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/Driver/device" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/Driver" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/CLAmath" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/CLA" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/DCL" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/FPUfastRTS" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/power_measurement" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/sfra" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/spll" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/transforms" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/driverlib/f2838x/driverlib" --include_path="C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --advice:performance=all --define=_FLASH --define=DEBUG --define=CPU1 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="syscfg/$(basename $(<F)).d_raw" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/CPU1_FLASH/syscfg" --obj_directory="syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 --vcu_support=vcrc -O3 --opt_for_speed=4 --fp_mode=relaxed --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/utilities/rampgen" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/device_support/f2838x/common/include" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/device_support/f2838x/headers/include" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/App" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/Driver/device" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/Driver" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/CLAmath" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/CLA" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/DCL" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/FPUfastRTS" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/power_measurement" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/sfra" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/spll" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/libraries/transforms" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/driverlib/f2838x/driverlib" --include_path="C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --advice:performance=all --define=_FLASH --define=DEBUG --define=CPU1 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="D:/workspace/CCS/PCS150KW_c28x_cpu1_V4.0/CPU1_FLASH/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


