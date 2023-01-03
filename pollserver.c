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
#define MESSAGE_SIZE 50
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
  poll_fd_list[0].fd = listener;
  poll_fd_list[0].events = POLLIN;

  for (;;) {
    int poll_count = poll(poll_fd_list, fd_size, -1);
    if (poll_count == -1) {
      perror("server:poll");
      exit(1);
    }
    for (int i = 0; i < fd_size; i++) {
      // CAN BE POLLED
      if (poll_fd_list[i].revents && POLLIN) {
        // Is listener ready
        if (poll_fd_list[i].fd == listener) {
          //
          int new_fd;
          struct sockaddr_storage client_addr;
          char client_addr_str[INET6_ADDRSTRLEN];
          socklen_t client_addr_len = sizeof client_addr;
          struct sockaddr *client_addr_in = (struct sockaddr *)&client_addr;
          new_fd = accept(listener, client_addr_in, &client_addr_len);
          if (new_fd == -1) {
            perror("server:accept");
            continue;
          }
          struct sockaddr_in *agnostic_client_addr =
              get_agnostic_addr(client_addr_in);
          inet_ntop(client_addr.ss_family, agnostic_client_addr,
                    client_addr_str, sizeof client_addr_str);
          printf("Got connection from %s\n", client_addr_str);
          add_to_fd_list(&poll_fd_list, new_fd, &fd_count, &fd_size);

        } else {
          char message[MESSAGE_SIZE];
          int sender = poll_fd_list[i].fd;
          int bytes_recieved =
              recv(poll_fd_list[i].fd, message, MESSAGE_SIZE, 0);
          if (bytes_recieved == -1) {
            perror("server:recv");
          }
          for (int j = 0; j < fd_size; j++) {
            if (poll_fd_list[j].fd != listener &&
                poll_fd_list[j].fd != sender) {
              int bytes_sent =
                  send(poll_fd_list[j].fd, message, MESSAGE_SIZE, 0);
              if (bytes_sent == -1) {
                perror("server:send");
              }
            }
          }
          printf("got message %s", message);
        }
      }
    }
  }
  close(listener);
}
