#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <fcntl.h>
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

enum Status{
    waitForNick,
    waitForRequest,
    waitForResponse,
    Error,
    Quit
};

class Client{

private:
    int descriptor;
    struct timeval timer;
    
    int responseSize;
    int responsePos;
    
    size_t requestLen;
    Status status;

    TextHolder *buffer;
    std::map<std::string, Client*> *allusers;
    std::string nickname;
    int code;
    
    bool isAuthorized;

public:
    Client(int &port, std::map<std::string, Client *> &users);
    ~Client();
    void setTimer();
    TextHolder *getBuffer();
    void resetBuffer();
    int &getDescriptor();
    Status getStatus();
    struct timeval &getTimer();
    void handleRequest();
    void formResponse();
    void sendResponse();
    bool isInMap();
    std::string &getNick();
};

#include "Server.hpp"
#endif

