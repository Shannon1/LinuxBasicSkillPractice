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

const static size_t BUFFER_SIZE = 1024 * 4;

enum CHECK_STATE {
    CHECK_STATE_REQUESTLINE = 0,
    CHECK_STAT_HEADER
};

enum LINE_STATUS {
    LINE_OK = 0,
    LINE_BAD,
    LINE_OPEN
};

enum HTTP_CODE {
    NO_REQUEST,
    GET_REQUEST,
    BAD_REQUEST,
    FORBIDDEN_REQUEST,
    INTERNAL_ERROR,
    CLOSE_CONNECTION
};

static const std::string ret_correct = "I get a correct result\n";
static const std::string ret_wrong = "Something wrong\n";

static const char CR = '\r';
static const char LF = '\n';
static const char* CRLF = "\r\n";
static const char* GET_METHOD = "GET";
static const char* HTTP_VERSION = "HTTP/1.1";
static const char* HTTP_PROTO = "http://";
static const char* HEADER_HOST = "host:";

LINE_STATUS parse_line(char* buffer, int& checked_index, int& read_index)
{
    char temp;
    for (; checked_index < read_index; ++checked_index) {
        temp = buffer[checked_index];
        if (temp == CR) {
            if ((checked_index + 1) == read_index) {
                return LINE_OPEN;
            } else if (buffer[checked_index + 1] == LF) {
                buffer[checked_index++] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            } else {
                return LINE_BAD;
            }
        } else if (temp == LF) {
            if ((checked_index > 1)
                && (buffer[checked_index - 1] == CR)) {
                buffer[checked_index - 1] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            } else {
                return LINE_BAD;
            }
        } else {
            return LINE_OPEN;
        }
    }
}

HTTP_CODE parse_requestline(char* temp, CHECK_STATE& check_state)
{
    char* url = strpbrk(temp, " \t");
    if (!url) {
        return BAD_REQUEST;
    }

    *url++ = '\0';

    char* method = temp;
    if (strcasecmp(method, GET_METHOD) == 0) {
        std::cout << "The request method is GET\n";
    } else {
        return BAD_REQUEST;
    }

    url += strspn(url, " \t");
    char* version = strpbrk(url, " \t");
    if (!version) {
        return BAD_REQUEST;
    }

    *version++ = '0';
    version += strspn(version, " \t");
    if (strcasecmp(version, HTTP_VERSION) != 0) {
        return BAD_REQUEST;
    }

    if (strncasecmp(url, HTTP_PROTO, sizeof(HTTP_PROTO)) == 0) {
        url += 7;
        url = strchr(url, '/');
    }

    if (!url || url[0] != '/') {
        return BAD_REQUEST;
    }

    std::cout << "The request URL is: " << std::string(url) << std::endl;
    check_state = CHECK_STAT_HEADER;
    return NO_REQUEST;
}

HTTP_CODE parse_headers(char* temp)
{
    if (temp[0] == '\0') {
        return GET_REQUEST;
    } else if (strncasecmp(temp, HEADER_HOST, sizeof(HEADER_HOST)) == 0) {
        temp += sizeof(HEADER_HOST);
        temp += strspn(temp, " \t");
        std::cout << "the request host is: " << std::string(temp) << std::endl;
    } else {
        std::cout << "I can not handle this header\n";
    }
    return NO_REQUEST;
}

HTTP_CODE parse_content(char* buffer, int& checked_index, CHECK_STATE& check_state, int& read_index, int& start_line)
{
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret_code = NO_REQUEST;
    while ((line_status = parse_line(buffer, checked_index, read_index)) == LINE_OK) {
        char* temp = buffer + start_line;
        start_line = checked_index;

        switch (check_state) {
            case CHECK_STATE_REQUESTLINE:
            {
                ret_code = parse_requestline(temp, check_state);
                if (ret_code == BAD_REQUEST) return BAD_REQUEST;
                break;
            }
            case CHECK_STAT_HEADER:
            {
                ret_code = parse_headers(temp);
                if (ret_code == BAD_REQUEST) return BAD_REQUEST;
                else if (ret_code == GET_REQUEST) return GET_REQUEST;
                break;
            }
            default:
            {
                return INTERNAL_ERROR;
            }
        }
    }

    if (line_status == LINE_OK) {
        return NO_REQUEST;
    } else {
        return BAD_REQUEST;
    }
}

int main(int argc, char* argv[])
{
    if (argc <= 2) {
        std::cout << "Usage: \n\t" << std::string(argv[0]) << "ip port\n";
        return 1;
    }

    std::string ip(argv[1]);
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
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        int data_read = 0;
        int read_index = 0;
        int checked_index = 0;
        int start_line = 0;
        CHECK_STATE check_state = CHECK_STATE_REQUESTLINE;

        while (1) {
            data_read = recv(connfd, buffer + read_index, BUFFER_SIZE - read_index, 0);
            if (data_read == -1) {
                std::cout << "reading failed\n";
                break;
            } else if (data_read == 0) {
                std::cout << "remote client has closed the connection\n";
            }

            read_index += data_read;
            auto result = parse_content(buffer, checked_index, check_state, read_index, start_line);
            if (result == NO_REQUEST) {
                continue;
            } else if (result == GET_REQUEST) {
                send(connfd, ret_correct.c_str(), ret_correct.size(), 0);
                break;
            } else {
                send(connfd, ret_wrong.c_str(), ret_wrong.size(), 0);
                break;
            }
        }
        close(connfd);
    }

    close(sock);
    return 0;
}
