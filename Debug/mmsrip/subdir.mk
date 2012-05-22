################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mmsrip/error.c \
../mmsrip/main.c \
../mmsrip/mms.c 

OBJS += \
./mmsrip/error.o \
./mmsrip/main.o \
./mmsrip/mms.o 

C_DEPS += \
./mmsrip/error.d \
./mmsrip/main.d \
./mmsrip/mms.d 


# Each subdirectory must supply rules for building sources it contributes
mmsrip/%.o: ../mmsrip/%.c
	@echo 'Building file: $<'
	@echo 'invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


