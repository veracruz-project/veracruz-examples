#!/bin/bash

export PROGRAM_LOAD_CERTIFICATE="$(cat $1)"
export PROGRAM_LOAD_CERTIFICATE_FILE=$1
export PROGRAM_LOAD_KEY_FILE=$2

export FLASK_APP=ccfaas-server
flask run --host=0.0.0.0
