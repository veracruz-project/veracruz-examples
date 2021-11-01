#!/bin/bash

VERACRUZ_CLIENT=$(pwd)/veracruz-client

if [ $# -lt 4 ]
then
	echo "$0: <policy> <certificate file> <key file> [<program files to load>]"
	exit 1
fi

POLICY="$1"
[[ "${POLICY}" =~ ^/ ]] || POLICY="$(pwd)/${POLICY}"
CERTIFICATE="$2"
[[ "${CERTIFICATE}" =~ ^/ ]] || CERTIFICATE="$(pwd)/${CERTIFICATE}"
KEY=$3
[[ "${KEY}" =~ ^/ ]] || KEY="$(pwd)/${KEY}"
PROGRAM_DIR="$4"

shift 4

VERACRUZ_URL=$(grep veracruz_server_url "${POLICY}" | sed -e 's/^[^:]*: *\"//' -e 's/".*//')

VERACRUZ_HOST=$(echo "${VERACRUZ_URL}" | cut -d ":" -f 1)
VERACRUZ_PORT=$(echo "${VERACRUZ_URL}" | cut -d ":" -f 2)

OK=0
CUR_TIME=$(date +"%s")
MAX_TIME=$((${CUR_TIME}+120))
while [ $(date +"%s") -le ${MAX_TIME} ]
do
	nc -z -w 2 "${VERACRUZ_HOST}" "${VERACRUZ_PORT}" 
	if [ $? -eq 0 ]
	then
		OK=1
		break
	fi
	sleep 1
done
if [ ${OK} -eq 0 ]
then
	exit 1
fi

while [ $# -gt 0 ]
do
	PROGRAM_FILE=$1
	
        pushd ${PROGRAM_DIR} > /dev/null
# 	echo "Executing: ${VERACRUZ_CLIENT} ${POLICY} -p ${PROGRAM_FILE} --identity ${CERTIFICATE} --key ${KEY}" >> /tmp/log.txt
	OUTPUT=$(${VERACRUZ_CLIENT} ${POLICY} -p "${PROGRAM_FILE}" --identity "${CERTIFICATE}" --key "${KEY}" 2>&1)
	RESULT_CODE=$?
	popd > /dev/null
	echo "${OUTPUT}"

	NOK=$(echo "${OUTPUT}" | grep "Error")

	if [ ! -z "${NOK}" -o ${RESULT_CODE} -gt 0  ]
	then
		echo "${OUTPUT}" >> /tmp/error.log.txt
		exit 1
	fi

	shift 1
done

exit 0
