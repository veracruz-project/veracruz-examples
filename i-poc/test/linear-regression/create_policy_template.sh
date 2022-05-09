#!/bin/bash

function convert_cert() {
	INPUT_FILE=$1
	
	OUT_CERT=$(sed -e 's/\(BEGIN CERTIFICATE-----\)$/\1\\n/g' -e 's/^\(-----END CERTIFICATE\)/\\n\1/g' ${INPUT_FILE} | tr -d '\n' )
}
	
convert_cert data_client_cert.pem
INPUT_CERTIFICATE="${OUT_CERT}"

convert_cert program_client_cert.pem
PROGRAM_CERTIFICATE="${OUT_CERT}"

convert_cert result_client_cert.pem
OUTPUT_CERTIFICATE="${OUT_CERT}"

cat << !EOF > linear_regression_policy.json
{
    "ciphersuite": "TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256",
    "debug": true,
    "enable_clock": true,
    "execution_strategy": "JIT",
    "file_hashes": [
      {
          "file_path": "/program/linear-regression.wasm",
          "hash": "df371ac9dc5f492813e305d6989feb10faf9d1cb3f4b81bb0e5f768b1268e131"
      }
    ],
    "identities": [
        {
            "certificate": "${PROGRAM_CERTIFICATE}",
            "file_rights": [
                {
                    "file_name": "/program/",
                    "rights": 550470
                }
            ],
            "id": 0
        },
        {
            "certificate": "${INPUT_CERTIFICATE}",
            "file_rights": [
                {
                    "file_name": "/input/",
                    "rights": 534084
                }
            ],
            "id": 1
        },
        {
            "certificate": "${OUTPUT_CERTIFICATE}",
            "file_rights": [
                {
                    "file_name": "/output/",
                    "rights": 24582
                },
                {
                    "file_name": "/program/",
                    "rights": 24582
                },
                {
                    "file_name": "stdout",
                    "rights": 24582
                }
            ],
            "id": 2
        }
    ],
    "programs": [
        {
            "file_rights": [
                {
                    "file_name": "/input/",
                    "rights": 24582
                },
                {
                    "file_name": "/output/",
                    "rights": 550470
                }
            ],
            "id": 0,
            "program_file_name": "/program/linear-regression.wasm"
        }
    ]
}
!EOF
