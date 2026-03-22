TARGET := calc2
BUILD_DIR := build2
EXT_DIR := third_party
INC_DIR := include
SRC_DIR := source

IMGUI_DIR := $(EXT_DIR)/imgui
RL_IMGUI_DIR := $(EXT_DIR)/rlimgui
RAYLIB_DIR := $(EXT_DIR)/raylib
RAYLIB_CPP_DIR := $(EXT_DIR)/raylib-cpp

ifeq ($(OS),Windows_NT)

OBJ := obj
LIB := lib
C := /c
O := /OUT:
FO := /Fo:
I := /external:I
EXTERNAL := /external:W0
OUT := /OUT:
MD := /MD

CC  := cl
CXX := cl
LD  := link
AR  := lib

EXE := .exe

CALC_CXX_FLAGS := /nologo /W4 /std:c++20 /Zi /EHsc
IMGUI_CXX_FLAGS := /nologo /std:c++20 /O2
RL_IMGUI_CXX_FLAGS := /nologo /std:c++20 /O2

LINK_HACK := P:\\BuildTools\\Windows Kits\\10\Lib\\10.0.26100.0\\um\\x64\\

CALC_LINK_FLAGS := kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib

else

OBJ := o
LIB := a
C := -c
O := -o #space hack?
FO := -o #space hack?
CR := -cr
I := -I
OUT :=

CC  := gcc
CXX := g++
LD  := g++
AR  := ar


CALC_CXX_FLAGS := -Wall -Wextra -std=c++20 -ggdb
IMGUI_CXX_FLAGS := -std=c++20 -O2 -fPIC
RL_IMGUI_CXX_FLAGS := -std=c++20 -O2 -fPIC

endif


CALC_SRC := $(wildcard $(SRC_DIR)/*.cpp)
CALC_OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/$(SRC_DIR)/%.$(OBJ),$(CALC_SRC))

IMGUI_SRC := $(wildcard $(IMGUI_DIR)/*.cpp)
IMGUI_OBJ := $(patsubst $(IMGUI_DIR)/%.cpp,$(BUILD_DIR)/imgui/%.$(OBJ),$(IMGUI_SRC))
IMGUI_LIB := $(BUILD_DIR)/libimgui.$(LIB)

RL_IMGUI_SRC := $(wildcard $(RL_IMGUI_DIR)/*.cpp)
RL_IMGUI_OBJ := $(patsubst $(RL_IMGUI_DIR)/%.cpp,$(BUILD_DIR)/rlimgui/%.$(OBJ),$(RL_IMGUI_SRC))
RL_IMGUI_LIB := $(BUILD_DIR)/librlimgui.$(LIB)

ifeq ($(OS),Windows_NT)
RAYLIB_LIB := lib/raylib.lib
else
RAYLIB_LIB := $(BUILD_DIR)/libraylib.$(LIB)
endif



run: all
	$(BUILD_DIR)/$(TARGET)$(EXE)

all: $(TARGET)



# cl.exe /MD /O2 /openmp /fp:fast /GL /GA /EHsc /external:I include/ /external:W0 main.cpp /link lib/raylib.lib kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib /out:main.exe

# MAIN
ifeq ($(OS),Windows_NT)

$(TARGET): $(CALC_OBJ) $(IMGUI_LIB) $(RL_IMGUI_LIB) $(RAYLIB_LIB)
	$(LD)  $(O)$(BUILD_DIR)/$@$(EXE) $(CALC_OBJ)  $(IMGUI_LIB) $(RL_IMGUI_LIB) $(RAYLIB_LIB) $(CALC_LINK_FLAGS)

else

$(TARGET): $(CALC_OBJ) $(IMGUI_LIB) $(RL_IMGUI_LIB) $(RAYLIB_LIB)
	$(CXX) $(CALC_CXX_FLAGS) $(CALC_OBJ)  $(IMGUI_LIB) $(RL_IMGUI_LIB) $(RAYLIB_LIB) $(CALC_LINK_FLAGS) $(O)$(BUILD_DIR)/$@$(EXE)

endif

$(BUILD_DIR)/$(SRC_DIR)/%.$(OBJ): $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(MD) $(CALC_CXX_FLAGS) $(I)$(INC_DIR) $(EXTERNAL) $(C) $< $(FO)$@

# IMGUI
$(BUILD_DIR)/imgui/%.$(OBJ): $(IMGUI_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(MD) $(IMGUI_CXX_FLAGS) $(C) $< $(FO)$@

$(IMGUI_LIB): $(IMGUI_OBJ)
	$(AR) $(CR) $(OUT)$@ $^

# RLIMGUI
$(BUILD_DIR)/rlimgui/%.$(OBJ): $(RL_IMGUI_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(MD) $(RL_IMGUI_CXX_FLAGS) $(I)$(IMGUI_DIR) $(I)$(INC_DIR) $(EXTERNAL) $(C) $< $(FO)$@

$(RL_IMGUI_LIB): $(RL_IMGUI_OBJ)
	$(AR) $(CR) $(OUT)$@ $^

# RAYLIB
$(RAYLIB_LIB):
	$(MAKE) -C $(RAYLIB_DIR)/src
	mkdir -p $(BUILD_DIR)
	cp $(RAYLIB_DIR)/src/libraylib.a $@

clean:
	rm -rf $(BUILD_DIR)/$(SRC_DIR)

cleanall:
	rm -rf $(BUILD_DIR)