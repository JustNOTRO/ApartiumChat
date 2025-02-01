CC=g++

INCLUDES=-I./tcp_server -I./tcp_client

all: server client

server: tcp_server/server.cpp
	$(CC) $(INCLUDES) tcp_server/server.cpp -o server

client: tcp_client/client.cpp
	$(CC) $(INCLUDES) tcp_client/client.cpp -o client

clean:
	rm server client

terminal:
	gnome-terminal