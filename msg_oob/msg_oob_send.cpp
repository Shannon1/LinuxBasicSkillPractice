//
// Created by lee on 17-7-22.
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
    if (argc <= 2)
    {
        std::cout << "Usage:\n" << std::string(argv[0])
                  << " ip port" << std::endl;
    }

    std::string ip(argv[1]);
    uint16_t port = (uint16_t)atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
    address.sin_port = htons((uint16_t)port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ec = connect(sock, (struct sockaddr*)&address, sizeof(address));
    if (ec < 0)
    {
        std::cout << "connect failed!\n";
    }
    else
    {
        std::string oob_data = "abc";
        std::string normal_data = "123";
        send(sock, normal_data.c_str(), normal_data.size(), 0);
        send(sock, oob_data.c_str(), oob_data.size(), MSG_OOB);
        send(sock, normal_data.c_str(), normal_data.size(), 0);
    }

    close(sock);
    return 0;
}