#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int MakeSocket(int , int , int );
int ConnectSocket(int , struct sockaddr *, socklen_t);
int BindSocket(int , struct sockaddr *, socklen_t );
int ListenSocket(int ,int );
int AcceptSocket(int , struct sockaddr *, socklen_t *);
int CloseSocket(int );
ssize_t ReadSocket(int , void *, size_t , int );
ssize_t WriteSocket(int , void *, size_t , int );
void AllowMultipleConection(int);
