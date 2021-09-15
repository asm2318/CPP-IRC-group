#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#ifndef __APPLE__
    #include <wait.h>
#endif

#include <cstdio>
#include <iostream>
#include <map>
#include <sstream>

#include "TextHolder.hpp"
class Server;

enum Status{
    waitForNick,
    waitForRequest,
    waitForResponse,
    waitForResponseChain,
    Error,
    Quit
};

class Client{

private:
    
    int descriptor;
    struct timeval timer;
    struct sockaddr_in addr;
    char ip_address_str[INET_ADDRSTRLEN];
    
    int responseSize;
    int responsePos;
    
    size_t requestLen;
    Status status;

    TextHolder *buffer;
    std::map<std::string, Client*> *allusers;
    int code;
    
    bool isAuthorized;
    
    std::string nickname;
    std::string username;
    std::string realname;
    std::string identifier;
    
    Server *server;

public:
    Client(int &port, std::map<std::string, Client *> &users, Server *_server);
    ~Client();
    void setTimer();
    TextHolder *getBuffer();
    void resetBuffer();
    int &getDescriptor();
    Status getStatus();
    struct timeval &getTimer();
    void handleRequest(std::string const &str);
    void formResponse(std::string const &str);
    void sendResponse();
    bool isInMap();
    std::string &getNick();
    bool fillUserData();
    void bufferNick();
};

#include "Server.hpp"
#endif

