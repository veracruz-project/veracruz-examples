#!/bin/bash

HOSTIP=$1
HOSTPORT=5000
JSON_PARTIAL_POLICY="$(cat linear_regression_policy.json)"

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

echo "=============Submitting partial policy and getting full policy"
pretty_print_wget "$(wget -q --content-on-error -nv  -O- --post-data="${JSON_PARTIAL_POLICY}" --header='Content-Type:application/json' http://${HOSTIP}:${HOSTPORT}/veracruz 2>&1)" veracruz-policy.json full.output

echo "=============Extracting useful part"
cat ./full.output | jq -r ".policy" | head -c -1 > policy.json

echo "=============Waiting for Veracruz deployment"
sleep 5

echo "=============Provisioning program"
$client_path $BUNDLE_PATH/policy.json \
    --program /program/linear-regression.wasm=$PROGRAM_PATH/linear-regression.wasm \
    --identity $BUNDLE_PATH/program_client_cert.pem \
    --key $BUNDLE_PATH/program_client_key.pem

echo "=============Provisioning data"
$client_path $BUNDLE_PATH/policy.json \
    --data /input/linear-regression.dat=$BUNDLE_PATH/linear-regression.dat \
    --identity $BUNDLE_PATH/data_client_cert.pem \
    --key $BUNDLE_PATH/data_client_key.pem

echo "=============Requesting computation"
$client_path $BUNDLE_PATH/policy.json \
    --compute /program/linear-regression.wasm \
    --identity $BUNDLE_PATH/result_client_cert.pem \
    --key $BUNDLE_PATH/result_client_key.pem

echo "=============Querying results"
$client_path $BUNDLE_PATH/policy.json \
    --result /output/linear-regression.dat=results.dat \
    --identity $BUNDLE_PATH/result_client_cert.pem \
    --key $BUNDLE_PATH/result_client_key.pem

echo "=============Removing veracruz instance"
INSTANCE_NAME=$(cat policy.json | jq ".veracruz_server_url")
INSTANCE_HASH=$(sha256sum policy.json)

pretty_print_wget "$(curl -X DELETE "http://${HOSTIP}:${HOSTPORT}/veracruz/${INSTANCE_NAME}?instance_hash=${INSTANCE_HASH}")"

exit 0
