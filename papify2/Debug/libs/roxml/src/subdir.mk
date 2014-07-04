################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libs/roxml/src/roxml-internal.c \
../libs/roxml/src/roxml-parse-engine.c \
../libs/roxml/src/roxml.c 

OBJS += \
./libs/roxml/src/roxml-internal.o \
./libs/roxml/src/roxml-parse-engine.o \
./libs/roxml/src/roxml.o 

C_DEPS += \
./libs/roxml/src/roxml-internal.d \
./libs/roxml/src/roxml-parse-engine.d \
./libs/roxml/src/roxml.d 


# Each subdirectory must supply rules for building sources it contributes
libs/roxml/src/%.o: ../libs/roxml/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/alejo/Desktop/gdem/Link to workspace/papify2/libs/roxml/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


