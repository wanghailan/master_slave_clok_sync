################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
backup/%.obj: ../backup/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=none -me -Ooff --include_path="D:/workspace/CCS/ethernet_ex4_ptp_basic_master" --include_path="D:/workspace/CCS/ethernet_ex4_ptp_basic_master/device" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/driverlib/f2838x/driverlib_cm" --include_path="C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=DEBUG --define=_FLASH --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="backup/$(basename $(<F)).d_raw" --obj_directory="backup" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


