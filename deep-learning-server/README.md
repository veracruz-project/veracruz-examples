## Centralized Deep Learning Application

This is an application that supports training neural networks inside TEEs by leveraging Veracruz. 

This application is actively under development. Please keep in touch!

### Build

1- Install `WASI SDK` by following [https://github.com/WebAssembly/wasi-sdk](https://github.com/WebAssembly/wasi-sdk)

2- Replace the `CC` as your wasi-sdk directory in `Makefile`, and run `make`

3- Now you should see `darknet.wasm` under the main directory

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

### TODO
1. Load multiple fictions of a dataset before training (multiple data providers in Veracruz computation) (Done)
2. Secure aggregation support for federated learning: i) darknet model (Done) ii) other format models, e.g., Tensorflow, Pytorch, etc
3. (TBD) In centralized learning: GPU training (privacy-preserving + integrity)
3. (TBD) In federated learning: client-side training integrity check with GPU training
