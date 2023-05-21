#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <stdbool.h>

float EPS = 0.01;
float fun_param_1 = 1;
float fun_param_2 = 2;
float fun_param_3 = 3;

#define RCVBUFSIZE 32 /* Size of receive buffer */

void DieWithError(char *errorMessage); /* Error handling function */

int main(int argc, char *argv[])
{
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int echoStringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv()
                                        and total bytes read */

    if ((argc < 2) || (argc > 3)) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> [<Echo Port>]\n",
                argv[0]);
        exit(1);
    }

    servIP = argv[1]; /* First arg: server IP address (dotted quad) */

    if (argc == 3)
        echoServPort = atoi(argv[2]); /* Use given port, if any */
    else
        echoServPort = 7;

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);      /* Server port */

    char lb[1];

    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");
    while (true)
    {
        bytesRcvd = read(sock, lb, 1);
        // bytesRcvd = read(sock, rb, SIZE_F);
        printf("%s", lb);
        if (lb[0] == '#')
        {
            break;
        }
        fflush(stdout);
    }
    printf("\n"); /* Print a final linefeed */
    sleep(2);

    close(sock);
    exit(0);
}
