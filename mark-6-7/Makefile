all: TCPEchoClientLoop TCPEchoServer-Fork TCPObserver
TCPEchoClientLoop: TCPEchoClientLoop.c DieWithError.c
	gcc TCPEchoClientLoop.c DieWithError.c -o TCPEchoClientLoop
TCPEchoServer-Fork: TCPEchoServer.h TCPEchoServer-Fork.c DieWithError.c
	gcc TCPEchoServer-Fork.c DieWithError.c \
	CreateTCPServerSocket.c AcceptTCPConnection.c \
	-o TCPEchoServer-Fork
TCPObserver: TCPObserver.c DieWithError.c
	gcc TCPObserver.c DieWithError.c -o TCPObserver
