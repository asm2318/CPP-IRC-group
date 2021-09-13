#include "Exception.hpp"

Exception::Exception(std::string message) throw(): message(message){}
Exception::~Exception() throw(){}
const char* Exception::what() const throw()
{
    return (message.c_str());
}
