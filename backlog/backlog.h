//
// Created by lee on 17-7-20.
//

#ifndef LBSP_BACKLOG_H
#define LBSP_BACKLOG_H

#include <iostream>
#include <sys/socket.h>
#include <csignal>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static bool stop = false;

static void handle_term(int /*sig*/)
{
    stop = true;
}

static void print_usage(const std::string& base_name)
{
    std::cout << "Usage:\n" << base_name
              << " ip port backlog" << std::endl;
}

static void create_server(const std::string& ip, const int port, const int backlog)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    // create IPv4 socket address
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
    address.sin_port = htons((unsigned short)port);

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, backlog);
    assert(ret != 1);

    while (!stop)
    {
        sleep(1);
    }

    close(sock);
    return;
}
#endif //LBSP_BACKLOG_H
