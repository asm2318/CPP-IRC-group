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
    targetToChannel = false;
    privateChat = NULL;
    std::cout << "Client_" << descriptor << " connected\n";
}
    
Client::~Client() {
    std::cout << "Client_" << descriptor << " has disconnected\n";
    /*std::vector<Channel *>::iterator it = mychannels.begin();
    while (it != mychannels.end()){
        (*it)->removeUser(nickname);
        it++;
    }*/
    leaveAllChannels();
    close(descriptor);
    delete buffer;
    buffer = NULL;
    privateChat = NULL;
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
        //std::cout << "Status set to Prequit\n";
        //reservedStatus = Prequit;
        leaveAllChannels();
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
        /*if (nickname.empty())
            code = 431;
        else */
        if (nickIsAcceptable()) {
            if (allusers->count(nickname))
                code = 433;
            else {
                allusers->insert(std::make_pair(nickname, this));
                isAuthorized = true;
                code = 1;
            }
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
                buffer->clear();
                return ;
            }
        } else if (buffer->isMsg()) {
            //std::cout << "MSG\n";
            if (!handleMessage()) {
                buffer->clear();
                //std::cout << "BAD\n";
                return ;
            }
        } /*else if (buffer->isNames()) {
            code = 353;
            formResponse(str);
        }*/
        else if (buffer->isPart()) {
            if (!leaveChannel()) {
                buffer->clear();
                return ;
            }
        } else if (buffer->isTopic()) {
            if (!updateTopic()) {
                buffer->clear();
                return ;
            }
        } else {
            buffer->clear();
            return ;
        }
    }
    formResponse(str);
}



void Client::formResponse(std::string const &str) {
    if (code == 330 || targetToChannel || code == 333) {
        buffer->fillMessage(":" + identifier + " " + buffer->getBuffer().substr(0, buffer->bufferSize() - 2) + "\n\r\n\n");
        std::map<std::string, Client *>::iterator it = (*channel).second->getUsers()->begin();
        while (it != (*channel).second->getUsers()->end()) {
            //std::cout << "Check user: " << (*it).first << "\n";
            if ((*it).second != this || code == 330)
                (*it).second->outerRefillBuffer(buffer->getMessage());
            else
                code = 0;
            it++;
        }
        if (code == 333)
            outerRefillBuffer(buffer->getMessage());
        targetToChannel = false;
    } else if (privateChat != NULL) {
        privateChat->outerRefillBuffer(":" + identifier + " " + buffer->getBuffer().substr(0, buffer->bufferSize() - 2) + "\n\r\n\n");
        privateChat = NULL;
        buffer->clear();
    } else if (reservedStatus == Null){
        resetBuffer();
        buffer->fillBuffer(str);
    }
    switch (code) {
        case 1: {
            //std::cout << "USERNAME: " << username << "\n";
            if (!username.empty()) {
                identifier = nickname + "!" + username + "@" + ip_address_str;
                buffer->fillBuffer(" 001 " + nickname + " :Welcome to the Internet Relay Network " + identifier + "\n\r\n\n");
            } else {
                resetBuffer();
                status = waitForRequest;
                return ;
            }
            break ;
        }
        case 322: {
            
            buffer->fillBuffer(" 322 " + nickname + " " + (*channel).first + " " + (*channel).second->getUsersNumberStr() + " :" + (*channel).second->getTopic() + "\n\r\n\n");
            break ;
        }
        case 323: {
            buffer->fillBuffer(" 323 " + nickname + "\n\r\n\n");
            break ;
        }
        case 331: {
            buffer->fillBuffer(" 331 " + nickname + " " + (*channel).first + " :No topic is set\n\r\n\n");
            break ;
        }
        case 332: {
            buffer->fillBuffer(" 332 " + nickname + " " + (*channel).first + " :" + (*channel).second->getTopic() + "\n\r\n\n");
            break ;
        }
        case 353: {
            buffer->fillBuffer(" 353 " + nickname + " = " + (*channel).first + " :");
            std::map<std::string, Client *>::iterator it = (*channel).second->getUsers()->begin();
            while (it != (*channel).second->getUsers()->end()) {
                //std::cout << "Check user: " << (*it).first << "\n";
                if ((*channel).second->isOperator((*it).second))
                    buffer->fillBuffer("@");
                buffer->fillBuffer((*it).second->getNick() + " ");
                it++;
            }
            buffer->fillBuffer("\n\r\n\n");
            break ;
        }
        case 366: {
            buffer->fillBuffer(" 366 " + nickname + " " + (*channel).first + " :End of NAMES list\n\r\n\n");
            break ;
        }
        case 431: {
            buffer->fillBuffer(" 431 * :No nickname given\n\r\n\n");
            break ;
        }
        case 432: {
            buffer->fillBuffer(" 432 * " + nickname + " :Erroneous nickname\n\r\n\n");
            break ;
        }
        case 433: {
            buffer->fillBuffer(" 433 * " + nickname + " :Nickname is already in use\n\r\n\n");
            break ;
        }
    }
    responsePos = 0;
    responseSize = buffer->bufferSize();
    status = waitForResponse;
}

void Client::sendResponse()
{
    //std::cout << nickname << ": sending response\n";
    if (buffer->empty())
    {
        status = waitForRequest;
        if (reservedStatus == Null)
            resetBuffer();
        else
            handleReserved();
        return ;
    }
    
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
        } else if (code == 331 || code == 332) {
            status = waitForResponseChain;
            code = 353;
        } else if (code == 353) {
            status = waitForResponseChain;
            code = 366;
        } else
            status = waitForRequest;
        
        if (reservedStatus == Null)
            resetBuffer();
        else
            handleReserved();
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
    size_t pos1 = 5;
    size_t pos2 = buffer->getBuffer().find("\r\n", pos1);
    if (pos2 == std::string::npos)
        return (false);
    std::string channelName, topic, password;
    size_t pos3 = buffer->getBuffer().find(" :", pos1);
    if (pos3 != std::string::npos) { // && pos3 < pos2) //server part inside request
        topic = buffer->getBuffer().substr(pos3 + 2, pos2 - pos3 - 2);
        pos2 = pos3;
    } else
        topic = "";
    size_t pos4 = buffer->getBuffer().find(" ", pos1);
    if (pos4 != std::string::npos && pos4 != pos3) {
        password = buffer->getBuffer().substr(pos4 + 1, pos2 - pos4 - 1);
        pos2 = pos4;
    } else
        password = "";
    
        
    channelName = buffer->getBuffer().substr(pos1, pos2 - pos1);
    channel = server->getChannelsList()->find(channelName);
    if (channel != server->getChannelsList()->end())
    {
        (*channel).second->addUser(this);
        mychannels.push_back((*channel).second);
    } else {
        server->createChannel(channelName, this);
        channel = server->getChannelsList()->find(channelName);
        (*channel).second->setPassword(password);
        (*channel).second->setTopic(topic);
    }
    code = 330;
    return (true);
}

bool Client::leaveChannel() {
    size_t pos1 = 5;
    size_t pos2 = buffer->getBuffer().find(" :", pos1);
    if (pos2 == std::string::npos && (pos2 = buffer->getBuffer().find("\r\n", pos1)) == std::string::npos)
        return (false);
    std::string channelName;
    size_t pos3 = buffer->getBuffer().find(":", pos1);
    if (pos3 != std::string::npos && pos3 < pos2) //server part inside request
        channelName = buffer->getBuffer().substr(pos1, pos3 - pos1);
    else
        channelName = buffer->getBuffer().substr(pos1, pos2 - pos1);
    channel = server->getChannelsList()->find(channelName);
    if (channel != server->getChannelsList()->end())
    {
        (*channel).second->removeUser(nickname);
        std::vector<Channel *>::iterator it = mychannels.begin();
        while (it != mychannels.end()) {
            if (*it == (*channel).second) {
                mychannels.erase(it);
                //targetToChannel = true;
                code = 333;
                //std::cout << "Code set to " << code << "\n";
                return (true);
            }
            it++;
        }
    }
    return (false);
}

void Client::leaveAllChannels() {
    //std::cout << "Leaving all channels\n";
    std::vector<Channel *>::iterator it1 = mychannels.begin();
    while (it1 != mychannels.end())
    {
        (*it1)->removeUser(nickname);
        std::string quitMessage = ":" + identifier + " PART " + (*it1)->getName() + " :user disconnected from server\n\r\n\n";
        std::map<std::string, Client *>::iterator it2 = (*channel).second->getUsers()->begin();
        while (it2 != (*channel).second->getUsers()->end()) {
            (*it2).second->outerRefillBuffer(quitMessage);
            it2++;
        }
        it1++;
    }
    mychannels.clear();
}

void Client::setStatus(Status st) {
    status = st;
}

void Client::outerRefillBuffer(std::string const &str) {
    if (!buffer->empty()) {
        //std::cout << nickname << " - reserved status set\n";
        reservedStatus = waitForResponse;
        buffer->fillBufferReserved(str);
    } else {
        //std::cout << nickname << " - status set\n";
        status = waitForResponse;
        buffer->fillBuffer(str);
        responsePos = 0;
        responseSize = buffer->bufferSize();
    }
    //std::cout << nickname << " status is " << status << "\n";
    //std::cout << nickname << " - buffer: " << buffer->getBuffer();
}

void Client::handleReserved() {
    if (reservedStatus == Null)
        return ;
    std::swap(status, reservedStatus);
    if (status == Quit)
        return ;
    buffer->handleReserved();
    if (buffer->reserveIsEmpty()) // reservedStatus == waitForRequest &&
        reservedStatus = Null;
    responsePos = 0;
    responseSize = buffer->bufferSize();
}

bool Client::handleMessage() {
    //std::cout << "Message handling\n";
    size_t pos1 = 8;
    size_t pos2 = buffer->getBuffer().find(" ", pos1);
    if (pos2 == std::string::npos)
        return (false);
    std::string target;
    size_t pos3 = buffer->getBuffer().find(":", pos1);
    if (pos3 != std::string::npos && pos3 < pos2) //server part inside request
        target = buffer->getBuffer().substr(pos1, pos3 - pos1);
    else
        target = buffer->getBuffer().substr(pos1, pos2 - pos1);
    if (target[0] == '#') {
        channel = server->getChannelsList()->find(target);
        if (channel != server->getChannelsList()->end())
            targetToChannel = true;
        else
            return (false);
    } else {
        privateChat = server->findUser(target);
        if (privateChat == NULL) {
            //return (false); //Private chat with user here
            privateChat = this;
            buffer->clear();
            buffer->fillBuffer("PRIVMSG " + target + " :No such user - " + target + "\r\n");
            pos2 = buffer->getBuffer().find(" :");
        }
    }
    pos2++;
    pos1 = buffer->getBuffer().find("\r\n", pos2);
    if (pos1 == std::string::npos) {
        targetToChannel = false;
        return (false);
    }
    pos1+=2;
    //buffer->fillMessage(":" + identifier + " " + buffer->getBuffer().substr(pos2, pos1 - pos2));
    code = 0;
    if (buffer->getBuffer()[pos2] != ':')
        buffer->insert(pos2, ':');
    return (true);
}

std::string &Client::getID()
{
    return (identifier);
}

bool Client::readyForReserve() {
    //bool check = (status == waitForRequest);
    //std::cout << nickname << ": Reserve empty = " << buffer->reserveIsEmpty() << " | buffer empty = " << buffer->empty() << " | waitForRequest = " << check << "\n";
    return (!buffer->reserveIsEmpty());// && buffer->empty() && status == waitForRequest);
}

void Client::addChannel(Channel *c) {
    mychannels.push_back(c);
}

bool Client::nickIsAcceptable() {
    size_t length = nickname.size();
    if (!length) {
        code = 431;
        return (false);
    }
    char c;
    for (size_t i = 0; i < length; i++) {
        c = nickname[i];
        if (!((c >= 34 && c <= 36) || (c >= 38 && c <= 41) || (c >= 43 && c <= 57) || (c >= 60 && c <= 63) || (c >= 65 && c <= 125))) {
            code = 432;
            return (false);
        }
    }
    return (true);
}

bool Client::updateTopic() {
    size_t pos1 = 6;
    size_t pos2 = buffer->getBuffer().find(" :", pos1);
    if (pos2 == std::string::npos)
        return (false);
    std::string channelName = buffer->getBuffer().substr(pos1, pos2 - pos1);
    std::map<std::string, Channel *>::iterator it = server->getChannelsList()->find(channelName);
    if (it == server->getChannelsList()->end() || !(*it).second->isOperator(this))
        return (false);
    pos1 = buffer->getBuffer().find("\r\n", pos2);
    if (pos1 == std::string::npos)
        return (false);
    (*it).second->setTopic(buffer->getBuffer().substr(pos2, pos1 - pos2));
    return (true);
}
