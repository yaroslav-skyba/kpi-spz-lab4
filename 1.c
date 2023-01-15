#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>

int sockets;

void handle(int sig) {
	close(sockets);
	
	exit(0);
}

int main() {
	struct sockaddr_in server = {
		.sin_family = PF_INET,
		.sin_addr.s_addr = htonl(INADDR_ANY),
		.sin_port = htons(3218),
	};

	int logFile;
	
	if ((logFile = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
		printf("An error occurred while creating a log file: %s", strerror(errno));
		
		return -1;
	}

	struct sigaction sa;
	
	sa.sa_handler = handle;

	char buffer[1024];

	snprintf(buffer, 1024, "A parent: pid = %d; gid = %d; sid = %d\n", getpid(), getgid(), getsid(getpid()));
	
	if (write(logFile, buffer, strlen(buffer)) == -1) {
		printf("An error occurred while writing: %s", strerror(errno));
		
		return -1;
	}

	int socket;
	
	pid_t pid = fork();

	if (pid < 0) {
		printf("An error occurred while forking: %s\n", strerror(errno));
		
		return -1;
	} else if (pid == 0) {
		snprintf(buffer, 1024, "A demon: pid = %d; gid = %d; sid = %d.\n", getpid(), getgid(), getsid(getpid()));
		
		if (write(logFile, buffer, strlen(buffer)) == -1) {
			printf("An error occurred while writing: %s", strerror(errno));
			
			return -1;
		}
		
		sigfillset(&(sa.sa_mask));
		
		sigaction(SIGPIPE, &sa, NULL);
		
		socket = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
		
		if (socket == -1) {
			printf("An error occurred while creating a socket: %s\n", strerror(errno));
			
			return -1;
		}
		
		if (write(logFile, "A demon: a socket is created\n", sizeof("A demon: a socket is created\n") - 1) == -1) {
			printf("An error occurred while writing: %s", strerror(errno));
			
			return -1;
		}

		if (bind(socket, (struct sockaddr *)&server, sizeof(server)) == -1) {
			printf("An error occurred while binding: %s\n", strerror(errno));
			
			return -1;
		}
		
		if (write(logFile, "A demon: binded\n", sizeof("A demon: binded\n") - 1) == -1) {
			printf("error with writing: %s", strerror(errno));
			
			return -1;
		}

		if (listen(socket, 8) == -1) {
			printf("An error occurred while listening: %s\n", strerror(errno));
			
			return -1;
		}
		
		if (write(logFile, "A demon: listening\n", sizeof("A demon: listening\n") - 1) == -1) {
			printf("An error occurred while writing: %s", strerror(errno));
			
			return -1;
		}

		while (1) {
			if ((sockets = accept(socket, NULL, NULL)) == -1) {
				printf("An error occurred while accepting: %s\n", strerror(errno));
				
				continue;
			}

			if (fork() == 0) {
				snprintf(buffer, 1024, "A child: created\nChild: pid = %d; gid = %d; sid = %d\n", getpid(), getgid(), getsid(getpid()));
				
				if (write(logFile, buffer, strlen(buffer)) == -1) {
					printf("An error occurred while writing: %s", strerror(errno));
					
					return -1;
				}
				
				char bufferRec[1024];
				
				while (recv(sockets, bufferRec, 1024, 0) > 0) {
					if (strcmp(bufferRec, "close") == 0) {
						send(sockets, bufferRec, 1024, 0);
						
						break;
					}

					time_t currentTime;
					
					time(&currentTime);
					
					struct tm * local = localtime(&currentTime);
					
					char tempBuffer[1024];
					
					strcpy(tempBuffer, bufferRec);
					
					snprintf(bufferRec, 1024, "%02d:%02d:%02d_%d: %s", local->tm_hour, local->tm_min, local->tm_sec, getpid(), tempBuffer);
					snprintf(buffer, 1024, "recv = %s\n\n", bufferRec);
					
					if (write(logFile, buffer, strlen(buffer)) == -1) {
						printf("An error occurred while writing: %s", strerror(errno));
						
						return -1;
					}

					send(sockets, bufferRec, 1024, 0);
				}

				if (write(logFile, "A child: closed\n", sizeof("A child: closed\n") - 1) == -1) {
					printf("An error occurred while writing: %s", strerror(errno));
					
					return -1;
				}

				close(sockets);
				
				return 0;
			}

			close(sockets);
		}
	} else {
		if (write(logFile, "A parent: a child is created\n", sizeof("A parent: a child is created\n") - 1) == -1) {
			printf("error with writing: %s", strerror(errno));
			
			return -1;
		}
	}
	
	return 0;
}
