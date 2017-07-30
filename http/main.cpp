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
static const std::string CRLF = "\r\n";

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

}

int main(int argc, char* argv)
{

}
