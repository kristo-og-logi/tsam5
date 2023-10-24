.PHONY: server client selectServer selectClient

server:
	g++ -std=c++11 server.cpp -o server && ./server

client:
	g++ -std=c++11 example_client.cpp -o client && ./client 127.0.0.1 4044 


selectServer:
	g++ -std=c++11 Client.cpp serverConnect.cpp serverCommands.cpp clientCommands.cpp selectServer.cpp -o selectServer && ./selectServer 4000 4001

selectClient:
	g++ -std=c++11 selectClient.cpp -o selectClient && ./selectClient 127.0.0.1 4001

compileServer:
	g++ -std=c++11 Client.cpp serverConnect.cpp serverCommands.cpp clientCommands.cpp selectServer.cpp -o selectServer

compileClient:
	g++ -std=c++11 selectClient.cpp -o selectClient
