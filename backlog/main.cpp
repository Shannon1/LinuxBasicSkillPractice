#include <iostream>
#include "backlog.h"


int main(int argc, char* argv[]) {
    signal(SIGTERM, handle_term);

    if (argc < 3)
    {
        print_usage(argv[0]);
    }

    create_server(argv[1], atoi(argv[2]), atoi(argv[3]));

    return 0;
}