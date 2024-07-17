all: client server

client: client.o common.o
	gcc $^ -o ./bin/$@ -lpthread

server: server.o common.o
	gcc $^ -o ./bin/$@ -lpthread

%.o: %.c
	gcc -c $< -o $@

clean:
	rm -f ./bin/client ./bin/server *.o

.PHONY: all clean