# docker image for running I-PoC iotex-s3-app
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

RUN apt-get update -y;apt-get upgrade -y;apt-get install -y openssl netcat curl python3-pip;apt-get -y clean

RUN pip3 install flask jsonschema requests pyopenssl boto3

WORKDIR /root

COPY . /root

CMD /root/iotex-s3-app.sh
