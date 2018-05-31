all: main

main: main.c ae.c
	gcc -g -O0 -Wall -o $@ $^

clean:
	rm -f main
