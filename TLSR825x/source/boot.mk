################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
OUT_DIR += /source

OBJS += \
$(OUT_PATH)/source/cstartup_825x.o

BOOT_FLAG := -DMCU_STARTUP_825X

ifeq ($(USE_FREE_RTOS), 1)
	BOOT_FLAG += -DUSE_FREE_RTOS
endif

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/source/%.o: ./source/%.S
	@echo 'Building file: $<'
	@$(TC32_PATH)tc32-elf-gcc $(BOOT_FLAG) -c -o"$@" "$<"
