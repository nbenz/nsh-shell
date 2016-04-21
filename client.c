#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "defs.h"

unsigned long promptForINETAddress();
unsigned long nameToAddr();
void readAndPrint(int);

int connectToHost(char*, int);
void run(int);

int main(int argc, char *argv[]) {
  int fd;
  int port = DEFAULT_PORT;
  char *host = DEFAULT_HOST;

  int c;

  opterr = 0;
  while ((c = getopt (argc, argv, "p:h:")) != -1) {
    switch(c) {
      case 'p':
        port = atoi(optarg);
        if(port == 0) {
          fprintf(stderr, "error: invalid port: %s", optarg);
          exit(EXIT_FAILURE);
        }
        break;
      case 'h':
        host = optarg;
        break;
      default:
        abort();
    }
  }

  fd = connectToHost(host, port);

  if(fd == -1) {
    fprintf(stderr, "Unable to connect to %s:%d\n", host, port);
  } else {
    run(fd);
  }
}

int connectToHost(char *host, int port) {
  int clientFd, serverLen, result, i;
  struct sockaddr_in serverINETAddress;
  struct sockaddr* serverSockAddrPtr;
  unsigned long inetAddress;

  serverSockAddrPtr = (struct sockaddr*) &serverINETAddress;
  serverLen = sizeof(serverINETAddress);

  inetAddress = nameToAddr(host); 

  if (inetAddress == 0) return 0;

  bzero ((char*)&serverINETAddress,sizeof(serverINETAddress));
  serverINETAddress.sin_family = AF_INET;
  serverINETAddress.sin_addr.s_addr = inetAddress; 
  serverINETAddress.sin_port = htons (port);

  clientFd = socket (AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
  
  for(i = 0; i < MAX_TRIES; i++) {
    fprintf(stderr, "Connecting to %s:%d ...\n", host, port);
    result = connect(clientFd, serverSockAddrPtr, serverLen);
    if(result == 0) {
      fprintf(stdout, "Connection established on %s:%d\n", host, port);
      return clientFd;
    }
    else if (result == -1) sleep (1);
  }

  return -1;
}

unsigned long nameToAddr (name)
  char* name;

{
  char hostName [100];
  struct hostent* hostStruct;
  struct in_addr* hostNode;

  if (isdigit (name[0])) return (inet_addr (name));

  hostStruct = gethostbyname (name);
  if (hostStruct == NULL) return (0); 

  hostNode = (struct in_addr*) hostStruct->h_addr;

  return (hostNode->s_addr); 
}

void run(int fd) {
  char *line;
  size_t length;
  int bytes;

  while(1) {
    readAndPrint(fd);
    line = malloc(BUFSIZE + 1);
    length = getline(&line, &length, stdin);

    bytes = write(fd, line, length);

    if(bytes != length)
      fprintf(stderr, "client: error writing");

    if(strcmp(line, "exit\n") == 0)
      break;

    free(line);
  }
}

void readAndPrint(int fd) {
  char c;
  int n;

  n = read(fd, &c, 1);
  while(n > 0 && c != MESSAGE_END) {
    putchar(c);
    n = read(fd, &c, 1);
  }
}
