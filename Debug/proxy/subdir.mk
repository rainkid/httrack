################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../proxy/main.c \
../proxy/proxytrack.c \
../proxy/store.c 

OBJS += \
./proxy/main.o \
./proxy/proxytrack.o \
./proxy/store.o 

C_DEPS += \
./proxy/main.d \
./proxy/proxytrack.d \
./proxy/store.d 


# Each subdirectory must supply rules for building sources it contributes
proxy/%.o: ../proxy/%.c
	@echo 'Building file: $<'
	@echo 'invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


