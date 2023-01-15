#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

int main() {
	int socket;

	char buffer[1024], result[1024];

	struct sockaddr_in server = {
		.sin_family = PF_INET,
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK),
		.sin_port = htons(3218),
	};

	if ((socket = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)) == -1) {
		printf("An error occurred while creating a socket: %s\n", strerror(errno));
		
		return -1;
	}

	if (connect(socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
		printf("An error occurred while connecting: %s\n", strerror(errno));
		
		return -1;
	}

	while (strcmp(buffer, "close") != 0) {
		printf("\nEnter something:\n");
		
		scanf("%s", &buffer);
		
		send(socket, buffer, 1024, 0);
		
		if (recv(socket, result, 1024, 0) > 0) {
			printf("%s", result);
		} else {
			printf("An error occurred while responding: %s\n", strerror(errno));
			
			close(socket);
			
			return -1;
		}
	}
	
	return 0;
}
