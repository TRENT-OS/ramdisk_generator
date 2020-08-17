#
# RamDisk generator
#
# Copyright (C) 2019-2020, Hensoldt Cyber GmbH
#

cmake_minimum_required(VERSION 3.10)

#-------------------------------------------------------------------------------
# configure and use the SDK

set(OS_SDK_PASSIVE_CMAKE ON)
if (NOT OS_SDK_PATH)
    message(FATAL_ERROR "OS_SDK_PATH not set")
endif()
if (NOT EXISTS "${OS_SDK_PATH}")
    message(FATAL_ERROR "OS_SDK_PATH not found: ${OS_SDK_PATH}")
endif()
include("${OS_SDK_PATH}/CMakeLists.txt")

os_set_config_file("system_config.h" CREATE_CONFIG_PROJECT)
add_os_libs_subdirectories()


#-------------------------------------------------------------------------------
project(rdgen C)

add_executable(${PROJECT_NAME}
    rdgen.c
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Werror
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        os_core_api
        os_libs
)
