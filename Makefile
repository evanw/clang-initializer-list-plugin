ROOT = $(shell echo ~/.emsdk_portable/clang/3.2_64bit)

SOURCE = plugin.cpp
TARGET = plugin.dylib
TEST = hello.cpp

CFLAGS += -fno-rtti
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wno-unused-parameter
CFLAGS += -D__STDC_LIMIT_MACROS
CFLAGS += -D__STDC_CONSTANT_MACROS
CFLAGS += -fstrict-aliasing
CFLAGS += -fPIC
CFLAGS += -dynamiclib
CFLAGS += -I$(ROOT)/include
CFLAGS += -L$(ROOT)/lib
CFLAGS += -Wl,-headerpad_max_install_names
CFLAGS += -Wl,-flat_namespace
CFLAGS += -Wl,-undefined
CFLAGS += -Wl,suppress
CFLAGS += -stdlib=libstdc++

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

TFLAGS += -load $(TARGET)
TFLAGS += -plugin print-fns

default: clean build test

build:
	cc $(CFLAGS) $(LIBS) $(SOURCE) -o $(TARGET)

test:
	$(ROOT)/bin/clang $(TFLAGS:%=-Xclang %) -fsyntax-only $(TEST)

clean:
	rm -f $(TARGET)
