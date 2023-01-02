#include <arpa/inet.h>
#include <asm-generic/socket.h>
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
void *get_agnostic_addr(struct sockaddr *addr) {
  if (addr->sa_family == AF_INET) {
    struct sockaddr_in *v4addr = (struct sockaddr_in *)addr;
    return &(v4addr->sin_addr);
  }
  struct sockaddr_in6 *v6addr = (struct sockaddr_in6 *)addr;
  return &(v6addr->sin6_addr);
}

int add_to_fd_list(struct pollfd **poll_fd_list, int new_fd, int *fd_count,
                   int *fd_size) {
  if (*fd_count == *fd_size) {
    fprintf(stderr, "could not add to fd list, limit reached\n");
    return 1;
  }
  (*poll_fd_list)[*fd_count].fd = new_fd;
  (*poll_fd_list)[*fd_count].events = POLLIN;
  (*fd_count)++;
  return 0;
}
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
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
        -1) {
      perror("server:setsockopt");
      exit(1);
    }
    if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
      perror("server:bind");
      exit(1);
    }
    break;
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
  printf("server: listening now\n");

  int fd_count = 0;
  int fd_size = 5;
  struct pollfd *poll_fd_list = malloc(sizeof *poll_fd_list * fd_size);

  for (;;) {
    int poll_count = poll(poll_fd_list, fd_size, -1);
    if (poll_count == -1) {
      perror("server:poll");
      exit(1);
    }
  }
}
