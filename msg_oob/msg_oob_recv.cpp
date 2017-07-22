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


static const int BUF_SIZE = 1024;

enum {
    NORMAL,
    OOB
};

void print_recv_msg(const std::string msg_recv, const int msg_type)
{
    std::cout << "got " << msg_recv.size() << " bytes of ";
    switch (msg_type)
    {
        case NORMAL:
            std::cout << "normal";
            break;
        case OOB:
            std::cout << "oob";
            break;
        default:
            break;
    }
    std::cout << " data: " << msg_recv << std::endl;
}

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
    address.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ec = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ec != -1);

    ec = listen(sock, 5);
    assert(ec != -1);

    struct sockaddr_in client_sock;
    socklen_t client_sock_len = sizeof(client_sock);
    int connfd = accept(sock, (struct sockaddr*)&client_sock, &client_sock_len);
    if (connfd < 0)
    {
        std::cout << "errno is: " << errno << std::endl;
    } else {
        char buffer[BUF_SIZE];

        bzero(buffer, BUF_SIZE);
        recv(connfd, buffer, BUF_SIZE - 1, 0);
        print_recv_msg(buffer, NORMAL);

        bzero(buffer, BUF_SIZE);
        recv(connfd, buffer, BUF_SIZE - 1, MSG_OOB);
        print_recv_msg(buffer, OOB);

        bzero(buffer, BUF_SIZE);
        recv(connfd, buffer, BUF_SIZE - 1, MSG_OOB);
        print_recv_msg(buffer, OOB);

        close(connfd);
    }

    close(sock);
    return 0;
}