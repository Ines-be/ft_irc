#include "../include/Server.hpp"

Client	*Server::getClientFromVector(int clientSocket) { 
	
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		if ((*it)->getSocket() == clientSocket)
			return (*it);

	return (NULL);
}

Client	*Server::getClientFromVector(std::string nick) { 
	
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		if ((*it)->getNick() == nick)
			return (*it);

	return (NULL);
}

Channel	*Server::getChannelFromVector(std::string name) {

	for (std::vector<Channel*>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it)
		if ((*it)->getName() == name)
			return (*it);

	return (NULL);
}

void	Server::eraseClientFromVector(int clientSocket) {
	std::vector<Client*>::iterator	it;

	if (_clients.empty())
		return ;
	for (it = _clients.begin(); it != _clients.end(); ++it) {
		if ((*it)->getSocket() == clientSocket) {
			close((*it)->getSocket());
			(*it)->setSocket(-1);
			_clients.erase(it);
			return ;
		}
	}
}

int	Server::maxClientFd()
{
	if (this->_clients.empty())
		return (-1);

	std::vector<Client*>::iterator it = this->_clients.begin();
	int max = (*it)->getSocket();
		
	while (it != this->_clients.end())
	{
		if ((*it)->getSocket() > max)
			max = (*it)->getSocket();
		++it;
	}

	return max;
}

std::vector<std::string>	split(std::string str, char c) {

	std::string		tmp;
	std::vector<std::string>	res;
	int	i = 0;
	int	r = 0;

	while (str[i] != '\0') {
		while (str[i] && (str[i] == c || str[i] == ' ' || (str[i] >= 8 && str[i] <= 13)))
			++i;
		r = 0;
		while (str[i + r] && (!(str[i + r] == c || (str[i + r] >= 8 && str[i + r] <= 13))))
			++r;
		tmp = str.substr(i, r);
		res.push_back(tmp);
		i += r;
	}
	return (res);
}

void	trimCmdName(const std::string& nick, std::string &msg, std::string cmdName)
{
	if (msg.find_first_of(' ') + 1 == msg.length())
		throw Server::err(needMoreParams(nick, cmdName));
	msg = msg.substr(msg.find_first_of(' '), msg.length());
	msg = msg.substr(msg.find_first_not_of(' '), msg.length());
}

void	trimEndSpace(std::string &str)
{
	if (str[str.size() - 1] == ' ')
		str = str.substr(0, str.size() - 1);
}

void	Server::checkNickname(std::string &msg)
{
	if (msg.empty())
		throw Server::err(noNickNameGiven());

	if (msg.size() > 9)
		throw Server::err(erroneusNickname(msg));

	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		if (it != _clients.end() && (*it)->getNick() == msg)
			throw Server::err(nickNameInUse((*it)->getNick()));

	for (int r = 0; msg[r]; ++r) {
		if (msg[r] == ' ' || !std::isprint(msg[r]) || (msg[r] >= 8 && msg[r] <= 12)) {
			throw Server::err(erroneusNickname(msg));
		} 
	}
}

bool	validChannelName(std::string chan) {
	
	if ((chan[0] && (!(chan[0] == '&' || chan[0] == '#' || chan[0] == '+' || chan[0] == '!')))|| 
		std::find(chan.begin(), chan.end(), ',') != chan.end() ||
		std::find(chan.begin(), chan.end(), 7) != chan.end() || chan.size() < 2)
		return (false);
	return (true);
}

std::string	timeToString(std::time_t t) {
	
	std::ostringstream oss;
	oss << t;
	return (oss.str());
}

int	checkPort(char *av)
{
	std::string str = av;
	std::stringstream portVal;
	int port;

	if (str.size() > 5)
		return -1;

	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isdigit(str[i]))
			return -1;
	}
	
	portVal << str;
	portVal >> port;

	if (port < 1024 || port > 65535)
		return (-1);
	
	return (port);
}

int	errorMsg(std::string msg)
{
	std::cerr << msg << std::endl;
	return (1);
}
