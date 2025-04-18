################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/al3010.c \
../Core/Src/dma.c \
../Core/Src/ecx343.c \
../Core/Src/gpio.c \
../Core/Src/i2c.c \
../Core/Src/i2s.c \
../Core/Src/lt7911_fw.c \
../Core/Src/lt7911ux.c \
../Core/Src/main.c \
../Core/Src/pingpong_buf.c \
../Core/Src/rpr0521.c \
../Core/Src/spi.c \
../Core/Src/stm32f7xx_hal_msp.c \
../Core/Src/stm32f7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f7xx.c 

OBJS += \
./Core/Src/al3010.o \
./Core/Src/dma.o \
./Core/Src/ecx343.o \
./Core/Src/gpio.o \
./Core/Src/i2c.o \
./Core/Src/i2s.o \
./Core/Src/lt7911_fw.o \
./Core/Src/lt7911ux.o \
./Core/Src/main.o \
./Core/Src/pingpong_buf.o \
./Core/Src/rpr0521.o \
./Core/Src/spi.o \
./Core/Src/stm32f7xx_hal_msp.o \
./Core/Src/stm32f7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f7xx.o 

C_DEPS += \
./Core/Src/al3010.d \
./Core/Src/dma.d \
./Core/Src/ecx343.d \
./Core/Src/gpio.d \
./Core/Src/i2c.d \
./Core/Src/i2s.d \
./Core/Src/lt7911_fw.d \
./Core/Src/lt7911ux.d \
./Core/Src/main.d \
./Core/Src/pingpong_buf.d \
./Core/Src/rpr0521.d \
./Core/Src/spi.d \
./Core/Src/stm32f7xx_hal_msp.d \
./Core/Src/stm32f7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f7xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F723xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/al3010.cyclo ./Core/Src/al3010.d ./Core/Src/al3010.o ./Core/Src/al3010.su ./Core/Src/dma.cyclo ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/ecx343.cyclo ./Core/Src/ecx343.d ./Core/Src/ecx343.o ./Core/Src/ecx343.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/i2c.cyclo ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/i2s.cyclo ./Core/Src/i2s.d ./Core/Src/i2s.o ./Core/Src/i2s.su ./Core/Src/lt7911_fw.cyclo ./Core/Src/lt7911_fw.d ./Core/Src/lt7911_fw.o ./Core/Src/lt7911_fw.su ./Core/Src/lt7911ux.cyclo ./Core/Src/lt7911ux.d ./Core/Src/lt7911ux.o ./Core/Src/lt7911ux.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/pingpong_buf.cyclo ./Core/Src/pingpong_buf.d ./Core/Src/pingpong_buf.o ./Core/Src/pingpong_buf.su ./Core/Src/rpr0521.cyclo ./Core/Src/rpr0521.d ./Core/Src/rpr0521.o ./Core/Src/rpr0521.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/stm32f7xx_hal_msp.cyclo ./Core/Src/stm32f7xx_hal_msp.d ./Core/Src/stm32f7xx_hal_msp.o ./Core/Src/stm32f7xx_hal_msp.su ./Core/Src/stm32f7xx_it.cyclo ./Core/Src/stm32f7xx_it.d ./Core/Src/stm32f7xx_it.o ./Core/Src/stm32f7xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f7xx.cyclo ./Core/Src/system_stm32f7xx.d ./Core/Src/system_stm32f7xx.o ./Core/Src/system_stm32f7xx.su

.PHONY: clean-Core-2f-Src

