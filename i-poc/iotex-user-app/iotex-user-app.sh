#!/bin/bash

export AWS_ACCESS_KEY_ID="<REPLACE WITH AWS_ACCESS_KEY_ID>"
export AWS_SECRET_ACCESS_KEY="<REPLACE WITH AWS_SECRET_ACCESS_KEY>"
export AWS_SESSION_TOKEN="<REPLACE WITH AWS_SESSION_TOKEN>"

S3_REGION="<REPLACE WITH S3 REGION>"
S3_BUCKET="<REPLACE WITH S3 BUCKET"
S3_FILE="<REPLACE WITH S3 FILE?"

CCFAAS_HOSTIP=$(kubectl get service ccfaas-server-app  -o custom-columns=:.spec.externalIPs[0] --no-headers 2>/dev/null)
if [ "${CCFAAS_HOSTIP}" == "<none>" ]
then
	echo "CCFaaS service not found"
	exit 1
fi
CCFAAS_HOSTPORT=$(kubectl get service ccfaas-server-app  -o custom-columns=:.spec.ports[0].port --no-headers 2>/dev/null)
if [ "${CCFAAS_HOSTPORT}" == "<none>" ]
then
	echo "CCFaaS service port not found"
	exit 1
fi

CCFAAS_URL="http://${CCFAAS_HOSTIP}:${CCFAAS_HOSTPORT}"

IOTEX_S3_HOSTIP=$(kubectl get service iotex-s3-app  -o custom-columns=:.spec.externalIPs[0] --no-headers 2>/dev/null)
if [ "${IOTEX_S3_HOSTIP}" == "<none>" ]
then
	echo "CCFaaS service not found"
	exit 1
fi
IOTEX_S3_HOSTPORT=$(kubectl get service iotex-s3-app -o custom-columns=:.spec.ports[0].port --no-headers 2>/dev/null)
if [ "${IOTEX_S3_HOSTPORT}" == "<none>" ]
then
	echo "CCFaaS service port not found"
	exit 1
fi

IOTEX_S3_URL="http://${IOTEX_S3_HOSTIP}:${IOTEX_S3_HOSTPORT}"


echo "CCFaaS in ${CCFAAS_URL} and Iotex-s3 Faas in ${IOTEX_S3_URL}"


./iotex-user-app.py "test1" "${CCFAAS_URL}" "${IOTEX_S3_URL}" "${S3_BUCKET}" "${S3_FILE}" "region_name=${S3_REGION}" "aws_access_key_id=${AWS_ACCESS_KEY_ID}" "aws_secret_access_key=${AWS_SECRET_ACCESS_KEY}"
