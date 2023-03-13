# Makefile for the I-PoC Iotex-s3-app
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

VERSION=0.8.6

all: image

veracruz-client: ../veracruz-client
	cp ../veracruz-client veracruz-client

iotex-s3-veracruz-app.imageOK: Dockerfile iotex-s3-app.py veracruz-client
	docker build --output type=docker -t iotex-s3-veracruz-app:$(VERSION) .
	touch iotex-s3-veracruz-app.imageOK

image: iotex-s3-veracruz-app.imageOK
	docker image tag iotex-s3-veracruz-app:$(VERSION) veracruz/iotex-s3-veracruz-app:$(VERSION)

iotex-s3-veracruz-app.tar.gz: iotex-s3-veracruz-app.imageOK
	docker image save -o iotex-s3-veracruz-app.tar iotex-s3-veracruz-app:$(VERSION)
	gzip iotex-s3-veracruz-app.tar

image-save: iotex-s3-veracruz-app.tar.gz

image-push: image
	docker push veracruz/iotex-s3-veracruz-app:$(VERSION)

