#include "Channel.hpp"

Channel::Channel(std::string const &_name): name(_name){
    topic = "";
}

Channel::~Channel(){
    users.clear();
    operators.clear();
}

void Channel::addUser(Client *client) {
    users.insert(std::make_pair(client->getNick(), client));
    std::cout << "\033[1;33mClient_" << client->getDescriptor() << " | " << client->getNick() << " has joined to channel " << name << "\033[0m\n";
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

bool Channel::isOperator(Client *client) {
    std::vector<Client *>::iterator it = operators.begin();
    while (it != operators.end()) {
        if (*it == client)
            return (true);
        it++;
    }
    return (false);
}
