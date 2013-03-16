#define _POSIX_C_SOURCE 199309

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "compute.h"


int main(int argc, char *argv[])
{

	long iops;
	char sendline[MAXLINE], recvline[MAXLINE];

	/* Connect to server */
	sockfd = initClient(argv[1], atoi(argv[2]));

	/* Benchmark system */
	iops = getIOPS();

	/*format data */
	sprintf(sendline, "i%ld", iops);
	fprintf(stdout, "IOPS: %s\n", sendline);

	/* send IOPS */
	send(sockfd, sendline, strlen(sendline)-1, 0);

	if (recv(sockfd, recvline, MAXLINE, 0) == 0) {
		//error: server terminated prematurely
		fprintf(stderr, "The server terminated prematurely.\n");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "Range received: %s\n", recvline);

	close(sockfd);
	sleep(1);
	exit(EXIT_SUCCESS);
}

void signalHandler(int signum, siginfo_t *info, void *ptr)
{

	close(sockfd);
	exit(EXIT_SUCCESS);
}

int initClient(char *ipaddr, int port)
{
	struct sockaddr_in servaddr;
	int mysocket;

	if((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Problem in creating the socket");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr= inet_addr(ipaddr);
	servaddr.sin_port =  htons(port);

	if(connect(mysocket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Problem in connecting to the server");
		close(mysocket);
		exit(EXIT_FAILURE);
	}

	return mysocket;
}

long getIOPS(void)
{

	long i, j, k, lim;
	double result;

	lim = 100000000;

	clock_t begin, end;

	j = 10;

	begin = clock();
	for(i = 0; i < lim; i++){
		k = i % j;
	}
	end = clock();

	result = (double) (end - begin)/CLOCKS_PER_SEC;
	k = (long)lim / result;

	return k;
}
