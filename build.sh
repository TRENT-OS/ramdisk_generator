#!/bin/bash -ue

BUILD_SCRIPT_DIR=$(cd `dirname $0` && pwd)

# by convention, the build always happens in a dedicated sub folder of the
# current folder where the script is invoked in
BUILD_DIR=build_rdgen

if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters, OS_SDK_PATH needed!"
    exit 1
fi
OS_SDK_PATH=$1
if [[ ! -e ${OS_SDK_PATH} ]]; then
    echo "OS_SDK_PATH invalid: ${OS_SDK_PATH}"
    exit 1
fi


if [[ -e ${BUILD_DIR} ]] && [[ ! -e ${BUILD_DIR}/rules.ninja ]]; then
    echo "clean broken build folder and re-initialize it"
    rm -rf ${BUILD_DIR}
fi

if [[ ! -e ${BUILD_DIR} ]]; then
    # use subshell to configure the build
    (
        mkdir -p ${BUILD_DIR}
        ABS_OS_SDK_PATH=$(realpath ${OS_SDK_PATH})
        cd ${BUILD_DIR}

        CMAKE_PARAMS=(
            -D OS_SDK_PATH:PATH=${ABS_OS_SDK_PATH}

            # Build type is release with debugging info so that binaries
            # are at the same time optimized and debug-able, what might be
            # useful when analyzing tool related issues.
            -D CMAKE_BUILD_TYPE=RelWithDebInfo
        )

        cmake ${CMAKE_PARAMS[@]} -G Ninja ${BUILD_SCRIPT_DIR}
    )
fi

# build in subshell
(
    cd ${BUILD_DIR}
    ninja
)
