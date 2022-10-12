#!/usr/bin/env python3
#
# Implements the user side of the Iotex application
# Request an instance of the function from CCFaaS
# Request an instance of iotex-s3-app
# Read results from Veracruz instance
# Terminate instance when finished
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSING.markdown` file in the Veracruz I-PoC
# licensing and copyright information.


import requests
import os
import re
import json
import sys
import secrets
from OpenSSL import crypto, SSL
from socket import gethostname
from pprint import pprint
from time import gmtime, mktime
import subprocess

def certToStringVeracruz(certUse):
    """Returns the certificate in certUse (X509 OpenSSL)  in  PEM string format"""
    certStr = re.sub('([^-])\n([^-])|\n$','\g<1>\g<2>',crypto.dump_certificate(crypto.FILETYPE_PEM, certUse).decode('utf-8'))
    return certStr

def keyToStringVeracruz(keyUse):
    """Returns the key in keyUse (Pkey OpenSSL)  in  PEM string format"""
    keyStr = re.sub('([^-])\n([^-])|\n$','\g<1>\g<2>',crypto.dump_privatekey(crypto.FILETYPE_PEM, keyUse).decode('utf-8'))
    return keyStr

if __name__ == "__main__":
    if len(sys.argv) < 5:
       print(sys.argv[0]+": <uniqueID> <URL of CCFaaS> <URL of iotex-S3> <bucket of S3> <File in S3> <decryption key path> <decryption IV path> <S3 authentication>")
       print("      S3 authentication is format <entry>=<value> where entries are: region_name, aws_access_key_id, aws_secret_access_key")
       os._exit(1)

    uniqueID = sys.argv[1]
    ccfaasURL = sys.argv[2]
    iotexS3URL = sys.argv[3]
    s3_auth={ "bucket" : sys.argv[4],
         "filename" : sys.argv[5] }
    outputFile=sys.argv[5]+".output"

    decryption_key_path=sys.argv[6]
    decryption_iv_path=sys.argv[7]

    entries = ["region_name", "aws_access_key_id", "aws_secret_access_key"]

    for i in range(8,len(sys.argv)):
        entry,value = sys.argv[i].split('=',1)
        if not entry in entries:
            print("entry=\""+entry+"\" not reecognied")
            os._exit(1)
        s3_auth[entry] = value

    USER_CERT_FILE = "USERcert.pem"
    USER_KEY_FILE = "USERkey.pem"
    S3_CERT_FILE = "S3cert_"+uniqueID+".pem"
    S3_KEY_FILE = "S3key_"+uniqueID+".pem"
     
    if not (os.path.exists(USER_CERT_FILE) and os.path.exists(USER_KEY_FILE)):
        os.system("openssl ecparam -name prime256v1 -genkey -noout -out "+USER_KEY_FILE)
        os.system("openssl req -x509 -key "+USER_KEY_FILE+" -out "+USER_CERT_FILE+" -config cert.conf")

    usercert = crypto.load_certificate(crypto.FILETYPE_PEM, open(USER_CERT_FILE, 'rb').read(-1))
    userk = crypto.load_privatekey(crypto.FILETYPE_PEM, open(USER_KEY_FILE, 'rb').read(-1))
    print("User certificate loaded from "+USER_CERT_FILE+" and key from "+USER_KEY_FILE)

    os.system("openssl ecparam -name prime256v1 -genkey -noout -out "+S3_KEY_FILE)
    os.system("openssl req -x509 -key "+S3_KEY_FILE+" -out "+S3_CERT_FILE+" -config cert.conf")

    s3cert = crypto.load_certificate(crypto.FILETYPE_PEM, open(S3_CERT_FILE, 'rb').read(-1))
    s3k = crypto.load_privatekey(crypto.FILETYPE_PEM, open(S3_KEY_FILE, 'rb').read(-1))
    print("S3 certificate created")

    iotexAppRequestJson = {
       "function":"vod",
       "instanceid": uniqueID,
       "identities": [
           {
               "certificate": certToStringVeracruz(s3cert),
               "file_rights": [
                   {
                       "file_name": "/s3_app_input/",
                       "rights": 534084
                   }
               ]
           },
           {
               "certificate": certToStringVeracruz(usercert),
               "file_rights": [
                   {
                       "file_name": "/user_input/",
                       "rights": 534084
                   },
                   {
                        "file_name": "/output/",
                        "rights": 24582
                   },
                   {
                        "file_name": "stdout",
                        "rights": 24582
                   },
                   {
                        "file_name": "stderr",
                        "rights": 24582
                   },
                   {
                        "file_name": "/program/",
                        "rights": 536879104
                   }
               ]
           }
       ]
    }

    print("Creating instance URL="+ccfaasURL+"/instance")
    
    try:
        iotexAppResponse = requests.post(ccfaasURL+"/instance",
                                headers = {"Content-Type":"application/json"},
                                data=json.dumps(iotexAppRequestJson, indent = 4))
    except requests.exceptions.HTTPError as http_err:
        print(f'HTTP error occurred: {http_err}')  # Python 3.6
        os._exit(1)
    except Exception as err:
        print(f'Other error occurred: {err}')  # Python 3.6
        os._exit(1)
    
    if iotexAppResponse.status_code != 200:
        print("Http request to CCFaaS returned "+str(iotexAppResponse.status_code)+" with text "+iotexAppResponse.text)  # Python 3.6
        os._exit(1)
    
    try:
        policy = iotexAppResponse.json()
    except JSONDecodeError as err:
        print("Http request to CCFaaS returned "+str(err))  # Python 3.6
        os._exit(1)
    
    print("Response = "+str(iotexAppResponse),flush=True)

    policy_filename = "policy_"+uniqueID

    print("Writing policy to "+policy_filename)

    policy_file = open(policy_filename,"w")
    policy_file.write(policy["policy"])
    policy_file.close()
    
    iotexS3AppRequestJson = {
        "s3" : s3_auth,
        "veracruz" : {
                "filename" : "/s3_app_input/in_enc.h264",
                "policy" : policy["policy"],
                "certificate" : certToStringVeracruz(s3cert),
                "key" : keyToStringVeracruz(s3k),
        }
    }

    print("Creating s3 app URL="+iotexS3URL+"/s3_stream_veracruz")

    error_str = None
    try:
        iotexS3AppResponse = requests.post(iotexS3URL+"/s3_stream_veracruz",
                                headers = {"Content-Type":"application/json"},
                                data=json.dumps(iotexS3AppRequestJson, indent = 4))
    except requests.exceptions.HTTPError as http_err:
        print(f'HTTP error occurred: {http_err}')  # Python 3.6
        error_str = "S3 app failed"
    except Exception as err:
        print(f'Other error occurred: {err}')  # Python 3.6
        error_str = "S3 app failed"
 
    if iotexS3AppResponse.status_code != 200:
        print("Http request to S3 app returned "+str(iotexS3AppResponse.text))  # Python 3.6
        error_str = "S3 app failed"

    if error_str is None:
        execute_string="./execute_program.sh "+policy_filename+" "+USER_CERT_FILE+" "+USER_KEY_FILE+" /output/prediction.0.jpg "+outputFile+" /program/detector.wasm "+decryption_key_path+" "+decryption_iv_path
        print("execute: "+execute_string,flush=True)
        if os.system(execute_string) != 0:
            print("execute retuned eror so cleaning up",flush=True)
    
    print("Deleting instance URL="+ccfaasURL+"/instance/"+uniqueID)
    try:
        iotexAppResponse = requests.delete(ccfaasURL+"/instance/"+uniqueID)
    except requests.exceptions.HTTPError as http_err:
        print(f'HTTP error occurred: {http_err}')  # Python 3.6
        os._exit(1)
    except Exception as err:
        print(f'Other error occurred: {err}')  # Python 3.6
        os._exit(1)
    
    if iotexAppResponse.status_code != 200:
        print("Http request to CCFaaS returned "+str(iotexAppResponse.status_code))  # Python 3.6
        os._exit(1)
    
    os._exit(0)
    
