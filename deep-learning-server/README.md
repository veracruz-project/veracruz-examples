## Deep Learning Server Application

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

# or use freestanding execution engine
RUST_LOG=info [FREESTANDING EE EXECUTABLE] --program dl-server.wasm --input-source args.cfg cfg data --output-source model -d -e
```

Arguments in the `args.cfg` will be automatically loaded. After training, the trained model will be saved into `model` directory.

After training (by default it is `2000` batches, see `mnist_lenet.cfg` file), you can then conduct prediction or valid on this built model.

To test on one image, using the commands below. You will see the prediction of this image.
```
cp args_files/prediction_one.cfg args.cfg
wasmtime --dir=./ dl-server.wasm

# or use freestanding execution engine
RUST_LOG=info [FREESTANDING EE EXECUTABLE] --program dl-server.wasm --input-source args.cfg cfg model data/mnist/mnist.names.list data/mnist/images_client1/t_00000_c5.png -d -e
```

To test on the validation dataset (10000 images), using the commands below. You will see all predictions and the top 1 and top 5 average accuracies.
```
cp args_files/prediction_multi.cfg args.cfg
wasmtime --dir=./ dl-server.wasm

# or use freestanding execution engine
RUST_LOG=info [FREESTANDING EE EXECUTABLE] --program dl-server.wasm --input-source args.cfg cfg model data -d -e
```


### Run Use Case 2 (YOLO object detection)

To get the YOLO pre-trained model and labels prepared, first run:
```
make yolo_detection
```

Then you can run the object detection on one image:
```
cp args_files/detect.cfg args.cfg
wasmtime --dir=./ dl-server.wasm

# or use freestanding execution engine
RUST_LOG=info [FREESTANDING EE EXECUTABLE] --program dl-server.wasm --input-source args.cfg cfg model data/coco.names data/dog.jpg --output-source ./ -d -e
```

The prediction image can be found under the example root directory.


### Run Use Case 3 (model aggregation)

We support two model formats 1) Darknet 2) ONNX.

**Darknet** is the default format in our example, which all the above training and YOLO detection are based on.
To aggregate several existing Darknet models as a global one:
```
cp args_files/aggregation_darknet.cfg args.cfg
wasmtime --dir=./ dl-server.wasm

# or use freestanding execution engine
RUST_LOG=info [FREESTANDING EE EXECUTABLE] --program dl-server.wasm --input-source args.cfg cfg model --output-source model -d -e
```

**ONNX**, i.e., [Open Neural Network Exchange](https://onnx.ai/), is an interoperable model format, which can act as the intermediate among models of Tensorflow, Pytorch, etc.
To aggregate several existing ONNX models as a global one:
```
cp args_files/aggregation_onnx.cfg args.cfg
wasmtime --dir=./ dl-server.wasm

# or use freestanding execution engine
RUST_LOG=info [FREESTANDING EE EXECUTABLE] --program dl-server.wasm --input-source args.cfg model --output-source model -d -e
```

**ONNX** with Tensorflow and Pytorch as local training framework. (The best way to set up these ML framework is using conda + python 3.7)
```
# Client 1 (preliminary: tensorflow and tf2onnx installed)
cd model && python mnist_tensorflow.py
python -m tf2onnx.convert --saved-model tensorflow_mnist --output tensorflow_mnist.onnx

# Client 2 (preliminary: pytorch installed)
cd model && python mnist_pytorch.py

# aggregate two ONNX models
cp args_files/aggregation_onnx_c2.cfg args.cfg
wasmtime --dir=./ dl-server.wasm
```


Note: all commands are configured in the `args.cfg`. Check different `args_files/_XXX.xfg` to see how the arguments are configured, and edit this file to test different datasets, models, and functions.


### TODO
- Run inside real-time Veracruz execution engine (Policy, etc)