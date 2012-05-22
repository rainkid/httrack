################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../minizip/ioapi.c \
../minizip/iowin32.c \
../minizip/mztools.c \
../minizip/unzip.c \
../minizip/zip.c 

OBJS += \
./minizip/ioapi.o \
./minizip/iowin32.o \
./minizip/mztools.o \
./minizip/unzip.o \
./minizip/zip.o 

C_DEPS += \
./minizip/ioapi.d \
./minizip/iowin32.d \
./minizip/mztools.d \
./minizip/unzip.d \
./minizip/zip.d 


# Each subdirectory must supply rules for building sources it contributes
minizip/%.o: ../minizip/%.c
	@echo 'Building file: $<'
	@echo 'invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


