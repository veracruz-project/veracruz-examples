#!/bin/bash

export PUBLIC_HOSTNAME=$(curl http://169.254.169.254/latest/meta-data/public-hostname)
export LOCAL_IP=$(curl http://169.254.169.254/latest/meta-data/local-ipv4)

#helm template \
helm install i-poc-example \
	--set-file config.CACERT=../../main-k3s/CACert.pem \
	--set-file config.CAKEY=../../main-k3s/CAKey.pem \
	--set-file config.PROGCERT=../../main-k3s/PROGCert.pem \
	--set-file config.PROGKEY=../../main-k3s/PROGKey.pem \
	--set config.externalIPUse=${LOCAL_IP} \
	--set-file config.nitroHash=../../hash \
	--set config.veracruzEndpointHostname=${PUBLIC_HOSTNAME} \
	.
