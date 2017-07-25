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
#include <netdb.h>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage:\n\t" << std::string(argv[0]) << " hostname.\n";
        return 1;
    }


    struct hostent* hostinfo = gethostbyname(argv[1]);
    assert(hostinfo);

    struct servent* servinfo = getservbyname("daytime", "tcp");
    assert(servinfo);

    std::cout << "daytime port is " << ntohs((uint16_t)servinfo->s_port) << std::endl;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = (uint16_t)servinfo->s_port;
    address.sin_addr = *(struct in_addr*)*hostinfo->h_addr_list;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int result = connect(sock, (struct sockaddr*)&address, sizeof(address));
    assert(result != -1);

    char buffer[128];
    auto read_size = read(sock, buffer, sizeof(buffer));
    //assert(result > 0);

    buffer[read_size] = '\0';
    std::cout << "the day time is: " << std::string(buffer) << std::endl;
    close(sock);
    return 0;
}