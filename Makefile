CXX = clang++
CXXFLAGS = -O2 -Wall -Wextra -Wpedantic -std=c++20 -I"include" -L"lib"
SRC = src/*.cpp

ifeq ($(OS),Windows_NT)
	# WINDOWS SETTINGS
	TARGET = bin/simulator.exe
	CXXFLAGS += -lgdi32 -lgdiplus -luser32 -static-libgcc -static-libstdc++ -static
	SRC += src/platform/win/window.cpp
	CLEAN_CMD = del /f /q
else
	# MAC / LINUX SETTINGS
	TARGET = bin/simulator
	CXXFLAGS += -framework Cocoa -fobjc-arc -framework QuartzCore -framework CoreText
	SRC += src/platform/mac/window.mm
	CLEAN_CMD = rm -f
endif

all: build run

build:
	$(CXX) $(SRC) $(CXXFLAGS) -o $(TARGET)

run:
	./$(TARGET)

clean:
	$(CLEAN_CMD) $(TARGET)
