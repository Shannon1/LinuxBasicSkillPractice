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
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <sys/sendfile.h>


int main(int argc, char* argv[])
{
    if (argc <= 2) {
        std::cout << "Usage:\n\t" << std::string(argv[0])
                  << " ip port .\n";
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
    assert(sock > 0);

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
        int pipefd[2];
        auto ec_pipe = pipe(pipefd);
        assert(ec_pipe != -1);

        auto ec_splice = splice(connfd, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE|SPLICE_F_MOVE);
        assert(ec_splice !=-1);

        ec_splice = splice(pipefd[0], NULL, connfd, NULL, 32768, SPLICE_F_MORE|SPLICE_F_MOVE);
        assert(ec_splice !=-1);

        close(connfd);
    }

    close(sock);
    return 0;
}