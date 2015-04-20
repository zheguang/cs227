CC = g++
CFLAGS = -g -std=c++11 -pedantic -Wall -Werror
LDLIBS = -lnuma

srcDir := ../

HybridMemory.o: $(srcDir)/HybridMemory.cpp $(srcDir)/HybridMemory.hpp $(srcDir)/FatalError.hpp
	$(CC) $(CFLAGS) -c $< -o $@

tuple.o: tuple.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# For canonical kd-tree
kd-tree.o: kd-tree.cpp
	$(CC) $(CFLAGS) -c kd-tree.cpp

kd: kd-tree.o HybridMemory.o tuple.o
	$(CC) $(CFLAGS) -o test_kd tuple.o kd-tree.o HybridMemory.o test_kd.cpp $(LDLIBS)


# For Cache sensitive kd-tree
#kd: cs-kd-tree.o HybridMemory.o tuple.o
#	$(CC) $(CFLAGS) -o test_cskd tuple.o cs-kd-tree.o HybridMemory.o test_cskd.cpp $(LDLIBS)

clean: 
	rm *.o test_kd 
