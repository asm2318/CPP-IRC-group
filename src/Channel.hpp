#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include "Client.hpp"

class Channel{
private:
    std::string name;
    std::map<std::string, Client *> users;
    std::string password;
    
public:
    Channel(std::string const &_name);
    ~Channel();
    void addUser(Client *client);
};

#endif
