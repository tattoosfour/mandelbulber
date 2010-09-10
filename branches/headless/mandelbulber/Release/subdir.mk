################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Render3D.cpp \
../algebra.cpp \
../callbacks.cpp \
../cimage.cpp \
../common_math.cpp \
../files.cpp \
../fractal.cpp \
../image.cpp \
../interface.cpp \
../loadsound.cpp \
../morph.cpp \
../settings.cpp \
../shaders.cpp \
../texture.cpp \
../undo.cpp 

OBJS += \
./Render3D.o \
./algebra.o \
./callbacks.o \
./cimage.o \
./common_math.o \
./files.o \
./fractal.o \
./image.o \
./interface.o \
./loadsound.o \
./morph.o \
./settings.o \
./shaders.o \
./texture.o \
./undo.o 

CPP_DEPS += \
./Render3D.d \
./algebra.d \
./callbacks.d \
./cimage.d \
./common_math.d \
./files.d \
./fractal.d \
./image.d \
./interface.d \
./loadsound.d \
./morph.d \
./settings.d \
./shaders.d \
./texture.d \
./undo.d 


# Each subdirectory must supply rules for building sources it contributes
Render3D.o: ../Render3D.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O2 -march=native -mfpmath=387,sse -ffast-math -Wall -c -fmessage-length=0 `pkg-config --cflags gtk+-2.0 gthread-2.0;` -MMD -MP -MF"$(@:%.o=%.d)" -MT"Render3D.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O2 -march=native -mfpmath=387,sse -ffast-math -Wall -c -fmessage-length=0 `pkg-config --cflags gtk+-2.0 gthread-2.0;` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


