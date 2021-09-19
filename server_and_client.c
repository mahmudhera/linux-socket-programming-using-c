#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <ucontext.h>

#define PORT 8080

ucontext_t uctx_func, uctx_server, uctx_client, uctx_main;
char * remote_hostname;

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
void server_function(void)
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

	int x;
	ucontext_t uctx_func_received;

	// read uctx from client
	n = read(newsockfd, &uctx_func_received, sizeof(uctx_func_received));
	// TODO: what to do with this context now??
	
	if (n < 0) error("ERROR reading from socket");
	printf("Here is the message: BLANK\n");

	// write to the client
	n = write(newsockfd,"I got your message",18);
	if (n < 0) error("ERROR writing to socket");

	// close the sockets     
	close(newsockfd);
	close(sockfd);

	// TODO: prepare the context now in uctx_func
	// currently, it just starts the func altogether from the top	
	swapcontext(&uctx_server, &uctx_func);

}


void client_function(void)
{
	char * server_address = remote_hostname;
	puts(server_address);
	int sockfd, n, portno = PORT;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];

	printf("Client has been invoked\n");

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
	//printf("Please enter the message: ");
	//bzero(buffer,256);
	//fgets(buffer,255,stdin);
	int x = 12345;

	// TODO: what to do with uctx_func??
	// write the context to the server
	//n = write(sockfd, &x, sizeof(x));
	n = write(sockfd, &uctx_func, sizeof(uctx_func));
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
	swapcontext(&uctx_client, &uctx_main);

	//return NULL;
}


void test (void)
{
	printf("This is from a thread\n");

	swapcontext(&uctx_func, &uctx_client);

	printf("This is also from a thread\n");
}


void init (void)
{
	char func_stack[16384], server_stack[16384], client_stack[16384];
	
	if (getcontext(&uctx_client) == -1)
		error("getcontext");
	uctx_client.uc_stack.ss_sp = client_stack;
	uctx_client.uc_stack.ss_size = sizeof(client_stack);
	uctx_client.uc_link = &uctx_main;
	makecontext(&uctx_client, client_function, 0);

	if (getcontext(&uctx_func) == -1)
     		error("getcontext");	
	uctx_func.uc_stack.ss_sp = func_stack;
	uctx_func.uc_stack.ss_size = sizeof(func_stack);
	uctx_func.uc_link = &uctx_main;
	makecontext(&uctx_func, test, 0);

	if (getcontext(&uctx_server) == -1)
		error("getcontext");
	uctx_server.uc_stack.ss_sp = server_stack;
	uctx_server.uc_stack.ss_size = sizeof(server_stack);
	uctx_server.uc_link = &uctx_main;
	makecontext(&uctx_server, server_function, 0);

	printf("Init complete\n");
}


/*
arguments:
0 - name
1 - remote_host_name
2 - mode, 0=client, 1=server
*/
int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("Not enough arguments.\n");
		printf("Usage: %s remote_hostname mode\n", argv[0]);
	}

	int client = 1;
	int server_mode = atoi(argv[2]);
	remote_hostname = argv[1];
	int portno = PORT;

	init();
	if (server_mode) {
		swapcontext(&uctx_main, &uctx_server);
	} else {
		swapcontext(&uctx_main, &uctx_func);
	}
	
	return 0;
}
