#include "TextHolder.hpp"
#include "Client.hpp"

TextHolder::TextHolder() {
    //std::cout << "Textholder start\n";
    buffer = "";
    content = "";
    //std::cout << "Textholder end\n";
}

TextHolder::~TextHolder() {
    //std::cout << "Textholder destructor\n";
    buffer.clear();
    content.clear();
}

void TextHolder::fillBuffer(const char *c, int buf_size) {
    buffer.append(c, static_cast<size_t>(buf_size));
}

void TextHolder::fillBuffer(std::string const str) {
    buffer.append(str);
}

void TextHolder::fillContent(const char *c, int buf_size) {
    content.append(c, static_cast<size_t>(buf_size));
}

void TextHolder::fillContent(std::string const str) {
    content = str;
}

std::string &TextHolder::getBuffer() {
    return (buffer);
}

std::string &TextHolder::getContent() {
    return (content);
}

void TextHolder::concatenate() {
    buffer += content;
    content.clear();
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

bool TextHolder::isQuit()
{
    if (!buffer.compare(0, 6, "QUIT :"))
        return (true);
    return (false);
}

bool TextHolder::isList()
{
    if (!buffer.compare("LIST\r\n") || !buffer.compare(0, 5, "LIST "))
        return (true);
    return (false);
}
