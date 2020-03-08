#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include<string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define MAX_PENDING 128
#define MAX_LENGTH 1024

char buffer[MAX_LENGTH];

void err_handler(char err[255]) {
	printf("Error: %s\n", err);
	exit(1);
}

struct Msg {
	int msg_type;
	int msg_length;
	char msg[MAX_LENGTH];
};

int create_tcp_socket(int port) {
	int server_socket;
	server_socket = socket(AF_INET , SOCK_STREAM , 0);

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;

	int status = bind(server_socket , (struct sockaddr *) &server_address , sizeof(server_address));
	if(status == -1) err_handler("Binding failed");
	return server_socket;
}


int create_udp_socket(int port) {
	int server_socket;
	server_socket = socket(AF_INET , SOCK_DGRAM, 0);

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;

	int status = bind(server_socket , (struct sockaddr *) &server_address , sizeof(server_address));
	if(status == -1) err_handler("Binding failed");
	return server_socket;
}



int main()
{
	int tcp_fd = create_tcp_socket(3002);
	int udp_fd = create_udp_socket(3003);

	while(listen(tcp_fd , MAX_PENDING) != -1) {
		struct sockaddr_in cli_addr;
		int len = sizeof(cli_addr);
		int cli_id = accept(tcp_fd, (struct sockaddr *) &cli_addr, &len);
		inet_ntop(AF_INET, &cli_addr.sin_addr, buffer, INET_ADDRSTRLEN);

		pid_t pid = fork();
		printf("%d\n", pid);
		if(pid == 0) {

			int rcvMsgSize;
			printf("connection from: %s, port: %d\n", buffer, ntohs(cli_addr.sin_port));

			if((rcvMsgSize = recv(cli_id, buffer, MAX_LENGTH, 0)) < 0) err_handler("Error in recv()");

			char repl[MAX_LENGTH];
			//TODO: create a function whick encode msg in string
			/*strcat(repl, "Type:1*");
			strcat(repl, "Msg Length:4*");
			strcat(repl, "Msg:3003");
			printf("%d\n", strlen(repl));*/

			if(send(cli_id, "3003", 4, 0) != 4) err_handler("Error in sending bytes");


			//Receive from client
			int udp_msg_length = 0, cli_udp_id;
			int flags = 0;
			int count = recvfrom(udp_fd, (char *)buffer, MAX_LENGTH, 0, (struct sockaddr *) &cli_addr, &len);
			printf("%d\n", count);
			int port = atoi(buffer);

			printf("%d\n", port);

			struct sockaddr_in addrport;
			addrport.sin_family = AF_INET;
			addrport.sin_port = htons(port);
			addrport.sin_addr.s_addr = htonl(INADDR_ANY);

			sendto(udp_fd, "Hello", 5, 0, (struct sockaddr *) &addrport, sizeof(addrport));
			printf("Hello message sent.\n");

			exit(EXIT_SUCCESS);
		}

	}


	return 0;
}
