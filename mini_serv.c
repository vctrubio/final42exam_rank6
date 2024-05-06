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

void	MsgToClients(char* buffer, int socket_id, int max_socket, int data_client[MAX_CLIENT][2], fd_set* write)
{
	for (int i = 0; i <= max_socket; i++)
	{
		if (i != socket_id && FD_ISSET(i, write) && data_client[socket_id][0] > 0)
			send(i, buffer, strlen(buffer), 0); 
	}
}


int extract_message(char **buf, char **msg)
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
	int server_socket;
	struct sockaddr_in servaddr; 

	// socket create and verification 
	server_socket = socket(AF_INET, SOCK_STREAM, 0); 
	if (server_socket == -1)
		MsgError(0);

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(server_socket, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		MsgError(0);
	if (listen(server_socket, 120) != 0)
		MsgError(0);

	fd_set active, write, read;
	FD_ZERO(&active);
	FD_SET(server_socket, &active);
	
	int client_id = 0;
	int max_socket = server_socket;
	int data_client[MAX_CLIENT][2];
	char *msgs[MAX_CLIENT];
	char buffer[BUFFER_SIZE];
	
	while(42)
	{
		write = active;
		read = active;
		if (select(max_socket + 1, &read, &write, NULL, NULL) < 0)
			continue;


		for(int socket_id = 0; socket_id <= max_socket; socket_id++)
		{
			if (FD_ISSET(socket_id, &read))
			{

				if (server_socket == socket_id)
				{
					int new_client = accept(server_socket, NULL, NULL);
					if (new_client < 0)
						continue;
					if (new_client > max_socket)
						max_socket = new_client;



					data_client[new_client][0] = new_client;
					data_client[new_client][1] = client_id++;
					bzero(buffer, BUFFER_SIZE);
					sprintf(buffer, "server: client %d just arrived\n", data_client[new_client][1]);
					MsgToClients(buffer, new_client, max_socket, data_client, &write);
					FD_SET(new_client, &active);



				}
				else
				{

					bzero(buffer, BUFFER_SIZE);
					ssize_t bytes_rec = recv(socket_id, buffer, BUFFER_SIZE - 1,0);
					if (bytes_rec <= 0)
					{
						bzero(buffer, BUFFER_SIZE);
						sprintf(buffer, "server: client %d just left\n", data_client[socket_id][1]);
						MsgToClients(buffer, socket_id, max_socket, data_client, &write);

						data_client[socket_id][0] = 0;
						data_client[socket_id][1] = 0;
						free(msgs[socket_id]);
						msgs[socket_id] = NULL;
						close(socket_id);
						FD_CLR(socket_id, &active);
					}




					else
					{
						if (msgs[socket_id] != NULL)
							msgs[socket_id] = str_join(msgs[socket_id], buffer);
						else
						{
							msgs[socket_id] = calloc(sizeof(char), BUFFER_SIZE + 1);
							if (msgs[socket_id] == 0)
								MsgError(0);
							strcpy(msgs[socket_id], buffer);
						}


						if (strstr(msgs[socket_id], "\n") != NULL)
						{
							while(42)
							{
								char* msg_aux;
								int em_status;
								em_status = extract_message(&msgs[socket_id], &msg_aux);
								if (em_status < 0)
									MsgError(0);
								if (em_status == 0)
									break;



								bzero(buffer, BUFFER_SIZE);
								sprintf(buffer, "client %d: %s", data_client[socket_id][1], msg_aux);
								MsgToClients(buffer, socket_id, max_socket, data_client, &write);
								free(msg_aux);
							}
						}
					}
				}
			}
		}		
		while (data_client[max_socket][0] <= 0)
			max_socket--;
	}
}







