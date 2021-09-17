#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

Server::Server(int port): _port(port) {
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptor < 0)
        throw Exception("Socket creation exception");
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    addr.sin_port = htons(port);
    int reuse = 1;
    if (setsockopt(descriptor, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        throw Exception("Setsockopt(SO_REUSEADDR) exception");
    #ifdef __APPLE__
    if (setsockopt(descriptor, SOL_SOCKET, SO_NOSIGPIPE, &reuse, sizeof(reuse)) < 0)
        throw Exception("Setsockopt(SO_NOSIGPIPE) exception");
    #else
        signal(SIGPIPE, SIG_IGN);
    #endif
    if (bind(descriptor, (sockaddr *)&addr, sizeof(addr)) < 0)
        throw Exception("Bind to port/ip exception");
    if (listen(descriptor, SOMAXCONN) < 0)
        throw Exception("Listening exception");
    /*struct in_addr ipAddr = addr.sin_addr;
    char ip_addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddr, ip_addr_str, INET_ADDRSTRLEN );
    ip_address_str = std::string(ip_addr_str);*/
    
    
    //allusers["jnoma"] = NULL;
    allchannels["#general"] = new Channel("#general");
    allchannels["#admin"] = new Channel("#admin");
}

Server::~Server() {
    close(descriptor);
    
    std::map<std::string, Channel *>::iterator it = allchannels.begin();
    while (it != allchannels.end())
    {
        delete (*it).second;
        it = allchannels.erase(it);
    }
    //allchannels.clear();
    cleaner();
}

void Server::refillSets() {
    //std::cout << "SET REFILL\n";
    FD_ZERO(&read_current);
    FD_ZERO(&write_current);
    FD_SET(descriptor, &read_current);
    struct timeval curTime;
    gettimeofday(&curTime, 0);
    std::vector<Client *>::iterator itC = allclients.begin();
    while (itC != allclients.end()) {
        if ((*itC)->getStatus() == Error || (*itC)->getStatus() == Quit) {//} || curTime.tv_sec - (*itC)->getTimer().tv_sec >= TIMEOUT) {
            if ((*itC)->isInMap())
                allusers.erase((*itC)->getNick());
            delete (*itC);
            itC = allclients.erase(itC);
            continue ;
        }
        if ((*itC)->getStatus() == waitForNick || (*itC)->getStatus() == waitForRequest)
            FD_SET((*itC)->getDescriptor(), &read_current);
        else if ((*itC)->getStatus() == waitForResponse || (*itC)->getStatus() == waitForResponseChain)
            FD_SET((*itC)->getDescriptor(), &write_current);
            
        itC++;
    }
}

int Server::getLastSock() {
    int last_sock = -1;
    int descr;
    for (size_t i = 0; i < allclients.size(); i++)
    {
        descr = allclients[i]->getDescriptor();
        if (descr > last_sock)
            last_sock = descr;
    }
    if (descriptor > last_sock)
        last_sock = descriptor;
    return (last_sock);
}

int Server::selector() {
    //std::cout << "Selector\n";
    timeout.tv_sec = 5;
    return (select(getLastSock() + 1, &read_current, &write_current, NULL, &timeout));
}

size_t Server::clientsCount() {
    return (allclients.size());
}

void Server::handleConnections() {
    //std::cout << "Handle connection\n";
    if (FD_ISSET(descriptor, &read_current)) {
        try{
            Client *client_sock = new Client(descriptor, allusers, this);
            allclients.push_back(client_sock);
        }catch (Exception &e){
            std::cout << "Connection refused\n";
        }
    }
}

void Server::readRequests() {
    //std::cout << "READ REQUESTS BLOCK\n";
    int ret;
    int descr;
    std::vector<Client *>::iterator itC = allclients.begin();
    while (itC != allclients.end())
    {
        descr = (*itC)->getDescriptor();
        if (FD_ISSET(descr, &read_current)) {
            ret = read(descr, buf, BUFFERSIZE);
            if (ret > 0) {
                (*itC)->setTimer();
                (*itC)->getBuffer()->fillBuffer(buf, ret);
                bzero(&buf, ret);
                //std::cout << "Client_" << (*itC)->getDescriptor() << ": " << (*itC)->getBuffer()->getBuffer() << "\n";
                if ((*itC)->getBuffer()->isFull())
                    (*itC)->handleRequest(HOSTNAME);
                itC++;
            }
            else if (ret <= 0) {
                if ((*itC)->isInMap())
                    allusers.erase((*itC)->getNick());
                delete (*itC);
                itC = allclients.erase(itC);
            }
        }
        else
            itC++;
    }
}

void Server::sendResponses() {
    //std::cout << "SEND RESPONSES BLOCK\n";
    int ret;
    int descr;
    std::vector<Client *>::iterator itC = allclients.begin();
    while (itC != allclients.end())
    {
        descr = (*itC)->getDescriptor();
        if ((*itC)->getStatus() == waitForResponseChain)
            (*itC)->formResponse(HOSTNAME);
        if (FD_ISSET(descr, &write_current))
            (*itC)->sendResponse();
        itC++;
    }
}

void Server::cleaner() {
    std::vector<Client *>::iterator it = allclients.begin();
    while (it != allclients.end()) {
        delete (*it);
        it = allclients.erase(it);
    }
}

std::map<std::string, Channel *> *Server::getChannelsList(){
    return (&allchannels);
}
