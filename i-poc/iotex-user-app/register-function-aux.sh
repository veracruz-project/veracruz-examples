#!/bin/bash

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

function load_file_veracruz() {
	FUNCTION_NAME=$1
	TYPE_LOAD=$2 # program or data_file
	PROGRAM_NAME=$3
	LOCAL_FILE_NAME=$4

	if [ ! -e "${FILE_NAME}" ]
	then
		echo "ERROR: file \"${FILE_NAME}\" does not exist, bailing out"
		exit 1
	fi

	pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-file="${LOCAL_FILE_NAME}" http://${CCFAAS_HOST}:${CCFAAS_PORT}/function/${FUNCTION_NAME}/${TYPE_LOAD}/$(echo -n "${PROGRAM_NAME}" | base64) 2>&1)"
}
