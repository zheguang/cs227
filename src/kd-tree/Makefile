CC = g++
CFLAGS = -g -std=c++11 -pedantic -Wall -Werror
LDLIBS = -lnuma

srcDir := ../

HybridMemory.o: $(srcDir)/HybridMemory.cpp $(srcDir)/HybridMemory.hpp $(srcDir)/FatalError.hpp
	$(CC) $(CFLAGS) -c $< -o $@

kd-tree.o: kd-tree.cpp
	$(CC) $(CFLAGS) -c kd-tree.cpp

all: kd-tree.o HybridMemory.o
	$(CC) $(CFLAGS) -o main kd-tree.o HybridMemory.o main.cpp $(LDLIBS)

clean: 
	rm *.o main 
