.PHONY: server client selectServer

server:
	g++ -std=c++11 server.cpp -o server && ./server

client:
	g++ -std=c++11 example_client.cpp -o client && ./client 127.0.0.1 4044 


selectServer:
	g++ -std=c++11 selectServer.cpp -o selectServer && ./selectServer 4000 4001
