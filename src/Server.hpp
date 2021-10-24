#ifndef SERVER_HPP
#define SERVER_HPP
#define BUFFERSIZE 1024*1024
#define TIMEOUT 120
#define HOSTNAME ":localhost"
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

#include "Exception.hpp"
class Channel;
class Client;

class Server{
private:
    fd_set read_current;
    fd_set write_current;
    std::vector<Client *> allclients;
    std::map<std::string, Client *> allusers;
    std::map<std::string, Channel *> allchannels;
    char buf[BUFFERSIZE + 1];
    int _port;
    int descriptor;
    struct timeval timeout;
    struct sockaddr_in addr;
    //std::string ip_address_str;
    
public:
    Server(int port, std::string host);
    ~Server();
    
    void refillSets();
    int getLastSock();
    int selector();
    size_t clientsCount();
    void handleConnections();
    void readRequests();
    void sendResponses();
    void cleaner();
    std::map<std::string, Channel *> *getChannelsList();
    Client *findUser(std::string const &nick);
    bool createChannel(std::string const &name, Client *client);
};

#endif
