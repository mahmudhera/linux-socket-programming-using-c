This is a simple socket programming demo. The goal is to write two programs that can communicate with each other using TCP/IP sockets. The readme file contains whatever I learnt (re-learnt) while doing this.

### Creating socket

```
#include<sys/socket.h>
socket_desc = socket(AF_INET , SOCK_STREAM , 0);
```

Arguments:
1. AF_INET: IPv4 protocol
1. SOCK_STREAM: TCP socket, not datagrams
1. 0: there is only one protocol in use here


### Creating a remote server
```
#include<sys/socket.h>
struct sockaddr_in server;
server.sin_addr.s_addr = inet_addr("0.0.0.0"); 	// ip address of the server
server.sin_family = AF_INET;			// which internet??
server.sin_port = htons( 8081 );		// port where the server is working...
```

We just set these parameters. This now is a remote server. We will talk with this server using our socket. To do that, we have to connect the socket with this remote server.

### Connecting remote server with the socket

```
connect(socket_desc , (struct sockaddr *)&server , sizeof(server));
```

If failed, will return -1.

### Sending data using the socket
```
send(socket_desc , message , strlen(message);
```

Will return -ve if failed. Arguments:
1. socket descriptor
1. buffer whose data will be sent
1. size in int, # of bytes to send starting this address in the second argument

### Receiving data
```
recv(socket_desc, server_reply , 2000 , 0);
```
Will return -ve if failed. Arguments:
1. socket descriptor
1. buffer where to store data
1. max num of bytes
