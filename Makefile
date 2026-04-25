ifeq ($(OS),Windows_NT)
	# WINDOWS SETTINGS
	TARGET = bin/simulator.exe
	CLEAN_CMD = del /f /q
else
	# MAC / LINUX SETTINGS
	TARGET = bin/simulator
	CLEAN_CMD = rm -f
endif

CXX = clang++
CXXFLAGS = -O2 -Wall -Wextra -Wpedantic -std=c++17 -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -I"include" -L"lib"
SRC = src/*.cpp src/glad.c

all: build run

build:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run:
	./$(TARGET)

clean:
	$(CLEAN_CMD) $(TARGET)
