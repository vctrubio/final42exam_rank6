#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <sys/select.h>

#define ERRORA "Wrong number of arguments\n"
#define ERROR "Fatal error\n"
#define BUFFER_SIZE 40
#define M1 "client "
#define M2 ": "

#define LOGIN "server: client %d just arrived\n"
#define LOGOT "server: client %d just left\n"
#define MAX FD_SETSIZE


/*
 * hndMsg
 * error
 * broadcast
 * getClient
 *
 */

typedef struct Client
{
	int id;
	int fd;
	char *buff;
} Client;


void error(char *msg)
{
	write(2, msg, strlen(msg));
	exit(1);
}

void broadcast(char *msg, Client *c, int igSock)
{
	for (int i = 0; i < MAX; i++)
	{
		if (c[i].fd >= 0 && c[i].fd != igSock)
			send(c[i].fd, msg, strlen(msg), 0);
	}
}

Client *getClient(Client *c, int sock)
{

	for (int i = 0; i < MAX; i++)
	{
		if (c[i].fd == sock)
			return &c[i];
	}
	return NULL;
}


int xmsg(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add, int freeB)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	if (freeB)
		free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


void hndMsg(Client *c, Client *ptr)
{
	char *msg = NULL;
	if (ptr->buff == NULL)
		return;

	int ret = xmsg(&ptr->buff, &msg);
	if (ret < 0)
		error(ERROR);
	char id[10];
	sprintf(id, "%d", ptr->id);
	while (ret == 1)
	{
		char *fmsg = str_join(M1, id, 0);
		fmsg = str_join(fmsg, M2, 1);
		fmsg = str_join(fmsg, msg, 1);

		broadcast(fmsg, c, ptr->fd);
		free(fmsg);
		free(msg);
		ret = xmsg(&ptr->buff, &msg);
		if (ret < 0)
			error(ERROR);

	}

	if (ptr->buff && strlen(ptr->buff) == 0)
	{
		free(ptr->buff);
		ptr->buff = NULL;
	}
}

int main() {
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(8081); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n");
	if (listen(sockfd, 10) != 0) {
		printf("cannot listen\n"); 
		exit(0); 
	}
	printf("fdoud love.\n");
}
