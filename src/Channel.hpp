#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <sstream>

#include "Client.hpp"

class Channel{
private:
    std::string name;
    std::map<std::string, Client *> users;
    std::string password;
    std::string topic;
    
public:
    Channel(std::string const &_name);
    ~Channel();
    void addUser(Client *client);
    std::string const &getName();
    size_t getUsersNumber();
    std::string const getUsersNumberStr();
    std::string const &getTopic();
    std::map<std::string, Client *> *getUsers();
};

#endif
