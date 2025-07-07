cmake_minimum_required(VERSION 3.18)
project(FuzzyMac LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_language(OBJCXX)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB CPP_SOURCES src/*.cpp)
file(GLOB MM_SOURCES  src/*.mm)
set(SOURCES ${CPP_SOURCES} ${MM_SOURCES})

# wx-config flags (combined)
execute_process(
    COMMAND sh -c "~/Developer/Libraries/wxWidgets/macbuild/wx-config --libs"
    OUTPUT_VARIABLE WX_LDFLAGS_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND sh -c "~/Developer/Libraries/wxWidgets/macbuild/wx-config --cxxflags"
    OUTPUT_VARIABLE WX_CFLAGS_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

separate_arguments(WX_LDFLAGS NATIVE_COMMAND "${WX_LDFLAGS_RAW}")
separate_arguments(WX_CFLAGS NATIVE_COMMAND "${WX_CFLAGS_RAW}")

# =========================
# GUI App Target (FuzzyMac)
# =========================

add_executable(FuzzyMac MACOSX_BUNDLE ${SOURCES})

target_compile_options(FuzzyMac PRIVATE ${WX_CFLAGS})
target_link_libraries(FuzzyMac PRIVATE ${WX_LDFLAGS} "-framework Carbon" "-framework AppKit")

# Bundle settings
set_target_properties(FuzzyMac PROPERTIES
    MACOSX_BUNDLE TRUE
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/App"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/res/Info.plist"
)

# =========================
# CLI Target (FuzzyMacCLI)
# =========================
add_executable(FuzzyMacCLI ${SOURCES})

target_compile_definitions(FuzzyMacCLI PRIVATE CLI_TOOL)
target_compile_options(FuzzyMacCLI PRIVATE ${WX_CFLAGS})
target_link_libraries(FuzzyMacCLI PRIVATE ${WX_LDFLAGS} "-framework Carbon" "-framework AppKit")

# Set output directory to `cli/`
set_target_properties(FuzzyMacCLI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/cli/"
)

