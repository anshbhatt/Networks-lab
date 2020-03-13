#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define MAX_LENGTH 1024

char buffer[MAX_LENGTH];

// ERROR HANDLING: Printing out the error
void err_handler(char err[255])
{
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

		strcpy(result, "");

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
		strcpy(msg, msg_str);

		return;
}

//Creating the UDP socket
int create_udp_socket(int port, char *ip)
{
		int client_socket;
		client_socket = socket(AF_INET, SOCK_DGRAM, 0);

		struct sockaddr_in client_address;
		client_address.sin_family = AF_INET;
		client_address.sin_port = htons(port);
		client_address.sin_addr.s_addr = inet_addr(ip);

		int status = bind(client_socket, (struct sockaddr *) &client_address, sizeof(client_address));
		if(status == -1) err_handler("UDP Binding failed");
		return client_socket;
}


int main(int argc, char* argv[])
{
		//Get the server IP and port from the command line
		int server_port = atoi(argv[2]);
		char socket_ip[25] = "";
		strcpy(socket_ip, argv[1]);

		printf("%s, %s\n", socket_ip, INADDR_ANY);

		// Creating a server port
		int socket_id = socket(PF_INET, SOCK_STREAM, 0);


		struct sockaddr_in address_port;
		address_port.sin_family = AF_INET;
		address_port.sin_port = htons(server_port);
		address_port.sin_addr.s_addr = inet_addr(socket_ip);

		// Connecting to the TCP server socket
		int status = connect(socket_id, (struct sockaddr *) &address_port, sizeof(address_port));
		if(status == -1) err_handler("TCP Connecting failed");

		//Requesting for a UDP conection to be made
		char udp_bool;
		printf("UDP request to be made? (y/n): ");
		scanf("%c", &udp_bool);
		printf("%c\n",udp_bool);
		if (udp_bool == 'y')
		{
				// Sends the request for UDP connection.
				int message_size;
				char message[MAX_LENGTH] = "";
				encode_message(1, "UDP PORT TO BE GIVEN", message);

				if(send(socket_id, message, strlen(message), 0) != strlen(message))
						err_handler("Error in sending UDP request. ");
				printf("Message sent: UDP PORT TO BE GIVEN\n");

				//Receiving the UDP port number
				if((message_size = recv(socket_id, buffer, MAX_LENGTH, 0)) < 0)
						err_handler("Error in receiving UDP port. ");

				int type,length;
				strcpy(message,"");
				decode_message(buffer, &type, &length, message);
				printf("TYPE: %d, Length: %d\nMsg: %s\n",type, length, message);

				//Changing the server port
				int port = atoi(message);
				printf("SERVER UDP PORT: %d\n", port);
				address_port.sin_port = htons(port);

				// Closing the TCP connection
				close(socket_id);

				// Creating the client UDP socket
				int client_port;
				printf("Client port number for UDP connection: ");
				scanf("%d", &client_port);
				int udp_fd = create_udp_socket(client_port, socket_ip);

				// Sending a data request using UDP port to the server
				fflush(stdin);
				strcpy(message, "");
				printf("Data request to be sent: ");
				scanf("%s", message);

				char reply_message[MAX_LENGTH] = "";
				encode_message(3, message, reply_message);
				printf("%s\n", reply_message);
				sendto(udp_fd, reply_message, strlen(reply_message), 0, (struct sockaddr *) &address_port, sizeof(address_port));
				printf("Message sent: %s\n", reply_message);

				// Receiving the data response from the server
				char buff[MAX_LENGTH] = "";

				message_size = recvfrom(udp_fd, (char *)buff, MAX_LENGTH, 0, (struct sockaddr *) &address_port, sizeof(address_port));
				decode_message(buff, &type, &length, message);
				printf("TYPE: %d, Length: %d\nMsg: %s\n",type, length, message);

				// Closing the UDP connection
				close(udp_fd);

		}

		return 0;

}

// TODO: Space sep message
