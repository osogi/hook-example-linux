CPP=clang++

.PHONY: tetris
tetris:
	./tetris/openblok.elf --data ./tetris/data/

.PHONY: inject
inject: injection_lib.so
	LD_PRELOAD=$$PWD/injection_lib.so ./tetris/openblok.elf --data ./tetris/data/

all: injection_lib.so

injection_lib.so: injection_lib.cpp util.cpp
	$(CPP) -D_GNU_SOURCE -fPIC -shared -o injection_lib.so injection_lib.cpp util.cpp
