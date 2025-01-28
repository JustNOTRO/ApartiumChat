CC=g++

server: server/server.cpp
	$(CC) server.cpp -o server

client: client/client.cpp
	$(CC) client.cpp -o client

clean:
	rm server client

terminal:
	gnome-terminal