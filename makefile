CC=g++
CFLAGS = -std=c++17 -pthread

SkipList: server client

server: main.cpp ./Socket/Socket.cpp ./server/server.cpp ./con/con.cpp
	$(CC) $^ -o ./bin/server $(CFLAGS)

client: ./client/client.cpp
	$(CC) $^ -o ./bin/client $(CFLAGS)

clean: 
	rm -f ./bin/*
