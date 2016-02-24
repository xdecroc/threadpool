CFLAGS=-std=c++11 
LIBS=-pthread

all: threadpl 

threadpl: main.o
	g++ -g main.o $(LIBS) -o threadpl

main.o: main.cpp ThreadPool.h
	g++ -g -c main.cpp $(CFLAGS) 


clean:
	rm *o threadpl
