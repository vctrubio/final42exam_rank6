#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

const int MAX_CLIENT = 13000;
const int BUFFER_SIZE = 500000;

void	MsgError(char* msg)
{
	if (msg == NULL)
		msg = "Fatal error\n";
	write(2, msg, strlen(msg));
	exit(1);
}

void	broadcast(char* buffer, int ptrF, int max_socket, int client[MAX_CLIENT][2], fd_set* write)
{
	for (int i = 0; i <= max_socket; i++)
	{
		if (i != ptrF && FD_ISSET(i, write) && client[ptrF][0] > 0)
			send(i, buffer, strlen(buffer), 0); 
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
				MsgError(0);
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
		MsgError(0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


int main(int argc, char** argv) 
{
	if (argc != 2)
		MsgError("Wrong number of arguments");
	int sockFd;
	struct sockaddr_in servaddr; 

	// socket create and verification 
	if (sockFd = socket(AF_INET, SOCK_STREAM, 0) < 0)
		MsgError(0);

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockFd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		MsgError(0);
	if (listen(sockFd, 120) != 0)
		MsgError(0);

	fd_set fd, write, read;
	FD_ZERO(&fd);
	FD_SET(sockFd, &fd);
	
	int id = 0;
	int max_socket = sockFd;
	int client[MAX_CLIENT][2];
	char *msgs[MAX_CLIENT];
	char buffer[BUFFER_SIZE];
	
	while(42)
	{
		write = fd;
		read = fd;
		if (select(max_socket + 1, &read, &write, NULL, NULL) < 0)
			continue;

		for(int ptrF = 0; ptrF <= max_socket; ptrF++)
		{
			if (FD_ISSET(ptrF, &read))
			{
				if (sockFd == ptrF)
				{
					int newC = accept(sockFd, NULL, NULL);
					if (newC < 0)
						continue;
					if (newC > max_socket)
						max_socket = newC;

					client[newC][0] = newC;
					client[newC][1] = id++;

					bzero(buffer, BUFFER_SIZE);
					sprintf(buffer, "server: client %d just arrived\n", client[newC][1]);
					broadcast(buffer, newC, max_socket, client, &write);
					FD_SET(newC, &fd);
				}
				else
				{
					bzero(buffer, BUFFER_SIZE);
					ssize_t bytes_rec = recv(ptrF, buffer, BUFFER_SIZE - 1, 0);

					if (bytes_rec <= 0)
					{
						bzero(buffer, BUFFER_SIZE);
						sprintf(buffer, "server: client %d just left\n", client[ptrF][1]);
						broadcast(buffer, ptrF, max_socket, client, &write);
						client[ptrF][0] = 0;
						client[ptrF][1] = 0;
						free(msgs[ptrF]);
						msgs[ptrF] = NULL;
						close(ptrF);
						FD_CLR(ptrF, &fd);
					}
					else
					{
						if (msgs[ptrF] != NULL)
							msgs[ptrF] = str_join(msgs[ptrF], buffer);
						else
						{
							msgs[ptrF] = calloc(sizeof(char), BUFFER_SIZE + 1);
							if (msgs[ptrF] == 0)
								MsgError(0);
							strcpy(msgs[ptrF], buffer);
						}

						if (strstr(msgs[ptrF], "\n") != NULL)
						{
							while(42)
							{
								char* msg;
								int ret;
	
								ret = xmsg(&msgs[ptrF], &msg);
								if (ret < 0)
									MsgError(0);
								if (ret == 0)
									break;
								
								bzero(buffer, BUFFER_SIZE);
								sprintf(buffer, "client %d: %s", client[ptrF][1], msg);
								broadcast(buffer, ptrF, max_socket, client, &write);
								free(msg);
							}
						}
					}
				}
			}
		}		
		while (client[max_socket][0] <= 0)
			max_socket--;
	}
}







