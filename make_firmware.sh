#!/bin/bash

GIT_TAG=$(git describe --tags --abbrev=0)
FIRMWARE_DIR=build-output

mkdir -p ${FIRMWARE_DIR}

for MODEL in X7 XLITE X9D X9D+ X9E X10 X12S SKY9X 9XRPRO; do
    pushd .
    DIR=build-${MODEL}
    mkdir -p ${DIR}
    cd ${DIR}
    rm CMakeCache.txt
    getopts :c ARG
    if [ ${ARG} == 'c' ]; then
        make clean
    fi
    cmake  -DMULTIMODULE=ON -DLUA_COMPILER=ON -DLOG_TELEMETRY=ON -DHELI=OFF -DDANGEROUS_MODULE_FUNCTIONS=ON -DPCB=${MODEL} ../
    PATH=${OPENTX_ARM_TOOLCHAIN_PATH}:${PATH} make firmware
    cp firmware.bin ../${FIRMWARE_DIR}/opentx-firmware-${GIT_TAG}-${MODEL}.bin
    popd
done
