#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

Client::Client(int &port, std::map<std::string, Client *> &users, Server *_server): allusers(&users), nickname(""), username(""), realname(""), server(_server) {
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
    reservedStatus = Null;
    responsePos = 0;
    gettimeofday(&timer, 0);
    buffer = new TextHolder();
    code = 0;
    isAuthorized = false;
    struct in_addr ipAddr = addr.sin_addr;
    inet_ntop(AF_INET, &ipAddr, ip_address_str, INET_ADDRSTRLEN );
    chainCounter = 0;
    std::cout << "Client_" << descriptor << " connected\n";
}
    
Client::~Client() {
    std::cout << "Client_" << descriptor << " has disconnected\n";
    close(descriptor);
    delete buffer;
    buffer = NULL;
}

void Client::setTimer() {
    gettimeofday(&timer, 0);
}

TextHolder *Client::getBuffer() {
    return (buffer);
}

void Client::resetBuffer() {
    //std::cout << "Buffer reset\n";
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

void Client::handleRequest(std::string const &str) {
    std::cout << "\n\033[1;31mClient_" << descriptor << " | " << nickname << " | RECEIVED: <\033[0m" << buffer->getBuffer() << "\033[1;31m>END\033[0m\n";
    if (buffer->isQuit()) {
        status = Quit;
        return ;
    }
    //std::cout << "Username.empty() = " << username.empty() << "\n";
    
    if (username.empty() && fillUserData())
    {
        //std::cout << " | Fill user: username = " << username << " | name = " << realname << "\n";
        if (username.empty())
            username = nickname;
        if (realname.empty())
            realname = nickname;
        if (code)
        {
            formResponse(str);
            return ;
        }
    }
    
    if (status == waitForNick) {
        nickname.clear();
        bufferNick();
        if (nickname.empty())
            code = 431;
        else if (allusers->count(nickname))
            code = 433;
        else {
            allusers->insert(std::make_pair(nickname, this));
            isAuthorized = true;
            code = 1;
        }
    } else if (status == waitForRequest) {
        if (buffer->isList()) {
            chainCounter = server->getChannelsList()->size();
            channel = server->getChannelsList()->begin();
            if (channel != server->getChannelsList()->end())
                code = 322;
            else
                code = 323;
        } else if (buffer->isJoin()) {
            if (!joinChannel()) {
                resetBuffer();
                return ;
            }
        } else {
            resetBuffer();
            return ;
        }
    }
    formResponse(str);
}



void Client::formResponse(std::string const &str) {
    if (code == 330) {
        buffer->fillMessage(":" + identifier + " " + buffer->getBuffer());
        std::map<std::string, Client *>::iterator it = (*channel).second->getUsers()->begin();
        while (it != (*channel).second->getUsers()->end()) {
            std::cout << "Check user: " << (*it).first << "\n";
            (*it).second->outerRefillBuffer(buffer->getMessage());
            it++;
        }
    } else if (reservedStatus == Null){
        resetBuffer();
        buffer->fillBuffer(str);
    }
    switch (code) {
        case 1: {
            //std::cout << "USERNAME: " << username << "\n";
            if (!username.empty()) {
                identifier = nickname + "!" + username + "@" + ip_address_str;
                buffer->fillBuffer(" 001 " + nickname + " :Welcome to the Internet Relay Network " + identifier + "\n\r\n");
            } else {
                resetBuffer();
                status = waitForRequest;
                return ;
            }
            break ;
        }
        case 322: {
            
            buffer->fillBuffer(" 322 " + nickname + " " + (*channel).first + " " + (*channel).second->getUsersNumberStr() + " :" + (*channel).second->getTopic() + "\n\r\n");
            break ;
        }
        case 323: {
            buffer->fillBuffer(" 323 " + nickname + "\n\r\n");
            break ;
        }
        case 331: {
            buffer->fillBuffer(" 331 " + nickname + " " + (*channel).first + " :No topic is set\n\r\n");
            break ;
        }
        case 332: {
            buffer->fillBuffer(" 332 " + nickname + " " + (*channel).first + " :" + (*channel).second->getTopic() + "\n\r\n");
            break ;
        }
        case 431: {
            buffer->fillBuffer(" 431 * :No nickname given\n\r\n");
            break ;
        }
        case 433: {
            buffer->fillBuffer(" 433 * " + nickname + " :Nickname is already in use\n\r\n");
            break ;
        }
    }
    responsePos = 0;
    responseSize = buffer->bufferSize();
    status = waitForResponse;
}

void Client::sendResponse()
{
    std::cout << "\n\033[1;32mCODE: " << code << " | RESPONSE TO " << nickname << " IS: <\033[0m" << buffer->getBuffer() << "\033[1;32m>END\033[0m\n";
    int send_size = send(descriptor, buffer->getBuffer().substr(responsePos, responseSize - responsePos - 1).c_str(), responseSize - responsePos - 1, 0);
    if (send_size <= 0) {
        status = Error;
        return ;
    }
    responsePos += send_size;
    if (responsePos == responseSize - 1) {
        if (code > 430 && code < 437) {
            //std::cout << "Status: nick | nick = " << nickname << " | user = " << username << " | realname = " << realname << "\n";
            status = waitForNick;
        } else if (code == 322) {
            status = waitForResponseChain;
            channel++;
            if (channel == server->getChannelsList()->end())
                code++;
        } else if (code == 330) {
            //std::cout << "GET 330\n";
            status = waitForResponseChain;
            if ((*channel).second->getTopic().empty())
                code = 331;
            else
                code = 332;
        } else {
            //std::cout << "Status: request | nick = " << nickname << " | user = " << username << " | realname = " << realname << "\n";
            status = waitForRequest;
            
            /*if (code == 1) {
                channel = server->getChannelsList()->find("#general");
                if (channel != server->getChannelsList()->end()) {
                    channel->second->addUser(this);
                    std::cout << nickname << " HAS BEEN ADDED TO CHANNEL " << channel->second->getName() << "\n";
                }
            }*/
        }
        if (reservedStatus == Null) {
            resetBuffer();
            //std::cout << "Response: buffer reset\n";
        }
        else {
            handleReserved();
            //std::cout << "Response: handle reserved\n";
        }
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

bool Client::fillUserData() {
    size_t pos1 = 0, pos2 = 0;
    pos1 = buffer->getBuffer().find("USER ");
    if (pos1 == std::string::npos)
        return (false);
    pos1 += 5;
    while ((buffer->getBuffer())[pos1] == ' ')
        pos1++;
    pos2 = buffer->getBuffer().find(" ", pos1 + 1);
    if (pos2 == std::string::npos)
        return (false);
    username.clear();
    username = buffer->getBuffer().substr(pos1, pos2 - pos1);
    while ((pos1 = buffer->getBuffer().find(" ", pos2)) != std::string::npos)
        pos2 = pos1 + 1;
    pos2--;
    pos1 = buffer->getBuffer().find("\r\n", pos2);
    if (pos1 == std::string::npos)
    {
        username.clear();
        return (false);
    }
    realname.clear();
    realname = buffer->getBuffer().substr(pos2, pos1 - pos2);
    return (true);
}

void Client::bufferNick() {
    size_t pos1 = 0, pos2 = 0;
    pos1 = buffer->getBuffer().find("NICK ");
    if (pos1 == std::string::npos) {
        nickname = "";
        return ;
    }
    pos1 += 5;
    pos2 = buffer->getBuffer().find("\r\n", pos1);
    if (pos2 == std::string::npos) {
        nickname = "";
        return ;
    }
    /*if (buffer[pos2 - 1] == '\r')
        pos2--;*/
    nickname = buffer->getBuffer().substr(pos1, pos2 - pos1);
}

bool Client::joinChannel() {
    std::cout << "Joining channel\n";
    size_t pos1 = 5;
    size_t pos2 = buffer->getBuffer().find("\r\n", pos1);
    if (pos2 == std::string::npos)
        return (false);
    std::string channelName = buffer->getBuffer().substr(pos1, pos2 - pos1);
    channel = server->getChannelsList()->find(channelName);
    if (channel != server->getChannelsList()->end())
    {
        (*channel).second->addUser(this);
        code = 330;
        return (true);
    }
    return (false);
}

void Client::setStatus(Status st) {
    status = st;
}

void Client::outerRefillBuffer(std::string const &str) {
    if (!buffer->getBuffer().empty()) {
        reservedStatus = waitForResponse;
        buffer->fillBufferReserved(str);
    } else {
        status = waitForResponse;
        buffer->fillBuffer(str);
        responsePos = 0;
        responseSize = buffer->bufferSize();
    }
}

void Client::handleReserved() {
    if (reservedStatus == Null)
        return ;
    std::swap(status, reservedStatus);
    buffer->handleReserved();
    if (reservedStatus == waitForRequest && buffer->reserveIsEmpty())
        reservedStatus = Null;
    responsePos = 0;
    responseSize = buffer->bufferSize();
}
