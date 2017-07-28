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
    if (argc != 2) {
        std::cout << "Usage:\n\t filename " << std::string(arvg[0]) << std::endl;
        return 1;
    }

    std::string filename(argv[1]);
    int filefd = open(filename.c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0666);
    assert(filefd > 0);

    int pipefd_stdout[2];
    auto ec_pipe = pipe(pipefd_stdout);
    assert(ec_pipe != -1);

    int pipefd_file[2];
    ec_pipe = pipe(pipefd_file);
    assert(ec_pipe != -1);

    auto ec_splice = splice(STDIN_FILENO, NULL, pipefd_stdout[1], NULL, 32768, SPLICE_F_MOVE|SPLICE_F_MORE);
    assert(ec_splice != 1);

    auto ec_tee = tee(pipefd_stdout[0], pipefd_file[1], 327686, SPLICE_F_NONBLOCK);
    assert(ec_tee != 1);

    ec_splice = splice(pipefd_file[0], NULL, filefd, NULL, 32768, SPLICE_F_MOVE|SPLICE_F_MORE);
    assert(ec_splice != 1);

    ec_splice = splice(pipefd_stdout[0], NULL, STDIN_FILENO, NULL, 32768, SPLICE_F_MOVE|SPLICE_F_MORE);
    assert(ec_splice != 1);

    close(filefd);
    close(pipefd_file[0]);
    close(pipefd_file[1]);
    close(pipefd_stdout[0]);
    close(pipefd_stdout[1]);

    return 0;
}