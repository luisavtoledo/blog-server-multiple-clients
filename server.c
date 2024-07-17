#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

sem_t m;

void *information (void *info) {
    struct T *data = (struct T *)info;
    int s = (*data).client.sock;
    struct BlogOperation request;
    struct BlogOperation message;

    struct Client client;
    client.sock = s;


    while(1){
        memset(&request, 0, sizeof(struct BlogOperation));
        size_t count = recv(s, &request, sizeof(struct BlogOperation), 0);

        message.client_id = request.client_id;
        message.operation_type = request.operation_type;
        message.server_response = 1;
        strcpy(message.topic, "");
        strcpy(message.content, "");

        if(request.operation_type == 1){
            //assigning id
            sem_wait(&m);
            int aux = 0;

            for(int i = 0; i < 10; i++){
                if( (*data).server->client[i].id == 0){
                    client.id = i + 1;
                    message.client_id = i + 1;
                    (*data).server->client[i] = client;
                    sem_post(&m);
                    break;
                }
            }
            sem_post(&m);
            printf("client %02d connected\n", message.client_id);

            size_t count = send(s, &message, sizeof(struct BlogOperation), 0);
			if (count != sizeof(struct BlogOperation)) {
				logexit("send");
            }
        }
        else if(request.operation_type == 2) {
            strcpy(message.topic, request.topic);
            strcpy(message.content, request.content);

            printf("new post added in %s by %02d\n", request.topic, request.client_id);

            //looking for topic
            int hasTopic = 0;
            int c = (*data).server->amount;
            for(int i = 0; i < c; i++) {
                if(strcmp((*data).server->topics[i].topic, request.topic) == 0){
                    //sending notification to subscribers
                    for(int j = 0; j < 10; j++) {
                        if((*data).server->topics[i].subscribed[j].id != 0) {
                            send((*data).server->topics[i].subscribed[j].sock, &message, sizeof(struct BlogOperation), 0);
                        }
                    }
                    hasTopic = 1;
                }
            }
            //creating topic
            if(!hasTopic) {
                struct Topic new_topic;
                strcpy(new_topic.topic, request.topic);
                new_topic.amount = 0;
                for(int i = 0; i < 10; i++) {
                    new_topic.subscribed[i].id = 0;
                    new_topic.subscribed[i].sock = 0;
                }
                sem_wait(&m);
                (*data).server->topics[(*data).server->amount] = new_topic;
                (*data).server->amount = (*data).server->amount + 1;
                sem_post(&m);
            }
        }
        else if(request.operation_type == 3) {
            int aux = (*data).server->amount;
            int j = 0;
            if(aux == 0){
                strcpy(message.content, "no topics available\n");
            }
            else {
                for(int i = 0; i < aux; i++) {
                    strcpy(&message.content[j], (*data).server->topics[i].topic);
                    j = j + strlen((*data).server->topics[i].topic);
                    strcpy(&message.content[j], ";");
                    j = j + 1;
                }
                strcpy(&message.content[j-1], "\n");

            }
            size_t count = send(s, &message, sizeof(struct BlogOperation), 0);
			if (count != sizeof(struct BlogOperation)) {
				logexit("send");
            }
        }
        else if(request.operation_type == 4) {
            //looking for topic
            int hasTopic = 0;
            int c = (*data).server->amount;
            int isSubscribed = 0;

            for(int i = 0; i < c; i++) {
                //checking if topic exists
                if(strcmp((*data).server->topics[i].topic, request.topic) == 0){
                    hasTopic = 1;
                    for(int j = 0; j < 10; j++) {
                        if((*data).server->topics[i].subscribed[j].id == client.id){
                            strcpy(message.content, "error: already subscribed\n");
                            isSubscribed = 1;
                            break;
                        }
                    }

                    if (!isSubscribed){
                        sem_wait(&m);
                        for(int k = 0; k < 10; k++) {
                            if((*data).server->topics[i].subscribed[k].id == 0){
                                (*data).server->topics[i].subscribed[k] = client;
                                break;
                            }
                        }
                        sem_post(&m);
                    }
                }
            }
            if(!hasTopic) {
                //creating topic
                struct Topic new_topic;
                strcpy(new_topic.topic, request.topic);
                new_topic.amount = 0;
                for(int i = 0; i < 10; i++) {
                    new_topic.subscribed[i].id = 0;
                    new_topic.subscribed[i].sock = 0;
                }
                
                //subscribing
                sem_wait(&m);
                for(int k = 0; k < 10; k++){
                    if(new_topic.subscribed[k].id == 0){
                        new_topic.subscribed[k] = client;
                        break;
                    }
                }
                sem_post(&m);

                new_topic.amount = new_topic.amount + 1;
                sem_wait(&m);
                (*data).server->topics[(*data).server->amount] = new_topic;
                (*data).server->amount = (*data).server->amount + 1;
                sem_post(&m);
            }

            strcpy(message.topic, request.topic);
            if(!isSubscribed)
                printf("client %02d subscribed to %s\n", request.client_id, request.topic);

            size_t count = send(s, &message, sizeof(struct BlogOperation), 0);
			if (count != sizeof(struct BlogOperation)) {
				logexit("send");
            }
        }
        else if(request.operation_type == 5) {
            int aux = request.client_id - 1;
            int c = (*data).server->amount;

            (*data).server->client[aux].id = 0;
            //removing client from topics
            for (int i =0; i < c; i++) {
                sem_wait(&m);
                for(int j = 0; j < 10; j++) {
                    if((*data).server->topics[i].subscribed[j].id == request.client_id){
                        (*data).server->topics[i].subscribed[j].id = 0;
                        (*data).server->topics[i].amount--;
                    }
                }
                sem_post(&m);
            }

            message.operation_type = 5;
            printf("client %02d disconnected\n", request.client_id);
            close(s);
            break;
        }
        else if(request.operation_type == 6){
            //looking for topic
            int aux = 0;
            int c = (*data).server->amount;

            for(int i = 0; i < c; i++) {
                if(strcmp((*data).server->topics[i].topic, request.topic) == 0){
                    aux = 1;

                    sem_wait(&m);
                    for(int j = 0; j < 10; j++) {
                        if((*data).server->topics[i].subscribed[j].id == request.client_id){
                            (*data).server->topics[i].subscribed[j].id = 0;
                            (*data).server->topics[i].amount--;
                        }
                    }
                    sem_wait(&m);
            
                }
            }
            printf("client %02d unsubscribed to %s\n", request.client_id, request.topic);
            strcpy(message.topic, request.topic);
        }
    }
}

int main(int argc, char **argv) {

    sem_init(&m, 0, 1);

    struct Server server;
    server.amount = 0;

    //initializing client ids
    for (int i = 0; i < 10; i++) {
        server.client[i].id = 0;
        server.client[i].sock = 0;
    }

    //initianting server (handles ipv4 and ipv6)
    struct sockaddr_storage storage;
    server_sock_address_init(argv[1], argv[2], &storage);

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *address = (struct sockaddr *)(&storage);
    if (0 != bind(s, address, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    struct T client_thread[10];

    struct sockaddr_storage client_storage;
    struct sockaddr *client_address = (struct sockaddr *)(&client_storage);
    socklen_t client_address_length = sizeof(client_storage);

    while (1) {

        //establishing connection with client
        int client_sock = accept(s, client_address, &client_address_length);

        if (client_sock == -1) {
            logexit("accept");
        }

        struct Client client;
        client.sock = client_sock;

        //thread
        struct T t;

        t.server = &server;
        t.client = client;

        pthread_t thread;
        pthread_create(&thread, NULL, information, &t);
       
    }

    exit(EXIT_SUCCESS);
}