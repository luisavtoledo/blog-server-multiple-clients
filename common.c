#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

void logexit(const char *message) {
    
	perror(message);
	exit(EXIT_FAILURE);
}

//treating ipv4 and ipv6 cases
int parse_address(const char *address, const char *port_string, struct sockaddr_storage *storage) {

    if (address == NULL || port_string == NULL) {
        return -1;
    }

    uint16_t port = (uint16_t)atoi(port_string);
    if (port == 0) {
        return -1;
    }
    port = htons(port);

    struct in_addr in_address_4;
    if (inet_pton(AF_INET, address, &in_address_4)) {
        struct sockaddr_in *address_ipv4 = (struct sockaddr_in *)storage;
        address_ipv4->sin_family = AF_INET;
        address_ipv4->sin_port = port;
        address_ipv4->sin_addr = in_address_4;
        return 0;
    }

    struct in6_addr in_address_6;
    if (inet_pton(AF_INET6, address, &in_address_6)) {
        struct sockaddr_in6 *address_ipv6 = (struct sockaddr_in6 *)storage;
        address_ipv6->sin6_family = AF_INET6;
        address_ipv6->sin6_port = port;
        memcpy(&(address_ipv6->sin6_addr), &in_address_6, sizeof(in_address_6));
        return 0;
    }

    return -1;
}

//treating ipv4 and ipv6 cases (server)
int server_sock_address_init(const char *protocol, const char *port_string, struct sockaddr_storage *storage) {

    uint16_t port = (uint16_t)atoi(port_string); 
    if (port == 0) {
        return -1;
    }
    port = htons(port); 

    memset(storage, 0, sizeof(*storage));
    if (0 == strcmp(protocol, "v4")) {
        struct sockaddr_in *address_ipv4 = (struct sockaddr_in *)storage;
        address_ipv4->sin_family = AF_INET;
        address_ipv4->sin_addr.s_addr = INADDR_ANY;
        address_ipv4->sin_port = port;
        return 0;
    } else if (0 == strcmp(protocol, "v6")) {
        struct sockaddr_in6 *address_ipv6 = (struct sockaddr_in6 *)storage;
        address_ipv6->sin6_family = AF_INET6;
        address_ipv6->sin6_addr = in6addr_any;
        address_ipv6->sin6_port = port;
        return 0;
    } else {
        return -1;
    }
}