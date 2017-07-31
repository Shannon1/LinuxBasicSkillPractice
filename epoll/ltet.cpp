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
#include <sys/epoll.h>
#include <pthread.h>

const static size_t MAX_EVENT_NUMBER = 1024;
const static size_t BUFFER_SIZE = 10;

int set_nonblocking(int fd)
{
    auto old_option = fcntl(fd, F_GETFL);
    auto new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);

    return old_option;
}

void add_fd(int epoll_fd, int fd, bool enable_et)
{
    epoll_event event{};
    event.data.fd = fd;
    event.events = EPOLLIN;
    if (enable_et) {
        event.events |= EPOLLIN;
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    set_nonblocking(fd);
}

void lt(epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buffer[BUFFER_SIZE];
    for (auto i = 0; i < number; ++i) {
        int sock_fd = events[i].data.fd;
        if (sock_fd == listen_fd) {
            struct sockaddr_in client_addr{};
            socklen_t client_addr_len = sizeof(client_addr);
            auto conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
            add_fd(epoll_fd, conn_fd, false);
        } else if (events[i].events & EPOLLIN) {
            // 只要socket读缓存中还有为读出的数据，这段段代码就会被触发
            std::cout << "event trigger once\n";
            bzero(buffer, BUFFER_SIZE);
            auto ret_recv = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
            if (ret_recv > 0) {
                std::cout << "get " << ret_recv << " bytes of data: "
                          << std::string(buffer) << std::endl;
            } else {
                close(sock_fd);
                continue;
            }
        } else {
            std::cout << "something else happened." << std::endl;
        }
    }
}

void et(epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buffer[BUFFER_SIZE];
    for (auto i = 0; i < number; ++i) {
        int sock_fd = events[i].data.fd;
        if (sock_fd == listen_fd) {
            struct sockaddr_in client_addr{};
            socklen_t client_addr_len = sizeof(client_addr);
            auto conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
            add_fd(epoll_fd, conn_fd, true);
        } else if (events[i].events & EPOLLIN) {
            // 这段代码不会被重复触发，所以可以循环读取数据，确保把socket缓存中的数据全部读出
            std::cout << "event trigger once.\n";
            while (true) {
                bzero(buffer, BUFFER_SIZE);
                auto ret_recv = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
                if (ret_recv < 0) {
                    // 对于非阻塞IO，下面的条件成立表示数据已经全部读取完毕
                    // 此后epoll就能再次触发sock_fd上的EPOLLIN事件，驱动下一次操作
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                        std::cout << "read it later\n";
                        break;
                    }
                    close(sock_fd);
                    break;
                } else if (ret_recv == 0) {
                    close(sock_fd);
                } else {
                    std::cout << "get " << ret_recv << " bytes of data: "
                              << std::string(buffer) << std::endl;
                }
            }
        } else {
            std::cout << "something else happened.\n";
        }
    }
}

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

    auto listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd > 0);

    auto ec_bind = bind(listen_fd, (struct sockaddr*)&address, sizeof(address));
    assert(ec_bind != -1);

    auto ret_listen = listen(listen_fd, 5);
    assert(ret_listen != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epoll_fd = epoll_create(0);
    assert(epoll_fd > 0);

    while (true) {
        int ret_ewait = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
        if (ret_ewait < 0) {
            std::cout << "epoll failure.\n";
            break;
        }
        lt(events, ret_ewait, epoll_fd, listen_fd);
        //et(events, ret_ewait, epoll_fd, listen_fd);
    }

    close(listen_fd);
    return 0;
}