## Centralized Deep Learning Application

This is a Veracruz example that supports training neural networks inside an isolated area on an untrusted device.
(Note: currently only standalone execution is supported. No policy files are provided.)

### Build
```
cd veracruz-examples/deep-learning-server
make all
```
This will also download `wasi-sdk` for compiling this example. A `dl-server.wasm` binary will be outputted to the example root directory.

### Run test (model training)

Train a LeNet model on MNIST dataset. 

First download and convert the dataset:
```
cd deep-learning-server/data/mnist
python mnist_preparation.py
```

Then start training the a model:
```
wasmtime --dir=. darknet.wasm classifier train cfg/mnist.dataset cfg/mnist_lenet.cfg
```

The trained model will be saved into `model` directory.

### Run test (YOLO object detection)

Download a YOLO model into `model` folder by:

```
mkdir deep-learning-server/model
cd deep-learning-server/model
wget https://pjreddie.com/media/files/yolov3-tiny.weights`.
```

Create the labels that will be used in presenting detected objects:
```
cd deep-learning-server/data/labels
python make_labels.py
```

Run the test on one image:
```
wasmtime --dir=. darknet.wasm detect cfg/yolov3-tiny.cfg model/yolov3-tiny.weights data/dog.jpg
```

All commands can be configured in the `args_file.txt`. Then one can run without command-line arguments, such as:
```
wasmtime --dir=. darknet.wasm
```

### TODO
1. Load multiple fictions of a dataset before training (multiple data providers in Veracruz computation) (Done)
2. Secure aggregation support for federated learning: i) darknet model (Done) ii) other format models, e.g., Tensorflow, Pytorch, etc
3. (TBD) In centralized learning: GPU training (privacy-preserving + integrity)
3. (TBD) In federated learning: client-side training integrity check with GPU training
Previously