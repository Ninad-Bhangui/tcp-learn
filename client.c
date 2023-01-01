#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define MAXDATASIZE 100
/*
 * Steps:
 * 1. getaddrinfo
 * 2. create socket
 * 3. connect
 * 4. recv
 * 5. exit
 * */
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    struct sockaddr_in *saV4 = (struct sockaddr_in *)sa;
    return &saV4->sin_addr;
  } else {
    struct sockaddr_in6 *saV6 = (struct sockaddr_in6 *)sa;
    return &saV6->sin6_addr;
  }
}
int main(int argc, char *argv[]) {
  int status;
  int sockfd;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(argv[1], "3490", &hints, &servinfo)) != 0) {
    fprintf(stderr, "client: getaddrinfo: %s\n", gai_strerror(status));
    return 1;
  }
  for (p = servinfo; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {
      perror("client: socket");
      continue;
    }
    struct sock_addr *in_addr = get_in_addr(p->ai_addr);
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }
  freeaddrinfo(servinfo);
  int numbytes = recv(sockfd, buf, MAXDATASIZE, 0);
  if (numbytes == -1) {
    perror("client: recv");
    exit(1);
  }
  buf[numbytes] = '\0';

  printf("client: recieved '%s'\n", buf);

  close(sockfd);

  return 0;
}
