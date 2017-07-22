//
// Created by lee on 17-7-22.
//

#ifndef LBSP_ACCEPT_H
#define LBSP_ACCEPT_H


#include <iostream>
#include <sys/socket.h>
#include <csignal>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static void print_usage(const std::string& base_name)
{
    std::cout << "Usage:\n" << base_name
              << " ip port" << std::endl;
}

static void create_server(const std::string& ip, const int port)
{
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &address);
    address.sin_port = htons((uint16_t)port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    sleep(20);

    struct sockaddr_in client;
    socklen_t client_addrlen = sizeof(client);
    int connfd = accept(sock, (struct sockaddr*)&client, &client_addrlen);
    if (connfd < 0)
    {
        std::cout << "errno is:" << errno << std::endl;
    }
    else
    {
        char remote[INET_ADDRSTRLEN];
        std::string client_ip(inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN));
        int client_port = ntohs(client.sin_port);
        std::cout << "connected with ip: " << client_ip << " port: " << client_port << std::endl;

        close(connfd);
    }

    close(sock);
    return;
}

#endif //LBSP_ACCEPT_H
