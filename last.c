#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>


#define LOGIN "server: client %d just arrived\n"
#define LOGOT "server: client %d just left\n"
#define ERROR "Fatal error\n"
#define MAX 2
#define LOOP for (int i = 0; i < MAX; i++)
#define BUFFER_SIZE 500000

int cid = -1;

typedef struct Client {
	int id;
	int fd;
	char *buff;
}	Client;

void error(char *msg)
{
	write(2, msg, strlen(msg));
	exit(1);
}

void broadcast(char *msg, Client *c, int id)
{
	LOOP {
		if (c[i].fd >= 0 && c[i].id != id)
			send(c[i].fd, msg, strlen(msg), 0);
	}
}

void fire(Client *c)
{
	c->fd = -1;
	c->id = -1;
	c->buff = NULL;
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

char *str_join(char *buf, char *add)
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
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


int main(int ac, char **av) {
	if (ac != 2)
		error("Wrong number of arguments\n");
	int sockFd;
	struct sockaddr_in servaddr;

	// socket create and verification 
	sockFd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockFd == -1) { 
		error(ERROR);
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1]));
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockFd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		error(ERROR);
	} 
	else
		printf("Socket successfully binded..\n");
	if (listen(sockFd, 10) != 0) {
		error(ERROR);
	}
	

	char buff[BUFFER_SIZE +1];
	Client c[MAX];
	LOOP { fire(&c[i]); }

	fd_set fd;

	while (42)
	{
		FD_ZERO(&fd);
		FD_SET(sockFd, &fd);
		int maxFd = sockFd;

		LOOP {
			FD_SET(c[i].fd, &fd);
			if (c[i].fd > maxFd)
				maxFd = c[i].fd;
		}

		if (select(maxFd +1, &fd, NULL, NULL, NULL) < 0)
			error(ERROR);

		//if sockFd is set to fd, do LOGIN
		if (FD_ISSET(sockFd, &fd))
		{
			int newC = accept(sockFd, NULL, NULL);
			if (newC < 0)
				error(ERROR);
			
			LOOP {
				if (c[i].fd < 0) {
					c[i].id = ++cid;
					c[i].fd = newC;
					memset(buff, 0, BUFFER_SIZE);
					sprintf(buff, LOGIN, c[i].id);
					broadcast(buff, c, c[i].id);
					break;
				}
			}
		}
		//nextup
		LOOP {
			if (FD_ISSET(c[i].fd, &fd))
			{
				memset(buff, 0, BUFFER_SIZE);
				int rb = recv(c[i].fd, buff, BUFFER_SIZE, 0);
				if (rb < 0)
					error(ERROR);
				else if (rb == 0)
				{
					sprintf(buff, LOGOT, c[i].id);
					broadcast(buff, c, c[i].id);
					close(c[i].fd);
					if (c[i].buff)
						free(c[i].buff);
					fire(&c[i]);
				}
				else{
					buff[rb] = 0;
					if (c[i].buff)
						c[i].buff = str_join(c[i].buff, buff);
					else
					{
						c[i].buff = malloc((strlen(buff) + 1) * sizeof(char));
						strcpy(c[i].buff, buff);
				}

			}
			while (c[i].buff && strstr(c[i].buff, "\n"))
			{
				char *msg = NULL;
				xmsg(&c[i].buff, &msg);
				char output[strlen(msg) + 16];
				sprintf(output, "server: client %d %s", c[i].id, msg);
				broadcast(output, c, c[i].id);
				free(msg);
			}
			}
		}
	}
	close(sockFd);
	return 0;
}

{

	int maxFd = sockFd;
	FD_ZERO(&fd);
	FD_SET(sockFd, &fd);

	LOOP {
		FD_SET(c[i].fd, &fd);
		if (c[i].fd < maxFd)
			maxFd = c[i].fd;
	}

	if (select(maxFd + 1, &fd, NULL, NULL, NULL) < 0)
		error(ERROR);

	if (FD_ISSET(sockFd, &fd))
	{
		int newC = accept(sockFd, NULL, NULL);
		if (newC < 0)
			error(ERROR);
		LOOP {
			if (c[i].fd < 0)
			{
				c[i].fd = newC;
				c[i].id = ++cid;
				memset(buff, 0, BS);
				sprintf(buff, LOGIN, c[i].id);
				broadcast(buff, c, c[i].id);
				break;
			}
		}
	}

	LOOP {
		if (FD_ISSET(c[i].fd, &fd))
		{
			memset(buff, 0, BS);
			int rb = recv(c[i].fd, buff, BS, 0);
			if (rb < 0)
				error(ERROR);
			if (rb == 0)
			{
				sprintf(buff, LOGOT, c[i].id);
				broadcast(buff, c, c[i].id);
				close(c[i].fd);
				if (c[i].buff)
					free(c[i].buff);
				fire(&c[i]);
			}
			else{
				buff[rb] = 0;
				if (c[i].buff)
					c[i].buff = str_join(c[i].buff, buff);
				else
				{
					c[i].buff = malloc((strlen(buff) + 1) * sizeof(char));
					strcpy(c[i].buff, buff);
				}
			}






		}
		while(123)
		{

		}	
	}


	close(sockFd);
	return 0;

}









char *msg = NULL;
int ret = xmsg(&c[i].buff, &msg);
char out[strlen(msg) + 16];
sprintf(out, "client: %d: %s", c[i].id, msg);
broadcast(out, c, c[i].id);
free(msg);
