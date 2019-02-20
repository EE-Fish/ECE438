/*
** http_client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define RECVBUFSIZE 2048
#define REQBUFSIZE 1024
#define PATHSIZE 100

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
	int sockfd, numbytes;
	char recvbuf[RECVBUFSIZE] = {0};
	char reqbuf[REQBUFSIZE] = {0};
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	
	FILE *fp, *fd;
	char *temp;
	char path[PATHSIZE];


	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	temp = strtok(argv[1], ":");
	char protocol[strlen(temp) + 1];
	strcpy(protocol, temp);
	protocol[strlen(temp)] = '\0';
	
	temp = strtok(NULL, "/");
	char host[strlen(temp) + 1];
	strcpy(host,temp);
	host[strlen(temp)] = '\0';

	temp = strtok(NULL, "\0");
	memset(path, '\0', PATHSIZE);
	if(temp == NULL){
		sprintf(path, "/");	
	}
	else sprintf(path,"/%s", temp);
	
	temp = strtok(host, ":");
	temp = strtok(NULL, "\0");

	char port[3];
	if(temp == NULL){
		strcpy(port, "80");
		port[2] = '\0';		
	}
	else{
		strcpy(port, temp);
		port[strlen(temp)] = '\0';
	}

	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
	
	//HTTP GET construction
	memset(reqbuf, '\0', sizeof reqbuf);
	sprintf(reqbuf, "GET %s HTTP/1.1\r\n", path);
	sprintf(reqbuf + strlen(reqbuf), "User-Agent: Wget/1.12(linus-gnu)\r\n");
	sprintf(reqbuf + strlen(reqbuf), "Host: %s:%s\r\n", host, port);
	sprintf(reqbuf + strlen(reqbuf), "Connection: Keep-Alive\r\n\r\n");
	printf("%s\n", reqbuf);
	
	//Send HTTP requst
	send(sockfd, reqbuf, strlen(reqbuf), 0);	
	
	//Receive HTTP response message
	fp = fopen("temp_output","wb");
	memset(recvbuf, '\0', sizeof recvbuf);	
	while(1){
		numbytes = recv(sockfd, recvbuf, RECVBUFSIZE - 1, 0);
		if (numbytes == -1) {
	    		perror("recv");
	    		exit(1);
		}
		else if(numbytes == 0){
			break;
		}
		else {
			fwrite(recvbuf, sizeof(char), numbytes, fp);
			memset(recvbuf, '\0', sizeof recvbuf);		
		}
	}
	
	fclose(fp);
	
	fp = fopen("temp_output", "rb");
	fd = fopen("output", "wb");
	memset(reqbuf, '\0', sizeof reqbuf);
	int head = 0;
    do {
        	numbytes = fread(reqbuf, sizeof(char), REQBUFSIZE, fp);
        	if (!head) {
            		temp = strstr(reqbuf, "\r\n\r\n") + 4;
            		fwrite(temp, sizeof(char), numbytes - (temp - reqbuf), fd);
            		head = 1;
        	} 
		else {
            		fwrite(reqbuf, sizeof(char), numbytes, fd);
        	}
    	} while (numbytes != 0);
	
	fclose(fd);

    	fclose(fp);

    	remove("temp_output");
	
	close(sockfd);

	return 0;
}
