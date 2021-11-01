# Makefile for the I-PoC example
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

IMAGE_SUBDIRS=vaas-server ccfaas-app iotex-s3-app

ALL: veracruz-client k8s-config

veracruz-client:
	CONTAINERID=$(shell docker create alexandref75arm/veracruz-nitro:v0.8); \
	docker cp $$CONTAINERID:/work/veracruz-client/veracruz-client veracruz-client; \
	docker cp $$CONTAINERID:/work/veracruz-client/hash hash; \
	docker rm $$CONTAINERID

images: $(IMAGE_SUBDIRS)
	for f in $^; do $(MAKE) -C $$f;done


k8s-config: main-k3s
	$(MAKE) -C $<

k8s-all: main-k3s
	$(MAKE) -C $< k8s-all
