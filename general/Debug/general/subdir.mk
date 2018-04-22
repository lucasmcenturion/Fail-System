################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../general/archivo.c \
../general/archivoConfig.c \
../general/logs.c \
../general/serializacion.c \
../general/sockets.c 

OBJS += \
./general/archivo.o \
./general/archivoConfig.o \
./general/logs.o \
./general/serializacion.o \
./general/sockets.o 

C_DEPS += \
./general/archivo.d \
./general/archivoConfig.d \
./general/logs.d \
./general/serializacion.d \
./general/sockets.d 


# Each subdirectory must supply rules for building sources it contributes
general/%.o: ../general/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


