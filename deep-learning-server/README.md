## Centralized Deep Learning Application

This is a Veracruz example that supports training neural networks inside an isolated area on an untrusted device.
(Note: currently only standalone execution is supported. No policy files are provided.)


### Build

1- To build the wasm binary, run:
```
cd veracruz-examples/deep-learning-server
make
```
This will also download `wasi-sdk` for compiling this example. A `dl-server.wasm` binary will be outputted to the example root directory.


### Run Use Case  1 (model training)

To get the data prepared, first run:
```
make mnist_training num-clients=10
```
`num-clients` determines the number of clients who hold different fractions of the complete dataset.

Then, train a LeNet model on MNIST dataset by:
```
cp args_file_classifier.cfg args_file.cfg
wasmtime --dir=./ dl-server.wasm
```

Arguments in the `args_file.cfg` will be automatically loaded. After training, the trained model will be saved into `model` directory.


### Run Use Case 2 (YOLO object detection)

To get the YOLO pre-trained model and labels prepared, first run:
```
make yolo_detection
```

Then you can run the object detection on one image:
```
cp args_file_detect.cfg args_file.cfg
wasmtime --dir=./ dl-server.wasm
```

The prediction image can be found under the example root directory.


### Run Use Case 3 (model aggregation)

To aggregate several existing models as a global one:
```
cp args_file_aggregation.cfg args_file.cfg
wasmtime --dir=./ dl-server.wasm
```

Note: all commands are configured in the `args_file.cfg`. Check different `args_file_XXX.xfg` to see how the arguments are configured, and edit this file to test different datasets, models, and functions.


### TODO
1. Secure aggregation of ONNX model, which then supports Tensorflow, Pytorch, etc. for federated learning
2. (TBD) In federated learning: client-side training integrity check with GPU training