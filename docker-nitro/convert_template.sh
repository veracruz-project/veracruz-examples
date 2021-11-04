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


PROXY_CERTIFICATE=$(cat CACert.pem | sed -e 's[$[\\n[' | tr -d "\n" | sed -e 's[\\n$[[')
PROG_CERTIFICATE=$(cat PROGCert.pem | sed -e 's[$[\\n[' | tr -d "\n" | sed -e 's[\\n$[[')

HASH=$(cat hash)

cat << EOF > dual_policy.json
{
  "ciphersuite": "TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256",
  "debug": false,
  "enclave_cert_expiry": {
    "day": 23,
    "hour": 23,
    "minute": 44,
    "month": 12,
    "year": 2021
  },
  "execution_strategy": "Interpretation",
  "identities": [
    {
      "certificate": "${PROG_CERTIFICATE}"
      "file_rights": [
        {
          "file_name": "linear-regression.wasm",
          "rights": 533572
        }
      ],
      "id": 0
    },
    {
      "certificate": "${PROG_CERTIFICATE}"
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
  "programs": [
    {
      "file_rights": [
        {
          "file_name": "input-0",
          "rights": 8198
        },
        {
          "file_name": "output",
          "rights": 533572
        }
      ],
      "id": 0,
      "pi_hash": "3fc011587de8a340c0ee36d733c3e52a42babc5fe6b12a074d94204495fd5877",
      "program_file_name": "linear-regression.wasm"
    }
  ],
  "proxy_attestation_server_url": "veracruz-nitro-proxy:3010",
  "proxy_service_cert": "${PROXY_CERTIFICATE}"
  "runtime_manager_hash_nitro": "${HASH}"
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
  "veracruz_server_url": "veracruz_nitro_server:3014"
}
EOF
