#!/bin/bash

if [ $# -ne 2 ]
then
    echo "Usage: $0 <IP to connect> <policy to use>"
    exit -1
fi

HOSTIP=$1
POLICY=$2

if [ -e ${POLICY} ]
then
	echo "File with policy '${POLICY}' not found"
	exit -1
fi

INSTANCE_HASH=$2
INSTANCE_ID=$3

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

HOSTPORT=5000

JSON_INPUT_OBJECT=$(cat ${POLICY})

START_OPERATION=$(date +"%s")

pretty_print_wget "$(wget -q --content-on-error -nv  -O- http://${HOSTIP}:${HOSTPORT}/veracruz)"

END_OPERATION=$(date +"%s")
echo "Total time="$((${END_OPERATION}-${START_OPERATION}))

START_OPERATION=$(date +"%s")

pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-data="${JSON_INPUT_OBJECT}" --header='Content-Type:application/json' http://${HOSTIP}:${HOSTPORT}/veracruz 2>&1)" veracruz-policy.json full.output

END_OPERATION=$(date +"%s")
echo "Total time="$((${END_OPERATION}-${START_OPERATION}))

exit 0
