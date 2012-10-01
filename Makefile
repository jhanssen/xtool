all: xtool

xtool: main.o
	gcc -o xtool main.o -lX11

main.o: main.cpp
	gcc -c -o main.o main.cpp

clean:
	rm -f main main.o *~
