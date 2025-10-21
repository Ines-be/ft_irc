#include "../include/Server.hpp"

bool Server::_signal = false;

Server::Server(int port, std::string password) : _port(port), _socket(-1), _nClients(0) , _exitFlag(0), _password(password), _kickerBot() ,_time(std::time(NULL)) {

	FD_ZERO(&this->_trackFds);
	FD_ZERO(&this->_readFds);

	funcMap["NICK "] = &Server::nickCmd;
	funcMap["PASS "] = &Server::passCmd;
	funcMap["USER "] = &Server::userCmd;
	funcMap["JOIN "] = &Server::joinCmd;
	funcMap["KICK "] = &Server::kickCmd;
	funcMap["PING "] = &Server::pingCmd;
	funcMap["MODE "] = &Server::modeCmd;
	funcMap["PART "] = &Server::partCmd;
	funcMap["INVITE "] = &Server::inviteCmd;
	funcMap["PRIVMSG "] = &Server::privMsgCmd;
	funcMap["TOPIC "] = &Server::topicCmd;
	this->_clients.clear();
	this->_channels.clear();
}

Server::~Server() {

	close (this->_socket);
	this->_socket = -1;

	for (std::vector<Channel*>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it)
		delete(*it);

	for (std::vector<Client*>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
		delete(*it);
}

bool	Server::getSignal() {
	return (_signal);
}

int	Server::getPort() const {
	return (this->_port);
}

const std::string &Server::getPassword() const {
	return (this->_password);
}

int	Server::getSocket() const {
	return (this->_socket);
}

sockaddr_in &Server::getSockaddr() {
	return (this->_add);
}

void	Server::setSignal(bool data) {
	_signal = data;
}

Server::err::err(std::string data) : _error(":InesRuguduka.congo " + data + "\r\n") {}

Server::err::~err() throw() {}

const char *Server::err::what() const throw() {
	return (this->_error.c_str());
}

bool	Server::run()
{
	try {
		this->createSocket();
		this->bindSocketToNetwork();
		this->listenForClients();
		this->handleSockets();
		this->closeSockets();

		return true;
	}
	catch(const std::exception& e) {
		std::cerr << e.what();
		return false;
	}
}

void	Server::createSocket()
{
	struct protoent* proto = getprotobyname("tcp");
	if (!proto)
		throw Server::err("Error: getprotobyname() fail");
	int fd = socket(AF_INET, SOCK_STREAM, proto->p_proto);
	if (fd < 0)
		throw Server::err("Error: socket() fail");

	this->_socket = fd;
	FD_SET(this->_socket, &this->_trackFds);
}

// bind le socket au port et a toutes les interfaces du reseau 0.0.0.0
void	Server::bindSocketToNetwork()
{
	sockaddr_in add;
	add.sin_family = AF_INET;
	add.sin_port = htons(this->getPort());
	add.sin_addr.s_addr = inet_addr("0.0.0.0"); // ou INADDR_ANY mais c'est - lisible

	this->_add = add;

	if (bind(this->_socket, reinterpret_cast<struct sockaddr *>(&this->_add), sizeof(this->_add)) < 0)
		throw Server::err("Error: bind() fail. Make sure you used a valid port.");
}

// met le socket en mode ecoute (on attend les connexions entrantes)
void	Server::listenForClients()
{
	// SOMAXCONN = 128, c'est le max possible si on met + ca modifie en 128 tout seul, et on a aucune raison de mettre moins dans ce contexte en soit
	if (listen(this->getSocket(), SOMAXCONN) < 0)
		throw Server::err("Error: listen() fail");
}

void	Server::handleSockets()
{
	int res = 0;
	int nfds = 1;

	while (!this->_signal)
	{
		if (_exitFlag || this->_signal)
			break ;
		_readFds = _trackFds;
		nfds = std::max(this->_socket, maxClientFd());
		res = select(nfds + 1, &this->_readFds, NULL, NULL, NULL);
		if (res < 0)
			continue;
		for (int i = 0; i <= nfds; i++) {
			if (FD_ISSET(i, &_readFds) && i == this->_socket && !this->_signal)
				acceptClients();
			else if (FD_ISSET(i, &_readFds) && i != this->_socket && !this->_signal)
				handleClient(i);
		}
	}
}

void	Server::closeSockets()
{
	for (std::vector<Client*>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		if ((*it)->getSocket() > 0)
			close((*it)->getSocket());
		(*it)->setSocket(-1);
	}
	close(this->getSocket());
}

void	Server::getIrssiInfo(int clientSocket)
{
	try {
		while (1) {
			Client		*auth = getClientFromVector(clientSocket);
			std::string	data = retrieveMessage(clientSocket);
			try {
				std::vector<std::string>	tmp = split(data, '\n');
				if (tmp.size() < 3)
					throw Server::err("Not connected to server");

				if (!std::strncmp(tmp[1].c_str(), "PASS ", 5))
					passCmd(auth, tmp[1]);
				if (!auth->getPass())
					write(clientSocket , ":server NOTICE Auth :**PASS required**\r\n", 40);
				
				if (!std::strncmp(tmp[2].c_str(), "NICK ", 5))
					nickCmd(auth, tmp[2]);
				if (!std::strncmp(tmp[3].c_str(), "USER ", 5))
					userCmd(auth, tmp[3]);
				if (auth->getValid_op() == 3)
					welcomeClient(clientSocket);
				auth->setBuffer("");
			}
			catch (const std::exception& e) {
				if (getClientFromVector(clientSocket))
					getClientFromVector(clientSocket)->setBuffer("");
				std::cerr << e.what() << std::endl;
			}
		}
	}
	catch (const std::exception& e) {
		if (getClientFromVector(clientSocket))
			getClientFromVector(clientSocket)->setBuffer("");
		std::cerr << e.what() << std::endl;
	}
}

void	Server::acceptClients()
{
	socklen_t addSize = sizeof(this->getSockaddr());
	int clientSocket = accept(this->getSocket(), reinterpret_cast<struct sockaddr *>(&this->getSockaddr()), &addSize);
	if (clientSocket < 0)
		throw Server::err("Error: accept() fail");

	fcntl(clientSocket, F_SETFL, O_NONBLOCK); // on ajoute le flag pour rendre le fd non bloquant

	write(clientSocket, "Connection to server successful.\r\nEnter these commands to process. \r\n", 69);
	write(clientSocket, ":server NOTICE Auth :1. ..**PASS <password>**.. \r\n", 50);
	write(clientSocket, ":server NOTICE Auth :2. ..**NICK <nickname>**.. \r\n", 50);
	write(clientSocket, ":server NOTICE Auth :3. ..**USER <username>**.. \r\n", 50);

	this->_nClients++;
	Client	*nvx = new Client(clientSocket);

	this->_clients.push_back(nvx);
	FD_SET(clientSocket, &this->_trackFds); // ajouter clientSocket a la liste des fds surveilles par select()

	this->getIrssiInfo(clientSocket);	
}

void	Server::welcomeClient(int clientSocket)
{
	Client		*auth = getClientFromVector(clientSocket);

	std::ostringstream	oss;
	oss << this->_time;

	auth->rpl(welcome(auth->getNick()));
	auth->rpl(yourHost(auth->getNick()));
	auth->rpl(created(auth->getNick(), oss.str()));
	auth->rpl(myInfo(auth->getNick()));
	auth->setAuth(true);
}

void	Server::authentificate(int clientSocket) {

	try {
		Client		*auth = getClientFromVector(clientSocket);
		std::string	msg = retrieveMessage(clientSocket);

		auth->addToBuffer(msg);
		std::cout << "raw: '" << auth->getBuffer() << "'\r\n";
		if (!(auth->getBuffer().length() >= 2 && auth->getBuffer().substr(auth->getBuffer().length() - 2, auth->getBuffer().length()) == "\r\n"))
			return ;
		std::string	tmp = auth->getBuffer().substr(0, auth->getBuffer().size() - 2);
		if (!std::strncmp(tmp.c_str(), "PASS ", 5))
			passCmd(auth, tmp);
		else if (!auth->getPass())
			write(clientSocket , ":server NOTICE Auth :**PASS required\r\n", 38);
		else if (!std::strncmp(tmp.c_str(), "NICK ", 5))
			nickCmd(auth, tmp);
		else if (!std::strncmp(tmp.c_str(), "USER ", 5))
			userCmd(auth, tmp);
		if (auth->getValid_op() == 3)
			welcomeClient(clientSocket);
		auth->setBuffer("");
	}
	catch (const std::exception& e) {
		if (getClientFromVector(clientSocket))
			getClientFromVector(clientSocket)->setBuffer("");
		write(clientSocket, e.what(), std::strlen(e.what()));
	}
}

void	Server::handleClient(int fd)
{
	if (getClientFromVector(fd) && !getClientFromVector(fd)->getAuth())
		authentificate(fd);
	else
	{
		try
		{
			this->receiveFromClients(fd);
		}
		catch(const std::exception& e)
		{
			if (getClientFromVector(fd))
					getClientFromVector(fd)->setBuffer("");
			write(fd, e.what(), std::strlen(e.what()));
			std::cerr << e.what() << '\n';
		}	
	}
}

void	Server::receiveFromClients(int fd)
{
	Client*		tmp = getClientFromVector(fd);
	std::string	msg = retrieveMessage(fd);

	std::string	prompt;

	tmp->addToBuffer(msg);
	if (!(tmp->getBuffer().length() >= 2 && tmp->getBuffer().substr(tmp->getBuffer().length() - 2, tmp->getBuffer().length()) == "\r\n"))
		return ;
	try {
		std::cout << "raw: '" << tmp->getBuffer() << "'\r\n";
		prompt = tmp->getBuffer().substr(0, tmp->getBuffer().size() - 2);
		
		if (prompt == "QUIT" || !strncmp("QUIT ", prompt.c_str(), 5)){
			quitCmd(tmp, prompt);
			return ;
		}
		else
			execFunc(tmp, prompt);
	}
	catch (const std::exception& e) {
		write(fd, e.what(), std::strlen(e.what()));
	}
	tmp->setBuffer("");
}

std::string	Server::retrieveMessage(int fd) {
	char	str[SIZE_MAX_MSG];

	memset(str, 0, SIZE_MAX_MSG);
	ssize_t bytesRecv = recv(fd, &str, sizeof(SIZE_MAX_MSG) - 1, 0);
	if (bytesRecv == -1)
		throw Server::err("Nothing to retrieve");
	if (bytesRecv < 0 || bytesRecv > 512)
	{
		Client *client = getClientFromVector(fd);
		
		if (bytesRecv > 512)
			throw Server::err(inputTooLong(client->getNick()));
		throw Server::err("Error: Problem while recv msg from client");
	}
	if (bytesRecv == 0) {
		Client		*auth = getClientFromVector(fd);

		std::cout << auth->getNick() << " disconnected\r\n";
		if (!_channels.empty())
		{
			std::vector<Channel*>	tmp = auth->getChannels();
			for (std::vector<Channel*>::iterator it = tmp.begin(); it != tmp.end(); ++it) // le supprime de tous les channels 
				this->leaveChannel(auth, *it);
		}
		eraseClientFromVector(fd);
		FD_CLR(fd, &this->_trackFds);
		delete auth;
		throw Server::err("");
	}
	return (std::string(str));
}

void	Server::sendMsgToAllClients(std::string msg)
{
    for (std::vector<Client*>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
        write((*it)->getSocket(), msg.c_str(), msg.length());
}

bool	Server::deleteEmptyChan(Channel *chan)
{
	if (chan->getNbrOfMembers() != 0)
		return false;

	for (std::vector<Channel*>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it) {
		if ((*it) == chan)
		{
			delete  *it;
			this->_channels.erase(it);
			return true;
		}
	}

	return false;
}

void	Server::leaveChannel(Client *c, Channel *chan)
{
	c->delChannel(chan);
	chan->delMember(c);

	this->deleteEmptyChan(chan);
}

void	Server::leaveChannel(Client *c, Channel *chan, std::string msg)
{
	if (!chan->findClient(c))
		return ;
	
	chan->sendMsgToAllMembers(msg);

	c->delChannel(chan);
	chan->delMember(c);
	this->deleteEmptyChan(chan);
}
