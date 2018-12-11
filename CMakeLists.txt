# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Zenty)

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

