CC = clang++
CFLAGS = -std=c++11 \
	-I /usr/local/include \
	-I lib/include \
	-I src/main \
	-I src/cpu/a65000 \
	-I src/disassembler/a65000 \
	-I src/gpu/G1 \
	-L /usr/local/lib \
	-L lib \
	-l sdl2 \
	-l sdl2_ttf \
	-l sdl2_image \
	-l nfd \
	-framework AppKit

SOURCES = src/main/main.mm \
	src/main/RSCore.cpp \
	src/main/RSDebugWindow.cpp \
	src/main/RSFonts.cpp \
	src/main/RSMainWindow.cpp \
	src/main/RSMonitorInterpreter.cpp \
	src/main/RSTime.cpp \
	src/main/RSWindow.cpp \
	src/main/SDLFramework.cpp \
	src/cpu/a65000/A65000CPU.cpp \
	src/disassembler/a65000/A65000Disassembler.cpp \
	src/gpu/G1/GPU-G1.cpp

all: makefile
	rm bin/retrosim
	$(CC) $(CFLAGS) $(SOURCES) -o bin/retrosim
