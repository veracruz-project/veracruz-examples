#!/bin/bash

CCFAAS_HOST=${1:-"127.0.0.1"}
CCFAAS_PORT=${2:-"5010"}

echo "Acessing CCFaaS at ${CCFAAS_HOST}:${CCFAAS_PORT}"

JSON_FUNCTION_POLICY=$(cat function_policy_small.json)
FUNCTION_NAME="vod_small"

BUNDLE_PATH="."
client_path="../../veracruz-client"
PROGRAM_PATH="."

. register-function-aux.sh

echo "=============Deleting function"
pretty_print_wget "$(curl -X DELETE http://${CCFAAS_HOST}:${CCFAAS_PORT}/function/${FUNCTION_NAME} 2>&1)"

echo "=============Registering function"
pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-data="${JSON_FUNCTION_POLICY}" --header='Content-Type:application/json' http://${CCFAAS_HOST}:${CCFAAS_PORT}/function 2>&1)"

echo "=============Provisioning program"
load_file_veracruz "${FUNCTION_NAME}" "program" "/program/detector.wasm" "$PROGRAM_PATH/detector.wasm" 
#pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-file=$PROGRAM_PATH/detector.wasm --header='Content-Type:application/json' http://${CCFAAS_HOST}:${CCFAAS_PORT}/function/${FUNCTION_NAME}/program/$(echo -n "/program/detector.wasm" | base64) 2>&1)"

echo "=============Provisioning data"
load_file_veracruz "${FUNCTION_NAME}" "data_file" "/program_data/coco.names" "$BUNDLE_PATH/input/coco.names"
load_file_veracruz "${FUNCTION_NAME}" "data_file" "/program_data/yolov3.cfg" "$BUNDLE_PATH/input/yolov3-tiny.cfg"
load_file_veracruz "${FUNCTION_NAME}" "data_file" "/program_data/yolov3.weights" "$BUNDLE_PATH/input/yolov3-tiny.weights"
#pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-file=$BUNDLE_PATH/input/coco.names --header='Content-Type:application/json' http://${CCFAAS_HOST}:${CCFAAS_PORT}/function/${FUNCTION_NAME}/data_file/$(echo -n "/program_data/coco.names" | base64) 2>&1)"
#pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-file=$BUNDLE_PATH/input/yolov3-tiny.cfg --header='Content-Type:application/json' http://${CCFAAS_HOST}:${CCFAAS_PORT}/function/${FUNCTION_NAME}/data_file/$(echo -n "/program_data/yolov3.cfg" | base64) 2>&1)"
#pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-file=$BUNDLE_PATH/input/yolov3-tiny.weights --header='Content-Type:application/json' http://${CCFAAS_HOST}:${CCFAAS_PORT}/function/${FUNCTION_NAME}/data_file/$(echo -n "/program_data/yolov3.weights" | base64) 2>&1)"
