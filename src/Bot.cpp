#include "Bot.hpp"

Bot::Bot(std::string serverPort, std::string serverPass)
{
	std::stringstream ss;
	ss << serverPort;
	ss >> this->_serverPort;
	this->_serverPass = serverPass;
	this->_nickname = "calcbot";
	this->_username = "calcbot";
	this->_realname = "calcbot";
	this->errflag = 0;
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

void Bot::connect_to_server(void)
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
	if (connect(_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
	{
		std::cerr << "Cannot connect to server." << std::endl;
		exit(1);
	}
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
	sendData("PASS " + _serverPass + "\r\n");
	usleep(100);
	sendData("NICK " + _nickname + "\r\n");
	usleep(100);
	sendData("USER " + _username + " 0 * " + _realname  + "\r\n");
	usleep(100);
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
	std::string user;
	std::string cmd;
	std::string text;
	std::string response;

	user = msg.substr(1, msg.find("!") - 1);
	cmd = msg.substr(msg.find(" ") + 1, msg.length());
	cmd = cmd.substr(0, cmd.find(" "));
	text = msg.substr(msg.find(" ") + 1, msg.length());
	text = text.substr(text.find(" ") + 1, text.length());
	text = text.substr(text.find(" ") + 1, text.length());
	text = text.substr(1, text.rfind("\r") - 2);
	response = makeResponse(text);
	if (cmd.compare("PRIVMSG") == 0)
		sendData("PRIVMSG " + user + " :" + response + "\r\n");
}

std::string Bot::makeResponse(std::string text)
{
	this->errflag = 0;
	if (check_correct(text) != 0)
		return ("Syntax error.");
	while (text.find(" ") != std::string::npos)
		text.erase(text.find(" "));
	if (check_correct2(text) != 0)
		return ("Syntax error.");
	return (calculate(text));
}

int Bot::check_correct(std::string text)
{
	size_t i = 0;
	int brackets = 0;
	while (i < text.length())
	{
		if (text[i] == '(')
			brackets++;
		if (text[i] == ')')
			brackets--;
		if (brackets == -1)
			return (-1);
		if (text[i] == '.')
		{
			if (i == 0 || i == (text.length() - 1))
				return (-1);
			if (text[i-1] < '0' || text[i-1] > '9' || text[i+1] < '0' || text[i+1] > '9')
				return (-1);
		}
		if ((text[i] >= '(' && text[i] <= '+') || (text[i] >= '-' && text[i] <= '9') || text[i] == '^' || text[i] == ' ')
			i++;
		else
			return (-1);
	}
	if (brackets != 0)
		return (-1);
	return (0);
}

int Bot::check_correct2(std::string text)
{
	size_t i = 0;
	while (i < text.length())
	{
		if (text[i] == '+' || text[i] == '-' || text[i] == '*' || text[i] == '/' || text[i] == '^')
		{
			if ((text[i] == '*' || text[i] == '/' || text[i] == '^') && (i == 0 || i == (text.length() - 1)))
				return (-1);
			if ((text[i] == '+' || text[i] == '-') && i == (text.length() - 1))
				return (-1);
			if ((text[i-1] == ')' || (text[i-1] >= '0' && text[i-1] <= '9')) && (text[i+1] == '(' || (text[i+1] >= '0' && text[i+1] <= '9')))
				i++;
			else
				return (-1);
		}
		else
			i++;
	}
	return (0);
}

std::string Bot::calculate(std::string text)
{
	std::string left;
	std::string right;
	size_t i = 0;
	int brackets = 0;
	int priority = 1;
	int pivot;
	char oper;

	if (text[0] == '(' && text[text.length() - 1] == ')' && isOuterBrackets(text) == 1)
	{
		text.erase(text.begin());
		text.erase(text.end() - 1);
	}
	if (this->errflag == 1)
		return ("Error: cannot divide by zero.");
	else if (this->errflag == 2)
		return ("Error: cannot take 0 in 0-th power.");
	else if (countOperators(text) == 0)
		return (text);
	else if (countOperators(text) == 1)
		return (operate(text));
	else
	{
		while (i < text.length())
		{
			if (text[i] == '(')
				brackets++;
			if (text[i] == ')')
				brackets--;
			if (brackets == 0 && priority == 1 && text[i] == '^')
			{
				pivot = i;
				oper = text[i];
			}
			if (brackets == 0 && priority <= 2 && (text[i] == '*' || text[i] == '/'))
			{
				priority = 2;
				pivot = i;
				oper = text[i];
			}
			if (brackets == 0 && (text[i] == '+' || text[i] == '-'))
			{
				priority = 3;
				pivot = i;
				oper = text[i];
			}
			i++;
		}
		left = text.substr(0, pivot);
		right = text.substr(pivot + 1, text.length() - pivot);
		left = calculate(left);
		right = calculate(right);
		if (this->errflag == 1)
			return ("Error: cannot divide by zero.");
		else if (this->errflag == 2)
			return ("Error: cannot take 0 in 0-th power.");
		return (operate(left + oper + right));
	}
}

int Bot::isOuterBrackets(std::string text)
{
	int ct = 0;
	size_t i = 1;
	while (i < text.length() - 1)
	{
		if (text[i] == '(')
			ct++;
		if (text[i] == ')')
			ct--;
		if (ct < 0)
			return (0);
		i++;
	}
	if (ct == 0)
		return (1);
	else
		return (0);
}

int Bot::countOperators(std::string text)
{
	size_t i = 0;
	int ct = 0;
	while (i < text.length())
	{
		if (text[i] == '+' || text[i] == '-' || text[i] == '*' || text[i] == '/' || text[i] == '^')
			ct++;
		i++;
	}
	return (ct);
}

std::string Bot::operate(std::string text)
{
	double a;
	double b;
	double c;
	std::stringstream ss1;
	std::stringstream ss2;
	std::stringstream ss3;
	std::string result;

	size_t i = 0;
	while (text[i] != '+' && text[i] != '-' && text[i] != '*' && text[i] != '/' && text[i] != '^')
		i++;
	ss1 << text.substr(0, i);
	ss1 >> a;
	ss2 << text.substr(i + 1, text.length() - i);
	ss2 >> b;
	if (text[i] == '+')
		c = a + b;
	else if (text[i] == '-')
		c = a - b;
	else if (text[i] == '*')
		c = a * b;
	else if (text[i] == '/')
	{
		if (b == 0)
		{
			this->errflag = 1;
			return ("Error: cannot divide by zero.");
		}
		else
			c = a / b;
	}
	else
	{
		if (a == 0 && b == 0)
		{
			this->errflag = 2;
			return ("Error: cannot take 0 in 0-th power.");
		}
		else
			c = pow(a, b);
	}
	ss3 << std::fixed << c;
	ss3 >> result;
	return (result);
}
