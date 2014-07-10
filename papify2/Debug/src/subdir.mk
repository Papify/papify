################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cmake_editor.c \
../src/eventLib.c \
../src/papify.c \
../src/papify_lib.c \
../src/xcf_creator.c 

OBJS += \
./src/cmake_editor.o \
./src/eventLib.o \
./src/papify.o \
./src/papify_lib.o \
./src/xcf_creator.o 

C_DEPS += \
./src/cmake_editor.d \
./src/eventLib.d \
./src/papify.d \
./src/papify_lib.d \
./src/xcf_creator.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/aarias/git/papify/papify2/libs/roxml/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


