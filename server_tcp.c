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

// C substring function definition
void substring(char s[], char sub[], int p, int l) {
   int c = 0;

   while (p+c <= l) {
      sub[c] = s[p+c];
      c++;
			if(s[p+c] == '\0') break;
   }
   sub[c] = '\0';
}

// return pos of delimiters (if which_del is 1 => return first del's pos and so on)
// here : is also del since in (Type:1) here info after : is useful
int pos_of_del(char *str, int which_del, char del) {
	int i=0;
	while(str[i] != '\0') {
		if(which_del == 0) return i-1;
		if(str[i] == del) which_del--;
		i++;
	}
}

char *itoa(int num, char *str)
{
        if(str == NULL)
        {
                return NULL;
        }
        sprintf(str, "%d", num);
        return str;
}

void encode_msg(int t, int len, char * msg, char *res) {
	// ~ is delimiter
	char type[4]="";

	strcat(res, "Type:"); strcat(res, (char *)itoa(t, type));
	strcat(res, "~");
	char length[4]="";
	strcat(res, "Length:"); strcat(res, (char *)itoa(len, length));
	strcat(res, "~");
	strcat(res, "Msg:"); strcat(res, msg);
	return;
}

void decode_msg(char *str, int *type, int *length, char *msg) {
	int i=0;
	// first : and first ~ in between is type
	int first_del = pos_of_del(str, 1, ':'), sec_del = pos_of_del(str, 1, '~');
	char type_str[128]="";
	substring(str, type_str, first_del+1, sec_del-1);
	*type = atoi(type_str);



	char length_str[128]="";
	first_del = pos_of_del(str, 2, ':'); sec_del=pos_of_del(str, 2, '~');
	substring(str, length_str, first_del+1, sec_del-1);
	*length = atoi(length_str);


	int msg_start = pos_of_del(str, 3, ':');
	char msg_str[1024]="";
	substring(str, msg_str, msg_start+1, 1025);
	strcat(msg, msg_str);

	return;
}

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

	char msg[1024]="", res[1024]="";
	encode_msg(1, 5, "Hello Dear", res);
	printf("%s\n", res);

	int t, l;
	decode_msg(res, &t, &l, msg);
	printf("%d  %d  %s\n", t, l, msg);

	/*
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
			printf("%d\n", strlen(repl));

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
		}*/




	return 0;
}
