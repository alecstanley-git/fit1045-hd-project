ifeq ($(OS),Windows_NT)
	# WINDOWS SETTINGS
	TARGET = executables/simulator.exe
	CLEAN_CMD = del /f /q
else
	# MAC / LINUX SETTINGS
	TARGET = executables/simulator
	CLEAN_CMD = rm -f
endif

CXX = clang++
CXXFLAGS = -Wall -O2 -std=c++17
SRC = main.cpp helper/utilities.cpp

all: build run

build:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run:
	./$(TARGET)

clean:
	$(CLEAN_CMD) $(TARGET)
