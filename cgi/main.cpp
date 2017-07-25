//
// Created by lee on 17-7-24.
//

#include <iostream>
#include <sys/socket.h>
#include <csignal>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage:\n\t" << std::string(argv[0])
                  << " ip port.\n";
        return 1;
    }

    std::string ip = std::string(argv[1]);
    uint16_t port = (uint16_t)atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
    address.sin_port = htons(port);

    auto sock = socket(AF_INET, SOCK_STREAM, 0);
    auto ec_bind = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ec_bind != -1);

    auto ec_listen = listen(sock, 5);
    assert(ec_listen != -1);

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    auto connfd = accept(sock, (struct sockaddr*)&client_addr, &client_addr_len);
    if (connfd < 0) {
        std::cout << "errno is " << errno << std::endl;
    } else {
        close(STDOUT_FILENO);
        dup(connfd);
        std::cout << "abcd\n";
        close(connfd);
    }

    close(sock);
    return 0;
}