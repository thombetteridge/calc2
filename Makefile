CXX := g++
TARGET := app.exe

SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst src/%.cpp,build/%.o,$(SRC))
PCH := src/pch.hpp.gch

CXXFLAGS := -std=c++23 -g3 -O0
CXXFLAGS += -Wall -Wextra -Wpedantic -Wshadow -Wconversion
CXXFLAGS += -isystem ext -Isrc

LFLAGS := -lgdi32 -lstdc++exp

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LFLAGS)

$(PCH): src/pch.hpp | build
	$(CXX) $(CXXFLAGS) -x c++-header -MMD -MP -MF build/pch.d -MT $@ $< -o $@

build/%.o: src/%.cpp $(PCH) | build
	$(CXX) $(CXXFLAGS) -include src/pch.hpp -MMD -MP -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build $(TARGET)

-include $(OBJ:.o=.d) build/pch.d

.PHONY: all clean


