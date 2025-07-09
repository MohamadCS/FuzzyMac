cmake_minimum_required(VERSION 3.18)
project(FuzzyMac LANGUAGES C CXX OBJCXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

enable_language(OBJCXX)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB CPP_SOURCES src/*.cpp)
file(GLOB CPP_HEADERS include/FuzzyMac/*.hpp)
file(GLOB MM_SOURCES  src/*.mm)

set(SOURCES ${CPP_SOURCES} ${MM_SOURCES})
set(HEADERS ${CPP_HEADERS})


set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@6/lib/cmake")

find_package(Qt6 COMPONENTS Core Gui Widgets Concurrent REQUIRED)


# =========================
# GUI App Target (FuzzyMac)
# =========================

add_executable(FuzzyMac ${SOURCES} ${HEADERS})

target_compile_definitions(FuzzyMac PRIVATE)

target_link_libraries(FuzzyMac PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Concurrent "-framework Carbon" "-framework AppKit")

set_target_properties(FuzzyMac PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "FuzzyMac"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.fuzzymac.app"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/res/Info.plist"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    MACOSX_BUNDLE_INFO_STRING "A Fuzzy finder for macOs"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/app/"
)





# =========================
# CLI Target (FuzzyMacCLI)
# =========================


add_executable(FuzzyMacCLI ${SOURCES} ${HEADERS})

target_compile_definitions(FuzzyMacCLI PRIVATE CLI_TOOL)

target_link_libraries(FuzzyMacCLI PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Concurrent "-framework Carbon" "-framework AppKit")

set_target_properties(FuzzyMacCLI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/cli/"
)

