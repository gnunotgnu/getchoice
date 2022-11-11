# makefile
.PHONY: all clean

BINARY=getchoice
#CXX=clang++
CXX=g++
#FLAGS=-std=c++2a -Wall -Wextra -ferror-limit=1 -fdiagnostics-show-template-tree
FLAGS=-std=c++2a -Wall -Wextra

all: main.o qwerty.o
	$(CXX) -o $(BINARY) qwerty.o main.o $(LIBS) 

main.o: main.cpp
	$(CXX) $(FLAGS) -c -o main.o main.cpp

qwerty.o: qwerty.hpp qwerty.cpp
	$(CXX) $(FLAGS) -c -o qwerty.o qwerty.cpp

clean:
	rm $(BINARY) *.o
