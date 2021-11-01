FROM ubuntu:20.04

RUN apt-get update -y;apt-get upgrade -y;apt-get install -y netcat curl python3-pip;apt-get -y clean

RUN pip3 install flask jsonschema requests pyopenssl

WORKDIR /root

RUN mkdir functionDB instanceDB

COPY . /root

CMD /root/ccfaas-server.sh /root/config/PROGCert.pem /root/config/PROGKey.pem
