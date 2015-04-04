#
# Copyright (c) 2015 Brown University. All rights reserved.
#
.PHONY: all clean

CPPFLAGS := -O3 -pedantic -std=c++11 -Wall -Werror

all: DataGen.out

DataGen.out: DataGen.o
	g++ $(CPPFLAGS) $^ -o $@

DataGen.o: DataGen.cpp GenCommon.hpp DataDef.hpp
	g++ $(CPPFLAGS) -c $< -o $@

clean:
	rm -f *.o *.out
