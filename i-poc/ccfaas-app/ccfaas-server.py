#!/usr/bin/env python3
#
# Implements a CCFaaS server
# Objects are functions,instances,programs and data
# REST API (CRD) is provided for each object
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSING.markdown` file in the Veracruz I-PoC
# licensing and copyright information.


from flask import Flask,request,abort
import os
import json
import time
import jsonschema
import shutil
import requests
import re
import json
import hashlib
import base64
import operator
from OpenSSL import crypto, SSL
from socket import gethostname
from pprint import pprint
from time import gmtime, mktime

def get_function_db(name):
    """ return a json object with function registered or None if it does not exist """
    function_name_base64 = base64.b64encode(name.encode()).decode()
    try:
        function_file = open("functionDB/"+function_name_base64,"r")
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        return None

    try:
        jsondata = json.loads(function_file.read())
    except JSONDecodeError as err:
        print("Decode error or JSON on file: {0}".format(err),flush=True)
        function_file.close()
        return None
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        function_file.close()
        return None

    function_file.close()
    return jsondata

def get_functions_db():
    """ return a json object with a list of functions registered """
    # return a list of functions registed
    functions = []
    for rootdir,dirs,filenames in os.walk("functionDB/"):
        for filename in filenames:
            if filename[-9:] != "_programs":
               functions.append(base64.b64decode(filename.encode()).decode())
        break

    return functions

def create_function_db(name,jsondata):
    """ Register a function (creates the required DB objects) """
    function_name_base64 = base64.b64encode(name.encode()).decode()
    try:
        function_file = open("functionDB/"+function_name_base64,"x")
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        return False

    try:
        function_file.write(json.dumps(jsondata))
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        function_file.close()
        return False
    function_file.close()

    try:
        os.mkdir("functionDB/"+function_name_base64+"_programs")
        os.mkdir("functionDB/"+function_name_base64+"_data")
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        return False
    return True 

def remove_function_db(name):
    """ Remove the named function from the database """
    function_name_base64 = base64.b64encode(name.encode()).decode()
    try:
        shutil.rmtree("functionDB/"+function_name_base64+"_programs")
        shutil.rmtree("functionDB/"+function_name_base64+"_data")
        os.remove("functionDB/"+function_name_base64)
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        return False

    return True

def get_instance_db(name):
    """ return a json object with instance registered or None if it does not exist """
    instance_name_base64 = base64.b64encode(name.encode()).decode()
    try:
        instance_file = open("instanceDB/"+instance_name_base64,"r")
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        return None

    try:
        jsondata = json.loads(instance_file.read())
    except JSONDecodeError as err:
        print("Decode error or JSON on file: {0}".format(err),flush=True)
        instance_file.close()
        return None
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        instance_file.close()
        return None

    instance_file.close()
    return jsondata

def get_instances_db():
    """ return a json object with a list of instances registered """
    instances = []
    for rootdir,dirs,filenames in os.walk("instanceDB/"):
        for filename in filenames:
            if filename[-9:] != "_metadata":
               instances.append(base64.b64decode(filename.encode()).decode())
        break

    return instances

def create_instance_db(name,jsondata):
    """ Register an instance (creates the required DB objects) """
    instance_name_base64 = base64.b64encode(name.encode()).decode()
    try:
        instance_file = open("instanceDB/"+instance_name_base64,"x")
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        return False

    try:
        instance_file.write(json.dumps(jsondata))
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        instance_file.close()
        return False
    instance_file.close()

    try:
        os.mkdir("instanceDB/"+instance_name_base64+"_metadata")
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        return False
    return True 

def remove_instance_db(name):
    """ Remove the named instance from the database """
    instance_name_base64 = base64.b64encode(name.encode()).decode()
    try:
        shutil.rmtree("instanceDB/"+instance_name_base64+"_metadata")
        os.remove("instanceDB/"+instance_name_base64)
    except OSError as err:
        print("OS error: {0}".format(err),flush=True)
        return False

    return True

def get_programs_db(name):
    """ Remove a json object with a list of programs registered for this function """
    function_name_base64 = base64.b64encode(name.encode()).decode()
    programs = []
    for rootdir,dirs,filenames in os.walk("functionDB/"+function_name_base64+"_programs"):
        for filename in filenames:
            programs.append(filename)
        break
    return programs

def get_data_db(name):
    """ Remove a json object with a list of data files registered for this function """
    function_name_base64 = base64.b64encode(name.encode()).decode()
    data_files = []
    for rootdir,dirs,filenames in os.walk("functionDB/"+function_name_base64+"_data"):
        for filename in filenames:
            data_files.append(filename)
        break
    return data_files

def certStrToStringVeracruz(certUse):
    """ Convert a certificate PEM formate from in a string with newlines to a way that Veracruz accepts """
    certStr = re.sub('([^-])\n([^-])|\n$','\g<1>\g<2>',certUse)
    return certStr

def delete_instance(name,instanceID,hashId):
    """ Delete an instance from the database and from VaaS, terminating it """
    remove_instance_db(name)
    # get pod name = first portion of hostname + port#
    hostURL=os.environ['VAAS_ACCESS_URL']+"/veracruz/"+instanceID+"?instance_id=CCFaaS&instance_hash="+hashId
    try:
        vaasAppResponse = requests.delete(hostURL),
    except requests.exceptions.HTTPError as http_err:
        print(f'HTTP error occurred: {http_err}',flush=True)  # Python 3.6
        return "<p>Error accessing the vaas_server "+str(http_err)+"</p>",500
    except Exception as err:
        print(f'Other error occurred: {err}',flush=True)  # Python 3.6
        return "<p>Error accessing the vaas_server "+str(err)+"</p>",500
    return None

app = Flask(__name__)

@app.route('/function', methods=['GET'])
def get_functions_REST():
    print("Received functions get",flush=True)
    error = None
    if request.method != 'GET':
        print("Received something different than GET",flush=True)
        return "<p>Not supported!</p>",400

    functions = get_functions_db()
        
    return {"functions" : functions}

@app.route('/function/<name>', methods=['GET'])
def get_function_REST(name):
    print("Received function get for name="+name,flush=True)
    error = None
    if request.method != 'GET':
        print("Received something different than GET",flush=True)
        return "<p>Not supported!</p>",400

    # get pod name = first portion of hostname + port#
    jsonData = get_function_db(name)
    if jsonData is None:
        return "<p>function "+name+" not found</p>",404
        
    return jsonData

@app.route('/function/<name>', methods=['DELETE'])
def delete_function_REST(name):
    print("Received function delete for name"+name,flush=True)
    error = None
    if request.method != 'DELETE':
        print("Received something different than DELETE",flush=True)
        return "<p>Not supported!</p>",400

    # get pod name = first portion of hostname + port#
    if not remove_function_db(name):
        return "<p>Veracruz instance '"+name+"' does not existe!</p>",404

    return "<p>Function "+name+" deleted</p>"

@app.route('/function/<name>/program', methods=['GET'])
def get_function_programs_REST(name):
    # return a list of programs registed)
    return {"programs":get_programs_db(name)}

@app.route('/function/<name>/program/<progname>', methods=['POST'])
def post_function_program_REST(name,progname):
    function_name_base64 = base64.b64encode(name.encode()).decode()
    print("Received program "+progname+" for function name="+name,flush=True)
    error = None
    if request.method != 'POST':
        print("Received something different than POST",flush=True)
        return "<p>Not supported!</p>",400

    # get pod name = first portion of hostname + port#
    jsonData = get_function_db(name)
    if jsonData is None:
        return "<p>function "+name+" not found</p>",404

    # check if a program with name progname exist on the function description

    print("Checking if program "+progname+" is on function name="+name,flush=True)
    for program in jsonData["programs"]:
        if program["program_file_name"] == progname: 
            print("Found "+program["program_file_name"]+" program",flush=True)
            try:
                program_file = open("functionDB/"+function_name_base64+"_programs/"+progname,"xb")
            except OSError as err:
                print("OS error: {0}".format(err),flush=True)
                return "<p>programn "+progname+" for function "+name+" already exists</p>",400

            if request.content_length >= 10000000:
                return "<p>programn "+progname+" for function "+name+" hash "+m.hexdigest()+" is too large ("+str(request.content_length)+"i) > 10M</p>",400

            bufferData = request.get_data()

            m = hashlib.sha256()
            m.update(bufferData)
            if m.hexdigest() != program["pi_hash"]:
                return "<p>programn "+progname+" for function "+name+" hash "+m.hexdigest()+" does not match function description "+program["pi_hash"]+" already exists</p>",400

            try:
                program_file.write(bufferData)
            except OSError as err:
                print("OS error: {0}".format(err),flush=True)
                program_file.close()
                return False
            program_file.close()

            return "<p>function loaded!</p>"

    return "<p>programn "+progname+" does not exist on function "+name+"</p>",404

@app.route('/function/<name>/program/<progname>', methods=['GET'])
def get_function_program_REST(name,progname):
    function_name_base64 = base64.b64encode(name.encode()).decode()
    print("Get program "+progname+" for function name="+name,flush=True)
    error = None
    if request.method != 'GET':
        print("Received something different than GET",flush=True)
        return "<p>Not supported!</p>",400

    if not os.path.exists("functionDB/"+function_name_base64+"_programs/"+progname):
        return "<p>programn "+progname+" for function "+name+" is not loaded</p>",404

    return "<p>programn "+progname+" is loaded in function "+name+"</p>"

@app.route('/function/<name>/program/<progname>', methods=['DELETE'])
def delete_function_program_REST(name,progname):
    function_name_base64 = base64.b64encode(name.encode()).decode()
    print("Delete program "+progname+" for function name="+name,flush=True)
    error = None
    if request.method != 'DELETE':
        print("Received something different than DELETE",flush=True)
        return "<p>Not supported!</p>",400

    if not os.path.exists("functionDB/"+function_name_base64+"_programs/"+progname):
        return "<p>programn "+progname+" for function "+name+" is not loaded</p>",404

    os.remove("functionDB/"+name+"_programs/"+progname)

    return "<p>programn "+progname+" is deleted from function "+name+"</p>"

@app.route("/function", methods=['POST'])
def post_function_REST(): # create
    print("Received function create",flush=True)
    error = None
    if request.method != 'POST':
        print("Received something different than POST",flush=True)
        return "<p>Not supported!</p>",400
    if not request.is_json:
        print("Received something different than json data",flush=True)
        return "<p>input should a json object!</p>",400

    requestJson = request.get_json()

    json_file_rights_schema = {
        "type": "array",
        "items" : {
            "type": "object",
            "properties": {
                 "file_name": { "type": "string"},
                 "rights": { "type": "integer"},
            },
            "additionalProperties": False
        },
        "additionalProperties": False
    }

    json_program_schema = {
        "type": "array",
        "items" : {
             "type": "object",
             "properties": {
                 "file_rights": json_file_rights_schema,
                 "id": { "type":"integer" },
                 "pi_hash":  { "type":"string" },
                 "program_file_name":  { "type":"string" },
             },
             "required": ["file_rights","id","pi_hash","program_file_name"],
             "additionalProperties": False
        },
        "additionalProperties": False
    }

    json_data_file_schema = {
        "type": "array",
        "items" : {
             "type": "object",
             "properties": {
                 "data_file":  { "type":"string" },
                 "pi_hash":  { "type":"string" },
                 "priority": { "type":"integer" }
             },
             "required": ["pi_hash","data_file"],
             "additionalProperties": False
        },
        "additionalProperties": False
    }

    json_policy_input_schema = {
        "$schema": "https://json-schema.org/draft/2020-12/schema",
        "type": "object",
        "properties": {
            "function": { "type": "string"},
            "execution_strategy": { "type": "string"},
            "max_memory_mib": { "type": "integer"},
            "programs":  json_program_schema,
            "data_files": json_data_file_schema
        },
        "required": ["function","execution_strategy","max_memory_mib", "programs"],
        "additionalProperties": False
    }

    print("Checking if json is correct",flush=True)
    try:
        jsonschema.validate(instance=requestJson, schema=json_policy_input_schema)
    except jsonschema.exceptions.ValidationError as err:
        print("Received incorrect json data"+str(err),flush=True)
        return "<p>Json object is not correct "+str(err)+"</p>",400

    #TODO: Insert checks to verify if semmanticall the json is correct (check if the certificates/keys are valid for exmple)

    print("Json is correct",flush=True)

    print("Processing create function",flush=True)

    if not create_function_db(requestJson["function"],requestJson):
        print("Function already exists "+requestJson["function"],flush=True)
        return "<p>Could not create the function "+requestJson["function"]+"</p>",400

    return requestJson

@app.route('/instance', methods=['GET'])
def get_instances_REST():
    print("Received instances get",flush=True)
    error = None
    if request.method != 'GET':
        print("Received something different than GET",flush=True)
        return "<p>Not supported!</p>",400

    instances = get_instances_db()
        
    return {"instances" : instances}

@app.route("/instance", methods=['POST'])
def post_instance_REST(): # create
    print("Received instance create",flush=True)
    error = None
    if request.method != 'POST':
        print("Received something different than POST",flush=True)
        return "<p>Not supported!</p>",400
    if not request.is_json:
        print("Received something different than json data",flush=True)
        return "<p>input should a json object!</p>",400

    requestJson = request.get_json()

    json_file_rights_schema = {
        "type": "array",
        "items" : {
            "type": "object",
            "properties": {
                 "file_name": { "type": "string"},
                 "rights": { "type": "integer"},
            },
            "additionalProperties": False
        },
        "additionalProperties": False
    }

    json_identity_schema = {
        "type": "array",
        "items" : {
             "type": "object",
             "properties": {
                 "file_rights": json_file_rights_schema,
                 "certificate":  { "type":"string" },
             },
             "required": ["file_rights","certificate"],
             "additionalProperties": False
        },
        "additionalProperties": False
    }

    json_policy_input_schema = {
        "$schema": "https://json-schema.org/draft/2020-12/schema",
        "type": "object",
        "properties": {
            "instanceid": { "type": "string"},
            "function": { "type": "string"},
            "identities":  json_identity_schema,
        },
        "required": ["function","instanceid","identities"],
        "additionalProperties": False
    }

    print("Checking if json is correct",flush=True)
    try:
        jsonschema.validate(instance=requestJson, schema=json_policy_input_schema)
    except jsonschema.exceptions.ValidationError as err:
        print("Received incorrect json data"+str(err),flush=True)
        return "<p>Json object is not correct "+str(err)+"</p>",400

    print("Check semantically if json is correct",flush=True)

    print("Json is correct",flush=True)

    print("Processing create function",flush=True)

    # get existing function registed
    jsonFunction = get_function_db(requestJson["function"])
    if jsonFunction is None:
        return "<p>function "+requestJson["function"]+" not found</p>",404

    # check if a program with name progname exists on the function description

    if not create_instance_db(requestJson["instanceid"],requestJson):
        print("instance already exists "+requestJson["instanceid"],flush=True)
        return "<p>Could not create instance already exists "+requestJson["instanceid"]+"</p>",400

    # Build the policy to be sent to VaaS to get a running instance


    # for every local program in the function DB insert our keys
    programs = get_programs_db(requestJson["function"])
    program_file_rights = []
    for program in programs:
        program_file_rights.append({
                     "file_name": program,
                     "rights": 533572
                }
            )

    data_files = get_data_db(requestJson["function"])
    for data_file in data_files:
        program_file_rights.append({
                     "file_name": data_file,
                     "rights": 533572
                }
            )

    program_identities = requestJson["identities"]
    if len(program_file_rights) > 0:
        program_identities.append({
                     "certificate": certStrToStringVeracruz(os.environ['PROGRAM_LOAD_CERTIFICATE']),
                     "file_rights": [*program_file_rights]
                }
            )

    instance_policy = {
        "ciphersuite": "TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256",
        "debug": False,
        "enable_clock": True,
        "execution_strategy": jsonFunction["execution_strategy"],
        "max_memory_mib": jsonFunction["max_memory_mib"],
        "programs": jsonFunction["programs"],
        "identities": [*program_identities],
	"instance_id":"CCFaaS"
    }
    # Fix identities ID

    for identity_id in range(len(instance_policy["identities"])):
        instance_policy["identities"][identity_id]["id"] = identity_id

    hostURL=os.environ['VAAS_ACCESS_URL']+"/veracruz"
    
    print("Sending POST to vaas URL="+hostURL,flush=True)
    
    try:
        vaasAppResponse = requests.post(hostURL,
               headers = {"Content-Type":"application/json"},          
               data=json.dumps(instance_policy, indent = 4))
    except requests.exceptions.HTTPError as http_err:
        print(f'HTTP error occurred: {http_err}',flush=True)
        remove_instance_db(requestJson["instanceid"])
        return "<p>Error accessing the vaas_server "+str(http_err)+"</p>",500
    except Exception as err:
        print(f'Other error occurred: {err}',flush=True)
        remove_instance_db(requestJson["instanceid"])
        return "<p>Error accessing the vaas_server "+str(err)+"</p>",500
    
    if vaasAppResponse.status_code != 200:
        print("instance_policy="+json.dumps(instance_policy, indent = 4),flush=True)
        print("Http request to VAAS returned "+str(vaasAppResponse.status_code))  # Python 3.6
        remove_instance_db(requestJson["instanceid"])
        return "<p>Error accessing the vaas_server response code was "+str(vaasAppResponse.status_code)+"</p>",500
    
    try:
        policy = vaasAppResponse.json()
    except JSONDecodeError as err:
        print("Http request to VAAS returned "+str(err),flush=True)
        remove_instance_db(requestJson["instanceid"])
        return "<p>Error accessing the vaas_server, json response was "+str(err)+"</p>",500
    
    print("Response = "+str(vaasAppResponse),flush=True)

    instance_name_base64 = base64.b64encode(requestJson["instanceid"].encode()).decode()
    full_policy_file = open("instanceDB/"+instance_name_base64+"_metadata/full_policy","w")
    full_policy_file.write(json.dumps(policy))
    full_policy_file.close()

    policy_file = open("instanceDB/"+instance_name_base64+"_metadata/policy","w")
    policy_file.write(policy["policy"])
    policy_file.close()

    print("Programs to load "+str(programs),flush=True)
    if len(programs) > 0:
        function_name_base64 = base64.b64encode(requestJson["function"].encode()).decode()
        execute_string="/root/load_program.sh "+"instanceDB/"+instance_name_base64+"_metadata/policy \""+os.environ['PROGRAM_LOAD_CERTIFICATE_FILE']+"\" \""+os.environ['PROGRAM_LOAD_KEY_FILE']+"\" \"functionDB/"+function_name_base64+"_programs\""
        for program in programs:
      	    execute_string=execute_string+" \""+program+"\""

        print("execute: "+execute_string,flush=True)
        if os.system(execute_string) != 0:
            print("execute retuned eror so cleaning up",flush=True)
            result = delete_instance(requestJson["instanceid"],policyJson["veracruz_server_url"])
            if not result is None:
                return result
            return "<p>Loading programs failed so instance removed<\p>",500
    
    print("Data files to load "+str(data_files),flush=True)
    if len(data_files) > 0:
        data_files_load = sorted(jsonFunction["data_files"],key=operator.itemgetter('priority')) 
        function_name_base64 = base64.b64encode(requestJson["function"].encode()).decode()
        execute_string="/root/load_data.sh "+"instanceDB/"+instance_name_base64+"_metadata/policy \""+os.environ['PROGRAM_LOAD_CERTIFICATE_FILE']+"\" \""+os.environ['PROGRAM_LOAD_KEY_FILE']+"\" \"functionDB/"+function_name_base64+"_data\""
        for data_files_entry in data_files_load:
            if data_files_entry["data_file"] in data_files:
                execute_string=execute_string+" \""+data_files_entry["data_file"]+"\""

        print("execute: "+execute_string,flush=True)
        if os.system(execute_string) != 0:
            print("execute retuned eror so cleaning up",flush=True)
            result = delete_instance(requestJson["instanceid"],policyJson["veracruz_server_url"])
            if not result is None:
                return result
            return "<p>Loading programs failed so instance removed<\p>",500

    return {"instance":requestJson["instanceid"],"policy":policy["policy"]}

@app.route('/instance/<name>', methods=['GET'])
def get_instance_REST(name):
    print("Received instance get for name "+name,flush=True)
    error = None
    if request.method != 'GET':
        print("Received something different than GET",flush=True)
        return "<p>Not supported!</p>",400

    try:
        instance_name_base64 = base64.b64encode(name.encode()).decode()
        full_policy_file = open("instanceDB/"+instance_name_base64+"_metadata/full_policy","r")
        full_policy = json.loads(full_policy_file.read())
        full_policy_file.close()
    except Exception as err:
        print(f'Other error occurred: {err}',flush=True)  # Python 3.6
        remove_instance_db(name)
        return "<p>Veracruz instance '"+name+"' does not exist!</p>",404

    return {"instance":name,"policy":full_policy["policy"]}

@app.route('/instance/<name>', methods=['DELETE'])
def delete_instance_REST(name):
    print("Received instance delete for name "+name,flush=True)
    error = None
    if request.method != 'DELETE':
        print("Received something different than DELETE",flush=True)
        return "<p>Not supported!</p>",400

    try:
        instance_name_base64 = base64.b64encode(name.encode()).decode()
        full_policy_file = open("instanceDB/"+instance_name_base64+"_metadata/full_policy","r")
        full_policyJson =json.loads(full_policy_file.read())
        full_policy_file.close()
    except Exception as err:
        print(f'Other error occurred: {err}',flush=True)  # Python 3.6
        remove_instance_db(name)
        return "<p>Veracruz instance '"+name+"' removed!</p>",404

    policy=json.loads(full_policyJson["policy"])

    print("Trying to remove instance from VaaS "+policy["veracruz_server_url"],flush=True)

    result = delete_instance(name,policy["veracruz_server_url"],full_policyJson["instance_hash"])
    if not result is None:
        return result

    return "<p>instance "+name+" deleted</p>"

@app.route('/function/<name>/data_file', methods=['GET'])
def get_functiondata_REST(name):
    # return a list of data_files registed)
    return {"data_files":get_data_db(name)}

@app.route('/function/<name>/data_file/<data_file>', methods=['POST'])
def post_function_data_file_REST(name,data_file):
    function_name_base64 = base64.b64encode(name.encode()).decode()
    print("Received data_file "+data_file+" for function name="+name,flush=True)
    error = None
    if request.method != 'POST':
        print("Received something different than POST",flush=True)
        return "<p>Not supported!</p>",400

    # get pod name = first portion of hostname + port#
    jsonData = get_function_db(name)
    if jsonData is None:
        return "<p>function "+name+" not found</p>",404

    # check if a data_file with name data_file exists on the function description

    print("Checking if data_file "+data_file+" is on function name="+name,flush=True)
    for data_file in jsonData["data_files"]:
        if data_file["data_file"] == data_file: 
            print("Found "+data_file["data_file"]+" data_file",flush=True)
            # TODO: Verify if the hash match
            try:
                data_file_file = open("functionDB/"+function_name_base64+"data/"+data_file,"xb")
            except OSError as err:
                print("OS error: {0}".format(err),flush=True)
                return "<p>data_file "+data_file+" for function "+name+" already exists</p>",400

            if request.content_length >= 300000000:
                return "<p>data_file "+data_file+" for function "+name+" hash "+m.hexdigest()+" is too large ("+str(request.content_length)+"i) > 300M</p>",400

            bufferData = request.get_data()

            m = hashlib.sha256()
            m.update(bufferData)
            if m.hexdigest() != data_file["pi_hash"]:
                return "<p>data_file "+data_file+" for function "+name+" hash "+m.hexdigest()+" does not match function description "+data_file["pi_hash"]+" already exists</p>",400

            try:
                data_file_file.write(bufferData)
            except OSError as err:
                print("OS error: {0}".format(err),flush=True)
                data_file_file.close()
                return False
            data_file_file.close()

            return "<p>data_file "+data_file+" loaded!</p>"

    return "<p>data_file "+data_file+" does not exist on function "+name+"</p>",404

@app.route('/function/<name>/data_file/<data_file>', methods=['GET'])
def get_function_data_file_REST(name,data_file):
    function_name_base64 = base64.b64encode(name.encode()).decode()
    print("Get data_file "+data_file+" for function name="+name,flush=True)
    error = None
    if request.method != 'GET':
        print("Received something different than GET",flush=True)
        return "<p>Not supported!</p>",400

    if not os.path.exists("functionDB/"+function_name_base64+"data/"+data_file):
        return "<p>data_file "+data_file+" for function "+name+" is not loaded</p>",404

    return "<p>data_file "+data_file+" is loaded in function "+name+"</p>"

@app.route('/function/<name>/data_file/<data_file>', methods=['DELETE'])
def delete_function_data_file_REST(name,data_file):
    function_name_base64 = base64.b64encode(name.encode()).decode()
    print("Delete data_file "+data_file+" for function name="+name,flush=True)
    error = None
    if request.method != 'DELETE':
        print("Received something different than DELETE",flush=True)
        return "<p>Not supported!</p>",400

    if not os.path.exists("functionDB/"+function_name_base64+"_data/"+data_file):
        return "<p>data_file "+data_file+" for function "+name+" is not loaded</p>",404

    os.remove("functionDB/"+name+"_data/"+data_file)

    return "<p>data_file "+data_file+" is deleted from function "+name+"</p>"

