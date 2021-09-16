#include "TextHolder.hpp"
#include "Client.hpp"

TextHolder::TextHolder() {
    //std::cout << "Textholder start\n";
    buffer = "";
    bufferReserved = "";
    message = "";
    //std::cout << "Textholder end\n";
}

TextHolder::~TextHolder() {
    //std::cout << "Textholder destructor\n";
    buffer.clear();
    bufferReserved.clear();
    message.clear();
}

void TextHolder::fillBuffer(const char *c, int buf_size) {
    buffer.append(c, static_cast<size_t>(buf_size));
}

void TextHolder::fillBuffer(std::string const &str) {
    buffer.append(str);
}

void TextHolder::fillBufferReserved(std::string const &str) {
    bufferReserved.append(str);
}

std::string &TextHolder::getBuffer() {
    return (buffer);
}

std::string &TextHolder::getBufferReserved() {
    return (bufferReserved);
}

bool TextHolder::isFull() {
    return (endsWith("\r\n"));
}

bool TextHolder::endsWith(std::string const &ending) {
    if (ending.size() > buffer.size())
        return (false);
    return std::equal(ending.rbegin(), ending.rend(), buffer.rbegin());
}

size_t TextHolder::bufferSize() {
    return (buffer.size());
}

/*size_t TextHolder::bufferOutSize() {
    return (bufferOut.size());
}*/

bool TextHolder::isQuit() {
    if (!buffer.compare(0, 6, "QUIT :"))
        return (true);
    return (false);
}

bool TextHolder::isList() {
    if (!buffer.compare("LIST\r\n") || !buffer.compare(0, 5, "LIST "))
        return (true);
    return (false);
}

bool TextHolder::isJoin() {
    if (!buffer.compare(0, 5, "JOIN "))
        return (true);
    return (false);
}

void TextHolder::refillBuffer(std::string const &str) {
    buffer = str + " " + buffer;
}

bool TextHolder::reserveIsEmpty() {
    return (bufferReserved.empty());
}

void TextHolder::handleReserved() {
    buffer.clear();
    size_t pos = bufferReserved.find("\r\n");
    if (pos == std::string::npos) {
        bufferReserved.clear();
        return ;
    }
    pos += 2;
    fillBuffer(bufferReserved.substr(0, pos));
    bufferReserved.erase(0, pos);
}

void TextHolder::fillMessage(std::string const &str) {
    message.append(str);
    buffer.clear();
}

std::string const &TextHolder::getMessage() {
    return (message);
}
