ifeq ($(OS),Windows_NT)
	# WINDOWS SETTINGS
	TARGET = bin/simulator.exe
	CLEAN_CMD = del /f /q
else
	# MAC / LINUX SETTINGS
	TARGET = bin/simulator
	EXTRA_FLAGS = -framework Cocoa
	CLEAN_CMD = rm -f
endif

CXX = clang++
CXXFLAGS = -O2 -Wall -Wextra -Wpedantic -std=c++17 -I"include" -L"lib"
SRC = src/*.cpp src/*.mm

all: build run

build:
	$(CXX) $(SRC) $(CXXFLAGS) $(EXTRA_FLAGS) -o $(TARGET)

run:
	./$(TARGET)

clean:
	$(CLEAN_CMD) $(TARGET)
