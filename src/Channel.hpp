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
    std::vector<std::string> operators;
    //std::set<std::string> banlist;
    
public:
    Channel(std::string const &_name);
    ~Channel();
    void addUser(Client *client);
    std::string const &getName();
    size_t getUsersNumber();
    std::string const getUsersNumberStr();
    std::string const &getTopic();
    std::map<std::string, Client *> *getUsers();
    void removeUser(std::string const &nick);
    bool empty();
    bool isOperator(std::string const &nick);
    void addOperator(std::string const &nick);
    void removeOperator(std::string const &nick);
    void setTopic(std::string const &str);
    void setPassword(std::string const &str);
    bool isPasswordMatched(std::string const &str);
    bool operatorRequest(std::string const &name, bool add);
};

#endif
