.SUFFIXES: .o .cpp .c

CFLAGS:=-DUSE_TCL_STUBS -DSWIGTCL -O3 -g3 -fopenmp -DHAVE_64BIT -march=core2 $(CFLAGS) -I ~/bin/include/

LDFLAGS=-L ~/bin/lib64/ -lmfhdf -ldf -lz -ljpeg

.c.o:
	gcc $(CFLAGS) -fPIC -Wall -c -o $@ $<

.cpp.o:
	g++ $(CFLAGS) -fPIC -Wall -pedantic -c -o  $@ $<

OBJ = hdfpp.o hdfpp_wrap.o
HDR = hdfpp.hpp


all: hdfpp64.so

hdfpp64.so: $(OBJ) 
	g++ $(CFLAGS) -shared -fPIC -Wl,--export-dynamic -o $@ $(OBJ) $(LDFLAGS) -ltclstub8.5 -lpthread -lm

hdfpp_wrap.cpp: hdfpp.i $(HDR)
	swig -Wall -tcl -c++ -DHAVE_64BIT -o hdfpp_wrap.cpp hdfpp.i 

docs:
	doxygen

tags:
	ctags -R

test:


clean: 
	rm -rf $(OBJ) hdfpp64.so hdfpp_wrap.cpp
