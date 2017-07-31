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

const static size_t BUFFER_SIZE = 1024;

int main(int argc, char* argv[])
{
    if (argc <= 2) {
        std::cout << "Usage:\n\t" << std::string(argv[0])
                  << " ip port.\n";
        return 1;
    }

    std::string ip = std::string(argv[1]);
    auto port = (uint16_t)atoi(argv[2]);

    struct sockaddr_in address{};
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

    struct sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    auto connfd = accept(sock, (struct sockaddr*)&client_addr, &client_addr_len);
    
    if (connfd < 0) {
        std::cout << "errno is " << errno << std::endl;
        close(sock);
        return 1;
    } 
    
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    fd_set read_fds{};
    fd_set exception_fds{};
    FD_ZERO(&read_fds);
    FD_ZERO(&exception_fds);
    
    while (true) {
        FD_SET(connfd, &read_fds);
        FD_SET(connfd, &exception_fds);
        
        auto ret_select = select(connfd + 1, &read_fds, nullptr, &exception_fds, nullptr);
        if (ret_select < 0) {
            std::cout << "selection failed.\n";
            break;
        }
        
        if (FD_ISSET(connfd, &read_fds)) {
            auto ret_recv = recv(connfd, buffer, sizeof(buffer) - 1, 0);
            if (ret_recv <= 0) break;
            
            std::cout << "get " << ret_recv << " bytes of data: " 
                      << std::string(buffer) << std::endl;
        } else if (FD_ISSET(connfd, &exception_fds)) {
            auto ret_recv = recv(connfd, buffer, sizeof(buffer) - 1, MSG_OOB);
            if (ret_recv <= 0) break;

            std::cout << "get " << ret_recv << " bytes of oob data: "
                      << std::string(buffer) << std::endl;
        }
        
    }
    
    
    close(connfd);
    close(sock);
    return 0;
}