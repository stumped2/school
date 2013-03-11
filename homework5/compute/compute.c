#define _POSIX_C_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 9091 /*port*/
#define LIM 100000000

int main(int argc, char **argv) {

	int sockfd;
	struct sockaddr_in servaddr;
	char sendline[MAXLINE];

	//basic check of the arguments
	//additional checks can be inserted
	if (argc != 2) {
		perror("Usage: TCPClient <IP address of the server");
		exit(EXIT_FAILURE);
	}

	//Create a socket for the client
	//If sockfd<0 there was an error in the creation of the socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Problem in creating the socket");
		exit(EXIT_FAILURE);
	}

	//Creation of the socket
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(SERV_PORT); //convert to big-endian order

	//Connection of the client to the socket
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror("Problem in connecting to the server");
		exit(EXIT_FAILURE);
	}


	/*for(;;){
		fscanf(stdin, "%s", sendline);
		send(sockfd, sendline, strlen(sendline)+1, 0);
	}*/

	while (fgets(sendline, MAXLINE, stdin) != NULL ) {

		//sendline[strlen(sendline) - 1] = '\0';
		send(sockfd, sendline, strlen(sendline)+1, 0);

		/*if (recv(sockfd, recvline, MAXLINE, 0) == 0) {
			//error: server terminated prematurely
			fprintf(stderr, "The server terminated prematurely.\n");
			exit(EXIT_FAILURE);
		}*/
		//fprintf(stdout, "%s", "String received from the server: ");
		//fputs(recvline, stdout);
	}

	exit(EXIT_SUCCESS);
}
