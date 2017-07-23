//
// Created by lee on 17-7-23.
//

#include <iostream>
#include <sys/socket.h>
#include <csignal>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static const int BUFFER_SIZE = 1024;

int main(int argc, char* argv[]) {
    if (argc <= 3) {
        std::cout << "Usage:\n\t" << std::string(argv[0])
                  << " ip port buffer_size" << std::endl;
    }

    std::string ip(argv[1]);
    uint16_t port = (uint16_t)atoi(argv[2]);
    int buffer_size = atoi(argv[3]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
    address.sin_port = htons((uint16_t)port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    socklen_t len = sizeof(buffer_size);
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buffer_size, len);
    getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buffer_size, &len);

    std::cout << "the tcp send buffer size after setting is "
              << buffer_size << std::endl;

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
        char buffer[BUFFER_SIZE];
        memset(buffer, '\0', BUFFER_SIZE);
        while (recv(connfd, buffer, BUFFER_SIZE - 1, 0) > 0) {
            ;
        }
        close(connfd);
    }
    
    close(sock);
    return 0;
}