# tcp-learn

Learning how tcp works and implementing my own using the following guide: https://beej.us/guide/bgnet/html/

compile and run c code

```bash
gcc -g server.c -o server && ./server
```

telnet

```bash
telnet 127.0.0.1 3490
```

## server

The tcp server performs the following steps:

1.  `getaddrinfo`: Gets list of addresses and returns 0 on success or non zero error codes.
    These error codes can be parsed by gai\_strerror function into human readable string.
    It stores list of addresses in addrinfo res struct which is a linked list.

2.  `socket`: creates an actual socket and returns file descriptor. Requires arguments domain, type and protocol.
    *   domain/ai\_family: AF\_INET / AF\_INET6 / AF\_UNIX etc.
    *   type/ai\_socktype: SOCK\_STREAM / SOCK\_DGRAM (tcp/udp) etc.
    *   protocol/ai\_protocol: specifies protocol for returned socket addresses. (usually 0 which means any protocol). TODO: Understand better.

3.  `bind`: bind assigns address to socket. "Assignign name to socket". 0 returned on success and -1 returned on error and errno is set.

4.  `listen`: marks socket as passive socket (which means socket will be used to accept incoming connection requests. backlog specifes maximum length of pending connections for sockfd.
    If connection arrives when queue is full, will get error ECONNREFUSED. 0 returned on success. on failure -1 and errno.

5.  `accept`: extracts first connection request from sockfd, creates new socket and returns descriptor. New socket is not in listening state.
    needs sockaddr \* as argument which get's assigned client address. what we pass to sockaddr \* may be sockaddr\_storage as storage has more memory to accomodate both ipv4 and ipv6. Cast is required.

6.  To get ip address details from client sockaddr\_storage struct.
    *   cast sockaddr\_storage client to sockaddr\_in or sockaddr\_in6 depending on family. After casting return sin\_addr from each struct.
    *   cast sin\_addr from above (either in\_addr or in6\_addr struct) to sockaddr and pass to inet\_ntop.
    *   inet\_ntop returns string from address\_family, sockaddr, and length.

7.  `send`: Finally use new socket descriptor from accept. pass it string buffer, length and flags to send something to client. 
    Make sure this is in a different forked process to not block the main one and close sockfd in forked and newfd in parent.

## client

The tcp client performed following steps:

1. `getaddrinfo`: Gets list of addresses. Pass it the right host and port.
2. `socket`: create an actual socket and return file descriptor. (Same as server)
3. `connect`: connects socket to specified address. 0 on success, -1 on error with errno set.
4. Get ip address details from server to log same as server using `inet_ntop`.
5. `recv`:  recieve message from socket. returns number of bytes received or -1 on error with errno set.
