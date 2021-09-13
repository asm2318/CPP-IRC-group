#include "Server.hpp"
#include "Client.hpp"

Server::Server(int port): _port(port) {
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptor < 0)
        throw Exception("Socket creation exception");
    struct sockaddr_in addr;
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
}

Server::~Server() {
    close(descriptor);
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
        if ((*itC)->getStatus() == Error || (*itC)->getStatus() == Quit || curTime.tv_sec - (*itC)->getTimer().tv_sec >= TIMEOUT) {
            if ((*itC)->isInMap())
                allusers.erase((*itC)->getNick());
            delete (*itC);
            itC = allclients.erase(itC);
            continue ;
        }
        if ((*itC)->getStatus() == waitForNick || (*itC)->getStatus() == waitForRequest)
            FD_SET((*itC)->getDescriptor(), &read_current);
        else if ((*itC)->getStatus() == waitForResponse)
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
            Client *client_sock = new Client(descriptor, allusers);
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
                std::cout << "Client_" << (*itC)->getDescriptor() << ": " << (*itC)->getBuffer()->getBuffer() << "\n";
                if ((*itC)->getBuffer()->isFull())
                    (*itC)->handleRequest();
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
    int ret;
    int descr;
    std::vector<Client *>::iterator itC = allclients.begin();
    while (itC != allclients.end())
    {
        descr = (*itC)->getDescriptor();
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
