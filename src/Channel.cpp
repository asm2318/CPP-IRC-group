#include "Channel.hpp"

Channel::Channel(std::string const &_name): name(_name){
    
}

Channel::~Channel(){
    users.clear();
}

void Channel::addUser(Client *client) {
    users.insert(std::make_pair(client->getNick(), client));
}
