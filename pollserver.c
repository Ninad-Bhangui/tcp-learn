#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "9034"
#define BACKLOG 5

int get_listener() {
  struct addrinfo hints = {0}, *servinfo, *p;
  int listener;
  int yes = 1;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, PORT, &hints, &servinfo) != 0) {
    fprintf(stderr, "some error with getaddrinfo"); // TODO: get actual error
    exit(1);
  }
  for (p = servinfo; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
      perror("server:bind");
      exit(1);
    }
  }
  if (p == NULL) {
    fprintf(stderr, "no address found");
    exit(1);
  }
  if (listen(listener, BACKLOG) == -1) {
    perror("server:listen");
    exit(1);
  }
  return listener;
}
int main() {
  int listener;
  listener = get_listener();
  printf("server: listening now");
}
