all: TCPEchoClientLoop TCPEchoServer-Fork
TCPEchoClientLoop: TCPEchoClientLoop.c DieWithError.c
	gcc TCPEchoClientLoop.c DieWithError.c -o TCPEchoClientLoop
TCPEchoServer-Fork: TCPEchoServer.h TCPEchoServer-Fork.c DieWithError.c
	gcc TCPEchoServer-Fork.c DieWithError.c \
	CreateTCPServerSocket.c AcceptTCPConnection.c \
	-o TCPEchoServer-Fork
