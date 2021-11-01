#!/bin/bash
# Execute and read results of a program in a Veracruz instance 
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz I-PoC
# example repository root directory for copyright and licensing information.
#!/bin/bash

# echo "$0 $*" > /tmp/log.txt

function check_if_file_exists() {
	if [ ! -e "$1" ]
	then
		echo "$0: $2 file $1 does not exist"
		exit 1
	fi
}



VERACRUZ_CLIENT=$(pwd)/veracruz-client

if [ $# -lt 5 ]
then
	echo "$0: <policy> <certificate file out> <key file out> <output file veracruz> <output file name>"
	exit 1
fi

POLICY=$1
CERTIFICATE_OUT=$2
KEY_OUT=$3
OUTPUT_VERACRUZ=$4
OUTPUT_FILE_NAME=$5

check_if_file_exists "${POLICY}" "Policy"
check_if_file_exists "${CERTIFICATE_OUT}" "Certificate_out"
check_if_file_exists "${KEY_OUT}" "Key_out"

openssl rsa -in "${KEY_OUT}" -out "${KEY_OUT}.RSA.pem"

VERACRUZ_URL=$(grep veracruz_server_url "${POLICY}" | sed -e 's/^[^:]*: *\"//' -e 's/".*//')
VERACRUZ_HOST=$(echo "${VERACRUZ_URL}" | cut -d ":" -f 1)
VERACRUZ_PORT=$(echo "${VERACRUZ_URL}" | cut -d ":" -f 2)

# echo "Checking if ${VERACRUZ_HOST} ${VERACRUZ_PORT} is alive" >> /tmp/log.txt
OK=0
CUR_TIME=$(date +"%s")
MAX_TIME=$((${CUR_TIME}+120))
# echo "Waiting from ${CUR_TIME} to ${MAX_TIME} to see if is alive" >> /tmp/log.txt
while [ $(date +"%s") -le ${MAX_TIME} ]
do
# 	echo "Another try at "$(date +"%s") >> /tmp/log.txt
	nc -z -w 2 "${VERACRUZ_HOST}" "${VERACRUZ_PORT}" 
	if [ $? -eq 0 ]
	then
		OK=1
		break
	fi
	sleep 1
done
# date +"%Y%m%d%H%M%S" >> /tmp/log.txt
if [ ${OK} -eq 0 ]
then
# 	echo "${VERACRUZ_HOST} ${VERACRUZ_PORT} is not responding after 120 seconds" >> /tmp/log.txt
	exit 1
fi

echo ${VERACRUZ_CLIENT} ${POLICY} --results "${OUTPUT_VERACRUZ}=${OUTPUT_FILE_NAME}" --identity ${CERTIFICATE_OUT} --key ${KEY_OUT}
OUTPUT=$(${VERACRUZ_CLIENT} "${POLICY}" --results "${OUTPUT_VERACRUZ}=${OUTPUT_FILE_NAME}" --identity "${CERTIFICATE_OUT}" --key "${KEY_OUT}.RSA.pem" 2>&1)
echo "${OUTPUT}"
NOK=$(echo "${OUTPUT}" | grep "Error")
if [ ! -z "${NOK}" ]
then
	exit 1
fi

# echo "Exited OK" >> /tmp/log.txt
exit 0
