.SUFFIXES: .o .cpp .c

CFLAGS:= -DUSE_TCL_STUBS -DSWIGTCL -O3 -g3 -DHAVE_64BIT -DHAVE_HDF5 -march=core2 $(CFLAGS) -I ../StaticImglibs/build/include/

LDFLAGS=-L ../StaticImglibs/build/lib/ -static-libgcc -static-libstdc++ -lmfhdf -lhdf -lhdf5_hl -lhdf5 -lrt -lm -lz -ljpeg

.c.o:
	gcc $(CFLAGS) -fPIC -Wall -c -o $@ $<

.cpp.o:
	g++ $(CFLAGS) -fPIC -Wall -c -o  $@ $<

OBJ = hdfpp.o hdfpp_wrap.o
HDR = hdfpp.hpp


all: hdfpp64.so

hdfpp64.so: $(OBJ) 
	g++ $(CFLAGS) -shared -fPIC -Wl,--export-dynamic -o $@ $(OBJ) $(LDFLAGS) -ltclstub8.6 -lm -static-libgcc

hdfpp_wrap.cpp: hdfpp.i $(HDR)
	swig -Wall -tcl -c++ -DHAVE_64BIT -DHAVE_HDF5 -o hdfpp_wrap.cpp hdfpp.i 

docs:
	doxygen

tags:
	ctags -R

test:


clean: 
	rm -rf $(OBJ) hdfpp64.so hdfpp_wrap.cpp
