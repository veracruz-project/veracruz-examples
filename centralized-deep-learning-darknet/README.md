## Centralized Deep Learning Application

This is an application that supports training neural networks inside TEEs by leveraging Veracruz. 

This application is actively under development. Please keep in touch!

### Build

1- Install `WASI SDK` by following [https://github.com/WebAssembly/wasi-sdk](https://github.com/WebAssembly/wasi-sdk)

2- Replace the `CC` as your wasi-sdk directory in `Makefile`

3- In your terminal: `make`

4- Now you should see `darknet.wasm` under the main directory

### Run test (model training)

Train a small model on MNIST dataset (a subset with 3000 images under `data` directory). Run `data/mnist/download_and_convert_mnist.py` first to download and convert the image dataset.

```
wasmtime --dir=. darknet.wasm classifier train cfg/mnist.dataset cfg/mnist_lenet.cfg
```

The trained model will be saved into `model` directory.

### Run test (YOLO object detection)

Download a YOLO model into `model` folder by `wget https://pjreddie.com/media/files/yolov3-tiny.weights`.

Run the test:

```
wasmtime --dir=. darknet.wasm detect cfg/yolov3-tiny.cfg model/yolov3-tiny.weights data/dog.jpg
```