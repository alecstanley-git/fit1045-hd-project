ifeq ($(OS),Windows_NT)
	# WINDOWS SETTINGS
	TARGET = bin/simulator.exe
	EXTRA_FLAGS = -lopengl32 -lgdi32 -luser32 -lshell32 
	CLEAN_CMD = del /f /q
else
	# MAC / LINUX SETTINGS
	TARGET = bin/simulator
	EXTRA_FLAGS = -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo 
	CLEAN_CMD = rm -f
endif

CXX = clang++
CXXFLAGS = -O2 -Wall -Wextra -Wpedantic -std=c++17 -lglfw3 -I"include" -L"lib"
SRC = src/*.cpp src/glad.c

all: build run

build:
	$(CXX) $(SRC) $(CXXFLAGS) $(EXTRA_FLAGS) -o $(TARGET)

run:
	./$(TARGET)

clean:
	$(CLEAN_CMD) $(TARGET)
