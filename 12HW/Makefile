HALIDE_DISTRIB_PATH = $(realpath /opt/halide)
DNNLROOT = $(realpath /opt/oneDNN-2.7.2)

CXX = g++
CXXFLAGS = -Wall -O3 -std=c++17 -I $(HALIDE_DISTRIB_PATH)/include -I $(DNNLROOT)/include -I $(DNNLROOT)/examples -I $(DNNLROOT)/build/include
LDFLAGS = -ldl -lpthread -lz

LIBHALIDE_LDFLAGS = -Wl,-rpath,$(HALIDE_DISTRIB_PATH)/lib -L $(HALIDE_DISTRIB_PATH)/lib -lHalide
LIBDNNL_LDFLAGS = -Wl,-rpath,$(DNNLROOT)/build/src -L $(DNNLROOT)/build/src -ldnnl

.PHONY: all
all: matmul conv dilated_conv op_fuse

matmul: matmul.cpp halide_benchmark.h common.h
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBHALIDE_LDFLAGS) $(LIBDNNL_LDFLAGS) $(LDFLAGS)

conv: conv.cpp halide_benchmark.h common.h
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBHALIDE_LDFLAGS) $(LIBDNNL_LDFLAGS) $(LDFLAGS)

dilated_conv: dilated_conv.cpp halide_benchmark.h common.h
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBHALIDE_LDFLAGS) $(LIBDNNL_LDFLAGS) $(LDFLAGS)

op_fuse: op_fuse.cpp halide_benchmark.h common.h
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBHALIDE_LDFLAGS) $(LIBDNNL_LDFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf matmul conv dilated_conv op_fuse
