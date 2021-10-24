#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

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

void incorrect_params(void)
{
	std::cout << "Invalid parameters." << std::endl;
	std::cout << "Usage: ./ircserv [host:port_network:password_network] <port> <password>" << std::endl;
	exit(1);
}

int conv_to_int(std::string str)
{
	int num;
	std::stringstream ss;
	int i = 0;

	while (i < str.length())
	{
		if (!std::isdigit(str[i]))
			incorrect_params();
		i++;
	}
	ss << str;
	ss >> num;
	return (num);
}

int main (int argc, char *argv[])
{
    std::string host = "localhost";
    int port_network = 1000;
    std::string pass_network = "p";
    int port = 1001;
    std::string pass = "";

	if (argc >= 5)
		incorrect_params();
	else if (argc == 4)
	{
		std::string network = std::string(argv[1]);
		int i = 0;
		int ct = 0;
		while (i < network.length())
		{
			if (network[i] == ':')
				ct++;
			i++;
		}
		if (ct != 2 || (network.rfind(':') - network.find(':')) == 1)
			incorrect_params();
		host = network.substr(0, network.find(':'));
		port_network = conv_to_int(network.substr(network.find(':') + 1, network.rfind(':') - 1 - network.find(':')));
		pass_network = network.substr(network.rfind(':') + 1, network.length() - 1 - network.rfind(':'));
		port = conv_to_int(std::string(argv[2]));
		pass = std::string(argv[3]);
	}
	else if (argc == 3)
	{
		port = conv_to_int(std::string(argv[1]));
		pass = std::string(argv[2]);
	}
	else if (argc == 2)
		port = conv_to_int(std::string(argv[1]));

    Server *server;

    try {
        server = new Server(port, host, pass);
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
