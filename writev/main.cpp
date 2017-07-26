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

static const size_t BUFFER_SIZE = 1024;

static const std::vector<std::string> status_line{"200 OK", "500 Internal server error"};

int main(int argc, char* argv[])
{
    if (argc <= 3) {
        std::cout << "Usage:\n\t" << std::string(argv[0])
                  << " ip port filename.\n";
        return 1;
    }

    std::string ip = std::string(argv[1]);
    uint16_t port = (uint16_t)atoi(argv[2]);
    std::string filename = std::string(argv[3]);

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
//        char header_buf[BUFFER_SIZE];
//        memset(header_buf, '\0', BUFFER_SIZE);

        char* file_buf;
        struct stat file_stat;
        bool valid = true;

        int len = 0;
        if (stat(filename.c_str(), &file_stat) < 0) {
            valid = false;
        } else {
            if (S_ISDIR(file_stat.st_mode)) {
                valid = false;
            } else if (file_stat.st_mode & S_IROTH) {
                int fd = open(filename.c_str(), O_RDONLY);
                file_buf = new char[file_stat.st_size + 1];
                memset(file_buf, '\0', (size_t)file_stat.st_size + 1);
                if (read(fd, file_buf, file_stat.st_size) < 0) {
                    valid = false;
                }
            } else {
                valid = false;
            }
        }

        if (valid) {
            std::stringstream header_ss;
            header_ss << "HTTP/1.1 ";
            header_ss << status_line[0];
            header_ss << "\r\n";

            header_ss << "Content-Length: ";
            header_ss << file_stat.st_size;
            header_ss << "\r\n\r\n";
            std::string header_buf = header_ss.str();

            struct iovec iv[2];
            iv[0].iov_base = (void*)header_buf.c_str();
            iv[0].iov_len = header_buf.size();
            iv[1].iov_base = file_buf;
            iv[1].iov_len = file_stat.st_size;
            writev(connfd, iv, 2);
        } else {
            std::stringstream header_ss;
            header_ss << "HTTP/1.1 ";
            header_ss << status_line[1];
            header_ss << "\r\n\r\n";

            std::string header_buf = header_ss.str();
            send(connfd, header_buf.c_str(), header_buf.size(), 0);
        }
        close(connfd);
        delete [] file_buf;
    }

    close(sock);
    return 0;
}