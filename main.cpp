#include "server/server.h"

int main () {
    Server server(8989, 8);
    server.threadpool();
    server.initServer();
    server.eventLoop();
    
    return 0;
}