# Right now this is set up for emscripten but should be pretty easy to change
# to work with the default clang installations. The easiest way to use this
# with emscripten itself is to set the EMMAKEN_CFLAGS environment variable
# before calling emcc:
#
#   EMMAKEN_CFLAGS='-Xclang -load -Xclang plugin.dylib -Xclang -plugin -Xclang check-initializer-lists' emcc ...
#
CLANG_ROOT = $(shell echo ~/.emsdk_portable/clang/3.2_64bit)
EMSCRIPTEN_ROOT = $(shell echo ~/.emsdk_portable/emscripten/1.7.8)

SOURCE = plugin.cpp
TARGET = plugin.dylib
TEST = demo.cpp

CFLAGS += -fno-rtti
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wno-unused-parameter
CFLAGS += -D__STDC_LIMIT_MACROS
CFLAGS += -D__STDC_CONSTANT_MACROS
CFLAGS += -fstrict-aliasing
CFLAGS += -fPIC
CFLAGS += -dynamiclib
CFLAGS += -I$(CLANG_ROOT)/include
CFLAGS += -L$(CLANG_ROOT)/lib
CFLAGS += -Wl,-headerpad_max_install_names
CFLAGS += -Wl,-flat_namespace
CFLAGS += -Wl,-undefined
CFLAGS += -Wl,suppress
CFLAGS += -stdlib=libstdc++
CFLAGS += -exported_symbols_list exports.txt

LIBS += -lLLVMSupport
LIBS += -lLLVMMC
LIBS += -lLLVMMCParser
LIBS += -lLLVMCore
LIBS += -lLLVMBitReader
LIBS += -lLLVMTransformUtils
LIBS += -lclangFrontend
LIBS += -lclangAnalysis
LIBS += -lclangAST
LIBS += -lclangBasic
LIBS += -lclangDriver
LIBS += -lclangEdit
LIBS += -lclangLex
LIBS += -lclangParse
LIBS += -lclangSema
LIBS += -lclangSerialization

TFLAGS += -isystem $(EMSCRIPTEN_ROOT)/system/include/compat
TFLAGS += -isystem $(EMSCRIPTEN_ROOT)/system/include/libc
TFLAGS += -isystem $(EMSCRIPTEN_ROOT)/system/include/libcxx
TFLAGS += -D __EMSCRIPTEN__
TFLAGS += -U __APPLE__
TFLAGS += -fsyntax-only
TFLAGS += -triple le32-unknown-nacl
TFLAGS += -load $(TARGET)
TFLAGS += -plugin check-initializer-lists
TFLAGS += -fcolor-diagnostics
TFLAGS += -std=c++11

default: clean build test

build:
	cc $(CFLAGS) $(LIBS) $(SOURCE) -o $(TARGET)

test:
	$(CLANG_ROOT)/bin/clang -cc1 $(TFLAGS) $(TEST)

clean:
	rm -f $(TARGET)
