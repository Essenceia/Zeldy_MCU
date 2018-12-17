# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
set(ENV{IDF_PATH} "/home/rick/esp/esp-idf")
set(ENV{PATH} ENV{PATH} "$HOME/esp/xtensa-esp32-elf/bin")
set(ENV{PYTHON} "python2")
set(PYTHON "python2")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Zenldy)



add_custom_target(
        ESPRESSIF_BUILD_FLASH ALL
        COMMAND make flash
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
        ESPRESSIF_CONFIG ALL
        COMMAND make menuconfig
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_subdirectory(main)
#include(main/CMakeLists.txt)
#add_executable(${PROJECT_NAME} ${COMPONENT_SRCS})