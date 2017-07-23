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

static const int BUFFER_SIZE = 512;

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
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buffer_size, len);
    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buffer_size, &len);

    std::cout << "the tcp send buffer size after setting is "
              << buffer_size << std::endl;

    auto ec_conn = connect(sock, (struct sockaddr*)&address, sizeof(address));

    if (ec_conn != -1) {
        char buffer[BUFFER_SIZE];
        memset(buffer, 'a', BUFFER_SIZE);
        send(sock, buffer, BUFFER_SIZE, 0);
    }

    close(sock);
    return 0;
}