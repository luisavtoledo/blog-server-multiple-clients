#pragma once

#include <stdlib.h>
#include <semaphore.h>

#include <arpa/inet.h>

void logexit(const char *message);

int parse_address(const char *address, const char *port_string, struct sockaddr_storage *storage);

int server_sock_address_init(const char *protocol, const char *port_dtring, struct sockaddr_storage *storage);

struct BlogOperation {
    int client_id;
    int operation_type;
    int server_response;
    char topic[50];
    char content[2048];
};

struct Client {
    int id;
    int sock;
};

struct Topic {
    int amount;
    char topic[50];
    struct Client subscribed[10];
};

struct Server {
    int amount;
    struct Topic topics[10];
    struct Client client[10];
};

struct T {
    struct Client client;
    struct Server *server;
};