/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/*
Command line arguments:
0 - name
1 - port-number where I will listen to for clients
*/
int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	// check if port number provided
	if (argc < 2) {
	 fprintf(stderr,"ERROR, no port provided\n");
	 exit(1);
	}

	// create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	error("ERROR opening socket");

	// set the buffer to zero
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);

	// set my attributes in serv_add. This is me
	// my own address: INADDR_ANY
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	// bind myself (the server) with the socket
	// clients will try to communicate using this
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");

	// listen for connections
	printf("Listening for connections.\n");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);

	// accept the connection. all future communications with the client:
	// needs to be done through this new socket_descriptor
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) 
	  error("ERROR on accept");
	bzero(buffer,256);

	// read from client
	n = read(newsockfd,buffer,255);
	if (n < 0) error("ERROR reading from socket");
	printf("Here is the message: %s\n",buffer);

	// write to the client
	n = write(newsockfd,"I got your message",18);
	if (n < 0) error("ERROR writing to socket");

	// close the sockets     
	close(newsockfd);
	close(sockfd);
	return 0; 
}
