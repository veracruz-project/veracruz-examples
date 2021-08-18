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
cp args_files/classifier.cfg args.cfg
wasmtime --dir=./ dl-server.wasm
```

Arguments in the `args.cfg` will be automatically loaded. After training, the trained model will be saved into `model` directory.


### Run Use Case 2 (YOLO object detection)

To get the YOLO pre-trained model and labels prepared, first run:
```
make yolo_detection
```

Then you can run the object detection on one image:
```
cp args_files/detect.cfg args.cfg
wasmtime --dir=./ dl-server.wasm
```

The prediction image can be found under the example root directory.


### Run Use Case 3 (model aggregation)

We support two model formats 1) Darknet 2) ONNX.

**Darknet** is the default format in our example, which all the above training and YOLO detection are based on.
To aggregate several existing Darknet models as a global one:
```
cp args_files/aggregation_darknet.cfg args.cfg
wasmtime --dir=./ dl-server.wasm
```

**ONNX**, i.e., [Open Neural Network Exchange](https://onnx.ai/), is an interoperable model format, which can act as the intermediate among models of Tensorflow, Pytorch, etc.
To aggregate several existing ONNX models as a global one:
```
cp args_files/aggregation_onnx.cfg args.cfg
wasmtime --dir=./ dl-server.wasm
```


Note: all commands are configured in the `args.cfg`. Check different `args_files/_XXX.xfg` to see how the arguments are configured, and edit this file to test different datasets, models, and functions.


### TODO
- Tests such as 1) prediction results of training 2) aggregation results
- Run inside freestanding execution engine
- ONNX model federated learning: clients use Tensorflow, Pytorch, and then export trained model to ONNX for our aggregation
- Run inside real-time Veracruz execution engine (Policy, etc)