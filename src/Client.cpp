#include "../include/Client.hpp"

Client::Client(int fd) : _nick(""), _user(""), _socket(fd), _auth(false), _password(false), _operation_valid(0) {
	_msgBuffer.clear();
}

Client::~Client() {
}

std::string	Client::getBuffer() {
	return (this->_msgBuffer);
}

std::string	Client::getNick() const {
	return (this->_nick);
}

int	Client::getSocket() const {
	return (this->_socket);
}

bool	Client::getAuth() const {
	return (this->_auth);
}

bool	Client::getPass() const {
	return (this->_password);
}

int	Client::getValid_op() const {
	return (this->_operation_valid);
}

std::vector<Channel*>	Client::getChannels() {
	return (this->_channels);
}

std::string	Client::getUser() const {
	return (this->_user);
}

void	Client::setAuth(bool data) {
	this->_auth = data;
}

void	Client::setPass(bool data) {
	this->_password = data;
}

void	Client::setNick(std::string data) {
	this->_nick = data;
}

void	Client::setUser(std::string data) {
	this->_user = data;
}

void	Client::setBuffer(std::string data) {
	this->_msgBuffer = data;
}

void	Client::setSocket(int fd) {
	this->_socket = fd;
}

void	Client::addChannel(Channel* chan) {
	this->_channels.push_back(chan);
}

void	Client::delChannel(Channel* chan) {
	
	for (std::vector<Channel*>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it) {
		if ((*it) == chan) {
			this->_channels.erase(it);
			return;
		}
	}
}

bool	Client::isInvited(Channel *ref) const {

	std::vector<std::string>	list = ref->getUserInvited();

	for (std::vector<std::string>::iterator it = list.begin(); it != list.end(); ++it)
		if (*it == this->_nick)	
			return (true);

	return (false);
}

Channel	*Client::getChanFromClientVector(std::string name) {

	for (std::vector<Channel*>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it)
		if ((*it)->getName() == name)
			return (*it);

	return (NULL);
}

void	Client::rpl(std::string reply) {

	reply.insert(0, ":InesRuguduka.congo ");
	reply += "\r\n";
	write (this->_socket, reply.c_str(), reply.length());
}

void	Client::addToBuffer(std::string msg) {
	this->_msgBuffer += msg;
}

void	Client::incrValid_op() {
	this->_operation_valid++;
}

std::string	Client::makePrefix(std::string cmdName)
{
	return (":" + this->getNick() + "!" + this->getUser() + "@localhost " + cmdName + " ");
}