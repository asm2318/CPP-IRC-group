#include "TextHolder.hpp"
#include "Client.hpp"

TextHolder::TextHolder(): buffer(""), content(""){}

TextHolder::~TextHolder() {
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

std::string TextHolder::getNick() {
    size_t pos1 = 0, pos2 = 0;
    pos1 = buffer.find("NICK ");
    if (pos1 == std::string::npos)
        return ("");
    pos1 += 5;
    pos2 = buffer.find("\n", pos1);
    if (pos2 == std::string::npos)
        return ("");
    if (buffer[pos2 - 1] == '\r')
        pos2--;
    return (buffer.substr(pos1, pos2 - pos1));
}

size_t TextHolder::bufferSize() {
    return (buffer.size());
}

bool TextHolder::isQuit()
{
    if (!buffer.compare("QUIT :\r\n"))
        return (true);
    return (false);
}
