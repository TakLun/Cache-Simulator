GXX=g++ -g -std=c++0x -D__GXX_EXPERIMENTAL_CXX0X__

all: cache-sim

cache-sim: cache-sim.o
	$(GXX) cache-sim.o -o cache-sim
	
cache-sim.o: cache-sim.cpp
	$(GXX) -c cache-sim.cpp
	
clean:
	rm -f *.o *.txt cache-sim
