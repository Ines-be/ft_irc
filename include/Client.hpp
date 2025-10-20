#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <vector>
#include "Server.hpp"
#include "Channel.hpp"

class Channel;
class Server;

class	Client
{
	private:
		std::string	_nick;
		std::string	_user;
		int		_socket;
		bool		_auth;
		bool		_password;
		int		_operation_valid;
		std::string	_msgBuffer;
		std::vector<Channel*>	_channels;

	public:
		Client(int fd);
		~Client();

		std::string	getNick() const;
		bool		getAuth() const;
		bool		getPass() const;
		int		getSocket() const;
		int		getValid_op() const ;
		std::vector<Channel*>	getChannels();
		Channel		*getChanFromClientVector(std::string name);

		void		setAuth(bool data);
		void		setPass(bool data);
		void		setNick(std::string data);
		void		setUser(std::string data);
		std::string	getBuffer();
		void		addToBuffer(std::string msg);
		void		setBuffer(std::string data);
		
		std::string	getUser() const;
		void		setSocket(int fd);
		void		incrValid_op();
		void		addChannel(Channel* chan);
		void		delChannel(Channel* chan);		
		
		void		rpl(std::string reply);
		bool		isInvited(Channel *ref) const ;
		std::string	makePrefix(std::string cmdName);
};

#endif