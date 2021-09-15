#ifndef TEXTHOLDER_HPP
#define TEXTHOLDER_HPP
#include <iostream>
class Client;

class TextHolder{
private:
    std::string buffer;
    std::string content;
public:
    friend class Client;
    TextHolder();
    ~TextHolder();
    void fillBuffer(const char *c, int buf_size);
    void fillBuffer(std::string const str);
    void fillContent(const char *c, int buf_size);
    void fillContent(std::string const str);
    std::string &getBuffer();
    std::string &getContent();
    void concatenate();
    bool isFull();
    bool endsWith(std::string const &ending);
    size_t bufferSize();
    bool isQuit();
    bool isList();
};
#endif

