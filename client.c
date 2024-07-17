#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

void *message_information(void *information){
    int s = *(int*)information;

    while(1){
        struct BlogOperation message;
        size_t count = recv(s, &message, sizeof(struct BlogOperation), 0);

        if (message.operation_type == 2){
			printf("new post added in %s by %02d\n", message.topic, message.client_id);
            printf("%s", message.content);
        }
		else{
			printf("%s", message.content);
		}
    }
}

int main(int argc, char **argv) {

	struct sockaddr_storage storage;

	//handling ipv4 and ipv6
	parse_address(argv[1], argv[2], &storage);

	//creating socket
	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}

	//connecting to server
	struct sockaddr *address = (struct sockaddr *)(&storage);
	if (0 != connect(s, address, sizeof(storage))) {
		logexit("connect");
	}

	struct BlogOperation operation;
    int id = 0;
	memset(&operation, 0, sizeof(struct BlogOperation));
	struct BlogOperation message;

    //first message: new connection
    operation.client_id = id;
    operation.operation_type = 1;
    operation.server_response = 0;
    strcpy(operation.topic, "");
    strcpy(operation.content, "");

    size_t count = send(s, &operation, sizeof(struct BlogOperation), 0);
	if (count != sizeof(struct BlogOperation)) {
		logexit("send");
	}

    //receive id
    recv(s, &operation, sizeof(struct BlogOperation), 0);
    id = operation.client_id;

    //thread
    pthread_t thread;
    pthread_create(&thread, NULL, message_information, &s);

    char input[100];
    int error;
    char command[50];
    char extra1[50];
    char extra2[50];

	while(1) {

		error = 0;

		//getting input
		fgets(input, sizeof(input), stdin);
		sscanf(input, "%s %s %s", command, extra1, extra2);

		//setting operation
		if(strcmp(command, "publish") == 0){
			operation.operation_type = 2;
            strcpy(operation.topic, extra2);
            fgets(operation.content, sizeof(operation.content), stdin);
		} 
		else if(strcmp(command, "list") == 0){
			operation.operation_type = 3;
		}
		else if(strcmp(command, "subscribe") == 0){
			operation.operation_type = 4;
            strcpy(operation.topic, extra1);
		}
		else if(strcmp(command, "exit" )== 0){
			operation.operation_type = 5;
		}
		else if(strcmp(command, "unsubscribe") == 0){
			operation.operation_type = 6;
            strcpy(operation.topic, extra1);
		} 
		else{
			printf("error: command not found\n");
			error = 1;
		}	

		//sending operation
		if(error == 0){
			size_t count = send(s, &operation, sizeof(struct BlogOperation), 0);
			if (count != sizeof(struct BlogOperation)) {
				logexit("send");
			}
			if(operation.operation_type == 5){
				//closing socket
				close(s);
				break;
				
			}
		}else if(error == 1){
			continue;
		}

	}

	exit(EXIT_SUCCESS);
}