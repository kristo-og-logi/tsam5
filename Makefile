.PHONY: server client findMyIp

server:
	g++ -std=c++11 Client.cpp ip.cpp sendMessage.cpp createSocket.cpp serverConnect.cpp serverCommands.cpp clientCommands.cpp selectServer.cpp -o tsamgroup6 && ./tsamgroup6 4000 4001

client:
	g++ -std=c++11 runClient.cpp -o client && ./client 127.0.0.1 4001

# server compile 
scompile:
	g++ -std=c++11 Client.cpp ip.cpp sendMessage.cpp createSocket.cpp serverConnect.cpp serverCommands.cpp clientCommands.cpp server.cpp -o tsamgroup6

# client compile 
ccompile:
	g++  -lpthread -std=c++11 runClient.cpp -o client

findMyIp:
	g++ -std=c++11 findMyIp.cpp -o findMyIp && ./findMyIp
