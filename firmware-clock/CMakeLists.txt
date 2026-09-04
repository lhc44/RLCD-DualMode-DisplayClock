# 配置天气时钟 ESP-IDF 工程版本、项目名和额外组件目录。
cmake_minimum_required(VERSION 3.16)
set(PROJECT_VER "v1.5.41")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include($ENV{IDF_PATH}/tools/cmake/version.cmake)
set(EXTRA_COMPONENT_DIRS
    components/ExternLib/SensorLib
    components/ExternLib/esp_codec_dev
    components/ExternLib/codec_board
    components/ExternLib/leeebo__tinyusb_src)
if(IDF_VERSION_MAJOR GREATER_EQUAL 6)
    # ESP-IDF 6 removed its in-tree json component. ESP-SR 2.3 still names
    # that legacy component, so provide a small adapter backed by cJSON.
    list(APPEND EXTRA_COMPONENT_DIRS components/ExternLib/json)
endif()
project(weather_clock)
