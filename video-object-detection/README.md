# Video object detection example

This example combines and integrates two simpler examples, the video decoder and the [deep learning server](https://github.com/veracruz-project/veracruz-examples/tree/main/deep-learning-server).
The video decoder uses [`openh264`](https://github.com/veracruz-project/openh264) to decode an H264 video into individual frames, which are converted to RGB and made palatable to an object detector built on top of the [Darknet neural network framework](https://github.com/mofanv/darknet-src). The output is a list of detected objects, associated with their detection probability, and an optional prediction image showing each detected object in a bounding box.

## Build
* Install [`wasi sdk 14`](https://github.com/WebAssembly/wasi-sdk) and set `WASI_SDK_ROOT` to point to its installation directory
* Install `imagemagick`
* Install `nasm`
* Clone the repo and update the submodules:
  ```
  git submodule update --init
  ```
* Run `make` to build [`openh264`](https://github.com/veracruz-project/openh264), [`openh264-dec`](https://github.com/veracruz-project/openh264-dec), [`darknet`](https://github.com/mofanv/darknet-src) and the main program
* To get the YOLO pre-trained model and labels prepared, run:
  ```
  make yolo_detection
  ```

## Run
* Cut the MP4 video to a specific amount of frames (optional):
  ```
  ffmpeg -i in.mp4 -vf trim=start_frame=0:end_frame=<END_FRAME> -an in_cut.mp4
  ```
* Generate the input H.264 video from the video:
  ```
  ffmpeg -i in.mp4 -map 0:0 -vcodec copy -an -f h264 in.h264
  ```
* Run the example in wasmtime:
  ```
  wasmtime --enable-simd --dir=. detector.wasm in.h264
  ```
* Or in the freestanding execution engine:
  ```
  RUST_LOG=info RUST_BACKTRACE=1 freestanding-execution-engine -d in.h264 -d cfg/yolov3.cfg -d model/yolov3.weights -d data/coco.names `for i in data/labels/*_*.png; do echo "-d $i"; done | sort -V | xargs` -p detector.wasm -x jit -o true -e true -c true
  ```
* The prediction image can be found in the root directory
