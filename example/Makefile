#
# Copyright (c) 2015 Brown University.
#
.PHONY: clean all

CPPFLAGS := -g -pedantic -Wall -Werror -std=c++11
LDLIBS := -lnuma

srcDir := ../src

all: example.out

example.out: HybridMemoryExample.o HybridMemory.o
	g++ $^ -o $@ $(LDLIBS)

HybridMemoryExample.o: HybridMemoryExample.cpp $(srcDir)/HybridMemory.hpp
	g++ $(CPPFLAGS) -c $< -o $@

HybridMemory.o: $(srcDir)/HybridMemory.cpp $(srcDir)/HybridMemory.hpp $(srcDir)/FatalError.hpp
	g++ $(CPPFLAGS) -c $< -o $@

clean: 
	rm -f ./*.o example.out
