from flask import Flask,request,abort
from kubernetes import client, config
from kubernetes.client.rest import ApiException
import os
import json
import time
import jsonschema

def get_kubecon():
    try:
        config.load_incluster_config()
    except ApiException as e:
        config.load_kube_config()

    kubecon = client.CoreV1Api()
    return kubecon

app = Flask(__name__)

@app.route("/execute_function", methods=['POST'])
def execute_function():
    print("Received request on post")
    error = None
    if request.method != 'POST':
        print("Received something different than POST")
        return "<p>Not supported!</p>"
    if not request.is_json:
        print("Received something different than json data")
        return "<p>input should a json object!</p>"

    requestJson = request.get_json()

    json_input_schema = {
        "$schema": "https://json-schema.org/draft/2020-12/schema",
        "type": "object",
        "properties": {
            "function" : { "type": "string"},
            "interfaces" : {
                    "type": "object",
                    "patternProperties": {
                       "^.*$" : {"type": "string"}
                    },
                    "additionalProperties": False
            },
        },
        "required": ["function","interfaces"],
        "additionalProperties": False
    }


    # JSON_INPUT_OBJECT='{
    #          "function":"ojectdetection",
    #          "interfaces": {
    #                 "input-0":<CERTIFCATE IN PEM FORMAT>
    #                 "output":<CERTIFCATE IN PEM FORMAT>
    #              }
    #         }'
    # 

    print("Checking if json is correct")
    try:
        jsonschema.validate(instance=requestJson, schema=json_input_schema)
    except jsonschema.exceptions.ValidationError as err:
        print("Received incorrect json data")
        return "<p>Json object is not correct "+str(err)+"</p>"


    print("Json is correct")

    if requestJson['function'] == 'objectdetection':
        print("Processing objectdetection")
        input0Certificate=""
        outputCertificate=""
        for interfaceName in requestJson['interfaces']:
            if interfaceName == 'input-0':
                if input0Certificate != "":
                    return "<p>interface 'input-0'i repeated on the json object!</p>"
                input0Certificate = requestJson['interfaces'][interfaceName]
            elif interfaceName == 'output':
                if outputCertificate != "":
                    return "<p>interface 'output' repeated on the json object!</p>"
                outputCertificate = requestJson['interfaces'][interfaceName]
            else:
                return "<p>interface '"+interfaceName+"' unknown on the json object!</p>"

        identities = [
                {
                  "certificate": "-----BEGIN CERTIFICATE-----\nMIIDwzCCAqugAwIBAgIUPWHS0HfecylNhpsOsGeB6PTH5bYwDQYJKoZIhvcNAQELBQAwgYIxCzAJBgNVBAYTAlVTMQ4wDAYDVQQIDAVUZXhhczEPMA0GA1UEBwwGQXVzdGluMRYwFAYDVQQKDA1aaWJibGUgWmFiYmxlMSMwIQYJKoZIhvcNAQkBFhR6aWJibGVAemFiYmxlLnppYmJsZTEVMBMGA1UEAwwMemliYmxlemFiYmxlMB4XDTIwMDkxNjE5MjkwMFoXDTMwMDkxNDE5MjkwMFowgYIxCzAJBgNVBAYTAlVTMQ4wDAYDVQQIDAVUZXhhczEPMA0GA1UEBwwGQXVzdGluMRYwFAYDVQQKDA1aaWJibGUgWmFiYmxlMSMwIQYJKoZIhvcNAQkBFhR6aWJibGVAemFiYmxlLnppYmJsZTEVMBMGA1UEAwwMemliYmxlemFiYmxlMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtFDp4NHy03SxEgODdTmI5bWBvjCEKUg6rU445r1A7lIMGLtWVa5aSv4WoJ7cMkzFc7iqNZ4uTOgAjsaLyBF0qFyOsiu7fO2xgIhGFUQtKwG9FxfYIoF+h61bW80fb7VBXRE6/cG1+tXfFbiMAYzUW+mLr97xbN5FHySwFTmELbl0wV4vUX9v5P4N2Hmrg35IAKNkUoRuB8tqGK03bRtG7Ui6OK4BP61JujwbbDwhtFsk6v9LduyYvLxL3DUc/xl9Uyh54VyrlIilMuXMfsIzppEhx1QtiQWQuZ26KE/t+iumfpchhR5x3IuzoxuXoc7YxXGD7hhuKyEWMaC2s1gGSQIDAQABoy8wLTArBgNVHREEJDAigg16YWJibGUuemliYmxlghF3d3cuemFiYmxlLnppYmJsZTANBgkqhkiG9w0BAQsFAAOCAQEAIP+1j8E6kfk3w3dDCegczhiGnvRFLRoaRaldGD3Ppj8lOnjKVTfuQGjcWElrNE/Mwf4wrkYKMlksip+bu3xRxkwVX93EQn1aAZPXxSAZYTkGh4Seo5t9y9E0K4frhIcNFoDXITcF6UPck/Ehx8Rx8KXuGy8H4YfjPWjNrQMpr0wbdtrOaIksnDKsOnM2R2sr0v/6sG1Nmg5BiJl2zKRzK+arPTPTQIT4OnSz6mJzzId0tibNXfPkmdK/wWdOGB0saMtE/+M4p4xOtn9d4strrgtK3d5Cjdlv31g9uLyHHNF+SEQk+swE2tAr0EXj1245gTRMxF8R9XwOEkFIMFIl8Q==\n-----END CERTIFICATE-----",
                  "file_rights": [
                    {
                      "file_name": "linear-regression.wasm",
                      "rights": 533572
                    }
                  ],
                  "id": 0
                },
                {
                  "certificate": input0Certificate,
                  "file_rights": [
                    {
                      "file_name": "input-0",
                      "rights": 533572
                    },
                  ],
                  "id": 1
                },
                {
                  "certificate": outputCertificate,
                  "file_rights": [
                    {
                      "file_name": "output",
                      "rights": 8198
                    }
                  ],
                  "id": 2
                }
              ]
    
        programs = [
                {
                  "file_rights": [
                    {
                      "file_name": "input-0",
                      "rights": 8198
                    },
                    {
                      "file_name": "output",
                      "rights": 533572
                    }
                  ],
                  "id": 0,
                  "pi_hash": "3fc011587de8a340c0ee36d733c3e52a42babc5fe6b12a074d94204495fd5877",
                  "program_file_name": "linear-regression.wasm"
                }
              ]
    else:
        print("Function "+requestJson['function']+" not found")
        abort(404, description="function '"+requestJson['function']+"' not implemented!")

    kubeConnection = get_kubecon()

    for veracruzport in range(3014,3024):
        print("Trying port "+str(veracruzport))
       
        try:
            configmap  = kubeConnection.read_namespaced_config_map("default","veracruz-nitro-server-"+str(veracruzport))
        except ApiException as e:
            if e.status != 404:
                print("Exception when calling AdmissionregistrationApi->get_api_group: %s\n" % e)
                return "<p>internat error '"+str(e)+"'!</p>"
            print("Port "+str(veracruzport)+" is busy")
            configMap = None

        if not configMap is None:
            continue

        print("Found this port open "+str(veracruzport))
   
        policy = {
              "ciphersuite": "TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256",
              "debug": False,
              "enclave_cert_expiry": {
                "day": 23,
                "hour": 23,
                "minute": 44,
                "month": 12,
                "year": 2021
              },
              "execution_strategy": "Interpretation",
              "identities": identities,
              "programs": programs,
              "proxy_attestation_server_url": "veravruz-nitro-proxy:3010",
              "proxy_service_cert": "-----BEGIN CERTIFICATE-----\nMIICMzCCAdmgAwIBAgIUD4ChgtYHhO1aP16Oz4cwg+Tjd1owCgYIKoZIzj0EAwIw\nbzELMAkGA1UEBhMCVVMxDjAMBgNVBAgMBVRleGFzMQ8wDQYDVQQHDAZBdXN0aW4x\nETAPBgNVBAoMCFZlcmFjcnV6MQ4wDAYDVQQLDAVQcm94eTEcMBoGA1UEAwwTVmVy\nYWNydXpQcm94eVNlcnZlcjAeFw0yMTA0MjMxNjU4NTdaFw0yMjA0MjMxNjU4NTda\nMG8xCzAJBgNVBAYTAlVTMQ4wDAYDVQQIDAVUZXhhczEPMA0GA1UEBwwGQXVzdGlu\nMREwDwYDVQQKDAhWZXJhY3J1ejEOMAwGA1UECwwFUHJveHkxHDAaBgNVBAMME1Zl\ncmFjcnV6UHJveHlTZXJ2ZXIwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARTflYX\nv3iRLGicm9GbRaQvOXECgH8ZUDdpA9PmHfbCoS2N090X8LbP8GwGmp/gDfkwXXLN\n7mlkjv1X2hAhnUXFo1MwUTAdBgNVHQ4EFgQUdkf63ZhkILBpe7BERAek4GtQ+TUw\nHwYDVR0jBBgwFoAUdkf63ZhkILBpe7BERAek4GtQ+TUwDwYDVR0TAQH/BAUwAwEB\n/zAKBggqhkjOPQQDAgNIADBFAiEAl30tQ+haf0ucyAGMHpt82HitY1ieSg46ebBA\nWN68AYUCIFc+BX+rOoOanh6LNDl84gIuiY6CNsqVDV/Bwm/RL1JH\n-----END CERTIFICATE-----\n",
              "runtime_manager_hash_nitro": "89bad60af6e4bd934dc7705b7187b828631092c151b967c0b1638c5567234acd",
              "runtime_manager_hash_sgx": "",
              "runtime_manager_hash_tz": "",
              "std_streams_table": [
                {
                  "Stdin": {
                    "file_name": "stdin",
                    "rights": 8198
                  }
                },
                {
                  "Stdout": {
                    "file_name": "stdout",
                    "rights": 533572
                  }
                },
                {
                  "Stderr": {
                    "file_name": "stderr",
                    "rights": 533572
                  }
                }
              ],
              "veracruz_server_url": "veracruz-nitro-server:"+str(veracruzport)
            }
        configMap = client.V1ConfigMap(
                         metadata = client.V1ObjectMeta(
                                   name = "veracruz-nitro-server-"+str(veracruzport),
                                   namespace = "default"
                         ),
                         data = {
                             "policy.json":str(json.dumps(policy, indent = 4))
                             }
               )
 
        print("Creating configmap for port "+str(veracruzport))
        try:
            configMapCreate = kubeConnection.create_namespaced_config_map("default",configMap)
        except ApiException as e:
            print("Exception when calling create_namespaced_config_map: %s\n" % e)
            continue
    if veracruzport >= 3024:
        return "<p>no internal resources available!</p>"
 
    print("Configmap created for port "+str(veracruzport))

    print("Creating pod for port "+str(veracruzport))
    veracruzPod = client.V1Pod(
                metadata = client.V1ObjectMeta(
                          name = "veracruz-nitro-server-"+str(veracruzport),
                          namespace = "default",
                          labels = { "veracruz-nitro":"server"}
                ),
                spec = client.V1PodSpec(
                          service_account_name = "default",
                          automount_service_account_token = False,
                          dns_policy = "ClusterFirstWithHostNet",
                          hostname = "veracruz-nitro-server",
                          containers = [
                                  client.V1Container(
                                          name = "veracruz-nitro-server",
                                          image_pull_policy = "IfNotPresent",
                                          image = "alexandref75arm/veracruz-nitro:v0.5",
                                          command = [
                                                "/work/veracruz-server/veracruz-server",
                                                "/work/veracruz-server-policy/policy.json"
                                          ],
                                          ports = [
                                                client.V1ContainerPort(
                                                          container_port = veracruzport,
                                                          protocol = "TCP",
                                                          name = "veracruz-"+str(veracruzport)
                                                )
                                          ],
                                          resources = client.V1ResourceRequirements(
                                                  limits = {
                                                          "smarter-devices/nitro_enclaves": "1",
                                                          "smarter-devices/vsock": "1",
                                                          "hugepages-2Mi": "512Mi",
                                                          "memory": "2Gi",
                                                          "cpu": "250m"
                                                  },
                                                  requests = {
                                                          "smarter-devices/nitro_enclaves": "1",
                                                          "smarter-devices/vsock": "1",
                                                          "hugepages-2Mi": "512Mi",
                                                          "cpu": "10m",
                                                          "memory": "100Mi"
                                                  }
                                          ),
                                          volume_mounts = [
                                                  client.V1VolumeMount(
                                                        mount_path = "/dev/hugepages",
                                                        name = "hugepage",
                                                        read_only = False),
                                                  client.V1VolumeMount(
                                                        mount_path = "/work/veracruz-server-policy",
                                                        name = "config"),
                                                  client.V1VolumeMount(
                                                        mount_path = "/run/nitro_enclaves",
                                                        name = "run-enclaves"),
                                                  client.V1VolumeMount(
                                                        mount_path = "/var/log/nitro_enclaves",
                                                        name = "nitro-enclaves")
                                          ]

                                  )
                          ],
                          volumes = [
                                  client.V1Volume(
                                          name = "hugepage",
                                          empty_dir = client.V1EmptyDirVolumeSource(
                                                  medium = "HugePages")
                                  ),
                                  client.V1Volume(
                                          name = "config",
                                          config_map = client.V1ConfigMapVolumeSource(
                                                  name = "veracruz-nitro-server-3014")
                                  ),
                                  client.V1Volume(
                                          name = "run-enclaves",
                                          host_path = client.V1HostPathVolumeSource(
                                                  path = "/run/nitro_enclaves")
                                  ),
                                  client.V1Volume(
                                          name = "nitro-enclaves",
                                          host_path = client.V1HostPathVolumeSource(
                                                  path = "/var/log/nitro_enclaves")
                                  )
                          ]
                )
      )

    try:
        veracruzPodCreated = kubeConnection.create_namespaced_pod("default",veracruzPod)
    except ApiException as e:
        print("Exception when calling create_namespaced_pod: %s\n" % e)
        configMapCreate = kubeConnection.delete_namespaced_config_map("veracruz-nitro-server-"+str(veracruzport),"default")
        return "<p>Could not create a veracruz instance!</p>"
    print("Pod created for port "+str(veracruzport))

    print("Checking IP for Pod for port "+str(veracruzport))
    veracruzPodIP = None
    for i in range(20):
        try:
            veracruzPodStatus = kubeConnection.read_namespaced_pod_status("veracruz-nitro-server-"+str(veracruzport),"default")
        except ApiException as e:
            print("Exception when calling read_namespaced_pod_status: %s\n" % e)
            veracruzPodStatus = kubeConnection.delete_namespaced_pod("veracruz-nitro-server-"+str(veracruzport),"default")
            configMapCreate = kubeConnection.delete_namespaced_config_map("veracruz-nitro-server-"+str(veracruzport),"default")
            return "<p>Could not get status for veracruz instance!</p>"

        veracruzPodIP = veracruzPodStatus.status.pod_ip
        if not veracruzPodIP is None:
            break

        print("Veracruz pod status not set yet, waiting ")

        time.sleep(1)


    if veracruzPodIP is None:
        print("IP not available after 20s for Pod for port "+str(veracruzport))
        veracruzPodStatus = kubeConnection.delete_namespaced_pod("veracruz-nitro-server-"+str(veracruzport),"default")
        configMapCreate = kubeConnection.delete_namespaced_config_map("veracruz-nitro-server-"+str(veracruzport),"default")
        return "<p>Could not get status for veracruz instance!</p>"

    print("Found IP address for pod "+veracruzPodIP)

    veracruzEndpointSlice = client.V1beta1EndpointSlice(
                metadata = client.V1ObjectMeta(
                          name = "veracruz-nitro-server-"+str(veracruzport),
                          namespace = "default",
                          labels = { "kubernetes.io/service-name":"veracruz-nitro-server"}
                ),
                address_type = "IPv4",
                ports = [
                    client.V1beta1EndpointPort(
                        protocol = "TCP",
                        port = veracruzport, 
                        name = "veracruz-"+str(veracruzport)
                    )
                ],
                endpoints = [
                    client.V1beta1Endpoint(
                        addresses = [
                            veracruzPodIP
                        ],
                        conditions = client.V1beta1EndpointConditions(
                            ready = True
                        )
                    )
                ]
            )

    kubeConnectionBeta = client.DiscoveryV1beta1Api()
    try:
        veracruzEndpointSliceCreated = kubeConnectionBeta.create_namespaced_endpoint_slice("default",veracruzEndpointSlice)
    except ApiException as e:
        print("Exception when calling create_namespaced_endpoint_slice: %s\n" % e)
        veracruzPodStatus = kubeConnection.delete_namespaced_pod("veracruz-nitro-server-"+str(veracruzport),"default")
        configMapCreate = kubeConnection.delete_namespaced_config_map("veracruz-nitro-server-"+str(veracruzport),"default")
        return "<p>Could not create the networking configuration for veracruz instance!</p>"

    policy_file = open("policy-veracruz-nitro-server-"+str(veracruzport),"w")
    policy_file.write(str(json.dumps(policy, indent = 4)))
    policy_file.close()

    ret = os.system("./load-program.sh +policy-veracruz-nitro-server-"+str(veracruzport))
    if ret != 0:
        print("ENot able to load the program so stop everything")
        veracruzEndpointSlice = kubeConnectionBeta.delete_namespaced_endpoint_slice("veracruz-nitro-server-"+str(veracruzport),"default")
        veracruzPodStatus = kubeConnection.delete_namespaced_pod("veracruz-nitro-server-"+str(veracruzport),"default")
        configMapCreate = kubeConnection.delete_namespaced_config_map("veracruz-nitro-server-"+str(veracruzport),"default")
        return "<p>Could not load the program into veracruz!</p>"

    return policy

