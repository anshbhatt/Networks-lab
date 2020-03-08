#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>

#define MAX_LENGTH 500


void err_handler(char err[255]) {
	printf("Error: %s\n", err);
	exit(1);
}

int create_udp_socket(int port) {
	int cli_socket;
	cli_socket = socket(AF_INET , SOCK_DGRAM, 0);

	struct sockaddr_in cli_address;
	cli_address.sin_family = AF_INET;
	cli_address.sin_port = htons(port);
	cli_address.sin_addr.s_addr = INADDR_ANY;

	int status = bind(cli_socket , (struct sockaddr *) &cli_address , sizeof(cli_address));
	if(status == -1) err_handler("Binding failed");
	return cli_socket;
}


int main() {
    int sock_id = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrport;
    addrport.sin_family = AF_INET;
    addrport.sin_port = htons(3002);
    addrport.sin_addr.s_addr = htonl(INADDR_ANY);

    int status = connect(sock_id, (struct sockaddr *) &addrport, sizeof(addrport));

    if(status == -1) {
        printf("Error connecting\n");
        return 0;
    }

    char buff[1024];

    char echo_word[25];
    int rcvMsgSize;
        fgets(echo_word, 25, stdin);
        printf("%s  Sent\n", echo_word);
        if(send(sock_id, echo_word, 25, 0) != 25){ printf("Error sending msg\n"); return 0;}
        if((rcvMsgSize = recv(sock_id, buff, 1024, 0)) < 0) {
            printf("Error receiving msg\n"); return 0;
        };
        printf("%s Rcvd\n", buff);
        close(sock_id);

        // TODO: ceate fun which decode string then create udp port send msg

        int port = atoi(buff);
        printf("%d\n", port);
        addrport.sin_port = htons(port);
        int p;
        scanf("%d", &p);
        int udp_fd = create_udp_socket(p);
        char msg[MAX_LENGTH];
        sendto(udp_fd, "4000", 4, MSG_CONFIRM, (struct sockaddr *) &addrport, sizeof(addrport));
        printf("error\n");
        int count = recvfrom(udp_fd, (char *)buff, MAX_LENGTH, MSG_WAITALL, (struct sockaddr *) &addrport, sizeof(addrport));
        printf("%s jkj\n", buff);
        printf("%d\n", count);
        close(udp_fd);

}
