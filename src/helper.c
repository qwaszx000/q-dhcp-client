#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include"./defines.h"
#include"./structures.h"

int create_dhcp_socket(ARGS_OPTIONS *options)
{
	struct sockaddr_in client_bind;
	int s;
	int tmp;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == s){
		perror("Socket creation error");
		exit(-1);
	}

	tmp = 1;
	tmp = setsockopt(s,
		SOL_SOCKET, SO_BROADCAST, &tmp, sizeof(tmp));
	if(-1 == tmp){
		perror("setsockopt broadcast error");
		close(s);
		exit(-1);
	}

	tmp = setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE,
		options->interface_name, strlen(options->interface_name));
	if(-1 == tmp){
		perror("setsockopt bindtodevice error");
		close(s);
		exit(-1);
	}

	client_bind.sin_family = AF_INET;
	client_bind.sin_port = htons(CLIENT_UDP_PORT);
	client_bind.sin_addr.s_addr = htonl(INADDR_ANY); 
	tmp = bind(s, (struct sockaddr*)&client_bind, sizeof(client_bind));
	if(-1 == tmp){
		perror("bind error");
		close(s);
		exit(-1);
	}
	return s;
}
