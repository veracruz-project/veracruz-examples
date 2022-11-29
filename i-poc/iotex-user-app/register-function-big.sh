#!/bin/bash

HOSTIP=127.0.0.1
HOSTPORT=5010
JSON_FUNCTION_POLICY=$(cat function_policy_big.json)
FUNCTION_NAME="vod_big"

BUNDLE_PATH="."
client_path="../../veracruz-client"
PROGRAM_PATH="."

function pretty_print_wget() {
	echo "$1" | jq 2>/dev/null
	if [ $? -gt 0 ]
	then
		echo "$1"
	else
		if [ ! -z "$2" ]
		then
			echo "$1" | jq 2>/dev/null > $2
		fi
	fi
	if [ ! -z "$3" ]
	then
		echo "$1" >  $3
	fi
}

echo "=============Deleting function"
pretty_print_wget "$(curl -X DELETE http://${HOSTIP}:${HOSTPORT}/function/${FUNCTION_NAME} 2>&1)"

echo "=============Registering function"
pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-data="${JSON_FUNCTION_POLICY}" --header='Content-Type:application/json' http://${HOSTIP}:${HOSTPORT}/function 2>&1)"

echo "=============Provisioning program"
pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-file=$PROGRAM_PATH/detector.wasm --header='Content-Type:application/json' http://${HOSTIP}:${HOSTPORT}/function/${FUNCTION_NAME}/program/$(echo -n "/program/detector.wasm" | base64) 2>&1)"

echo "=============Provisioning data"
pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-file=$BUNDLE_PATH/input/coco.names --header='Content-Type:application/json' http://${HOSTIP}:${HOSTPORT}/function/${FUNCTION_NAME}/data_file/$(echo -n "/program_data/coco.names" | base64) 2>&1)"
pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-file=$BUNDLE_PATH/input/yolov3.cfg --header='Content-Type:application/json' http://${HOSTIP}:${HOSTPORT}/function/${FUNCTION_NAME}/data_file/$(echo -n "/program_data/yolov3.cfg" | base64) 2>&1)"
pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-file=$BUNDLE_PATH/input/yolov3.weights --header='Content-Type:application/json' http://${HOSTIP}:${HOSTPORT}/function/${FUNCTION_NAME}/data_file/$(echo -n "/program_data/yolov3.weights" | base64) 2>&1)"
