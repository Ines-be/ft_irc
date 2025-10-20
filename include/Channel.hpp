#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "Server.hpp"
#include <map>
#include <vector>

class Server;
class Client;

class	Channel
{
	private:
		std::map<Client*, bool>		_members;
		std::vector<std::string>	_userInvited;
		std::string		_name;
		std::string		_topic;
		std::string		_password;
		int			_nbrOfMembers;
		int			_capacity;
		bool			_inviteOnly;
		bool			_keySet;
		bool			_userLimit;
		bool			_topicRestriction;
		std::time_t		_time;
		std::time_t		_timeTopicChanged;
		std::string		_userTopicChanged;

	public:
		Channel(Client *auth, std::string data);
		~Channel();

		void		addMember(Client *c);
		void		delMember(Client *c);
		void		inviteMember(Client *c, Client *invited);
		void		supprInvitation(Client *c);

		std::map<Client*,bool>		getMembers() const ;
		std::string		getName() const ;
		std::string		getTopic() const ;
		std::string		getPassword() const ;
		int			getNbrOfMembers() const ;
		int			getCapacity() const ;
		bool			getInviteOnly() const ;
		bool			getKeySet() const ;
		bool			getUserLimit() const ;
		bool			getTopicRestriction() const ;
		
		std::time_t		getTime() const ;
		std::time_t		getTimeTopicChanged() const ;
		std::string		getUserTopicChanged() const ;

		void			setTimeTopic(std::time_t time);
		void			setUserTopic(std::string user);
		
		void		setPassword(bool sign, std::string data, Client *c);
		void		setInviteOnly(bool sign, Client *c);
		void		delUserLimit(Client *c);
		void		setUserLimit(std::string data, Client *c);
		void		setTopicRestriction(bool sign, Client *c);
		void		setOperator(Client *c, bool sign, std::string data);
		void		setTopic(std::string topic);
		void		validMode(std::string mode, bool* sign);
		void 		delegateOperator(Client *c);
		void		lastOperator(Client *c, bool sign);

		void		sendMsg(Client *c, std::string msg);
		void		sendMsgToAllMembers(std::string msg);


		void		deleteClient(Client *c);
		Client		*findClient(Client *c);
		bool		getClientStatus(Client *c);

		std::string	nameReply(std::string nick);
		std::vector<std::string>	getUserInvited();
		std::string	channelModeIs(std::string nick, std::string channel);
		void		sendChannelInfo(Client *c);
		void		sendNewMemberInfo(Client *c);
		void		sendModeInfo(Client *c, std::string mode);

};

#endif