# Makefile for the I-PoC k3s/k8s related files (YAML, etc)
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

all: linear_regression_policy.json

linear_regression_policy.json: data_client_cert.pem program_client_cert.pem result_client_cert.pem
	./create_policy_template.sh

data_client_cert.pem: data_client_key.pem
	openssl req -new -x509 -sha256 -nodes -days 1825 \
	 -subj "/C=US/ST=Texas/L=Austin/O=Zibble Zabble/emailAddress=zibble@zabble.zibble/CN=zibblezabble" \
	  -key $< \
	  -out $@ \
	  -config cert.conf

data_client_key.pem:
	openssl genrsa -out $@ 2048
	
program_client_cert.pem: program_client_key.pem
	openssl req -new -x509 -sha256 -nodes -days 1825 \
	 -subj "/C=US/ST=Texas/L=Austin/O=Zibble Zabble/emailAddress=zibble@zabble.zibble/CN=zibblezabble" \
	  -key $< \
	  -out $@ \
	  -config cert.conf

program_client_key.pem:
	openssl genrsa -out $@ 2048

result_client_cert.pem: result_client_key.pem
	openssl req -new -x509 -sha256 -nodes -days 1825 \
	 -subj "/C=US/ST=Texas/L=Austin/O=Zibble Zabble/emailAddress=zibble@zabble.zibble/CN=zibblezabble" \
	  -key $< \
	  -out $@ \
	  -config cert.conf

result_client_key.pem:
	openssl genrsa -out $@ 2048


