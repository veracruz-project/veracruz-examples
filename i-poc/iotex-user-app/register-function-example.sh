#!/bin/bash
# Register the linear-regression function at CCFaaS
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz I-PoC
# example repository root directory for copyright and licensing information.

function check_wget_response() {
	ERROR_CODE=0
	if [ "${RETURN_CODE}" -eq 0 ]
	then
		return
	fi
	if [ "${RETURN_CODE}" -ne 8 ]
	then
		echo "wget returned ${RETURN_CODE} error code"
		exit 1
	fi

	# server error 
	ERROR_CODE=$(echo "${RESULT}" | grep "HTTP/" | sed -e "s/^[ ]*HTTP\/[0-9.]* \([0-9]*\) .*/\1/")
	if [ -z "$*" ]
	then
		echo "Error code ${ERROR_CODE} unexpectly received"
		exit 1
	fi

	FOUND=$(echo "$*" | grep "${ERROR_CODE}" 2>/dev/null)
	if [ -z "${FOUND}" ]
	then
		echo "Error code ${ERROR_CODE} unexpectly received"
		exit 1
	fi
	case ${ERROR_CODE} in
		404)
			echo "Not found";;
		*)
			echo "Error code ${ERROR_CODE} received";;
	esac
}

function pretty_print_wget() {
 	# $1 method
 	# $2 URL
 	# $3 data
	if [ $# -lt 2 ]
	then
		echo "pretty_print_wget needs at least 2 parameters, called with: $*"
		exit 1
	fi
	case $1 in
		get)
			RESULT=$(wget --server-response -q -tries=1 -O ${TMPDIRUSE}/wget_output "http://${HOSTIP}:${HOSTPORT}/$2" 2>&1)
			RETURN_CODE=$? 
			;;
		post)
			RESULT=$(wget --server-response -q -tries=1 -O ${TMPDIRUSE}/wget_output --header=Content-Type:application/json --post-data="$3" "http://${HOSTIP}:${HOSTPORT}/$2" 2>&1)
			RETURN_CODE=$? 
			;;	
		postfile)
			RESULT=$(wget --server-response -q -tries=1 -O ${TMPDIRUSE}/wget_output --header=Content-Type:application/json --post-file="$3" "http://${HOSTIP}:${HOSTPORT}/$2" 2>&1)
			RETURN_CODE=$? 
			;;	
		*)
			RESULT=$(wget --server-response -q -tries=1 -O ${TMPDIRUSE}/wget_output --method=$1 "http://${HOSTIP}:${HOSTPORT}/$2" 2>&1)
			RETURN_CODE=$? 
			;;
	esac
	if [ "${RETURN_CODE}" -eq 0 ]
	then
		(jq -C 2>/dev/null < ${TMPDIRUSE}/wget_output || cat ${TMPDIRUSE}/wget_output) | sed -e "s/^/    /";echo
	fi
}

if [ $# -gt 2 ]
then
	HOSTIP="$1"
	HOSTPORT="$2"
else
	HOSTIP=$(kubectl get service ccfaas-server-app  -o custom-columns=:.spec.externalIPs[0] --no-headers 2>/dev/null)
	if [ "${HOSTIP}" == "<none>" ]
	then
		HOSTIP=""
	fi
	HOSTPORT=$(kubectl get service ccfaas-server-app  -o custom-columns=:.spec.ports[0].port --no-headers 2>/dev/null)
	if [ "${HOSTPORT}" == "<none>" ]
	then
		HOSTPORT=""
	fi
fi

TMPDIRUSE=$(mktemp -d "${TMPDIR:-/tmp/}$(basename $0).XXXXXXXXXXXX")

if [ -z "${HOSTIP}" -o -z "${HOSTPORT}" ]
then
	echo "Not able to find HOSTIP and HOSTPORT for CCFaaS"
	exit 1
fi

echo "Accessing CCFaaS at ${HOSTIP}:${HOSTPORT}"

echo "Getting function/linear-regression"
pretty_print_wget get "function/linear-regression"
check_wget_response 404
if [ "$ERROR_CODE" != "404" ]
then
	echo "Deleting function/linear-regression"
	pretty_print_wget delete "function/linear-regression"
	check_wget_response 

	echo "Getting function/linear-regression"
	pretty_print_wget get "function/linear-regression"
	check_wget_response 404
fi

echo "linear-regression function with data below"
JSON_INPUT_OBJECT='
{
  "function": "linear-regression",
  "execution_strategy": "Interpretation",
  "programs": [
    {
      "id": 0,
      "pi_hash": "3fc011587de8a340c0ee36d733c3e52a42babc5fe6b12a074d94204495fd5877",
      "program_file_name": "linear-regression.wasm",
      "file_rights": [
        {
          "file_name": "input-0",
          "rights": 8198
        },
        {
          "file_name": "output",
          "rights":  533572
        }
      ]
    }
  ]
}'

echo "${JSON_INPUT_OBJECT}" | jq

echo "Register linear-regression function"
pretty_print_wget post "function" "${JSON_INPUT_OBJECT}"
check_wget_response

echo "Create linear-regression  program linear-regression.wasm"
pretty_print_wget postfile "function/linear-regression/program/linear-regression.wasm" linear-regression.wasm
check_wget_response

rm -rf "${TMPDIRUSE}"
