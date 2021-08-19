# Makefile for the deep learning server example
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz deep learning server 
# example repository root directory for copyright and licensing information.

VPATH=./src/:./examples
EXEC=dl-server.wasm
OBJDIR=./obj/

COMMON= -Iinclude/ -Isrc/ -Iconnxr/src -Iconnxr/include -Iconnxr/protobuf

CC=../wasi-sdk-12.0/bin/clang --sysroot=../wasi-sdk-12.0/share/wasi-sysroot

CFLAGS=-Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC
OPTS=-Ofast
#OPTS=-O0 -g

CFLAGS+=$(OPTS)

LDFLAGS= -lm -g

OBJ=gemm.wasm utils.wasm cuda.wasm deconvolutional_layer.wasm convolutional_layer.wasm list.wasm image.wasm activations.wasm im2col.wasm col2im.wasm blas.wasm crop_layer.wasm dropout_layer.wasm maxpool_layer.wasm softmax_layer.wasm data.wasm matrix.wasm network.wasm connected_layer.wasm cost_layer.wasm parser.wasm option_list.wasm detection_layer.wasm route_layer.wasm upsample_layer.wasm box.wasm normalization_layer.wasm avgpool_layer.wasm layer.wasm local_layer.wasm shortcut_layer.wasm logistic_layer.wasm activation_layer.wasm rnn_layer.wasm gru_layer.wasm crnn_layer.wasm demo.wasm batchnorm_layer.wasm region_layer.wasm reorg_layer.wasm tree.wasm  lstm_layer.wasm l2norm_layer.wasm yolo_layer.wasm iseg_layer.wasm
EXECOBJA=aggregation.wasm classifier.wasm detector.wasm darknet.wasm


INCDIR+=connxr/include connxr/protobuf connxr/src
CPPFLAGS+=$(foreach DIR, $(INCDIR),-I $(DIR) )

CONNXR_BUILDDIR?=obj/connxr
SRCDIR+=connxr/src/operators connxr/protobuf
SRCS+=$(foreach DIR, $(SRCDIR), $(shell find $(DIR) -type f -name '*.c'))

SRCS+=connxr/src/inference.c connxr/src/trace.c connxr/src/utils.c connxr/src/test/test_utils.c
ONNXOBJ=$(SRCS:%.c=$(CONNXR_BUILDDIR)/%.wasm)


EXECOBJ = $(addprefix $(OBJDIR), $(EXECOBJA))
OBJS = $(addprefix $(OBJDIR), $(OBJ))
DEPS = $(wildcard src/*.h) Makefile include/darknet.h
CONNXRDEPS = $(wildcard connxr/src/*.h) $(wildcard connxr/protobuf/*.h) $(wildcard connxr/include/*.h)



all: wasisdk obj $(EXEC)

examples: mnist_training yolo_detection

$(EXEC): $(EXECOBJ) $(OBJS) $(ONNXOBJ)
	$(CC) $(COMMON) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)%.wasm: %.c $(DEPS) $(CONNXRDEPS)
	$(CC) $(COMMON) $(CFLAGS) -c $< -o $@

$(CONNXR_BUILDDIR)/%.wasm: %.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $(CFLAGS) $(CPPFLAGS) $^



ifeq ($(shell uname), Darwin)
	OS=macos
endif
ifeq ($(shell uname), Linux)
	OS=linux
endif
wasisdk:
	if [ ! -d "../wasi-sdk-12.0" ]; then \
		wget -P ../ https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-12/wasi-sdk-12.0-$(OS).tar.gz; \
		mkdir ../wasi-sdk-12.0; \
		tar xvf ../wasi-sdk-12.0-$(OS).tar.gz -C ../; \
		rm -f ../wasi-sdk-12.0-macos.tar.gz; \
	fi


onnx:
	$(MAKE) -C connxr


obj:
	mkdir -p obj


NC=$(num-clients)
ifeq ($(NC), )
	NC=10
endif
mnist_training:
	python data/mnist/mnist_preparation.py --num-clients=$(NC)


yolo_detection:
	if [ "$(OS)" = "macos" ]; then \
		brew install imagemagick ghostscript; \
	elif [ "$(OS)" = "linux" ]; then \
		sudo apt install imagemagick; \
	fi
	python data/labels/make_labels.py
	wget -P model/ https://pjreddie.com/media/files/yolov3-tiny.weights


.PHONY: wasisdk mnist_training yolo_detection clean


clean:
	rm -rf $(OBJS) $(EXEC) $(EXECOBJ) $(OBJDIR)/* obj connxr/build ../wasi-sdk-12.0
