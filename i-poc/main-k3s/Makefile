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

YAML_FILES= \
	   smarter-device-manager-configmap-ec2-nitro.yaml \
	   smarter-device-manager-ds-with-configmap-ec2-nitro.yaml \
	   veracruz-ccfaas-app-configmap.yaml \
	   veracruz-ccfaas-app-deploy.yaml \
	   veracruz-ccfaas-app-service.yaml \
	   veracruz-nitro-proxy-configmap.yaml \
	   veracruz-nitro-proxy-deploy.yaml \
	   veracruz-nitro-proxy-service.yaml \
	   veracruz-nitro-server-service.yaml \
	   veracruz-vaas-app-deploy.yaml \
	   veracruz-vaas-app-service.yaml

all: $(YAML_FILES)

config.vars: config.vars.template
	@echo "Please change the config.vars.template and save it as config.vars according"
	@echo "to your particular configuration"
	@exit 1

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
	#openssl ecparam -name prime256v1 -genkey -noout -out PROGKey.pem

PROGCert.pem: PROGKey.pem
	openssl req -new -x509 -sha256 -nodes -days 1825 \
	 -subj "/C=US/ST=Texas/L=Austin/O=Zibble Zabble/emailAddress=zibble@zabble.zibble/CN=zibblezabble" \
	  -key PROGKey.pem \
	  -out PROGCert.pem \
	  -config cert.conf


iotex-s3-app-service.yaml: iotex-s3-app-service.yaml.template config.vars
	./fix-templates-config.vars.sh $<

veracruz-nitro-proxy-configmap.yaml: veracruz-nitro-proxy-configmap.yaml.template CACert.pem CAKey.pem config.vars
	./fix-templates-config.vars.sh $<

veracruz-vaas-app-deploy.yaml: veracruz-vaas-app-deploy.yaml.template CACert.pem config.vars
	./fix-templates-config.vars.sh $<

veracruz-ccfaas-app-configmap.yaml: veracruz-ccfaas-app-configmap.yaml.template PROGCert.pem PROGKey.pem config.vars
	./fix-templates-config.vars.sh $<

veracruz-ccfaas-app-service.yaml: veracruz-ccfaas-app-service.yaml.template config.vars
	./fix-templates-config.vars.sh $<

veracruz-nitro-server-service.yaml: veracruz-nitro-server-service.yaml.template config.vars
	./fix-templates-config.vars.sh $<

veracruz-vaas-app-service.yaml: veracruz-vaas-app-service.yaml.template config.vars
	./fix-templates-config.vars.sh $<

k8s-smarter-device-manager: smarter-device-manager-configmap-ec2-nitro.yaml smarter-device-manager-ds-with-configmap-ec2-nitro.yaml
	kubectl apply -f smarter-device-manager-configmap-ec2-nitro.yaml
	kubectl apply -f smarter-device-manager-ds-with-configmap-ec2-nitro.yaml

k8s-smarter-device-manager-check: ALL
	kubectl get all | grep smarter-device-manager

k8s-attestation-service: veracruz-nitro-proxy-configmap.yaml veracruz-nitro-proxy-service.yaml veracruz-nitro-proxy-deploy.yaml
	kubectl apply -f veracruz-nitro-proxy-configmap.yaml
	kubectl apply -f veracruz-nitro-proxy-service.yaml
	kubectl apply -f veracruz-nitro-proxy-deploy.yaml

k8s-attestation-service-check: ALL
	kubectl get all | grep veracruz-nitro-proxy

k8s-vaas: veracruz-vaas-app-service.yaml veracruz-vaas-app-deploy.yaml veracruz-nitro-server-service.yaml
	kubectl apply -f veracruz-vaas-app-service.yaml
	kubectl apply -f veracruz-vaas-app-deploy.yaml
	kubectl apply -f veracruz-nitro-server-service.yaml

k8s-vaas-check: ALL
	kubectl get all | grep -e vaas -e veracruz-nitro-server

k8s-ccfaas: veracruz-ccfaas-app-configmap.yaml veracruz-ccfaas-app-service.yaml veracruz-ccfaas-app-deploy.yaml
	kubectl apply -f veracruz-ccfaas-app-configmap.yaml
	kubectl apply -f veracruz-ccfaas-app-service.yaml
	kubectl apply -f veracruz-ccfaas-app-deploy.yaml

k8s-ccfaas-check: ALL
	kubectl get all | grep ccfaas

k8s-iotex-s3-app: iotex-s3-app-service.yaml iotex-s3-app-deploy.yaml
	kubectl apply -f iotex-s3-app-service.yaml
	kubectl apply -f iotex-s3-app-deploy.yaml

k8s-all:k8s-smarter-device-manager k8s-attestation-service k8s-vaas k8s-ccfaas k8s-iotex-s3-app
