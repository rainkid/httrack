################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../error.o \
../htsalias.o \
../htsback.o \
../htsbauth.o \
../htscache.o \
../htscatchurl.o \
../htscore.o \
../htscoremain.o \
../htsfilters.o \
../htsftp.o \
../htshash.o \
../htshelp.o \
../htsindex.o \
../htsinthash.o \
../htsjava.o \
../htslib.o \
../htsmd5.o \
../htsmms.o \
../htsmodules.o \
../htsname.o \
../htsparse.o \
../htsrobots.o \
../htsserver.o \
../htsthread.o \
../htstools.o \
../htsweb.o \
../htswizard.o \
../htswrap.o \
../htszlib.o \
../httrack.o \
../ioapi.o \
../md5.o \
../mms.o \
../mztools.o \
../proxytrack.o \
../store.o \
../unzip.o \
../zip.o 

C_SRCS += \
../htsalias.c \
../htsback.c \
../htsbauth.c \
../htscache.c \
../htscatchurl.c \
../htscore.c \
../htscoremain.c \
../htsfilters.c \
../htsftp.c \
../htshash.c \
../htshelp.c \
../htsindex.c \
../htsinthash.c \
../htsjava.c \
../htslib.c \
../htsmd5.c \
../htsmms.c \
../htsmodules.c \
../htsname.c \
../htsparse.c \
../htsrobots.c \
../htsserver.c \
../htsthread.c \
../htstools.c \
../htsweb.c \
../htswizard.c \
../htswrap.c \
../htszlib.c \
../httrack.c \
../md5.c 

OBJS += \
./htsalias.o \
./htsback.o \
./htsbauth.o \
./htscache.o \
./htscatchurl.o \
./htscore.o \
./htscoremain.o \
./htsfilters.o \
./htsftp.o \
./htshash.o \
./htshelp.o \
./htsindex.o \
./htsinthash.o \
./htsjava.o \
./htslib.o \
./htsmd5.o \
./htsmms.o \
./htsmodules.o \
./htsname.o \
./htsparse.o \
./htsrobots.o \
./htsserver.o \
./htsthread.o \
./htstools.o \
./htsweb.o \
./htswizard.o \
./htswrap.o \
./htszlib.o \
./httrack.o \
./md5.o 

C_DEPS += \
./htsalias.d \
./htsback.d \
./htsbauth.d \
./htscache.d \
./htscatchurl.d \
./htscore.d \
./htscoremain.d \
./htsfilters.d \
./htsftp.d \
./htshash.d \
./htshelp.d \
./htsindex.d \
./htsinthash.d \
./htsjava.d \
./htslib.d \
./htsmd5.d \
./htsmms.d \
./htsmodules.d \
./htsname.d \
./htsparse.d \
./htsrobots.d \
./htsserver.d \
./htsthread.d \
./htstools.d \
./htsweb.d \
./htswizard.d \
./htswrap.d \
./htszlib.d \
./httrack.d \
./md5.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


