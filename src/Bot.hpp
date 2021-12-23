#ifndef FT_IRC1_BOT_HPP
#define FT_IRC1_BOT_HPP

#include <cmath>
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

	int errflag;

	void sendData(std::string msg);
	std::string makeResponse(std::string text);
	int check_correct(std::string text);
	int check_correct2(std::string text);
	std::string calculate(std::string text);
	int countOperators(std::string text);
	std::string operate(std::string text);
	int isOuterBrackets(std::string text);

public:
	Bot(std::string serverPort, std::string serverPass);
	~Bot(void);
	Bot(const Bot & other);
	Bot &operator=(const Bot & other);

	void connect_to_server(void);
	void auth(void);
	std::string receiveMessage(void);
	void parseMessage(std::string msg);

};

#endif
