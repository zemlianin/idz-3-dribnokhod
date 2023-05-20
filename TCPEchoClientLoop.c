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

float f(float x)
{
    return abs(fun_param_1 * x * x * x + fun_param_2 * x * x + fun_param_3 * x);
}

float q_integral(float left, float right, float f_left, float f_right, float intgrl_now)
{

    float mid = (left + right) / 2;
    float f_mid = f(mid); // Аппроксимация по левому отрезку

    float l_integral = (f_left + f_mid) * (mid - left) / 2; // Аппроксимация по правому отрезку

    float r_integral = (f_mid + f_right) * (right - mid) / 2;
    if (abs((l_integral + r_integral) - intgrl_now) > EPS)
    { // Рекурсия для интегрирования обоих значений
        l_integral = q_integral(left, mid, f_left, f_mid, l_integral);
        r_integral = q_integral(mid, right, f_mid, f_right, r_integral);
    }

    return (l_integral + r_integral);
}

void DieWithError(char *errorMessage); /* Error handling function */

int main(int argc, char *argv[])
{
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *echoString;                /* String to send to echo server */
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int echoStringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv()
                                        and total bytes read */

    if ((argc < 3) || (argc > 4)) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n",
                argv[0]);
        exit(1);
    }

    servIP = argv[1];     /* First arg: server IP address (dotted quad) */
    echoString = argv[2]; /* Second arg: string to echo */

    if (argc == 4)
        echoServPort = atoi(argv[3]); /* Use given port, if any */
    else
        echoServPort = 7; /* 7 is the well-known port for the echo service */

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);      /* Server port */

    /* Establish the connection to the echo server */
    int SIZE_F = sizeof(float);
    char lb[SIZE_F];
    char rb[SIZE_F];
    char ansb[SIZE_F];

    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");
    while (true)
    {
        bytesRcvd = read(sock, lb, SIZE_F);
        bytesRcvd = read(sock, rb, SIZE_F);
        // if ((bytesRcvd = recv(sock, rb, SIZE_F, 0)) <= 0)
        //     DieWithError("recv() failed or connection closed prematurely");
        float l = atof(lb);
        float r = atof(rb);
        // printf("%f", l);
        // printf("%f", r);
        sleep(3); // чтобы все не моментально происходило
        float area = q_integral(l, r, f(l), f(r), (f(l) + f(r)) * (r - l) / 2);
        printf("%f\t%f\t : %f\n", l, r, area);
        // sleep(1);
        // printf("%f-%f\n", l, r);
        if (l < 0 || r < 0)
        {
            break;
        }
        sprintf(ansb, "%f", area);
        send(sock, ansb, SIZE_F, 0);
    }
    printf("\n"); /* Print a final linefeed */
    sleep(2);

    close(sock);
    exit(0);
}
