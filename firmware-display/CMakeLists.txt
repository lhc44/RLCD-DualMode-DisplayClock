# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# ESP-IDF's default target macro emits extensionless compiler names. Override
# only its toolchain-file selection so this Windows installation uses the
# absolute .exe paths in the local bridge above.
macro(__target_set_toolchain)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/toolchain-esp32s3.cmake")
endmacro()
add_compile_options(-Wno-ignored-qualifiers)
project(usb_touch_screen)
