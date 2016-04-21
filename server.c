#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "defs.h"

int startServer(int);
void redirectIO(int);
void run(int);
void runShell();

int main(int argc, char *argv[]) {
  int port, fd;

  if(argc == 1)
    port = DEFAULT_PORT;
  else  {
    port = atoi(argv[1]);
    if(port == 0) {
      fprintf(stderr, "error: invalid port: %s\n", argv[1]);
      exit(EXIT_FAILURE);
    }
  }

  fd = startServer(port);
  redirectIO(fd); //Move to fork routine (when that exists)
  run(fd);
}

int startServer(int port) {
  int serverFd, clientFd, serverLen, clientLen;
  struct sockaddr_in serverINETAddress;
  struct sockaddr_in clientINETAddress;
  struct sockaddr* serverSockAddrPtr;
  struct sockaddr* clientSockAddrPtr;

  signal(SIGCHLD, SIG_IGN);

  clientSockAddrPtr = (struct sockaddr*) &clientINETAddress;
  clientLen = sizeof(clientINETAddress);

  serverFd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
  serverLen = sizeof(serverINETAddress);
  bzero((char*) &serverINETAddress, serverLen);

  serverINETAddress.sin_family = AF_INET;
  serverINETAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverINETAddress.sin_port = htons(port);

  serverSockAddrPtr = (struct sockaddr*) &serverINETAddress;
  bind(serverFd, serverSockAddrPtr, serverLen);
  listen(serverFd, 5);

  printf("Server listening on port: %d\n", port);
  fflush(stdout);

  clientFd = accept(serverFd, clientSockAddrPtr, &clientLen);

  printf("Client connected.\n");
  fflush(stdout);

  return clientFd;
}

void redirectIO(int fd) {
  dup2(fd, READ);
  dup2(fd, WRITE);
  close(fd);
}

void run(int fd) {
  char *line;
  size_t length;

  runShell();
}

