################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./Driver/Drv_I2CA.obj" \
"./Driver/Drv_I2C_EE.obj" \
"./Driver/Drv_I2C_RTC.obj" \
"./Driver/Drv_LED.obj" \
"./Driver/Drv_Timer.obj" \
"./Driver/Eth_mii.obj" \
"./Driver/f2838xif.obj" \
"./Driver/W5500/socket.obj" \
"./Driver/W5500/utility.obj" \
"./Driver/W5500/w5500.obj" \
"./Driver/W5500/w5500_conf.obj" \
"./Driver/device/cm.obj" \
"./lib/utils/lwiplib.obj" \
"./lib/utils/ustdlib.obj" \
"./src/ModbusRtu.obj" \
"./src/ModbusTcp.obj" \
"./src/bsp.obj" \
"./src/ipc_cm.obj" \
"./src/main.obj" \
"./src/modbus_Param.obj" \
"./src/obj_debug.obj" \
"./src/ptp_master_sync.obj" \
"./src/ptp_slave_sync.obj" \
"./src/startup_ccs.obj" \
"../2838x_flash_lnk_cm_lwip.cmd" \
"../Driver/device/driverlib_cm.lib" \
$(GEN_CMDS__FLAG) \
-l"D:/workspace/CCS/PCS150KW_c28x_cm_V4.0/Driver/device/driverlib_cm.lib" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include Driver/subdir_vars.mk
-include Driver/W5500/subdir_vars.mk
-include Driver/device/subdir_vars.mk
-include lib/utils/subdir_vars.mk
-include src/subdir_vars.mk
-include subdir_rules.mk
-include Driver/subdir_rules.mk
-include Driver/W5500/subdir_rules.mk
-include Driver/device/subdir_rules.mk
-include lib/utils/subdir_rules.mk
-include src/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
PCS150KW_c28x_cm_V4.0.out 

EXE_OUTPUTS__QUOTED += \
"PCS150KW_c28x_cm_V4.0.out" 


# All Target
all: PCS150KW_c28x_cm_V4.0.out

# Tool invocations
PCS150KW_c28x_cm_V4.0.out: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: Arm Linker'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=none -me -O2 --opt_for_speed=2 --define=PTP_SYNC_ROLE=PTP_SYNC_ROLE_MASTER --define=_FLASH --define=ccs -g --c99 --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual -z -m"enet_lwip_udp_cm.map" --heap_size=0 --stack_size=2048 -i"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/lib" -i"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" -i"C:/ti/ccs1280/ccs/tools/compiler" -i"D:/workspace/CCS/PCS150KW_c28x_cm_V4.0/Driver/device" --reread_libs --warn_sections --xml_link_info="enet_lwip_udp_cm_linkInfo.xml" --rom_model -o "PCS150KW_c28x_cm_V4.0.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)
	-$(RM) "Driver\Drv_I2CA.obj" "Driver\Drv_I2C_EE.obj" "Driver\Drv_I2C_RTC.obj" "Driver\Drv_LED.obj" "Driver\Drv_Timer.obj" "Driver\Eth_mii.obj" "Driver\f2838xif.obj" "Driver\W5500\socket.obj" "Driver\W5500\utility.obj" "Driver\W5500\w5500.obj" "Driver\W5500\w5500_conf.obj" "Driver\device\cm.obj" "lib\utils\lwiplib.obj" "lib\utils\ustdlib.obj" "src\ModbusRtu.obj" "src\ModbusTcp.obj" "src\bsp.obj" "src\ipc_cm.obj" "src\main.obj" "src\modbus_Param.obj" "src\obj_debug.obj" "src\ptp_master_sync.obj" "src\ptp_slave_sync.obj" "src\startup_ccs.obj" 
	-$(RM) "Driver\Drv_I2CA.d" "Driver\Drv_I2C_EE.d" "Driver\Drv_I2C_RTC.d" "Driver\Drv_LED.d" "Driver\Drv_Timer.d" "Driver\Eth_mii.d" "Driver\f2838xif.d" "Driver\W5500\socket.d" "Driver\W5500\utility.d" "Driver\W5500\w5500.d" "Driver\W5500\w5500_conf.d" "Driver\device\cm.d" "lib\utils\lwiplib.d" "lib\utils\ustdlib.d" "src\ModbusRtu.d" "src\ModbusTcp.d" "src\bsp.d" "src\ipc_cm.d" "src\main.d" "src\modbus_Param.d" "src\obj_debug.d" "src\ptp_master_sync.d" "src\ptp_slave_sync.d" "src\startup_ccs.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

