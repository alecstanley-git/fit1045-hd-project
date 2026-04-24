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
CXXFLAGS = -O2 -Wall -Wextra -Wpedantic -std=c++17
SRC = *.cpp helper/*.cpp

all: build run

build:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run:
	./$(TARGET)

clean:
	$(CLEAN_CMD) $(TARGET)
