#include "Client.hpp"
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
    responsePos = 0;
    gettimeofday(&timer, 0);
    buffer = new TextHolder();
    code = 0;
    isAuthorized = false;
    struct in_addr ipAddr = addr.sin_addr;
    inet_ntop(AF_INET, &ipAddr, ip_address_str, INET_ADDRSTRLEN );
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
    if (buffer->isQuit()) {
        status = Quit;
        return ;
    }
    std::cout << "Username.empty() = " << username.empty() << "\n";
    
    if (username.empty() && fillUserData())
    {
        std::cout << " | Fill user: username = " << username << " | name = " << realname << "\n";
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
    } else if (status == waitForRequest)
    {
        std::cout << "RECEIVED: " << buffer->getBuffer() << "\n";
        if (buffer->isList())
            code = 322;
        else {
            resetBuffer();
            return ;
        }
    }
    formResponse(str);
}



void Client::formResponse(std::string const &str) {
    resetBuffer();
    buffer->fillBuffer(str);
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
            buffer->fillBuffer(" 322 " + nickname + " #onechannel 1 :topic\n\r\n");
            break ;
        }
        case 323: {
            buffer->fillBuffer(" 323 " + nickname + "\n\r\n");
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
    std::cout << "CODE: " << code << " | RESPONSE IS: #" << buffer->getBuffer() << "#\n";
    int send_size = send(descriptor, buffer->getBuffer().substr(responsePos, responseSize - responsePos - 1).c_str(), responseSize - responsePos - 1, 0);
    if (send_size <= 0) {
        status = Error;
        return ;
    }
    responsePos += send_size;
    if (responsePos == responseSize - 1) {
        resetBuffer();
        if (code > 430 && code < 437) {
            std::cout << "Status: nick | nick = " << nickname << " | user = " << username << " | realname = " << realname << "\n";
            status = waitForNick;
        } else if (code == 322) {
            status = waitForResponseChain;
            code++;
        } else {
            std::cout << "Status: request | nick = " << nickname << " | user = " << username << " | realname = " << realname << "\n";
            status = waitForRequest;
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
