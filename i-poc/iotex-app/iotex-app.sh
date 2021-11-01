#!/bin/bash

nohup ./veracruz-clean-leftover-k8s-objects.sh&

export FLASK_APP=iotex-app
flask run --host=0.0.0.0
