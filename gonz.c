#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_CLIENTS 2
#define BUFFER_SIZE 42
#define ERR_ARGS "Wrong number of arguments\n"
#define ERR_SYS "Fatal error\n"
#define LOG_CONNECT "server: client %d just arrived\n"
#define LOG_DISCONNECT "server: client %d just left\n"
#define LOOP for (int i = 0; i < MAX_CLIENTS; i++)
#define UNUSED -1

typedef struct s_client {
	int	id;
	int	fd;
	char*	buffer;
} t_client;

void	error(char* msg) {
	write(STDERR_FILENO, msg, strlen(msg));
	exit(1);
}

void	broadcast(t_client* clients, char* msg, int id) {
	LOOP {
		if (clients[i].fd > UNUSED && clients[i].id != id)
			send(clients[i].fd, msg, strlen(msg), 0);
	}
}

void	initClient(t_client* client) {
	client->id = UNUSED;
	client->fd = UNUSED;
	client->buffer = NULL;
}

int extract_message(char **buf, char **msg)
{
	char	*newbuf = NULL;
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

int	last_id = -1;

int main(int argc, char** argv) {
	if (argc != 2)
		error(ERR_ARGS);

	int	server_fd;
	char	buffer[BUFFER_SIZE + 1];
	t_client	clients[MAX_CLIENTS];
	LOOP { initClient(&clients[i]); }

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		error(ERR_SYS);
	
	fd_set	read_fds;

	struct sockaddr_in	server_addr;
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	server_addr.sin_port = htons(atoi(argv[1])); 
  
	if ((bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr))) != 0)
		error(ERR_SYS) ;
	if (listen(server_fd, MAX_CLIENTS) != 0)
		error(ERR_SYS);






	while (42) {
		FD_ZERO(&read_fds);
		FD_SET(server_fd, &read_fds);
		int	max_fd = server_fd;

		LOOP {
			FD_SET(clients[i].fd, &read_fds);
			if (clients[i].fd > max_fd)
				max_fd = clients[i].fd;
		}

		if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
			error(ERR_SYS);

		if (FD_ISSET(server_fd, &read_fds)) {
			int	new_fd = accept(server_fd, NULL, NULL);
			if (new_fd < 0)
				error(ERR_SYS);

			LOOP {
				if (clients[i].fd < 0) {
					clients[i].id = last_id + 1; last_id++;
					clients[i].fd = new_fd;
					memset(buffer, 0, BUFFER_SIZE);
					sprintf(buffer, LOG_CONNECT, clients[i].id);
					broadcast(clients, buffer, clients[i].id);
					break ;
				}
			}
		}

		LOOP {
			if (FD_ISSET(clients[i].fd, &read_fds)) {
				memset(buffer, 0, BUFFER_SIZE);
				int	n = recv(clients[i].fd, buffer, BUFFER_SIZE, 0);
				if (n < 0)
					error(ERR_SYS);
				else if (n == 0) {
					sprintf(buffer, LOG_DISCONNECT, clients[i].id);
					broadcast(clients, buffer, clients[i].id);
					close(clients[i].fd);
					if (clients[i].buffer)
						free(clients[i].buffer);
					initClient(&clients[i]);
				}
				else {
					buffer[n] = 0;
					if (clients[i].buffer)
						clients[i].buffer = str_join(clients[i].buffer, buffer);
					else {
						clients[i].buffer = malloc((strlen(buffer) + 1 ) * sizeof(char));
						strcpy(clients[i].buffer, buffer);
					}
				}	
			}
			while (clients[i].buffer && strstr(clients[i].buffer, "\n")) {
				char*	msg = NULL;
				extract_message(&clients[i].buffer, &msg);
				char	output[strlen(msg) + 16];
				sprintf(output, "client %d: %s", clients[i].id, msg);
				broadcast(clients, output, clients[i].id);
				free(msg);
			}
		}
	}
	close(server_fd);
	return 0;
}