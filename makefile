all: main newsrv

FLAGS = -g -O0 -static-libstdc++ -static-libgcc -Wall

main: main.c ae.c network.c
	gcc ${FLAGS} -o $@ $^

newsrv: newsrv.cpp ae.c network.c conn_node_buf.c
	g++ ${FLAGS} -o $@ $^

clean:
	rm -f main newsrv
