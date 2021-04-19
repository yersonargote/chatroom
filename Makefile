all: server.o client.o
	gcc -o server server.o -pthread
	gcc -o client client.o -pthread

server.o: server.c
	gcc -c -o server.o server.c
client.o: client.c
	gcc -c -o client.o client.c

clean:
	rm -rf *.o server client
