#define _POSIX_C_SOURCE 199309

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define MAXLINE 4096

int sockfd;
struct sigaction act;


void signalHandler(int signum, siginfo_t *info, void *ptr)
{
	close(sockfd);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr;
	char sendline[MAXLINE], recvline[MAXLINE];

	memset(&act, 0, sizeof(act));
	act.sa_sigaction = signalHandler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &act, NULL );
	sigaction(SIGQUIT, &act, NULL );
	sigaction(SIGHUP, &act, NULL );

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
	servaddr.sin_port = htons(atoi(argv[2])); //convert to big-endian order

	//Connection of the client to the socket
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror("Problem in connecting to the server");
		exit(EXIT_FAILURE);
	}

	while(1){
		fscanf(stdin, "%s", sendline);
		send(sockfd, sendline, strlen(sendline), 0);
		if (recv(sockfd, recvline, MAXLINE, 0) == 0) {
			//error: server terminated prematurely
			fprintf(stderr, "The server terminated prematurely.\n");
			close(sockfd);
			exit(EXIT_FAILURE);
		}
		fprintf(stdout, "%s", "String received from the server: ");
		fputs(recvline, stdout);
		fputs("\n", stdout);
	}

	close(sockfd);

	exit(EXIT_SUCCESS);
}
