#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define LISTEN_BACKLOG 3

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    struct sockaddr_in *saV4 = (struct sockaddr_in *)sa;
    return &saV4->sin_addr;
  }
  struct sockaddr_in6 *saV6 = (struct sockaddr_in6 *)sa;
  return &saV6->sin6_addr;
}

void get_address_list(const char *node, const char *service,
                      struct addrinfo **res) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;     // don't care ipv4/6
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_flags = AI_PASSIVE;     // fill in my ip

  int status = getaddrinfo(NULL, "3490", &hints,
                           res); // hints is used as a filter criteria to
                                 // decide addresses to store in servinfo
  if (status != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n",
            gai_strerror(status)); // gai_strerror: parses status but how?
    exit(1);
  }
}

int bind_relevant_address(struct addrinfo *servlist) {
  struct addrinfo *p;
  int yes = 1;
  int sockfd;
  for (p = servlist; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    // sockfd = socket(12, p->ai_socktype, p->ai_protocol); //Uncomment and
    // comment above to test perror in use

    if (sockfd == -1) {
      perror("server: socket"); // perror? I think perror emits message from
                                // errno which is global
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    int bind_status = bind(sockfd, p->ai_addr, p->ai_addrlen);
    if (bind_status == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    break;
  }
  if (p == NULL) {
    fprintf(stderr, "server failed to bind\n");
    exit(1);
  }
  return sockfd;
}

int main(void) {

  int status;
  int sockfd;
  struct addrinfo *servinfo, *p; // will point to results
  struct sockaddr_storage client_addr;

  get_address_list(NULL, "3490", &servinfo);
  sockfd = bind_relevant_address(servinfo);
  freeaddrinfo(servinfo);

  if ((listen(sockfd, LISTEN_BACKLOG)) == -1) {
    perror("server: listen");
    close(sockfd);
    exit(1);
  }

  printf("server: waiting for connections..\n");
  while (1) {
    socklen_t client_addr_size = sizeof client_addr;
    int new_fd =
        accept(sockfd, (struct sockaddr *)&client_addr,
               &client_addr_size); // client_addr_size is a pointer becuase
                                   // accept will reduce it if fewer bytes are
                                   // filled into client_addr
    if (new_fd == -1) {
      printf("nothing to accept");
      perror("accept");
      continue;
    }
    char address_str[INET6_ADDRSTRLEN]; // stores address info in string

    struct sockaddr *client_in_addr = get_in_addr((struct sockaddr *)&client_addr);
        
    printf("server: ran till before inet_ntop\n");
    inet_ntop(client_addr.ss_family, client_in_addr, address_str,
              sizeof address_str);
    if (*address_str == '\0') {
      fprintf(stderr, "some error in inet_ntop");
      exit(1);
    }
    printf("server: ran till after inet_ntop\n");

    printf("server: got connection from %s\n", address_str);
    if (!fork()) {
      close(sockfd);
      if (send(new_fd, "Hello, world!", 13, 0) == -1)
        perror("send");
      close(new_fd);
      exit(0);
    }
    close(new_fd);
  }

  return 0;
}
