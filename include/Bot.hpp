#ifndef BOT_HPP
# define BOT_HPP

#include <vector>
#include <iostream>

class Server;
class Channel;
class Client;

class Bot
{
	private:
		std::string	_name;
		std::vector<Channel*>	_channels;
		std::vector<std::string>	_badWords;

	public:
		Bot();
		~Bot();

		std::string getName() const;
		std::vector<std::string>	getBadWords() const;

		bool	checkBadWords(Server *serv, Channel *chan, Client *c, std::string tmp);
};

#endif