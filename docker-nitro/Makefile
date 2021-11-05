# Makefile for the docker-nitro
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz docker nitro
# example repository root directory for copyright and licensing information.
#
#

#
#
DOCKER_IMAGE=veracruz/veracruz-nitro:v0.8

ALL: CACert.pem PROGCert.pem USERCert.pem hash-file dual_policy.json

CAKey.pem:
	openssl ecparam -name prime256v1 -genkey -noout -out CAKey.pem

CACert.pem: CAKey.pem
	openssl req -new -x509 -sha256 -nodes -days 1825 \
	 -subj "/C=US/ST=Texas/L=Austin/O=Zibble Zabble/emailAddress=zibble@zabble.zibble/CN=zibblezabble" \
	  -key CAKey.pem \
	  -out CACert.pem \
	  -config ca-cert.conf

PROGKey.pem:
	openssl genrsa -out PROGKey.pem 2048

PROGCert.pem: PROGKey.pem
	openssl req -new -x509 -sha256 -nodes -days 1825 \
	 -subj "/C=US/ST=Texas/L=Austin/O=Zibble Zabble/emailAddress=zibble@zabble.zibble/CN=zibblezabble" \
	  -key PROGKey.pem \
	  -out PROGCert.pem \
	  -config cert.conf

USERKey.pem:
	openssl genrsa -out USERKey.pem 2048

USERCert.pem: USERKey.pem
	openssl req -new -x509 -sha256 -nodes -days 1825 \
	 -subj "/C=US/ST=Texas/L=Austin/O=Zibble Zabble/emailAddress=zibble@zabble.zibble/CN=zibblezabble" \
	  -key USERKey.pem \
	  -out USERCert.pem \
	  -config cert.conf

run-proxy:
	docker run --rm -d \
		-v $(shell pwd)/CAKey.pem:/work/proxy-config-files/CAKey.pem \
		-v $(shell pwd)/CACert.pem:/work/proxy-config-files/CACert.pem \
		--name veracruz-proxy-${USER} \
		--hostname veracruz-proxy-${USER}\
		${DOCKER_IMAGE} \
		/work/proxy-attestation-server/proxy-attestation-server \
			0.0.0.0:3010 \
			--ca-cert /work/proxy-config-files/CACert.pem \
			--ca-key /work/proxy-config-files/CAKey.pem \
			--database-url /work/proxy-attestation-server/proxy-attestation-server.db

run-proxy-check:
	docker logs veracruz-proxy-${USER}

run-proxy-stop:
	-docker stop veracruz-proxy-${USER}

hash:
	CONTAINERID=$(shell docker create ${DOCKER_IMAGE}); \
        docker cp $$CONTAINERID:/work/veracruz-client/hash hash; \
        docker rm $$CONTAINERID

hash-file: hash

dual_policy.json: convert_template.sh CACert.pem PROGCert.pem USERCert.pem
	./convert_template.sh

run-server: dual_policy.json
	docker run --rm  -d \
		-v $(shell pwd)/dual_policy.json:/work/veracruz-server-policy/dual_policy.json \
		-v /run/nitro_enclaves:/run/nitro_enclaves \
		-v /var/log/nitro_enclaves:/var/log/nitro_enclaves \
		-p 3014:3014 \
		--device=/dev/vsock:/dev/vsock \
		--device=/dev/nitro_enclaves:/dev/nitro_enclaves \
		--name veracruz-server-${USER} \
		--hostname veracruz-server-${USER}\
		--add-host="veracruz-proxy-${USER}:$(shell docker inspect --format='{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' veracruz-proxy-${USER})"  \
		${DOCKER_IMAGE} \
		/work/veracruz-server/veracruz-server /work/veracruz-server-policy/dual_policy.json

run-server-debug: dual_policy.json
	docker run -d \
		-v $(shell pwd)/dual_policy.json:/work/veracruz-server-policy/dual_policy.json \
		-v /run/nitro_enclaves:/run/nitro_enclaves \
		-v /var/log/nitro_enclaves:/var/log/nitro_enclaves \
		-p 3014:3014 \
		--device=/dev/vsock:/dev/vsock \
		--device=/dev/nitro_enclaves:/dev/nitro_enclaves \
		--name veracruz-server-${USER} \
		--hostname veracruz-server-${USER}\
		--add-host="veracruz-proxy-${USER}:$(shell docker inspect --format='{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' veracruz-proxy-${USER})"  \
		${DOCKER_IMAGE} \
		/work/veracruz-server/veracruz-server /work/veracruz-server-policy/dual_policy.json

run-server-check:
	docker logs veracruz-server-${USER}

run-server-stop:
	-docker stop veracruz-server-${USER}
	-docker rm veracruz-server-${USER}

run-client: dual_policy.json
	docker run --rm -d \
		-v $(shell pwd)/dual_policy.json:/work/veracruz-server-policy/dual_policy.json \
		-v $(shell pwd)/PROGKey.pem:/work/proxy-config-files/PROGKey.pem \
		-v $(shell pwd)/PROGCert.pem:/work/proxy-config-files/PROGCert.pem \
		-v $(shell pwd)/USERKey.pem:/work/proxy-config-files/USERKey.pem \
		-v $(shell pwd)/USERCert.pem:/work/proxy-config-files/USERCert.pem \
		-v $(shell pwd)/linear-regression.wasm:/work/proxy-config-files/linear-regression.wasm \
		-v $(shell pwd)/linear-regression.dat:/work/proxy-config-files/linear-regression.dat \
		--name veracruz-client-${USER} \
		--hostname veracruz-client-${USER}\
		--add-host="veracruz-server-${USER}:$(shell docker inspect --format='{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' veracruz-server-${USER})"  \
		${DOCKER_IMAGE} \
		sleep inf

run-client-check:
	docker logs veracruz-client-${USER}

run-client-stop:
	-docker stop veracruz-client-${USER}

run-client-exec:
	docker exec -it veracruz-client-${USER} /bin/bash

run-client-exec-load-program:
	docker exec -it veracruz-client-${USER} /work/veracruz-client/veracruz-client /work/veracruz-server-policy/dual_policy.json \
			  --program linear-regression.wasm=/work/proxy-config-files/linear-regression.wasm \
	 		  --identity /work/proxy-config-files/PROGCert.pem \
         		  --key /work/proxy-config-files/PROGKey.pem

run-client-exec-load-dat:
	docker exec -it veracruz-client-${USER} /work/veracruz-client/veracruz-client /work/veracruz-server-policy/dual_policy.json \
			  --data input-0=/work/proxy-config-files/linear-regression.dat \
	 		  --identity /work/proxy-config-files/USERCert.pem \
         		  --key /work/proxy-config-files/USERKey.pem

run-client-exec-read-output:
	docker exec -it veracruz-client-${USER} /work/veracruz-client/veracruz-client /work/veracruz-server-policy/dual_policy.json \
			  --results linear-regression.wasm=/work/proxy-config-files/output.dat \
	 		  --identity /work/proxy-config-files/USERCert.pem \
         		  --key /work/proxy-config-files/USERKey.pem
	docker cp veracruz-client-${USER}:/work/proxy-config-files/output.dat output.dat 

shutdown: run-client-stop run-server-stop run-proxy-stop
