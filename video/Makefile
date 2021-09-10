OPENH264_LIB_PATH = openh264
OPENH264DEC_LIB_PATH = openh264-dec

# wasi sdk toolchain
WASI_SDK_SYSROOT=$(WASI_SDK_ROOT)/share/wasi-sysroot
CLANG_FLAGS=--target=wasm32-wasi

all: wasi

wasi: dec.cpp
	make -C $(OPENH264_LIB_PATH) libopenh264.a
	make -C $(OPENH264DEC_LIB_PATH)
	$(WASI_SDK_ROOT)/bin/clang++ --sysroot=$(WASI_SDK_SYSROOT) $(CLANG_FLAGS) dec.cpp -I $(OPENH264DEC_LIB_PATH)/inc -L $(OPENH264DEC_LIB_PATH) -lopenh264dec -L $(OPENH264_LIB_PATH) -lopenh264 -o dec.wasm

native: dec.cpp
	$(CXX) dec.cpp -I $(OPENH264DEC_LIB_PATH)/inc/ -L $(OPENH264DEC_LIB_PATH) -lopenh264dec -L $(OPENH264_LIB_PATH) -lopenh264 -lpthread -o dec

clean:
	rm dec dec.wasm
