# docker image for running Veracruz CCFaaS 
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT
#
# See the `LICENSE.markdown` file in the Veracruz I-PoC directory for licensing
# and copyright information.
#

FROM ubuntu:22.04

RUN apt-get update -y;apt-get upgrade -y;apt-get install -y netcat curl python3-pip;apt-get -y clean

RUN pip3 install flask jsonschema requests pyopenssl

WORKDIR /root

RUN mkdir functionDB instanceDB

COPY . /root

CMD /root/ccfaas-server.sh /root/config/PROGCert.pem /root/config/PROGKey.pem
