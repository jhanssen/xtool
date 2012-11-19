all: xtool

xtool: main.o
	g++ -o xtool main.o -lX11

main.o: main.cpp
	g++ -c -o main.o main.cpp

clean:
	rm -f main main.o *~
