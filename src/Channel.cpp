#include "Channel.hpp"

Channel::Channel(std::string const &_name): name(_name){
    topic = "";
}

Channel::~Channel(){
    users.clear();
}

void Channel::addUser(Client *client) {
    users.insert(std::make_pair(client->getNick(), client));
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
