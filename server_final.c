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

int free_udp_port = 3003;

// ERROR HANDLING: Printing out the error
void err_handler(char err[255]) {
	printf("Error: %s\n", err);
	exit(1);
}

// C substring function definition
void substring(char s[], char sub[], int p, int l) 
{
   	int c = 0;

    while (p+c <= l) 
    {
      	sub[c] = s[p+c];
      	c++;
		if(s[p+c] == '\0') break;
    }
   	sub[c] = '\0';
}

/* returns the position of n th delimiter 
(if which_del is 1, function returns first del's position and so on) */
int pos_of_del(char *str, int which_del, char del) 
{
	int i = 0;

	while(str[i] != '\0') 
	{
		if(which_del == 0) return i-1;
		if(str[i] == del) which_del--;
		i++;
	}
}

// Converts a number into a sring
char *itoa (int num, char *str)
{
        if(str == NULL) return NULL;
        sprintf(str, "%d", num);
        return str;
}

// Encodes the string to the apt message format
void encode_message(int t, char * message, char * result) 
{
	// | is delimiter
	char temp[4] = "";

	strcat(result, "Type:");
	strcat(result, (char *)itoa(t, temp));
	strcat(result, "|");
	
	int len = strlen(message);
	char temp2[4] = "";

	strcat(result, "Length:"); 
	strcat(result, (char *)itoa(len, temp2));
	strcat(result, "|");

	strcat(result, "Message:"); strcat(result, message);

	return;
}

void decode_message(char *str, int *type, int *length, char *msg) 
{
	int i=0;

	// The type is in between first : and first | 
	int first_del = pos_of_del(str, 1, ':'), sec_del = pos_of_del(str, 1, '|');
	char type_str[128]="";
	substring(str, type_str, first_del+1, sec_del-1);
	*type = atoi(type_str);


	// The length is in between second : and second | 
	char length_str[128]="";
	first_del = pos_of_del(str, 2, ':'); sec_del=pos_of_del(str, 2, '|');
	substring(str, length_str, first_del+1, sec_del-1);
	*length = atoi(length_str);

	// The message is after the third : 
	int msg_start = pos_of_del(str, 3, ':');
	char msg_str[1024]="";
	substring(str, msg_str, msg_start+1, 1025);
	strcat(msg, msg_str);

	return;
}

//Creating the TCP socket
int create_tcp_socket(int port) {
	int server_socket;
	server_socket = socket(AF_INET , SOCK_STREAM , 0);

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;

	int status = bind(server_socket , (struct sockaddr *) &server_address , sizeof(server_address));
	if(status == -1) err_handler("TCP Binding failed");
	return server_socket;
}

//Creating the UDP socket
int create_udp_socket(int port) {
	int server_socket;
	server_socket = socket(AF_INET , SOCK_DGRAM, 0);

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;

	int status = bind(server_socket , (struct sockaddr *) &server_address , sizeof(server_address));
	if(status == -1) err_handler("UDP Binding failed");
	return server_socket;
}



int main( int argc, char *argv[])
{
	//Get the server port from the command line
	int server_port = atoi(argv[1]);
	int tcp_fd = create_tcp_socket(server_port);

	// Listening for requests from the client side, maximum 128 concurrent requests!
	while(listen(tcp_fd , MAX_PENDING) != -1) 
	{
		struct sockaddr_in client_address;
		int len = sizeof(client_address);

		// Connecting to the client socket via TCP socket
		int client_id = accept(tcp_fd, (struct sockaddr *) &client_address, &len);

		// Converting a network adress to a string
		inet_ntop(AF_INET, &client_address.sin_addr, buffer, INET_ADDRSTRLEN);

		// Executing the leftover process in a child program for implementing concurrency
		pid_t pid = fork();
		if(pid == 0) 
		{

			int message_size;
			printf("connection from: %s, port: %d\n", buffer, ntohs(client_address.sin_port));

			if((message_size = recv(client_id, buffer, MAX_LENGTH, 0)) < 0) err_handler("Error in receiving bytes. ");
			// Buffer contains a Type 1 request for UDP port
			
			int type,length;
			char message[1024]="";
			decode_message(buffer, &type, &length, message);
			printf("Decoded message received: %s\n", message);

			if (type == 1 && !strcmp(message, "UDP PORT TO BE GIVEN"))
			{
				// Server sends back the alloted port number as response.
				int udp_fd = create_udp_socket(free_udp_port);
				char reply_message[MAX_LENGTH] = "";

				strcpy(message,"");
				sprintf(message, "%d", free_udp_port);
				encode_message(2, message, reply_message);
	
				if(send(client_id, reply_message, strlen(reply_message), 0) != strlen(reply_message))
					err_handler("Error in sending UDP port number. ");
				
				printf("Message sent: %s\n", reply_message);
				
				//Receive the data message on UDP port from the client
				message_size = recvfrom(udp_fd, (char *)buffer, MAX_LENGTH, 0, (struct sockaddr *) &client_address, &len);
				decode_message(buffer, &type, &length, message);
				printf("Decoded message received: %s\n", message);
/*
				//Sending a data response to the client
				int port = atoi(message);

				struct sockaddr_in address_port;
				address_port.sin_family = AF_INET;
				address_port.sin_port = htons(port);
				address_port.sin_addr.s_addr = htonl(INADDR_ANY);

				printf("Data response to be given: ");
				scanf("%s",message);
				strcpy(reply_message,"");
				encode_message(4, message, reply_message);
				sendto(udp_fd, reply_message, strlen(reply_message), 0, (struct sockaddr *) &address_port, sizeof(address_port));
				printf("Message sent: %s\n", reply_message);
*/
			}
			
			exit(EXIT_SUCCESS);
		}
	}
	return 0;
}
