################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
./src/MMDVM_TNC.c \
./src/TNC.c \
./src/port.c \
./src/serial_common.c \
./src/serial_posix.c 

C_DEPS += \
./src/MMDVM_TNC.d \
./src/TNC.d \
./src/port.d \
./src/serial_common.d \
./src/serial_posix.d 

OBJS += \
./src/MMDVM_TNC.o \
./src/TNC.o \
./src/port.o \
./src/serial_common.o \
./src/serial_posix.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ./src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/MMDVM_TNC.d ./src/MMDVM_TNC.o ./src/TNC.d ./src/TNC.o ./src/port.d ./src/port.o ./src/serial_common.d ./src/serial_common.o ./src/serial_posix.d ./src/serial_posix.o

.PHONY: clean-src

