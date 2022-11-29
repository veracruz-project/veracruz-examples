#!/bin/bash

if [ $# -lt 3 -o $# -gt 4 ]
then
    echo "Usage: vaas-delete <IP to connect> <instance name> <instance hash> [<instance id>]"
    exit -1
fi

HOSTIP=$1
INSTANCE_NAME=$2
INSTANCE_HASH=$3
INSTANCE_ID=$4

function pretty_print_wget() {
	echo "$1" | jq 2>/dev/null
	RES=$(echo "$1")
	if [ $? -gt 0 ]
	then
		echo "$1"
		if [ ! -z "$3" ]
		then
			echo "$1" >  $3
		fi
	else
		if [ ! -z "$2" ]
		then
			echo "$1" | jq > $2
		fi
	fi
}

#HOSTIP=54.216.123.218
HOSTPORT=5000

START_OPERATION=$(date +"%s")

pretty_print_wget "$(wget --content-on-error -nv  -O- http://${HOSTIP}:${HOSTPORT}/veracruz/veracruz-nitro-server:3014)"

END_OPERATION=$(date +"%s")

START_OPERATION=$(date +"%s")

if [ -z "${INSTANCE_ID}" ]
then
	echo "no ID"
	pretty_print_wget "$(curl -X DELETE "http://${HOSTIP}:${HOSTPORT}/veracruz/${INSTANCE_NAME}?instance_hash=${INSTANCE_HASH}")"
else
	pretty_print_wget "$(curl -X DELETE "http://${HOSTIP}:${HOSTPORT}/veracruz/${INSTANCE_NAME}?instance_hash=${INSTANCE_HASH}&instance_id=${INSTANCE_ID}")"
fi

END_OPERATION=$(date +"%s")

exit 0
