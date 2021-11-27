#include "Channel.hpp"

Channel::Channel(std::string const &_name): name(_name){
    topic = "";
    password = "";
}

Channel::~Channel(){
    users.clear();
    operators.clear();
}

void Channel::addUser(Client *client) {
    users.insert(std::make_pair(client->getNick(), client));
    std::cout << "\033[1;33mClient_" << client->getDescriptor() << " | " << client->getNick() << " has joined to channel " << name << "\033[0m\n";
}

void Channel::addUser(Client *client, std::string const &nick) {
    users.insert(std::make_pair(nick, client));
    std::cout << "\033[1;33mClient_" << client->getDescriptor() << " | " << client->getNick() << " has changed to " << nick << " in " << name << "\033[0m\n";
}

std::string const &Channel::getName() {
    return (name);
}

size_t Channel::getUsersNumber() {
    return (users.size());
}

std::string const Channel::getUsersNumberStr() {
    std::stringstream result;
    result << getUsersNumber();
    return (result.str());
}

std::string const &Channel::getTopic() {
    return (topic);
}

std::map<std::string, Client *> *Channel::getUsers() {
    return (&users);
}

void Channel::removeUser(std::string const &nick) {
    users.erase(nick);
}

bool Channel::empty() {
    return (users.empty());
}

bool Channel::isOperator(std::string const &nick) {
    std::vector<std::string>::iterator it = operators.begin();
    while (it != operators.end()) {
        if (*it == nick)
            return (true);
        it++;
    }
    return (false);
}

void Channel::addOperator(std::string const &nick) {
    operators.push_back(nick);
}

void Channel::removeOperator(std::string const &nick) {
    std::vector<std::string>::iterator it = operators.begin();
    while (it != operators.end()) {
        if (*it == nick) {
            operators.erase(it);
            return ;
        }
        it++;
    }
}

void Channel::setTopic(std::string const &str) {
    topic.clear();
    topic = str;
}

void Channel::setPassword(std::string const &str) {
    password.clear();
    password = str;
}

bool Channel::isPasswordMatched(std::string const &str) {
    if (password.empty())
        return (true);
    //if (password.size() != str.size())
    //    return (false);
    if (!password.compare(str))
        return (true);
    return (false);
}

bool Channel::operatorRequest(std::string const &name, bool add) {
    std::map<std::string, Client *>::iterator it = users.find(name);
    if (it == users.end()) {
        return (false);
    }
    bool isOp = isOperator((*it).first);
    if (add && !isOp) {
        addOperator((*it).first);
        return (true);
    } else if (!add && isOp) {
        removeOperator((*it).first);
        return (true);
    }
    return (false);
}

void Channel::swapUser(std::string const &oldNickname, std::string const &newNickname, Client *client) {
    std::vector<std::string>::iterator it = operators.begin();
    removeUser(oldNickname);
    addUser(client, newNickname);
    std::cout << "swap user OK\n";
    while (it != operators.end()) {
        if (*it == oldNickname) {
            operators.erase(it);
            operators.push_back(newNickname);
            std::cout << "operator OK\n";
            return ;
        }
        it++;
    }
}
