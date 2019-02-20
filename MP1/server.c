/*
** http_server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10	 // how many pending connections queue will hold
#define BUFFERSIZE 500
#define PACKAGESIZE 2048

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, new_fd, numbytes;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	
	char package[PACKAGESIZE];
	char buf[BUFFERSIZE];
	char *i, *j;
	FILE *fp;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if (argc != 2) {
	    fprintf(stderr,"usage: port\n");
	    exit(1);
	}

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");
	
	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);


		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
		
			numbytes = recv(new_fd, buf, BUFFERSIZE - 1, 0);
			buf[numbytes] = '\0';
			
			i = strstr(buf,"GET /");
			if((i == NULL) || (i != buf)){
				memset(buf, '\0', BUFFERSIZE);
				sprintf(buf, "HTTP/1.1 400 Bad Request\r\n\r\n");
				numbytes = send(new_fd, buf, strlen(buf), 0);
				if(numbytes == -1){
					perror("send");
					exit(1);
				}
				exit(1);
			}
			
			i = i + 5;
			j = strstr(i, "HTTP/") - 1;

			if(strncmp(j, " HTTP/", 6) != 0){
				memset(buf, '\0', BUFFERSIZE);
				sprintf(buf, "HTTP/1.1 400 Bad Request\r\n\r\n");
				numbytes = send(new_fd, buf, strlen(buf), 0);
				if(numbytes == -1){
					perror("send");
					exit(1);
				}
				exit(1);
			}
			
			// Get file path
			char path_to_file[j - i + 1];
			sprintf(path_to_file, "%.*s", (int)(j - i), i);
			path_to_file[j - i] = '\0';
			
			if(strncmp(i + strlen(i) - 4, "\r\n\r\n", 4) != 0){
				memset(buf, '\0', BUFFERSIZE);
				sprintf(buf, "HTTP/1.1 400 Bad Request\r\n\r\n");
				numbytes = send(new_fd, buf, strlen(buf), 0);
				if(numbytes == -1){
					perror("send");
					exit(1);
				}
				exit(1);				
			}
			
			fp = fopen(path_to_file, "rb");
			memset(buf, '\0', BUFFERSIZE);	
			if(fp == NULL){
				sprintf(buf, "HTTP/1.1 404 Whoops, file not found!\r\n\r\n");
				numbytes = send(new_fd, buf, strlen(buf), 0);
				if(numbytes == -1){
					perror("send");
					exit(1);
				}
				perror("fopen");
				exit(1);			
			}

			memset(package, '\0', PACKAGESIZE);
			sprintf(package, "HTTP/1.1 200 OK!\r\n\r\n");
			numbytes = fread(package + strlen(package), sizeof(char), PACKAGESIZE - strlen(package)-1, fp);
			numbytes = send(new_fd, package, strlen(package), 0);
			
			if(numbytes == -1){
				perror("send");
				exit(1);
			}
			
			memset(package, '\0', PACKAGESIZE);
			while((numbytes = fread(package, sizeof(char), PACKAGESIZE-1, fp)) != 0){
				numbytes = send(new_fd, package, strlen(package), 0);
				if(numbytes == -1){
					perror("send");
					exit(1);
				}
				memset(package, '\0', PACKAGESIZE);
			}			
			fclose(fp);
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
