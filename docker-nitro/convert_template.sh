#!/bin/bash
# Create a policy the the generated keys and  hash from the nitro-image
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz I-PoC
# example repository root directory for copyright and licensing information.


#PROXY_CERTIFICATE=$(cat CACert.pem | sed -e 's[$[\\n[' | tr -d "\n" | sed -e 's[\\n$[[')
PROXY_CERTIFICATE=$(cat CACert.pem | sed -e 's[$[\\n[' | tr -d "\n")
PROG_CERTIFICATE=$(cat PROGCert.pem | sed -e 's[$[\\n[' | tr -d "\n" | sed -e 's|\([^-]\)\\n\([^-]\)|\1\2|g' -e 's[\\n$[[')
USER_CERTIFICATE=$(cat USERCert.pem | sed -e 's[$[\\n[' | tr -d "\n" | sed -e 's|\([^-]\)\\n\([^-]\)|\1\2|g' -e 's[\\n$[[')
EXPIRE_DATA_STR=$(date  --date 'now +6 months' +'"day": %-d,\n"hour": %-H,\n"minute": %-M,\n"month": %-m,\n"year": %-Y')
EXPIRE_DATE=$(echo -e ${EXPIRE_DATA_STR} | sed -e "s/^/        /")

PROGRAM_NAME="linear-regression.wasm"
PROGRAM_HASH=$(sha256sum ${PROGRAM_NAME} | cut -d " " -f 1)

HASH=$(cat hash)

cat << EOF | head -c -1 > dual_policy.json
{
    "ciphersuite": "TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256",
    "debug": false,
    "enable_clock": true,
    "execution_strategy": "Interpretation",
    "programs": [
        {
            "id": 0,
            "pi_hash": "${PROGRAM_HASH}",
            "program_file_name": "${PROGRAM_NAME}",
            "file_rights": [
                {
                    "file_name": "input-0",
                    "rights": 8198
                },
                {
                    "file_name": "output",
                    "rights": 533572
                }
            ]
        }
    ],
    "identities": [
        {
            "certificate": "${PROG_CERTIFICATE}",
            "file_rights": [
                {
                    "file_name": "${PROGRAM_NAME}",
                    "rights": 533572
                }
            ],
            "id": 0
        },
        {
            "certificate": "${USER_CERTIFICATE}",
            "file_rights": [
                {
                    "file_name": "input-0",
                    "rights": 533572
                },
                {
                    "file_name": "output",
                    "rights": 8198
                }
            ],
            "id": 1
        }
    ],
    "enclave_cert_expiry": {
${EXPIRE_DATE}
    },
    "proxy_attestation_server_url": "veracruz-proxy-${USER}:3010",
    "proxy_service_cert": "${PROXY_CERTIFICATE}",
    "runtime_manager_hash_nitro": "${HASH}",
    "runtime_manager_hash_sgx": "",
    "runtime_manager_hash_tz": "",
    "std_streams_table": [
        {
            "Stdin": {
                "file_name": "stdin",
                "rights": 8198
            }
        },
        {
            "Stdout": {
                "file_name": "stdout",
                "rights": 533572
            }
        },
        {
            "Stderr": {
                "file_name": "stderr",
                "rights": 533572
            }
        }
    ],
    "veracruz_server_url": "veracruz-server-${USER}:3014"
}
EOF
