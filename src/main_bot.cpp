#include "Bot.hpp"

bool isWorking = true;

void signal_handler(int num)
{
	if (num == SIGQUIT)
		isWorking = false;
}

int main(int argc, char **argv)
{
	if (argc != 2 && argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <serverPort> [serverPass]" << std::endl;
		return 1;
	}

	if (signal(SIGQUIT, signal_handler) == SIG_ERR)
	{
		std::cout << "Error on signal init. Bot stopped." << std::endl;
		isWorking = false;
	}
	else
		std::cout << "Bot is working..." << std::endl;

	std::string pass;

	if (argc == 3)
		pass = argv[2];
	else
		pass = "";

	Bot bot(argv[1], pass);

	bot.connect_to_server();
	bot.auth();

	std::string msg;

	while(isWorking)
	{
		msg = bot.receiveMessage();
		if (msg.compare("") != 0)
			bot.parseMessage(msg);
	}
	return 0;
}