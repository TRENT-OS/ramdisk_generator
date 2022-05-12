#
# RamDisk generator
#
# Copyright (C) 2019-2022, HENSOLDT Cyber GmbH
#

cmake_minimum_required(VERSION 3.10)

#-------------------------------------------------------------------------------
# use the SDK
find_package("os-sdk" REQUIRED)
os_sdk_create_config_project("os_sdk_config" "system_config.h")
os_sdk_import_core_api()
os_sdk_import_libs()


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
        os_sdk_config
        os_core_api
        lib_utils
)
