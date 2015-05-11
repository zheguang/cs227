CC = g++
CFLAGS = -g -std=c++11 -pedantic -Wall -Werror 
LDLIBS = -lnuma

srcDir := ../

HybridMemory.o: $(srcDir)/HybridMemory.cpp $(srcDir)/HybridMemory.hpp $(srcDir)/FatalError.hpp
	$(CC) $(CFLAGS) -c $< -o $@

csb-tree.o: csb-tree.cpp
	$(CC) $(CFLAGS) -c csb-tree.cpp csb-tree.hpp config.hpp

test_csb.o: test_csb.cpp
	$(CC) $(CFLAGS) -c test_csb.cpp

csb: csb-tree.o test_csb.o HybridMemory.o 
	$(CC) $(CFLAGS) -o $@ csb-tree.o HybridMemory.o test_csb.o $(LDLIBS)

clean: 
	rm *.o csb
