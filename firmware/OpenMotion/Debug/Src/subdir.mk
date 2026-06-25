################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/gpio_driver.c \
../Src/main.c \
../Src/protocol.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/timer_driver.c \
../Src/uart_driver.c 

OBJS += \
./Src/gpio_driver.o \
./Src/main.o \
./Src/protocol.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/timer_driver.o \
./Src/uart_driver.o 

C_DEPS += \
./Src/gpio_driver.d \
./Src/main.d \
./Src/protocol.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/timer_driver.d \
./Src/uart_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F401RE -DSTM32F401xE -DSTM32 -DSTM32F401RETx -DSTM32F4 -c -I../Inc -I"/home/abdelrahman/STM32CubeIDE/workspace_2.1.1/OpenMotion/chip_headers/CMSIS/Device/ST/STM32F4xx/Include" -I"/home/abdelrahman/STM32CubeIDE/workspace_2.1.1/OpenMotion/chip_headers/CMSIS/Include" -I../chip_headers -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/gpio_driver.cyclo ./Src/gpio_driver.d ./Src/gpio_driver.o ./Src/gpio_driver.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/protocol.cyclo ./Src/protocol.d ./Src/protocol.o ./Src/protocol.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/timer_driver.cyclo ./Src/timer_driver.d ./Src/timer_driver.o ./Src/timer_driver.su ./Src/uart_driver.cyclo ./Src/uart_driver.d ./Src/uart_driver.o ./Src/uart_driver.su

.PHONY: clean-Src

