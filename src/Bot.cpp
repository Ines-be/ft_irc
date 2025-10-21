#include "Bot.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

Bot::Bot() : _name("bot") {
	this->_badWords.push_back("badword1");
	this->_badWords.push_back("badword2");
	this->_badWords.push_back("badword3");
}

Bot::~Bot() {}

std::string Bot::getName() const
{
    return (this->_name);
}

std::vector<std::string>    Bot::getBadWords() const
{
    return (this->_badWords);
}

bool	Bot::checkBadWords(Server *serv, Channel *chan, Client *c, std::string tmp) {

	for (std::vector<std::string>::iterator it = this->_badWords.begin(); it != this->_badWords.end(); ++it)
	{
		if (tmp.find(*it) != std::string::npos) {
			
			std::string msg = ":" + this->_name + "!" + this->_name + "@localhost KICK " + chan->getName() + " " + c->getNick();

			msg += " You've been kicked out, watch your words.\r\n";
			serv->leaveChannel(c, chan, msg);
			
			return (true);
		}
	}
	return (false);
}