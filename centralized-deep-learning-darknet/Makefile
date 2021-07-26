VPATH=./src/:./examples
EXEC=darknet.wasm
OBJDIR=./obj/

CC=/Users/fanvincentmo/Documents/wasi-sdk-12.0/bin/clang --sysroot=/Users/fanvincentmo/Documents/wasi-sdk-12.0/share/wasi-sysroot
NVCC=nvcc 
AR=ar
ARFLAGS=rcs
OPTS=-Ofast
LDFLAGS= -lm
COMMON= -Iinclude/ -Isrc/
CFLAGS=-Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC

ifeq ($(DEBUG), 1) 
OPTS=-O0 -g
endif

CFLAGS+=$(OPTS)


OBJ=gemm.wasm utils.wasm cuda.wasm deconvolutional_layer.wasm convolutional_layer.wasm list.wasm image.wasm activations.wasm im2col.wasm col2im.wasm blas.wasm crop_layer.wasm dropout_layer.wasm maxpool_layer.wasm softmax_layer.wasm data.wasm matrix.wasm network.wasm connected_layer.wasm cost_layer.wasm parser.wasm option_list.wasm detection_layer.wasm route_layer.wasm upsample_layer.wasm box.wasm normalization_layer.wasm avgpool_layer.wasm layer.wasm local_layer.wasm shortcut_layer.wasm logistic_layer.wasm activation_layer.wasm rnn_layer.wasm gru_layer.wasm crnn_layer.wasm demo.wasm batchnorm_layer.wasm region_layer.wasm reorg_layer.wasm tree.wasm  lstm_layer.wasm l2norm_layer.wasm yolo_layer.wasm iseg_layer.wasm
EXECOBJA=classifier.wasm darknet.wasm


EXECOBJ = $(addprefix $(OBJDIR), $(EXECOBJA))
OBJS = $(addprefix $(OBJDIR), $(OBJ))
DEPS = $(wildcard src/*.h) Makefile include/darknet.h

all: obj model $(EXEC)


$(EXEC): $(EXECOBJ) $(OBJS)
	$(CC) $(COMMON) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)%.wasm: %.c $(DEPS)
	$(CC) $(COMMON) $(CFLAGS) -c $< -o $@

obj:
	mkdir -p obj
model:
	mkdir -p model

.PHONY: clean

clean:
	rm -rf $(OBJS) $(SLIB) $(ALIB) $(EXEC) $(EXECOBJ) $(OBJDIR)/* obj model

