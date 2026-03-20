TARGET := calc2
BUILD_DIR := build2
EXT_DIR := third_party
INC_DIR := include
SRC_DIR := source

IMGUI_DIR := $(EXT_DIR)/imgui
RL_IMGUI_DIR := $(EXT_DIR)/rlimgui
RAYLIB_DIR := $(EXT_DIR)/raylib
RAYLIB_CPP_DIR := $(EXT_DIR)/raylib-cpp


CC  := gcc
CXX := g++
LD  := g++
AR  := ar

CALC_CXX_FLAGS := -Wall -Wextra -std=c++20 -ggdb
IMGUI_CXX_FLAGS := -std=c++20 -O2 -fPIC
RL_IMGUI_CXX_FLAGS := -std=c++20 -O2 -fPIC

CALC_SRC := $(wildcard $(SRC_DIR)/*.cpp)
CALC_OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/$(SRC_DIR)/%.o,$(CALC_SRC))

IMGUI_SRC := $(wildcard $(IMGUI_DIR)/*.cpp)
IMGUI_OBJ := $(patsubst $(IMGUI_DIR)/%.cpp,$(BUILD_DIR)/imgui/%.o,$(IMGUI_SRC))
IMGUI_LIB := $(BUILD_DIR)/libimgui.a

RL_IMGUI_SRC := $(wildcard $(RL_IMGUI_DIR)/*.cpp)
RL_IMGUI_OBJ := $(patsubst $(RL_IMGUI_DIR)/%.cpp,$(BUILD_DIR)/rlimgui/%.o,$(RL_IMGUI_SRC))
RL_IMGUI_LIB := $(BUILD_DIR)/librlimgui.a

RAYLIB_LIB := $(BUILD_DIR)/libraylib.a

run: all
	./$(TARGET)

all: $(TARGET)

# MAIN
$(TARGET): $(CALC_OBJ) $(IMGUI_LIB) $(RL_IMGUI_LIB) $(RAYLIB_LIB)
	$(CXX) $(CALC_CXX_FLAGS) $(CALC_OBJ) $(IMGUI_LIB) $(RL_IMGUI_LIB) $(RAYLIB_LIB) -o $@

$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CALC_CXX_FLAGS) -I$(INC_DIR) -c $< -o $@

# IMGUI
$(BUILD_DIR)/imgui/%.o: $(IMGUI_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(IMGUI_CXX_FLAGS) -c $< -o $@

$(IMGUI_LIB): $(IMGUI_OBJ)
	$(AR) -cr $@ $^

# RLIMGUI
$(BUILD_DIR)/rlimgui/%.o: $(RL_IMGUI_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(RL_IMGUI_CXX_FLAGS) -I$(IMGUI_DIR) -I$(INC_DIR) -c $< -o $@

$(RL_IMGUI_LIB): $(RL_IMGUI_OBJ)
	$(AR) -cr $@ $^

# RAYLIB
$(RAYLIB_LIB):
	$(MAKE) -C $(RAYLIB_DIR)/src
	mkdir -p $(BUILD_DIR)
	cp $(RAYLIB_DIR)/src/libraylib.a $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)