#ifndef TEXTHOLDER_HPP
#define TEXTHOLDER_HPP
#include <iostream>
class Client;

class TextHolder{
private:
    std::string buffer;
    std::string bufferReserved;
    std::string message;
public:
    friend class Client;
    TextHolder();
    ~TextHolder();
    void fillBuffer(const char *c, int buf_size);
    void fillBuffer(std::string const &str);
    void fillBufferReserved(std::string const &str);
    std::string &getBuffer();
    std::string &getBufferReserved();
    bool isFull();
    bool endsWith(std::string const &ending);
    size_t bufferSize();
    bool isQuit();
    bool isList();
    bool isJoin();
    bool isMsg();
    bool isNames();
    bool isPart();
    bool isTopic();
    void refillBuffer(std::string const &str);
    bool reserveIsEmpty();
    void handleReserved();
    void fillMessage(std::string const &str);
    std::string const &getMessage();
    void clear();
    bool empty();
    void insert(size_t pos, char c);
};
#endif

