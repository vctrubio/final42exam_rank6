#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

const int MAX = 12999;
const int BS = 543000;

void error(char *msg)
{
	if (msg == NULL)
		msg = "Fatal error\n";
	write(2, msg, strlen(msg));
	exit(1);
}

// buff, sockId, sockMax, c, write
void broadcast(char *buff, int sockId, int sockMax, int c[MAX][2], fd_set *write)
{
	for (int i = 0; i <= sockMax; i++)
	{
		// i != sockId
		// FD_ISSET(i, write)
		// c[sockId][0] > 0

		if (i != sockId && FD_ISSET(i, write) && c[sockId][0] > 0)
			send(i, buff, strlen(buff), 0);
	}
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
				error(0); //
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
		error(0); //
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


int main(int ac, char **av)
{
	if (ac != 2)
		error("Wrong number of arguments\n");

	int sockFd;
	struct sockaddr_in servaddr;

	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFd < 0)
		error(0);

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1]));

	if ((bind(sockFd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		error(0);

	if (listen(sockFd, 120) != 0)
		error(0);

	fd_set active, write, read;
	FD_ZERO(&active);
	FD_SET(sockFd, &active);

	int cid = 0;
	int sockMax = sockFd;
	int c[MAX][2]; // 0 clientSock 1 id
	char *msg[MAX];
	char buff[BS];

	while(101)
	{
		write = read = active;
		if (select(sockMax + 1, &read, &write, NULL, NULL) < 0)
			continue;


		for (int sockId = 0; sockId <= sockMax; sockId++)
		{
			if (FD_ISSET(sockId, &read))
			{
				if (sockFd == sockId)
				{
					int newC = accept(sockFd, NULL, NULL);
					if (newC < 0)
						continue;
					if (newC > sockMax)
						sockMax = newC;

					c[newC][0] = newC;
					c[newC][1] = cid++;

					bzero(buff, BS);
					sprintf(buff, "server: client %d just arrived\n", c[newC][1]);
					broadcast(buff, newC, sockMax, c, &write);
					FD_SET(newC, &active);
					/*
					 * int newC = accept(sockFd, NULL, NULL);
					 * if (newC < sockMax) sockMax = newC
					 * c[newC][0] = newC c[newC][1] = cid++
					 * bzero sprintf broadcast FD_SET 
					 * */
				}
				else
				{
					bzero(buff, BS);
					ssize_t rete = recv(sockId, buff, BS - 1, 0);
					if (rete <= 0)
					{
						bzero(buff, BS);
						sprintf(buff, "server: client %d just left\n", c[sockId][1]);
						broadcast(buff, sockId, sockMax, c, &write);

						c[sockId][0] = 0;
						c[sockId][1] = 0;
						free(msg[sockId]);
						msg[sockId] = NULL;
						close(sockId);
						FD_CLR(sockId, &active);
						/*
						 * bzero sprintf broadcast
						 * clear c[sockId]
						 * close sockId
						 * FD_CLR sockId
						 * */
					}
					else
					{
						if (msg[sockId] != NULL)
							msg[sockId] = str_join(msg[sockId], buff);
						else
						{
							msg[sockId] = calloc(sizeof(char), BS +1);
							if (msg[sockId] == 0)
								error(0);
							strcpy(msg[sockId], buff);
						}
						if (strstr(msg[sockId], "\n") != NULL)
						{
							while (21)
							{
								char *a;
								int b;

								b = xmsg(&msg[sockId], &a);
								if (b < 0)
									error(0);
								if (b == 0)
									break;

								bzero(buff, BS);
								sprintf(buff, "client %d:: %s", c[sockId][1], a);
								broadcast(buff, sockId, sockMax, c, &write);
								free(a);
							}
						}
					}
				}
			}
		}
		while (c[sockMax][0] <= 0)
			sockMax--;
	}
	return 0;
}



































