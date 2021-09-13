#include "Client.hpp"

Client::Client(int &port, std::map<std::string, Client *> &users): allusers(&users), nickname("") {
    struct sockaddr_in addr;
    int addrLen = sizeof(addr);
    descriptor = accept(port, (sockaddr *) &addr, (socklen_t *)&addrLen);
    if (descriptor < 0)
        throw Exception("Client connection exception");
    struct linger so_linger;
    so_linger.l_onoff = true;
    so_linger.l_linger = 0;
    setsockopt(descriptor, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
    if (fcntl(descriptor, F_SETFL, O_NONBLOCK) < 0)
        throw Exception("Client fcntl exception");
    requestLen = 0;
    status = waitForNick;
    responsePos = 0;
    gettimeofday(&timer, 0);
    buffer = new TextHolder();
    code = 0;
    isAuthorized = false;
    //std::cout << "Client_" << descriptor << " connected\n";
}
    
Client::~Client() {
    std::cout << "Client_" << descriptor << " has disconnected\n";
    close(descriptor);
    delete buffer;
}

void Client::setTimer() {
    gettimeofday(&timer, 0);
}

TextHolder *Client::getBuffer() {
    return (buffer);
}

void Client::resetBuffer() {
    delete buffer;
    buffer = new TextHolder();
}

int &Client::getDescriptor() {
    return (descriptor);
}

Status Client::getStatus() {
    return (status);
}

struct timeval &Client::getTimer() {
    return (timer);
}

void Client::handleRequest() {
    if (buffer->isQuit()) {
        status = Quit;
        return ;
    }
    if (status == waitForNick) {
        nickname.clear();
        nickname = buffer->getNick();
        if (nickname.empty()){
            code = 431;
            formResponse();
        } else if (allusers->count(nickname)) {
            code = 433;
            formResponse();
        } else {
            allusers->insert(std::make_pair(nickname, this));
            isAuthorized = true;
            code = 1;
            formResponse();
        }
    } else if (status == waitForRequest) {
        std::cout << "RECEIVED: " << buffer->getBuffer() << "\n";
        resetBuffer();
    }
}

void Client::formResponse() {
    resetBuffer();
    switch (code) {
        case 1: {
            buffer->fillBuffer("001 RPL_WELCOME\nWelcome to the Internet Relay Network\n" + nickname + "!" + nickname + "@127.0.0.1\r\n");
            break ;
        }
        case 431: {
            buffer->fillBuffer("431 ERR_NONICKNAMEGIVEN\n* :No nickname given\r\n");
            break ;
        }
        case 433: {
            buffer->fillBuffer("433 ERR_NICKNAMEINUSE\n" + nickname + " :Nickname is already in use\r\n");
            break ;
        }
    }
    responsePos = 0;
    responseSize = buffer->bufferSize();
    status = waitForResponse;
}

void Client::sendResponse()
{
    int send_size = send(descriptor, buffer->getBuffer().substr(responsePos, responseSize - responsePos - 1).c_str(), responseSize - responsePos - 1, 0);
    if (send_size <= 0) {
        status = Error;
        return ;
    }
    responsePos += send_size;
    if (responsePos == responseSize - 1) {
        resetBuffer();
        if (code > 430 && code < 437)
        {
            std::cout << "Status: nick\n";
            status = waitForNick;
        }
        else
        {
            std::cout << "Status: request | nick = " << nickname << "\n";
            status = waitForRequest;
        }
        code = 0;
    }
}

bool Client::isInMap()
{
    return (isAuthorized);
}

std::string &Client::getNick()
{
    return (nickname);
}
