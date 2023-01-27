#!/bin/bash

CCFAAS_HOST=${1:-"127.0.0.1"}
CCFAAS_PORT=${2:-"5010"}

echo "Acessing CCFaaS at ${CCFAAS_HOST}:${CCFAAS_PORT}"

FUNCTION_NAME="vod_big"


TAG="iotex-demo-v1.3.3"

BUNDLE_PATH="big-"${TAG}

. register-function-aux.sh

if [ ! -e "${BUNDLE_PATH}/detector.wasm" ]; then
		echo "=============Downloading tarball containing the program, program data (model and configuration) and example inputs (encrypted video, key, IV)"
		wget -q --content-on-error -nv https://github.com/veracruz-project/video-object-detection/releases/download/${TAG}/bundle_big.tar.gz
		mkdir ${BUNDLE_PATH}
		tar -xf bundle_big.tar.gz -C ${BUNDLE_PATH}
fi
PROGRAM_HASH=$(sha256sum "${BUNDLE_PATH}/detector.wasm" | cut -d " "  -f 1)
COCO_HASH=$(sha256sum "${BUNDLE_PATH}/coco.names" | cut -d " "  -f 1)
YOLOV3_CFG_HASH=$(sha256sum "${BUNDLE_PATH}/yolov3.cfg" | cut -d " "  -f 1)
YOLOV3_WEIGHTS_HASH=$(sha256sum "${BUNDLE_PATH}/yolov3.weights" | cut -d " "  -f 1)
JSON_FUNCTION_POLICY=$(cat function_policy_big.json | sed -e "s/PROGRAM_HASH/${PROGRAM_HASH}/g" -e "s/COCO_HASH/${COCO_HASH}/g" -e "s/YOLOV3_CFG_HASH/${YOLOV3_CFG_HASH}/g" -e "s/YOLOV3_WEIGHTS_HASH/${YOLOV3_WEIGHTS_HASH}/g")

echo "=============Deleting function"
pretty_print_wget "$(curl -X DELETE http://${CCFAAS_HOST}:${CCFAAS_PORT}/function/${FUNCTION_NAME} 2>&1)"

echo "=============Registering function"
pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-data="${JSON_FUNCTION_POLICY}" --header='Content-Type:application/json' http://${CCFAAS_HOST}:${CCFAAS_PORT}/function 2>&1)"

echo "=============Provisioning program"
load_file_veracruz "${FUNCTION_NAME}" "program" "/program/detector.wasm" "${BUNDLE_PATH}/detector.wasm" 

echo "=============Provisioning data"
load_file_veracruz "${FUNCTION_NAME}" "data_file" "/program_data/coco.names" "${BUNDLE_PATH}/coco.names"
load_file_veracruz "${FUNCTION_NAME}" "data_file" "/program_data/yolov3.cfg" "${BUNDLE_PATH}/yolov3.cfg"
load_file_veracruz "${FUNCTION_NAME}" "data_file" "/program_data/yolov3.weights" "${BUNDLE_PATH}/yolov3.weights"
