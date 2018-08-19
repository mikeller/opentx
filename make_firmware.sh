#!/bin/bash

GIT_TAG=$(git describe --tags --abbrev=0)
FIRMWARE_DIR=build-output

while getopts ":cm:" ARG; do
    case ${ARG} in
        c)
            echo "Doing clean build."

            MAKE_CLEAN=true

            ;;
        m)
            echo "Building single model ${OPTARG}."

            BUILD_MODEL=${OPTARG}

            ;;
    esac
done

build_model () {
    model=$1
    echo "Building ${model}."
    pushd .
    DIR=build-${model}
    mkdir -p ${DIR}
    cd ${DIR}
    rm CMakeCache.txt
    getopts :c ARG
    if [ "${MAKE_CLEAN}" ]; then
        make clean
    fi
    cmake  -DMULTIMODULE=ON -DLUA_COMPILER=ON -DLOG_TELEMETRY=ON -DHELI=OFF -DDANGEROUS_MODULE_FUNCTIONS=ON -DPCB=${model} ../
    PATH=${OPENTX_ARM_TOOLCHAIN_PATH}:${PATH} make firmware
    cp firmware.bin ../${FIRMWARE_DIR}/opentx-firmware-${GIT_TAG}-${model}.bin
    popd
}

mkdir -p ${FIRMWARE_DIR}

if [ "${BUILD_MODEL}" ]; then
    build_model ${BUILD_MODEL}
else
    for BUILD_MODEL in X7 XLITE X9D X9D+ X9E X10 X12S SKY9X 9XRPRO; do
        build_model ${BUILD_MODEL}
    done
fi
