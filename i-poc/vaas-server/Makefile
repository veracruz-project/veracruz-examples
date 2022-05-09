# Makefile for the Veracruz VaaS server
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
#
VERSION=0.85

all: image

veracruz-vaas-server.imageOK: Dockerfile vaas-server.py vaas-server.sh veracruz-clean-leftover-k8s-objects.sh
	docker build -t veracruz-vaas-server:$(VERSION) .
	touch veracruz-vaas-server.imageOK

.PHONY:
image: veracruz-vaas-server.imageOK
	docker image tag veracruz-vaas-server:$(VERSION) veracruz/veracruz-vaas-server:$(VERSION)

.PHONY:
veracruz-vaas-server.tar.gz: veracruz-vaas-server.imageOK
	docker image save -o veracruz-vaas-server.tar veracruz-vaas-server:$(VERSION)
	gzip veracruz-vaas-server.tar

.PHONY:
image-save: veracruz-vaas-server.tar.gz

.PHONY:
image-push: veracruz-vaas-server.imageOK 
	docker push veracruz/veracruz-vaas-server:$(VERSION)

