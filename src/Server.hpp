#ifndef SERVER_HPP
#define SERVER_HPP
#define BUFFERSIZE 1024*1024
#define TIMEOUT 120
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

#include "Exception.hpp"
class Client;

class Server{
private:
    fd_set read_current;
    fd_set write_current;
    std::vector<Client *> allclients;
    std::map<std::string, Client *> allusers;
    char buf[BUFFERSIZE + 1];
    int _port;
    int descriptor;
    struct timeval timeout;
    
public:
    Server(int port);
    ~Server();
    
    void refillSets();
    int getLastSock();
    int selector();
    size_t clientsCount();
    void handleConnections();
    void readRequests();
    void sendResponses();
    void cleaner();
};

#endif
