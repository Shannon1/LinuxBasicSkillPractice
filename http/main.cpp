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

HTTP_CODE parse_requestline(char* tmp, CHECK_STATE& check_state)
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

    if (strcasecmp(url, HTTP_PROTO, sizeof(HTTP_PROTO)) == 0) {
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
    } else if (strcasecmp(temp, HEADER_HOST, sizeof(HEADER_HOST)) == 0) {
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

int main(int argc, char* argv)
{

}
