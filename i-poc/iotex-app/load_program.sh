#!/bin/bash

POLICY=$1
PROGRAM_FILE_NAME=$2

i=0
while [ $i -lt 10 ]
do
	OUTPUT=$(./veracruz-client ${POLICY} -p ${PROGRAM_FILE_NAME} --identity program_client_cert.pem --key program_client_key.pem 2>&1)

	OK=$(echo "${OUTPUT}" | grep "Reqwest: Error")

	if [ -z "${OK}" ]
	then
		exit 0
	fi

	sleep 2
	i=$(($i+1))
done
# Not able to load the program into veracruz instance
exit 1



