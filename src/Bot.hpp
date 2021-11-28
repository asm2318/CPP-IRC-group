#ifndef FT_IRC1_BOT_HPP
#define FT_IRC1_BOT_HPP

#include "Exception.hpp"
#include <iostream>
#include <csignal>
#include <string>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#ifndef __APPLE__
	#include <wait.h>
#endif

class Bot
{
private:

	int			_serverPort;
	std::string _serverPass;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	bool		_isActive;
	int			_fd;
	struct sockaddr_in _addr;

	void sendData(std::string msg);

public:
	Bot(std::string serverPort, std::string serverPass);
	~Bot(void);
	Bot(const Bot & other);
	Bot &operator=(const Bot & other);

	void connect(void);
	void auth(void);
	std::string receiveMessage(void);
	void parseMessage(std::string msg);

};

#endif
