cmake_minimum_required(VERSION 3.30)

project(gui
    VERSION 0.1.0
    LANGUAGES C CXX
)

option(GUI_ENABLE_INSPECTOR "Enable the debug inspector UI" OFF)
option(GUI_ENABLE_INSTRUMENTATION "Enable render and layout instrumentation" ${GUI_ENABLE_INSPECTOR})
option(GUI_PROFILE "Build with profiling-friendly frame pointers" OFF)
set(GUI_FRAME_WIDTH "512.0" CACHE STRING "Initial GUI content width in points")
set(GUI_FRAME_HEIGHT "512.0" CACHE STRING "Initial GUI content height in points")

if(GUI_FRAME_WIDTH LESS_EQUAL 0 OR GUI_FRAME_HEIGHT LESS_EQUAL 0)
    message(FATAL_ERROR "GUI_FRAME_WIDTH and GUI_FRAME_HEIGHT must be positive")
endif()
set(METAL_CPP_ROOT "/Users/treja/metal-cpp" CACHE PATH "Path to metal-cpp")
set(METAL_CPP_EXTENSIONS_ROOT "/Users/treja/metal-cpp-extensions" CACHE PATH "Path to metal-cpp-extensions")

add_subdirectory(apple-extensions)
add_subdirectory(src)
