#!/bin/bash
# Start iotex-s2-app using flask
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz I-PoC
# example repository root directory for copyright and licensing information.
#!/bin/bash

export FLASK_APP=iotex-s3-app
flask run --host=0.0.0.0
