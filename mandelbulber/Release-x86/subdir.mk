################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Render3D.cpp \
../src/algebra.cpp \
../src/callbacks.cpp \
../src/cimage.cpp \
../src/common_math.cpp \
../src/database.cpp \
../src/files.cpp \
../src/fractal.cpp \
../src/image.cpp \
../src/interface.cpp \
../src/morph.cpp \
../src/settings.cpp \
../src/shaders.cpp \
../src/texture.cpp \
../src/timeline.cpp \
../src/undo.cpp 

OBJS += \
./src/Render3D.o \
./src/algebra.o \
./src/callbacks.o \
./src/cimage.o \
./src/common_math.o \
./src/database.o \
./src/files.o \
./src/fractal.o \
./src/image.o \
./src/interface.o \
./src/morph.o \
./src/settings.o \
./src/shaders.o \
./src/texture.o \
./src/timeline.o \
./src/undo.o 

CPP_DEPS += \
./src/Render3D.d \
./src/algebra.d \
./src/callbacks.d \
./src/cimage.d \
./src/common_math.d \
./src/database.d \
./src/files.d \
./src/fractal.d \
./src/image.d \
./src/interface.d \
./src/morph.d \
./src/settings.d \
./src/shaders.d \
./src/texture.d \
./src/timeline.d \
./src/undo.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O2 -ffast-math -Wall -c -fmessage-length=0 `pkg-config --cflags gtk+-2.0 gthread-2.0;` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


