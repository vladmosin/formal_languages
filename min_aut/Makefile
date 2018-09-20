CFLAGS=-Iinclude -g -Wall -Werror -std=c++11

all: min_aut

min_aut: min_aut.o
	g++ $(CFLAGS) min_aut.cpp -o min_aut
	
clean:
	rm -f min_aut.o min_aut
