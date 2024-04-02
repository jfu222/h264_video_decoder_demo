CC = gcc
CXX = g++
is_debug = 0
DEBUG =
prefix = ./build_linux
debug_flag=


ifeq ($(is_debug), 1)
	DEBUG = -g -O0
	debug_flag=-d
else
	DEBUG = -O3
	debug_flag=
endif

MAKEFILE_PATH = $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR = $(shell dirname $(MAKEFILE_PATH))
exe_version = $(shell awk '{if(/^\#define MAJOR_VERSION|^\#define MINOR1_VERSION|^\#define MINOR2_VERSION/){printf("%s.",$$3);}else if(/^\#define MINOR3_VERSION/){printf("%s",$$3);}}' h264_video_decoder_demo/version.h)

out_exe = h264_video_decoder_demo.$(exe_version)$(debug_flag)

#-----------------------
SOURCE_DIR = h264_video_decoder_demo

#-----------------------
INCLUDE = \
	-I. \
	-I./h264_video_decoder_demo

LIB_PATH = \
	-L.

CFLAGS = -c -Wall $(DEBUG) -fPIC -fstack-protector-all -std=c++11
LIBS = -lpthread

#-----------------------
BUILD_DIR = build
BIN_DIR = bin
LIB_DIR = lib
INTER_DIR = $(BUILD_DIR)/intermediates/h264_video_decoder_demo
CXX_EXTS = cpp cxx cc mm

#OBJS = $(SRCS:.cpp=.o)
CXX_SRCS :=$(foreach SD,$(SOURCE_DIR),$(foreach EXT, $(CXX_EXTS),$(shell find -L $(SD) -name "*.$(EXT)" )))
CXX_OBJS :=$(foreach EXT, $(CXX_EXTS),$(patsubst %.$(EXT),$(INTER_DIR)/%.o,$(filter %.$(EXT),$(CXX_SRCS))))

CXXFLAGS = $(CFLAGS) $(INCLUDE)

OBJS = $(CXX_OBJS)
BINARY_OUTPUT = $(BUILD_DIR)/$(BIN_DIR)/$(out_exe)


#----------------------------------
$(BINARY_OUTPUT): $(OBJS)
	@ mkdir -p $(dir $@)
	$(CXX) -o $@ $(filter %.o, $^) $(LIB_PATH) $(LIBS)

define compile_rule_cxx
$$(INTER_DIR)/%.o: %.$1 $$(THIS_MAKEFILE)
	@ mkdir -p $$(dir $$@)
	$$(CXX) $$(CXXFLAGS) -o $$@ $$<
endef

$(foreach EXT, $(CXX_EXTS), $(eval $(call compile_rule_cxx,$(EXT))))

#----------------------------------
clean:
	rm -rf ${OBJS} $(out_exe)

#----------------------------------
install:
	/bin/mkdir -p $(prefix)
	/usr/bin/install -c $(BINARY_OUTPUT) $(prefix)/
	cd $(prefix)/ && ln -sf $(out_exe) h264_video_decoder_demo

#----------------------------------
uninstall:
	rm -rf $(prefix)

