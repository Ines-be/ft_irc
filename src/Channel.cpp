#include "../include/Channel.hpp"

Channel::Channel(Client *auth, std::string data) : _name(data), _nbrOfMembers(1), _capacity(10), _inviteOnly(false),
		_keySet(false), _userLimit(false), _topicRestriction(false), _time(std::time(NULL)) {

	this->_members.insert(std::pair<Client*,bool>(auth, true));

	sendChannelInfo(auth);

	std::cout << auth->getNick() << " created " << _name << "  |    " << &(*this) << std::endl;
}

Channel::~Channel() {}

std::map<Client*,bool>		Channel::getMembers() const {
	return (this->_members);
}

std::string	Channel::getName() const {
	return (this->_name);
}

std::string	Channel::getTopic() const {
	return (this->_topic);
}

std::string	Channel::getPassword() const {
	return (this->_password);
}

int	Channel::getNbrOfMembers() const {
	return (this->_nbrOfMembers);
}

int	Channel::getCapacity() const {
	return (this->_capacity);
}

bool	Channel::getInviteOnly() const {
	return (this->_inviteOnly);
}

bool	Channel::getKeySet() const {
	return (this->_keySet);
}

bool	Channel::getUserLimit() const {
	return (this->_userLimit);
}

bool	Channel::getTopicRestriction() const {
	return (this->_topicRestriction);
}
std::time_t	Channel::getTime() const {
	return (this->_time);
}

std::time_t	Channel::getTimeTopicChanged() const {
	return (this->_timeTopicChanged);
}

std::string	Channel::getUserTopicChanged() const {
	return (this->_userTopicChanged);
}

std::vector<std::string>	Channel::getUserInvited() {
	return (this->_userInvited);
}

void	Channel::setTopic(std::string topic) {
	this->_topic = topic;
}

void	Channel::setTimeTopic(std::time_t time) {
	this->_timeTopicChanged = time;
}

void	Channel::setUserTopic(std::string user) {
	this->_userTopicChanged = user;
}

void	Channel::addMember(Client *c) {

	if (this->_capacity == this->_nbrOfMembers)
		throw (Server::err(channelIsFull(c->getNick(), this->_name)));
	else if (this->_inviteOnly && !c->isInvited(this))
		throw (Server::err(inviteOnlyChan(c->getNick(), this->_name)));

	this->_members.insert(std::pair<Client*,bool>(c, false));
	c->addChannel(this);
	this->_nbrOfMembers++;

	sendNewMemberInfo(c);

	if (c->isInvited(this))
		this->supprInvitation(c);
}

void	Channel::delMember(Client *c) {

	this->delegateOperator(c);
	if (this->findClient(c))
		this->_members.erase(this->findClient(c));
}

void	Channel::inviteMember(Client *c, Client *invited) {

	c->rpl(inviting(c->getNick(), invited->getNick(), this->_name));
	std::string prompt = c->makePrefix("INVITE") + invited->getNick() + " :" + this->_name + "\r\n"; 
	write(invited->getSocket(), prompt.c_str(), prompt.length());

	this->_userInvited.push_back(invited->getNick());
}

void	Channel::sendMsg(Client *c, std::string msg) {

	std::map<Client*,bool>::iterator	it = this->_members.begin();

	while (it != this->_members.end()) {
		if (it->first->getSocket() != c->getSocket())
			write(it->first->getSocket(), msg.c_str(), msg.length());
		++it;
	}
}

void	Channel::setPassword(bool sign, std::string data, Client *c) {
	if (sign && this->_password == data)
		return ;

	if (sign)
		sendModeInfo(c, "+k");
	else
		sendModeInfo(c, "-k");

	if (sign) {
		this->_password = data;
		this->_keySet = true;
	}
	else 
		this->_keySet = false;
}

void	Channel::setInviteOnly(bool sign, Client *c) {
	if (this->_inviteOnly == sign)
		return ;

	if (sign)
		sendModeInfo(c, "+i");
	else
		sendModeInfo(c, "-i");
	
	this->_inviteOnly = sign;
}

void	Channel::delUserLimit(Client *c) {

	if (!this->_userLimit)
		return ;

	this->_userLimit = false;
	this->_capacity = 10;

	std::string	prompt = c->makePrefix("MODE") + this->_name + " -l\r\n";

	sendMsgToAllMembers(prompt);
}

void	Channel::setUserLimit(std::string data, Client *c) {

	if (data.empty() || data == "")
		throw Server::err(needMoreParams(c->getNick(), "MODE"));

	std::string	prompt = c->makePrefix("MODE") + this->_name + " +l\r\n";

	this->_userLimit = true;

	for (unsigned int i = 0; i < data.size(); ++i) {
		if (!std::isdigit(data[i]))
			throw Server::err("MODE: ERR_INCORRECTUSE (!!!)\r\n");
	}

	this->_capacity = std::atoi(data.c_str());
	if (this->_capacity > 10)
		this->_capacity = 10;
	else if (this->_capacity == 0)
		this->_capacity = this->_nbrOfMembers;
	
	sendMsgToAllMembers(prompt);
}

void	Channel::setTopicRestriction(bool sign, Client *c) {
	if (this->_topicRestriction == sign)
		return ;

	if (sign)
		sendModeInfo(c, "+t");
	else
		sendModeInfo(c, "-t");

	this->_topicRestriction = sign;
}

void	Channel::setOperator(Client *c, bool sign, std::string nickNewOp) {

	try {
		c->getChanFromClientVector(this->_name) ? c->getChanFromClientVector(this->_name) : throw Server::err(notOnChannel(c->getNick(), this->_name));
		std::map<Client*,bool>::iterator	it = this->_members.begin();

		while (it != this->_members.end()) {
			if (it->first->getNick() == nickNewOp) {
				if (it->second == sign) {
					if (sign)
						throw Server::err("That user is already an operator");
					else 
						throw Server::err("That user is not an operator");
					return;
				}

				lastOperator(c, sign);

				sign ? it->second = true : it->second = false;

				if (sign)
					sendModeInfo(c, "+o");
				else
					sendModeInfo(c, "-o");
				return ;
			}
			++it;
		}
		throw Server::err(userNotInChannel(nickNewOp, this->_name));
	}
	catch (const std::exception& e) {
		write(c->getSocket(), e.what(), std::strlen(e.what()));
	}
}

void	Channel::lastOperator(Client *c, bool sign) {

	if (sign)
		return;
		
	for (std::map<Client*,bool>::iterator it = this->_members.begin(); it != this->_members.end(); ++it)
		if (it->first != c && it->second)
			return ;

	throw Server::err("");
}

void	Channel::validMode(std::string mode, bool* sign) {

	if (mode[0] != '+' && mode[0] != '-')
		throw Server::err(unknownMode(mode[0], this->_name));

	*sign = mode[0] == '+' ? true : false;
	
	for (int i = 1; mode[i]; ++i)
		if (!(mode[i] == 'i' || mode[i] == 't' || mode[i] == 'k' || mode[i] == 'o' || mode[i] == 'l'))
			throw Server::err(unknownMode(mode[i], this->_name));
}

void	Channel::sendMsgToAllMembers(std::string msg)
{
	if (this->_nbrOfMembers == 0)
		return ;

	for (std::map<Client*, bool>::iterator it = this->_members.begin(); it != this->_members.end(); ++it)
		write(it->first->getSocket(), msg.c_str(), msg.length());
}

void	Channel::delegateOperator(Client *c) {

	bool	flag = false;

	this->_nbrOfMembers--;
	if (!this->findClient(c))
		return ;
		
	if (this->_nbrOfMembers && this->getClientStatus(c)) {

		for (std::map<Client*,bool>::iterator it = this->_members.begin(); it != this->_members.end(); ++it)
			if (it->first != c && it->second)
				flag = true;
		
		if (!flag) {
			for (std::map<Client*,bool>::iterator it = this->_members.begin(); it != this->_members.end(); ++it)
				if (it->first != c)
					this->setOperator(it->first, true, it->first->getNick());
		}
	}
}

std::string	Channel::channelModeIs(std::string nick, std::string channel) {
	std::string rply = "324 " + nick + " " + channel;

	if (this->getInviteOnly() || this->getKeySet())
		rply += " #private ";
	else 
		rply += " #openchannel ";
	rply += "+o";
	if (this->getInviteOnly())
		rply += "i";
	if (this->getKeySet())
		rply += "k";
	if (this->getUserLimit())
		rply += "l";
	if (this->getTopicRestriction())
		rply += "t";
	return (rply);
}

void		Channel::deleteClient(Client *c)
{
	for (std::map<Client*, bool>::iterator it = this->_members.begin(); it != this->_members.end(); ++it) {
		if (it->first == c) {
			this->_members.erase(it);
			return ;
		}
	}
}

Client		*Channel::findClient(Client *c)
{
	for (std::map<Client*, bool>::iterator it = this->_members.begin(); it != this->_members.end(); ++it)
		if (it->first == c)
			return (it->first);

	return (NULL);
}

bool		Channel::getClientStatus(Client *c) {

	for (std::map<Client*,bool>::iterator it = this->_members.begin(); it != this->_members.end(); ++it)
		if (it->first == c)
			return (it->second);

	return (false);
}

void		Channel::supprInvitation(Client *c) {
	
	for (std::vector<std::string>::iterator it = this->_userInvited.begin(); it != this->_userInvited.end(); ++it) {
		if (*it == c->getNick()) {
			this->_userInvited.erase(it);
			return ;
		}
	}
}

std::string	Channel::nameReply(std::string nick) {

	std::string	rply = ":InesRuguduka.congo 353 " + nick;

	if (this->_inviteOnly || this->_keySet)
		rply += " * ";
	else
		rply += " = ";

	rply += this->_name + " :";

	for (std::map<Client*,bool>::iterator it = this->_members.begin(); it != this->_members.end();) {

		if (it->second)
			rply += "@";
		rply += it->first->getNick();

		++it;

		if (it != this->_members.end())
			rply += " ";

	}
	rply += "\r\n";

	return (rply);
}

void	Channel::sendChannelInfo(Client *c)
{
	std::string joinPrompt = c->makePrefix("JOIN") + this->_name + "\r\n";
	std::string modePrompt = c->makePrefix("MODE") + this->_name + "+o" + c->getNick() + "\r\n";
	write(c->getSocket(), joinPrompt.c_str(), joinPrompt.length());
	write(c->getSocket(), nameReply(c->getNick()).c_str(), nameReply(c->getNick()).length());
	std::string prompt = endOfNames(c->getNick(), this->_name);
	write(c->getSocket(), prompt.c_str(), prompt.length());
}

void	Channel::sendNewMemberInfo(Client *c)
{
	std::string prompt = c->makePrefix("JOIN") + this->_name + "\r\n";
	std::string eofnames = endOfNames(c->getNick(), this->_name);
	sendMsgToAllMembers(prompt);
	write(c->getSocket(), nameReply(c->getNick()).c_str(), nameReply(c->getNick()).length());
	write(c->getSocket(), eofnames.c_str(), eofnames.length());
}

void	Channel::sendModeInfo(Client *c, std::string mode)
{
	std::string	prompt = c->makePrefix("MODE") + this->_name + " ";

	prompt += mode + "\r\n";
	sendMsgToAllMembers(prompt);
}
