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
const static size_t BUFFER_SIZE = 1024;

struct fds {
    int epoll_fd;
    int sock_fd;
};

int set_nonblocking(int fd)
{
    auto old_option = fcntl(fd, F_GETFL);
    auto new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void add_fd(int epoll_fd, int fd, bool oneshot)
{
    epoll_event event{};
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if (oneshot) {
        event.events |= EPOLLONESHOT;
    }   // else { LT is default;}
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    set_nonblocking(fd);
}

void reset_oneshot(int epoll_fd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    set_nonblocking(fd);
}

void* work(void* arg)
{
    int sock_fd = ((fds*)arg)->sock_fd;
    int epoll_fd = ((fds*)arg)->epoll_fd;

    std::cout << "start new thread to receive data on fd: " << sock_fd << std::endl;
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    while (true) {
        auto ret_recv = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
        if (0 == ret_recv) {
            close(sock_fd);
            std::cout << "connection closed by remote.\n";
            break;
        }

        if (ret_recv < 0) {
            if (errno == EAGAIN) {
                reset_oneshot(epoll_fd, sock_fd);
                std::cout << "read later.\n";
                break;
            }
        }

        std::cout << "get content: " << std::string(buffer) << std::endl;
        sleep(5);
    }

    std::cout << "end thread receiving data on fd: " << sock_fd << std::endl;
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
    int epoll_fd = epoll_create(1234);
    assert(epoll_fd != -1);
    // 监听socket listen_fd上不能注册EPOLLONESHOT事件，否则程序只能处理一个客户连接
    // 后续的客户请求将不再触发EPOLLIN事件
    add_fd(epoll_fd, listen_fd, false);

    while (true) {
        int ret_ewait = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
        if (ret_ewait < 0) {
            std::cout << "epoll failure.\n";
            break;
        }

        for (auto i = 0; i < ret_ewait; ++i) {
            int sock_fd = events[i].data.fd;
            if (sock_fd == listen_fd) {
                struct sockaddr_in client_addr{};
                socklen_t client_addr_len = sizeof(client_addr);
                auto conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
                add_fd(epoll_fd, conn_fd, true);    // register EPOLLONESHOT for every connection
            } else if (events[i].events & EPOLLIN) {
                pthread_t thread;
                fds fds_worker{};
                fds_worker.epoll_fd = epoll_fd;
                fds_worker.sock_fd = sock_fd;
                pthread_create(&thread, NULL, work, (void*)&fds_worker);
            } else {
                std::cout << "something else happend\n";
            }
        }
    }

    close(listen_fd);
    return 0;
}