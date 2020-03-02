#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

int main()
{ 
	// Socket Setup Stage
	int client_socket;
	client_socket = socket(AF_INET , SOCK_STREAM , 0);

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	server_address.sin_addr.s_addr = INADDR_ANY;

	int connection_status = connect(client_socket , (struct sockaddr *) &server_address , sizeof(server_address));

	if (connection_status == -1){
		printf("Error connecting to the server \n\n");
	}
	else{
		char response[256];
		recv(client_socket , &response , sizeof(response) , 0);

		printf("Data recieved : %s\n", response);
	}

	close(client_socket);

	return 0;
}