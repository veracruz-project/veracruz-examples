#!/bin/bash
# Stript that convert templates into final files (convert PEM files to format that Veracruz policy wants)
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz I-PoC
# example repository root directory for copyright and licensing information.

FILE_TO_CREATE=$(basename $1 .template)

RUNTIME_MANAGER_HASH_NITRO_USE=$(cat ../hash)


. config.vars

REPLACE_CACERT_PEM=$(grep REPLACE_CACERT_PEM $1)
REPLACE_CACERT_TEXT='s|^[ ]*REPLACE_CACERT_PEM||'

if [ ! -z "${REPLACE_CACERT_PEM}" ]
then
        SPACES=$(echo "${REPLACE_CACERT_PEM}" | sed -e "s/^\(^[ ]*\).*$/\1/")
        REPLACE_CACERT_TEXT='s|^[ ]*REPLACE_CACERT_PEM|'$(sed -e "s/^/${SPACES}/" -e "s/$/\\\/" CACert.pem)'|'
        REPLACE_CACERT_TEXT="$(echo "${REPLACE_CACERT_TEXT}" | sed -e 's/\\|$/|/')"
fi

REPLACE_CAKEY_PEM=$(grep REPLACE_CAKEY_PEM $1)
REPLACE_CAKEY_TEXT='s|^[ ]*REPLACE_CAKEY_PEM||'

if [ ! -z "${REPLACE_CAKEY_PEM}" ]
then
        SPACES=$(echo "${REPLACE_CAKEY_PEM}" | sed -e "s/^\(^[ ]*\).*$/\1/")
        REPLACE_CAKEY_TEXT='s|^[ ]*REPLACE_CAKEY_PEM|'$(sed -e "s/^/${SPACES}/" -e "s/$/\\\/" CAKey.pem)'|'
        REPLACE_CAKEY_TEXT="$(echo "${REPLACE_CAKEY_TEXT}" | sed -e 's/\\|$/|/')"
fi

REPLACE_PROGCERT_PEM=$(grep REPLACE_PROGCERT_PEM $1)
REPLACE_PROGCERT_TEXT='s|^[ ]*REPLACE_PROGCERT_PEM||'

if [ ! -z "${REPLACE_PROGCERT_PEM}" ]
then
        SPACES=$(echo "${REPLACE_PROGCERT_PEM}" | sed -e "s/^\(^[ ]*\).*$/\1/")
        REPLACE_PROGCERT_TEXT='s|^[ ]*REPLACE_PROGCERT_PEM|'$(sed -e "s/^/${SPACES}/" -e "s/$/\\\/" PROGCert.pem)'|'
        REPLACE_PROGCERT_TEXT="$(echo "${REPLACE_PROGCERT_TEXT}" | sed -e 's/\\|$/|/')"
fi

REPLACE_PROGKEY_PEM=$(grep REPLACE_PROGKEY_PEM $1)
REPLACE_PROGKEY_TEXT='s|^[ ]*REPLACE_PROGKEY_PEM||'

if [ ! -z "${REPLACE_PROGKEY_PEM}" ]
then
        SPACES=$(echo "${REPLACE_PROGKEY_PEM}" | sed -e "s/^\(^[ ]*\).*$/\1/")
        REPLACE_PROGKEY_TEXT='s|^[ ]*REPLACE_PROGKEY_PEM|'$(sed -e "s/^/${SPACES}/" -e "s/$/\\\/" PROGKey.pem)'|'
        REPLACE_PROGKEY_TEXT="$(echo "${REPLACE_PROGKEY_TEXT}" | sed -e 's/\\|$/|/')"
fi


VERACRUZ_PORT_DEFINITION=$(for port in $(seq ${VERACRUZ_MIN_PORT_USE} 1 ${VERACRUZ_MAX_PORT_USE})
do
    echo -n "
    - protocol: TCP
      port: $port
      name: veracruz-$port"
done| sed -e "/^ *$/d")

VERACRUZ_PORT_DEFINITION=$(echo -n "s/^ *VERACRUZ_PORT_DEFINITION.*$/";echo "${VERACRUZ_PORT_DEFINITION}/g" | sed -e "/^$/d" -e "s/\([^g]\)$/\1\\\/g")

sed -e "s/EXTERNAL_IP_USE/${EXTERNAL_IP_USE}/g" \
    -e "s/VERACRUZ_MIN_PORT_USE/${VERACRUZ_MIN_PORT_USE}/g" \
    -e "s/VERACRUZ_MAX_PORT_USE/${VERACRUZ_MAX_PORT_USE}/g" \
    -e "s/VERACRUZ_ENDPOINT_HOSTNAME_USE/${VERACRUZ_ENDPOINT_HOSTNAME_USE}/g" \
    -e "s/RUNTIME_MANAGER_HASH_NITRO_USE/${RUNTIME_MANAGER_HASH_NITRO_USE}/g" \
    -e "${VERACRUZ_PORT_DEFINITION}" \
    -e "${REPLACE_PROGCERT_TEXT}" \
    -e "${REPLACE_PROGKEY_TEXT}" \
    -e "${REPLACE_CACERT_TEXT}" \
    -e "${REPLACE_CAKEY_TEXT}" \
    $1 >${FILE_TO_CREATE}

