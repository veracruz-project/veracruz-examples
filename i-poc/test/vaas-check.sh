#!/bin/bash

if [ $# -ne 1 ]
then
    echo "Usage: $0 <IP to connect>"
    exit -1
fi

HOSTIP=$1

function pretty_print_wget() {
	echo "$1" | jq 2>/dev/null
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

pretty_print_wget "$(wget -q --content-on-error -nv  -O- http://${HOSTIP}:${HOSTPORT}/veracruz)"

END_OPERATION=$(date +"%s")

exit 0
