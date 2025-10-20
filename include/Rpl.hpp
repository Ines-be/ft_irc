#ifndef RPL_HPP
# define RPL_HPP

# include "Server.hpp"

// RPL_WELCOME
inline std::string	welcome(std::string nick) {
	return ("001 " + nick + " :Welcome to the InesRugudukaCongo Network, " + nick + "!@localhost");
}

// RPL_YOURHOST
inline std::string	yourHost(std::string nick) {
	return ("002 " + nick + " :Your host is InesRuguduka.congo, running version 4.2");
}

// RPL_CREATED
inline std::string	created(std::string nick, std::string dateTime) {
	return ("003 " + nick + " :This server was created " + dateTime);
}

// RPL_MYINFO
inline std::string	myInfo(std::string nick) {
	return ("004 " + nick + " InesRuguduka.congo 1.0.0 o itkol kol");
}

// RPL_CREATIONTIME
inline std::string	creationTime(std::string nick, std::string channel, std::time_t dateTime) {

	std::ostringstream	oss;
	oss << dateTime;
	return ("329 " + nick + " " + channel + " " + oss.str());
}

// RPL_NOTOPIC
inline std::string	noTopic(std::string nick, std::string channel) {
	return ("331 " + nick + " " + channel + " :No topic is set");
}

// RPL_TOPIC
inline std::string	topic(std::string nick, std::string channel, std::string topic) {
	return ("332 " + nick + " " + channel + " " + topic);
}

// RPL_TOPICWHOTIME
inline std::string	topicWhoTime(std::string nick, std::string channel, std::string user,std::time_t lastChangedTime) {
	
	std::ostringstream	o;
	o << lastChangedTime;
	return ("333 " + nick + " " + channel + " " + user  + " " + o.str());
}

// RPL_INVITING
inline std::string	inviting(std::string nick, std::string invited, std::string channel) {
	return ("341 " + nick + " " + invited + " " + channel);
}

// RPL_ENDOFNAMES
inline std::string	endOfNames(std::string nick, std::string channel) {
	return ("366 " + nick + " " + channel + " :End of /NAMES list.\r\n");
}

#endif