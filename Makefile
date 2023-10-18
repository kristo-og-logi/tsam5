.PHONY: server client

server:
	g++ -std=c++11 server.cpp -o server && ./server

client:
	g++ -std=c++11 client.cpp -o client && ./client 127.0.0.1 4044 
