CC = clang++
CC_FLAGS = -std=c++23

WX_WIDGETS_LINK = $(shell ~/Developer/Libraries/wxWidgets/macbuild/wx-config --cxxflags --libs)


SRC_DIR = ./src
INCLUDE_DIR=./include/
BUILD_DIR = ./build


CPP_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CPP_SOURCES))


TARGET=prog


all: clean $(BUILD_DIR)/$(TARGET)
	@echo "Done!"


$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	@echo "Linking $<"
	@ $(CC) $(CC_FLAGS) $(OBJECTS) $(SOURCE_LIBS) $(WX_WIDGETS_LINK) -o $@ 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Building $@"
	@ $(CC) $(CC_FLAGS) $(SOURCE_LIBS) $(WX_WIDGETS_LINK) -c $< -o $@ 

clean:
	@echo "Cleaning..."
	@rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/$(TARGET)


.PHONY: all clean


