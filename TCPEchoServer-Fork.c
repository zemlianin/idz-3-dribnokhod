#include "TCPEchoServer.h" /* TCP echo server includes */
#include <sys/wait.h>      /* for waitpid() */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <stdbool.h>
#define RCVBUFSIZE 32

sem_t *sem;
float EPS = 0.01;
float fun_param_1 = 1;
float fun_param_2 = 2;
float fun_param_3 = 3;

double f(double x)
{
    return abs(fun_param_1 * x * x * x + fun_param_2 * x * x + fun_param_3 * x);
}

double q_integral(double left, double right, double f_left, double f_right, double intgrl_now)
{
    sleep(1); // чтобы успевать что-то видеть
    double mid = (left + right) / 2;
    double f_mid = f(mid); // Аппроксимация по левому отрезку

    double l_integral = (f_left + f_mid) * (mid - left) / 2; // Аппроксимация по правому отрезку

    double r_integral = (f_mid + f_right) * (right - mid) / 2;
    if (abs((l_integral + r_integral) - intgrl_now) > EPS)
    { // Рекурсия для интегрирования обоих значений
        l_integral = q_integral(left, mid, f_left, f_mid, l_integral);
        r_integral = q_integral(mid, right, f_mid, f_right, r_integral);
    }

    return (l_integral + r_integral);
}

int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    unsigned short echoServPort;     /* Server port */
    pid_t processID;                 /* Process ID from fork() */
    unsigned int childProcCount = 0; /* Number of child processes */
    const char *sem_name = "sem100";
    int SIZE = 256;

    int shm_fd;
    void *ptr;
    const char *name = "name";
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SIZE);
    ptr = mmap(0, SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);

    const char *names1 = "s1";
    int shm_fds1;
    void *ptrs1;
    shm_fds1 = shm_open(names1, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fds1, SIZE);
    ptrs1 = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fds1, 0);

    sprintf(ptr, "%f", 0.0);

    sprintf(ptrs1, "%f", 0.0);

    float left = 0;
    float right = 10;
    float step = (right - left) / 10;

    if ((sem = sem_open(sem_name, O_CREAT, 0666, 1)) == 0)
    {
        perror("sem_open: Can not create admin semaphore");
        exit(-1);
    };

    if (argc != 2) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]); /* First arg:  local port */
    servSock = CreateTCPServerSocket(echoServPort);
    int cur = 0;
    bool flag = false;

    for (;;) /* Run forever */
    {
        clntSock = AcceptTCPConnection(servSock);
        /* Fork child process and report any errors */
        if ((processID = fork()) < 0)
            DieWithError("fork() failed");
        else if (processID == 0) /* If this is the child process */
        {
            close(servSock); /* Child closes parent socket */

            char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
            int recvMsgSize;             /* Size of received message */

            while (cur <= 10)
            {
                sem_wait(sem);
                float cur = atof((char *)ptr);
                sprintf(ptr, "%f", cur + 1);
                sem_post(sem);
                if (cur > 10)
                {
                    break;
                }
                float l = left + cur * step;
                float r = left + (cur + 1) * step;
                // double area = q_integral(l, r, f(l), f(r), (f(l) + f(r)) * (r - l) / 2);
                // printf("%f\t%f\t : %f\n", l, r, area);
                int SIZE_F = 8;
                char lb[SIZE_F];
                char rb[SIZE_F];
                sprintf(lb, "%f", l);
                sprintf(rb, "%f", r);
                send(clntSock, lb, SIZE_F, 0);
                send(clntSock, rb, SIZE_F, 0);
                // recvMsgSize = recv(clntSock, echoBuffer, RCVBUFSIZE, 0);
                // float ans = atof(echoBuffer);
                // printf("%f\n", ans);
            }
            if (sem_unlink(sem_name) == -1)
            {
                perror("sem_unlink: Incorrect unlink of full semaphore");
                exit(-1);
            };

            /*if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
                DieWithError("recv() failed");

            while (recvMsgSize > 0)
            {
                if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
                    DieWithError("send() failed");
                if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
                    DieWithError("recv() failed");
            }*/

            close(clntSock); /* Close client socket */

            exit(0); /* Child process terminates */
        }

        printf("with child process: %d\n", (int)processID);
        close(clntSock);  /* Parent closes child socket descriptor */
        childProcCount++; /* Increment number of outstanding child processes */

        while (childProcCount) /* Clean up all zombies */
        {
            processID = waitpid((pid_t)-1, NULL, WNOHANG); /* Non-blocking wait */
            if (processID < 0)                             /* waitpid() error? */
                DieWithError("waitpid() failed");
            else if (processID == 0) /* No zombie to wait on */
                break;
            else
                childProcCount--; /* Cleaned up after a child */
        }
    }
    /* NOT REACHED */
}
