all: main newsrv

main: main.c ae.c network.c
	gcc -g -O0 -Wall -o $@ $^

newsrv: newsrv.cpp ae.c network.c conn_node_buf.c
	g++ -g -O0 -Wall -o $@ $^

clean:
	rm -f main newsrv
