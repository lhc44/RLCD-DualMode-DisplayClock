# Local ESP-IDF 6.0.2 toolchain bridge.
# This installation exposes the Windows Xtensa tools only as .exe files;
# make their absolute locations explicit after importing Espressif's flags.
set(_CMAKE_TOOLCHAIN_PREFIX xtensa-esp32s3-elf-)
include("$ENV{IDF_PATH}/tools/cmake/toolchain.cmake")
set(_RLCD_XTENSA "C:/Espressif/tools/xtensa-esp-elf/esp-15.2.0_20251204/xtensa-esp-elf/bin")
set(CMAKE_C_COMPILER "${_RLCD_XTENSA}/xtensa-esp32s3-elf-gcc.exe")
set(CMAKE_CXX_COMPILER "${_RLCD_XTENSA}/xtensa-esp32s3-elf-g++.exe")
set(CMAKE_ASM_COMPILER "${_RLCD_XTENSA}/xtensa-esp32s3-elf-gcc.exe")
