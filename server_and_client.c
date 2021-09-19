#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <unistd.h>

#define PORT 8080

void error(const char *msg)
{
    perror(msg);
    exit(0);
}


void print_hostname()
{
	char buffer[100];
	int ret;
	if ((ret = gethostname(buffer, sizeof(buffer))) == -1) {
		perror("gethostname");
		exit(1);
	}
	printf("Hostname: %s\n", buffer);
}


/*
Open a socket on this machine at given port number.
Then, listen for connections from a client.
Finally, make very simple communications
*/
void * server_function(void * arg)
{
	int portno = PORT;
	int sockfd, newsockfd;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	char buffer[256];
	int n;

	// create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	error("ERROR opening socket");

	// set the buffer to zero
	bzero((char *) &serv_addr, sizeof(serv_addr));

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

	return NULL;
}


void * client_function(void * server_addr) //call using gethostbyname(server address)
{
	char * server_address = (char *) server_addr;
	puts(server_address);
	int sockfd, n, portno = PORT;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	// get the server info from its address	
	server = gethostbyname(server_address);
	
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	// set attributes for the server. set internet property, then
	// byte-wise copy server's address, and set the port number
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	// connect to the server by placing it in the other end of the socket
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");

	// get a string input
	printf("Please enter the message: ");
	bzero(buffer,256);
	fgets(buffer,255,stdin);

	// write it to the server
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0) 
		error("ERROR writing to socket");
	bzero(buffer,256);

	// read from the server, and print
	n = read(sockfd,buffer,255);
	if (n < 0) 
		error("ERROR reading from socket");
	printf("%s\n",buffer);
	
	// close the socket
	close(sockfd);

	return NULL;
}


void * test (void * arg)
{
	printf("This is from a thread\n");
	return NULL;
}

/*
arguments:
0 - name
1 - remote_host_name
2 - mode, 0=client, 1=server
*/
int main(int argc, char *argv[])
{

	pthread_t thread_id;

	if (argc < 3) {
		printf("Not enough arguments.\n");
		printf("Usage: %s remote_hostname mode\n", argv[0]);

		//pthread_create(&thread_id, NULL, &test, NULL);
		//pthread_join(thread_id, NULL);
		//printf("The thread has joined me, I am main\n");

		return 0;
	}

	int server_mode = atoi(argv[2]);
	char * remote_hostname = argv[1];
	int portno = PORT;

	if (server_mode) {
		//server_function(NULL);
		pthread_create(&thread_id, NULL, &server_function, NULL);
		pthread_join(thread_id, NULL);
	} else {
		//client_function((void *)remote_hostname);
		pthread_create(&thread_id, NULL, &client_function, (void *)remote_hostname);
		pthread_join(thread_id, NULL);
	}
	
	return 0;
}
