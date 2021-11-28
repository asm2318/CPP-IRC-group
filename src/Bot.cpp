#include "Bot.hpp"

Bot::Bot(std::string serverPort, std::string serverPass)
{
	std::stringstream ss;
	ss << serverPort;
	ss >> this->_serverPort;
	this->_serverPass = serverPass;
	this->_nickname = "CalcBot";
	this->_username = "CalcBot";
	this->_realname = "CalcBot";
}

Bot::~Bot(void)
{
	close(_fd);
}

Bot::Bot(const Bot &other)
{
	if (this != &other)
	{
		_serverPort = other._serverPort;
		_serverPass = other._serverPass;
		_nickname = other._nickname;
		_username = other._username;
		_realname = other._realname;
	}
}

Bot &Bot::operator=(const Bot &other)
{
	if (this != &other)
	{
		_serverPort = other._serverPort;
		_serverPass = other._serverPass;
		_nickname = other._nickname;
		_username = other._username;
		_realname = other._realname;
	}
	return (*this);
}

void Bot::connect(void)
{
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
	{
		std::cerr << "Cannot create socket." << std::endl;
		exit(1);
	}
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_serverPort);
	_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/*hostent* _host = gethostbyname("127.0.0.1");
	inet_pton(_addr.sin_family, _host->h_name, &_addr.sin_addr);
	if (connect(_fd, reinterpret_cast<struct sockaddr*>(&_addr), sizeof(_addr)) < 0)
		throw Exception("Cannot connect to server.");*/
	fcntl(_fd, F_SETFL, O_NONBLOCK);
	_isActive = true;
}

void Bot::sendData(std::string msg)
{
	if (send(_fd, msg.c_str(), msg.length(), 0) < 0)
	{
		std::cerr << "Cannot send data." << std::endl;
		exit(1);
	}
}

void Bot::auth(void)
{
	sendData("PASS " + _serverPass + "\n");
	sendData("USER " + _username + " * 0 " + _realname  + "\n");
	sendData("NICK " + _nickname + "\n");
}

std::string Bot::receiveMessage()
{
	char buf[1024] = {0};
	std::string res;
	int rd = 0;
	while ((rd = recv(_fd, buf, 1023, 0)) > 0) {
		buf[rd] = 0;
		res += buf;
	}
	if (res == "")
		_isActive = false;
	return res;
}

void Bot::parseMessage(std::string msg)
{

}
