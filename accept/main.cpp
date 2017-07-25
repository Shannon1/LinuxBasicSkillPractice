#include <iostream>
//#include "backlog.h"
#include "accept.h"


int main(int argc, char* argv[]) {
    //signal(SIGTERM, handle_term);

    if (argc < 3)
    {
        print_usage(argv[0]);
        return 1;
    }

    //create_server(argv[1], atoi(argv[2]), atoi(argv[3]));
    create_server(argv[1], atoi(argv[2]));
    return 0;
}