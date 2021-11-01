from flask import Flask,request
import os
import json
import time
import boto3,botocore
from botocore.config import Config
import jsonschema
import tempfile
import subprocess

app = Flask(__name__)

@app.route("/s3_stream_veracruz", methods=['POST'])
def execute_function():
    error = None
    if request.method != 'POST':
        return "<p>Not supported!</p>"
    if not request.is_json:
        return "<p>input should a json object!</p>"

    requestJson = request.get_json()

    if not type(requestJson) is dict:
        return "<p>input should a correct json object!</p>"

    # Describe what kind of json you expect.
    json_input_schema = {
        "$schema": "https://json-schema.org/draft/2020-12/schema",
        "type": "object",
        "properties": {
            "s3" : {
                    "type": "object",
                    "properties": {
                        "region_name" : {"type": "string"},
                        "bucket" : {"type": "string"},
                        "filename" : {"type": "string"},
                        "aws_access_key_id" : {"type": "string"},
                        "aws_secret_access_key" : {"type": "string"},
                        "aws_session_token" : {"type": "string"}
                    },
                    "required": ["bucket","filename"],
                    "additionalProperties": False
            },
            "veracruz" : {
                    "type": "object",
                    "properties": {
                        "filename" : {"type": "string"},
                        "policy" : {"type": "string"},
                        "certificate" : {"type": "string"},
                        "key" : {"type": "string"}
                    },
                    "required": ["filename", "policy","certificate","key"],
                    "additionalProperties": False
            }
        },
        "additionalProperties": False
    }

    # JSON object to be received
    # { "s3" : {
    #        "bucket" : "<bucket>",
    #        "filename" : "<filename>",
    #        "aws_access_key_id" : "<aws_access_key>",
    #        "aws_secret_access_key" : "<aws_secret_access_key>",
    #        "aws_session_token" : "<aws_session_token>"
    #        },
    #  "veracruz" : {
    #        "filename" : "<filename on the policy>",
    #        "policy" : "<policy>",
    #        "certificate" : "<certificate>",
    #        "key" : "<key>"
    #        }
    # } 
    
    try:
        jsonschema.validate(instance=requestJson, schema=json_input_schema)
    except jsonschema.exceptions.ValidationError as err:
        return "<p>Json object is not correct "+str(err)+"</p>",400

    if ( not ("aws_access_key_id" in requestJson["s3"] and 
              "aws_secret_access_key" in requestJson["s3"] and 
              "aws_session_token" in requestJson["s3"] ) and
	     ("aws_access_key_id" in requestJson["s3"] or 
              "aws_secret_access_key" in requestJson["s3"] or 
              "aws_session_token" in requestJson["s3"] ) ):
        return "<p>aws credentials have to be all provided or none </p>",400

    #my_config = Config(
    #        region_name = 'eu-west-1',
    #    signature_version = 'v4',
    #    retries = {
    #        'max_attempts': 10,
    #        'mode': 'standard'
    #    }
    #)

    tmpdir = tempfile.mkdtemp()
    print("Created temporary directory "+tmpdir)
    fifoFilename = os.path.join(tmpdir, 'video_file')
    try:
        os.mkfifo(fifoFilename)
    except OSError as e:
        print("Failed to create FIFO: %s" % e)
        return "<p>not able to create fifo "+fifoFilename+" to transfer video data!</p>"

    certificateFilename = os.path.join(tmpdir, 'cert.pem')
    certFile = open(certificateFilename, 'w')
    certFile.write(requestJson["veracruz"]["certificate"]);
    certFile.close()
    print("Certificate created at "+certificateFilename)

    policyFilename = os.path.join(tmpdir, 'policy')
    policyFile = open(policyFilename, 'w')
    policyFile.write(requestJson["veracruz"]["policy"]);
    policyFile.close()
    print("Policy created at "+policyFilename)

    keyFilename = os.path.join(tmpdir, 'key.pem')
    keyFile = open(keyFilename, 'w')
    keyFile.write(requestJson["veracruz"]["key"]);
    keyFile.close()
    print("key created at "+keyFilename)

    print("S3 access object creating")
    if "region_name" in requestJson["s3"]: 
        if "aws_access_key_id" in requestJson["s3"]: 
            s3 = boto3.client('s3', region_name=requestJson["s3"]["region_name"],
                aws_access_key_id=requestJson["s3"]["aws_access_key_id"],
                aws_secret_access_key=requestJson["s3"]["aws_secret_access_key"],
                aws_session_token=requestJson["s3"]["aws_session_token"]
            )
        else:
            s3 = boto3.client('s3', region_name=requestJson["s3"]["region_name"])
    else:
        if "aws_access_key_id" in requestJson["s3"]: 
            s3 = boto3.client('s3',
                aws_access_key_id=requestJson["s3"]["aws_access_key_id"],
                aws_secret_access_key=requestJson["s3"]["aws_secret_access_key"],
                aws_session_token=requestJson["s3"]["aws_session_token"]
            )
        else:
            s3 = boto3.client('s3')

    print("S3 access object created")

    print("Starting the cat process")
    #veracruzSub = subprocess.Popen(["bash","-c","cat "+fifoFilename+" > "+requestJson["veracruz"]["filename"])
    veracruzSub = subprocess.Popen(["/bin/bash","-c","openssl rsa -in "+keyFilename+" -out "+keyFilename+".RSA.pem;./veracruz-client "+policyFilename+" --data "+requestJson["veracruz"]["filename"]+"="+fifoFilename+" --identity "+certificateFilename+" --key "+keyFilename+".RSA.pem"])

    fifo = open(fifoFilename, 'wb')
    try:
        s3.download_fileobj(requestJson["s3"]["bucket"],requestJson["s3"]["filename"],fifo)
    except botocore.exceptions.ClientError as error:
        print("Error in S3 "+str(error))

    #for i in range(10):
    #    fifo.write("Entry :"+str(i)+"\n")
    fifo.close()

    print("Waiting for the subprocess to end")
    veracruzRet = veracruzSub.wait()

    print("subprocess ended with retcode="+str(veracruzRet))
    os.remove(fifoFilename)
    os.remove(certificateFilename)
    os.remove(keyFilename)
    os.remove(keyFilename+".RSA.pem")
    os.remove(policyFilename)
    os.rmdir(tmpdir)
    return "<p>OK video file terminated</p>"
