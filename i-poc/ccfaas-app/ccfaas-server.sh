#!/bin/bash
# Start a CCFaaS server using flask
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz I-PoC
# example repository root directory for copyright and licensing information.

export PROGRAM_LOAD_CERTIFICATE="$(cat $1)"
export PROGRAM_LOAD_CERTIFICATE_FILE=$1
export PROGRAM_LOAD_KEY_FILE=$2

export FLASK_APP=ccfaas-server
flask run --host=0.0.0.0
