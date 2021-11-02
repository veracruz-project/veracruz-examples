# Makefile for the Veracruz CCFaaS server
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz I-PoC 
# example repository root directory for copyright and licensing information.
#
#
VERSION=0.8

all: image

veracruz-client: ../veracruz-client
	cp ../veracruz-client veracruz-client

veracruz-ccfaas-server.imageOK: Dockerfile ccfaas-server.py ccfaas-server.sh veracruz-client load_program.sh load_data.sh
	docker build -t veracruz-ccfaas-server:$(VERSION) .
	touch veracruz-ccfaas-server.imageOK

image: veracruz-ccfaas-server.imageOK
	docker image tag veracruz-ccfaas-server:$(VERSION) veracruz/veracruz-ccfaas-server:$(VERSION)

veracruz-ccfaas-server.tar.gz: veracruz-ccfaas-server.imageOK
	docker image save -o veracruz-ccfaas-server.tar veracruz-ccfaas-server:$(VERSION)
	gzip veracruz-ccfaas-server.tar

image-save: veracruz-ccfaas-server.tar.gz

image-push: image
	docker push veracruz/veracruz-ccfaas-server:$(VERSION)

