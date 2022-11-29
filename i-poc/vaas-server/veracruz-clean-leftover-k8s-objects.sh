#!/bin/bash
# Remove k3s/k8s objects when a Veracruz instance (pod) dies or terminates 
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz I-PoC
# example repository root directory for copyright and licensing information.

echo "Starting removing dnagling objects"

CONFIGMAPS_ON_SYSTEM=$(kubectl get configmaps --no-headers | grep veracruz-nitro-server- | cut -d " " -f 1)
ENDPOINTSLICES_ON_SYSTEM=$(kubectl get endpointslices --no-headers | grep veracruz-nitro-server- | cut -d " " -f 1)

sleep 10

COMPLETED_PODS_ON_SYSTEM=$(kubectl get pods --no-headers | grep veracruz-nitro-server- | grep "Completed" | cut -d " " -f 1)

if [ ! -z "${COMPLETED_PODS_ON_SYSTEM}" ]
then
	kubectl delete pod ${COMPLETED_PODS_ON_SYSTEM}
fi

PODS_ON_SYSTEM=$(kubectl get pods --no-headers | grep veracruz-nitro-server- | cut -d " " -f 1)

echo "CS = ${CONFIGMAPS_ON_SYSTEM}"
echo "EPS = ${ENDPOINTSLICES_ON_SYSTEM}"
echo "PODS = ${PODS_ON_SYSTEM}"

for i in ${CONFIGMAPS_ON_SYSTEM}
do
	OK=$(echo "${PODS_ON_SYSTEM}" | grep "^${i}\$")
	if [ -z "${OK}" ]
	then
		echo "${i} configmap, but corresponring node do not exists anymore, removing"
		kubectl delete configmap "${i}"
	else
		echo "${i} configmap OK"
	fi
done

for i in ${ENDPOINTSLICES_ON_SYSTEM}
do
	OK=$(echo "${PODS_ON_SYSTEM}" | grep "^${i}\$")
	if [ -z "${OK}" ]
	then
		echo "${i} endpointslices, but corresponring node do not exists anymore, removing"
		kubectl delete endpointslices "${i}"
	else
		echo "${i} endpointslices OK"
	fi
done

echo "From now on just listen to events and clean whenever the a pod is removed"

while true
do
	kubectl get pods --no-headers --watch-only |
	while read a
	do
		ID_COMPLETED=""
		EVENT_COMPLETED=$(echo $a | grep "veracruz-nitro-server-" | grep "Completed")
		if [ ! -z "${EVENT_COMPLETED}" ]
		then
			echo "ProcessedI completed: ${EVENT_COMPLETED}"
			ID_COMPLETED=$(echo ${EVENT_COMPLETED} | cut -d " " -f 1)
			kubectl delete pod "${ID_COMPLETED}"
		fi
		ID_TERMINATING=""
		EVENT_TERMINATING=$(echo $a | grep "veracruz-nitro-server-" | grep "Terminating")
		if [ ! -z "${EVENT_TERMINATING}" ]
		then
			echo "Processed terminating: ${EVENT_TERMINATING}"
			ID_TERMINATING=$(echo ${EVENT_TERMINATING} | cut -d " " -f 1)
		fi
		if [ -z "${ID_COMPLETED}" ]
		then
			if [ -z "${ID_TERMINATING}" ]
			then
				continue
			fi
			ID="${ID_TERMINATING}"
		else
			if [ -z "${ID_TERMINATING}" ]
			then
				ID="${ID_COMPLETED}"
			else
				ID="${ID_COMPLETED} ${ID_TERMINATING}"
			fi
		fi
		echo "Removing configmap and endpointslices ${ID}"
		kubectl delete configmap "${ID}"
		kubectl delete endpointslices "${ID}"
	done
done

exit 0
