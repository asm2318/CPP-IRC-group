#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "Server.hpp"

#ifndef FDCOUNT
    #define FDCOUNT 1024
#endif

bool isWorking;

void signal_handler(int num)
{
    if (num == SIGQUIT)
        isWorking = false;
}

int main (int argc, char *argv[])
{
    std::string host = "localhost";
    int port_network = 1000;
    std::string pass_network = "p";
    int port = 1001;
    std::string pass = "c";
    

    Server *server;

    try {
        server = new Server(port);
    } catch (Exception &e) {
        std::cout << e.what() << std::endl << " | Server stopped." << std::endl;
        exit (1);
    }
    isWorking = true;
    if (signal(SIGQUIT, signal_handler) == SIG_ERR) {
        std::cout << "Error on signal init. Server stopped." << std::endl;
        isWorking = false;
    }
    else
        std::cout << "Server is working..." << std::endl;
    struct rlimit limits_old;
    if (!getrlimit(RLIMIT_NOFILE, &limits_old) && limits_old.rlim_cur < FDCOUNT) {
        struct rlimit limits_new;
        limits_new.rlim_cur = FDCOUNT;
        limits_new.rlim_max = FDCOUNT;
        setrlimit(RLIMIT_NOFILE, &limits_new);
    }
    while (isWorking) {
        try {
            server->refillSets();
            int ret = server->selector();
            if (ret < 0) {
                std::cout << ret << ": Select error, skip cycle\n";
                server->cleaner();
                continue ;
            }
            if (!ret)
                continue ;
            server->handleConnections();
            server->readRequests();
            server->sendResponses();
        }
        catch (const std::bad_alloc& ex) {
            server->cleaner();
        }
    }
    std::cout << "Server is shutting down..." << std::endl;
    delete server;
    setrlimit(RLIMIT_NOFILE, &limits_old);
    exit(0);
}
