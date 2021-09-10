# OpenH264 decoder example

This example decodes an H264 video and saves the decoded frames to a file.

## Build
* Run `make` to build [`openh264`](https://github.com/veracruz-project/openh264), [`openh264-dec`](https://github.com/veracruz-project/openh264-dec) and the example

## Run
* Prerequisite: generate the input H264 video from an MP4 video:
  ```
  ffmpeg -i in.mp4 -map 0:0 -vcodec copy -an -f h264 in.h264
  ```
* Run the example in wasmtime:
  ```
  wasmtime --dir=. dec.wasm
  ```
* Or in the freestanding execution engine:
  ```
  RUST_LOG=debug RUST_BACKTRACE=1 cargo run -- -d in.h264 -p dec.wasm -x interp -o true -e true
  ```
* Read the output with VLC (adjust fps and video width and height to the original video):
  ```
  cvlc --rawvid-fps <fps> --rawvid-width <width> --rawvid-height <height> --rawvid-chroma I420 out.yuv
  ```
