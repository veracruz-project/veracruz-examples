#!/bin/bash
# Start an instance of k3s server using docker image
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz I-PoC
# example repository root directory for copyright and licensing information.

function check_main_ip() {
	if [ ! -z "${HOSTIP}" ]
	then
		return
	fi

	DEFAULT_ROUTE=$(ip route | grep default)

	if [ -z "${DEFAULT_ROUTE}" ]
	then
		echo "no main route, so bailing"
		exit -1
	fi

	DEFAULT_DEV=$(echo "${DEFAULT_ROUTE}" | sed -e "s/^.* dev \([^ ]*\) .*/\1/")

	if [ "${DEFAULT_DEV}" == "${DEFAULT_ROUTE}" ]
	then
		echo "no dev in the route, check route \"${DEFAULT_ROUTE}\""
		exit -1
	fi

	echo ${DEFAULT_DEV}

	HOSTIP=$(ip addr show dev ${DEFAULT_DEV} | grep " inet " | sed -e "s/^ *inet \([0-9.]*\)\/.*/\1/" )

	if [ -z "${HOSTIP}" ]
	then
		echo "Not able to find a suitable hostIP"
		exit -1
	fi

}

# Identification of the server, used for directories names and disambiguate tokens and KUBECONFIG files
SERVERNAME=Veracruz 
# Especific port to be used at the server
HOSTPORT=6443
# IP that the clients will be used to connect (If on the cloud it will probably be the external IP of the server)
HOSTIP=

check_main_ip

# Which version of k3s or k8s to use
DOCKERIMAGE=rancher/k3s:v1.21.4-engine0-k3s1

LOCALSERVERDIR=$(pwd)/${SERVERNAME}

#CONTAINERID=$(docker run -d --rm -p ${HOSTPORT}:${HOSTPORT} -v ${LOCALSERVERDIR}:/var/lib/rancher/k3s ${DOCKERIMAGE} server --tls-san ${HOSTIP} --advertise-address ${HOSTIP} --https-listen-port ${HOSTPORT} --disable-agent --no-deploy servicelb --no-deploy traefik --no-deploy metrics-server --no-flannel --no-deploy coredns)
CONTAINERID=$(docker run -d --rm -p ${HOSTPORT}:${HOSTPORT} -v ${LOCALSERVERDIR}:/var/lib/rancher/k3s ${DOCKERIMAGE} server --tls-san ${HOSTIP} --advertise-address ${HOSTIP} --https-listen-port ${HOSTPORT} --disable-agent --no-deploy traefik)

if [ $? -gt 0 ]
then
        echo "Docker failed......"
        exit -1
fi

echo "Container ID: ${CONTAINERID}"

echo "waiting for the container to generate the secrets"

sleep 30 # have to wait for k3s to generate the file

echo "Copying the secrets, token to token.${SERVERNAME} and kube.confg to kube.${SERVERNAME}.config"
docker cp ${CONTAINERID}:/etc/rancher/k3s/k3s.yaml kube.${SERVERNAME}.config
docker cp ${CONTAINERID}://var/lib/rancher/k3s/server/token token.${SERVERNAME}
sed -i -e "s/\(https:\/\/\)127.0.0.1/\1${HOSTIP}/" kube.${SERVERNAME}.config
