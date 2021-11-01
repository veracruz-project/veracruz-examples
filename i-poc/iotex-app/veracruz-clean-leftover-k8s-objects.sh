#!/bin/bash

#export KUBECONFIG=/etc/rancher/k3s/k3s.yaml
#export RANCHER_FILE="/var/lib/rancher/k3s/server/cred/node-passwd"

#while [ ! -e "${KUBECONFIG}" ]
#do
#        sleep 1
#done

echo "Starting removing dnagling objects"

CONFIGMAPS_ON_SYSTEM=$(kubectl get configmaps --no-headers | grep veracruz-nitro-server- | cut -d " " -f 1)
ENDPOINTSLICES_ON_SYSTEM=$(kubectl get endpointslices --no-headers | grep veracruz-nitro-server- | cut -d " " -f 1)

sleep 10

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
                EVENT_OK=$(echo $a | grep "veracruz-nitro-server-" | grep "Terminating")
                if [ -z "${EVENT_OK}" ]
                then
                        echo "NOT Processed: ${a}"
                        continue
                fi
                echo "Processed: ${a}"
		ID=$(echo $a | cut -d " " -f 1)
                if [ -z "${ID}" ]
                then
                        continue
                fi
                echo "Removing configmap and endpointslicesi ${ID}"
		kubectl delete configmap "${ID}"
		kubectl delete endpointslices "${ID}"
        done
done

exit 0
