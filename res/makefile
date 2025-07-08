CC = clang++
CC_FLAGS = -std=c++23 
WX_FLAGS = $(shell ~/Developer/Libraries/wxWidgets/macbuild/wx-config --cxxflags)
WX_LD = -framework Carbon -framework AppKit $(shell ~/Developer/Libraries/wxWidgets/macbuild/wx-config --libs)

SRC_DIR = ./src
INCLUDE_DIR = ./include/
BUILD_DIR = ./build
CLI_DIR = ./cli
UPDATE_BUILD = cp $(BUILD_DIR)/$(TARGET) ./FuzzyMac.app/Contents/MacOS/$(TARGET)

# Source files
CPP_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
MM_SOURCES = $(wildcard $(SRC_DIR)/*.mm)

# Object files
CPP_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CPP_SOURCES))
MM_OBJECTS  = $(patsubst $(SRC_DIR)/%.mm,  $(BUILD_DIR)/%.mm.o, $(MM_SOURCES))
OBJECTS     = $(CPP_OBJECTS) $(MM_OBJECTS)

TARGET = FuzzyMac

CLI ?= 0

ifeq ($(CLI),1)
	CC_FLAGS += -DCLI_TOOL
	UPDATE_BUILD = cp $(BUILD_DIR)/$(TARGET) $(CLI_DIR)/$(TARGET)
endif





# Default target
all: clean $(BUILD_DIR)/$(TARGET)
	@echo "✅ Build succeeded!"
	@$(UPDATE_BUILD)
	@killall $(TARGET) 2> /dev/null || true

# Link step
$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	@echo "🔗 Linking $@"
	@$(CC) $(CC_FLAGS) $(OBJECTS) $(WX_LD) $(WX_FLAGS) $(WX_WIDGETS_LINK) -o $@

# Build .cpp files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "⚙️  Compiling C++ $<"
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CC_FLAGS) -I$(INCLUDE_DIR) $(WX_FLAGS) -c $< -o $@

# Build .mm files
$(BUILD_DIR)/%.mm.o: $(SRC_DIR)/%.mm
	@echo "⚙️  Compiling Objective-C++ $<"
	@mkdir -p $(BUILD_DIR)
	@$(CC) -x objective-c++ $(CC_FLAGS) $(WX_FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Clean
clean:
	@echo "🧹  Cleaning..."
	@rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.mm.o $(BUILD_DIR)/$(TARGET)

.PHONY: all clean

