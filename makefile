$(CC) = gcc
all: server client servicemap
servicemap: servicemap.go
	go build servicemap.go
server: server.c 
	gcc server.c -o server
client: client.go
	go build client.go
depend:
	makedepend -I servicemap.go server.c client.go
clean:
	rm -f server client servicemap
