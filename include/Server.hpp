#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <cstring>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <ctime>
#include <sstream>
#include <sys/select.h>
#include "Client.hpp"
#include "Channel.hpp"
#include "Err.hpp"
#include "Rpl.hpp"
#include <vector>
#include <map>
#include <signal.h>
#include <cstdlib>
#include <algorithm>
#include <fcntl.h>
#include "Bot.hpp"

# define SIZE_MAX_MSG 10056

class Channel;
class Client;
class Bot;

class	Server
{
	private:
		int	_port;
		int	_socket;
		int	_nClients;
		int	_exitFlag;
		std::string	_password;
		std::vector<Client*>	_clients;
		std::vector<Channel*>	_channels;
		Bot	_kickerBot;
		sockaddr_in _add;
		fd_set	_trackFds;
		fd_set	_readFds;
		static bool	_signal;
		std::time_t		_time;

		typedef void (Server::*cmdFunc)(Client*, std::string);
		std::map<std::string, cmdFunc> funcMap;

	public:

		Server(int port, std::string password);
		~Server();

		static bool	getSignal();
		int	getPort() const;
		int	getSocket() const;
		sockaddr_in & getSockaddr();
		const std::string &	getPassword() const;

		static void	setSignal(bool data);

		void	createSocket();
		void	bindSocketToNetwork();
		void	listenForClients();
		int		maxClientFd();
		void	handleSockets();
		void	getIrssiInfo(int clientSocket);
		void	acceptClients();
		void	welcomeClient(int clientSocket);
		void	handleClient(int fd);
		bool	run();


		void	execFunc(Client *c, std::string prompt);
		void	execKick(std::vector<std::string> *info, Channel *ref, Client *c, std::string msg);
		void	parseJoinCmd(std::vector<std::string> *info, std::string msg);
		void	parseKickCmd(std::string nick, std::vector<std::string> *info, std::string msg);

		void	nickCmd(Client *c, std::string msg);
		void	userCmd(Client *c, std::string msg);
		void	passCmd(Client *c, std::string msg);
		void	joinCmd(Client *c, std::string msg);
		void	kickCmd(Client *c, std::string msg);
		void	modeCmd(Client *c, std::string msg);
		void	privMsgCmd(Client *c, std::string msg);
		void	inviteCmd(Client *c, std::string msg);
		void	partCmd(Client *c, std::string msg);
		void	quitCmd(Client *c, std::string msg);
		void	pingCmd(Client *c, std::string msg);
		void	topicCmd(Client *c, std::string msg);
		bool	joinChan(Client *c, std::string name, std::string pass);
		void	viewTopic(Client *c, std::string &msg);
		void	changeTopic(Client *c, std::string &msg);
		void	sendMsgToAllClients(std::string msg);
		void	sendMsgToAllRecipients(Client *c, std::vector<std::string> &tmp);

		std::string	retrieveMessage(int fd);
		void	receiveFromClients(int fd);
		void	authentificate(int clientSocket);
		void	closeSockets();
		bool	deleteEmptyChan(Channel *chan);
		void	leaveChannel(Client *c, Channel *chan);
		void	leaveChannel(Client *c, Channel *chan, std::string msg);
		bool	leaveAllChannel(Client *c);

		void		eraseClientFromVector(int clientSocket);
		Client		*getClientFromVector(int clientSocket);
		Client		*getClientFromVector(std::string nick);
		Channel		*getChannelFromVector(std::string name);
		void		checkNickname(std::string &msg);


		class err : public std::exception {
			private:
				const std::string    _error;

			public:
				err(std::string data);
				~err() throw() ;

				const char *what() const throw();
		};
};

std::vector<std::string>	split(std::string str, char c);
bool	validChannelName(std::string chan);
void	trimCmdName(const std::string& nick, std::string &msg, std::string cmdName);
void	trimEndSpace(std::string &str);
int		checkPort(char *av);
int		errorMsg(std::string msg);

#endif