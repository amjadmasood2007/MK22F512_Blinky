################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/Franklin_Library/TASK_SCHEDULER/task_schdlr.c 

OBJS += \
./Sources/Franklin_Library/TASK_SCHEDULER/task_schdlr.o 

C_DEPS += \
./Sources/Franklin_Library/TASK_SCHEDULER/task_schdlr.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/Franklin_Library/TASK_SCHEDULER/%.o: ../Sources/Franklin_Library/TASK_SCHEDULER/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"C:/GIT_REPO/MK22FN512_Blinky/MK22F512_Blinky/Static_Code/System" -I"C:/GIT_REPO/MK22FN512_Blinky/MK22F512_Blinky/Static_Code/PDD" -I"C:/GIT_REPO/MK22FN512_Blinky/MK22F512_Blinky/Static_Code/IO_Map" -I"C:\Freescale\KDS_v3\eclipse\ProcessorExpert/lib/Kinetis/pdd/inc" -I"C:/GIT_REPO/MK22FN512_Blinky/MK22F512_Blinky/Sources" -I"C:/GIT_REPO/MK22FN512_Blinky/MK22F512_Blinky/Generated_Code" -I"C:/GIT_REPO/MK22FN512_Blinky/MK22F512_Blinky/Sources/Franklin_Library/TASK_SCHEDULER" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


