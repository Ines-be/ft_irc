#include "../include/Server.hpp"

void	Server::execFunc(Client *c, std::string prompt)
{
	std::size_t	keySize = prompt.find(' ') + 1;
	if (keySize == std::string::npos)
		return ;

	std::map<std::string, cmdFunc>::iterator it = funcMap.find(prompt.substr(0, keySize));

	if (it != funcMap.end() && it->first.find(prompt.substr(0, keySize)) == 0)
		(this->*(it->second))(c, prompt);
	else
		throw Server::err(unknownCommand(c->getNick(), prompt));
}

void	Server::passCmd(Client *c, std::string msg) {

	trimCmdName(c->getNick(), msg, "PASS");

	try {
		if (c->getPass())
			throw Server::err(alreadyRegistered(c->getNick()));
		if (msg == this->_password) {
			c->setPass(true);
			c->incrValid_op();
			write(c->getSocket(), ":server NOTICE Auth :..**Correct password**.. \r\n", 48);
			return ;
		}
		if (msg.empty())
			throw Server::err(needMoreParams(c->getNick(), "PASS"));

		throw Server::err(passwdMismatch(c->getNick()));
		
	}
	catch (const std::exception& e) {
		write(c->getSocket(), e.what(), std::strlen(e.what()));
	}
}

void	Server::nickCmd(Client *c, std::string msg) {

	trimCmdName(c->getNick(), msg, "NICK");

	try {
		checkNickname(msg);
	
		if (c->getNick().empty())
			c->incrValid_op();
		c->setNick(msg);

		std::string displayMsg = ":server NOTICE Auth :..**Nickname set to " + c->getNick() + " **.. \r\n";
		write(c->getSocket(), displayMsg.c_str(), displayMsg.length());
	}
	catch (const std::exception& e) {
		write(c->getSocket(), e.what(), std::strlen(e.what()));
	}
}

void	Server::userCmd(Client *c, std::string msg) {

	trimCmdName(c->getNick(), msg, "USER");

	try {
		if (msg.empty())
			throw Server::err(needMoreParams(c->getNick(), "USER"));
		if (c->getAuth())
			throw Server::err(alreadyRegistered(c->getNick()));

		std::vector<std::string> spaceCheck = split(msg, ' ');
		
		if (c->getUser().empty())
			c->incrValid_op();
		c->setUser(spaceCheck[0]);

		std::string displayMsg = ":server NOTICE Auth :..**Username set to " + c->getUser() + " **.. \r\n";
		write(c->getSocket(), displayMsg.c_str(), displayMsg.length());
	}
	catch (const std::exception& e) {
		write(c->getSocket(), e.what(), std::strlen(e.what()));
	}
}

bool	Server::joinChan(Client *c, std::string name, std::string pass) {
	try
	{
		for (std::vector<Channel*>::iterator n = this->_channels.begin(); n != this->_channels.end(); ++n) {
			if ((*n)->getName() == name) {
				if ((*n)->getKeySet() && (pass.empty() || ((*n)->getPassword() != pass)))
					throw Server::err(badChannelKey(c->getNick(), name));
				(*n)->addMember(c);
				return (true);
			}
		}

		return (false);
	}
	catch (const std::exception& e) {
		write(c->getSocket(), e.what(), std::strlen(e.what()));
		return (true);
	}
}

void	Server::parseJoinCmd(std::vector<std::string> *info, std::string msg) {

	info[0] = split(msg, ' ');

	if (info[0].size() == 1)
		info[0].push_back("");

	info[1] = split(info[0][0], ',');
	info[2] = split(info[0][1], ',');

	if (info[2].size() < info[1].size())
		while (info[2].size() != info[1].size())
			info[2].push_back("");
}

void	Server::joinCmd(Client *c, std::string msg) {

	trimCmdName(c->getNick(), msg, "JOIN");

	if ((msg == "0" || msg == "#0") && leaveAllChannel(c))
		return ;

	std::vector<std::string>	info[3];

	parseJoinCmd(info, msg);

	for (std::vector<std::string>::iterator it = info[1].begin(), key = info[2].begin();
		it != info[1].end() && key != info[2].end(); ++it, ++key) 
	{
		try {
			if (!validChannelName(*it))
				throw Server::err(badChanMask(c->getNick(), *it));
			if (joinChan(c, *it, *key))
				continue;

			Channel		*nvx = new Channel(c, *it);

			this->_channels.push_back(nvx);
			c->addChannel(nvx);

			c->rpl(creationTime(c->getNick(),nvx->getName(), nvx->getTime()));
		}
		catch (const std::exception& e) {
			write(c->getSocket(), e.what(), std::strlen(e.what()));
		}
	}
}

bool	Server::leaveAllChannel(Client *c) {
	std::vector<Channel*>	ref = c->getChannels();
	for (std::vector<Channel*>::iterator it = ref.begin(); it != ref.end(); ++it)
		leaveChannel(c, *it);
	return (true);
}

void	Server::kickCmd(Client *c, std::string msg) {

	trimCmdName(c->getNick(), msg, "KICK");

	std::vector<std::string>	info[4];

	parseKickCmd(c->getNick(), info, msg);

	for (std::vector<std::string>::iterator it = info[2].begin(); it != info[2].end(); ++it) {
		try {
			Channel	*ref = NULL;

			validChannelName(*it) ? : throw Server::err(badChanMask(c->getNick(), *it));
			c->getChanFromClientVector(*it) ? ref = c->getChanFromClientVector(*it) : throw Server::err(notOnChannel(c->getNick(), *it));

			if (ref->findClient(c) && !ref->getClientStatus(c))
				throw Server::err(chanOpIsNeeded(c->getNick(), ref->getName()));

			execKick(info, ref, c, msg);
		}
		catch (const std::exception& e) {
			write(c->getSocket(), e.what(), std::strlen(e.what()));
		}
	}
}

void	Server::parseKickCmd(std::string nick, std::vector<std::string> *info, std::string msg) {

	info[0] = split(msg, ':');
	info[1] = split(msg, ' ');

	if (info[1].size() < 2)
		throw Server::err(needMoreParams(nick, "KICK"));

	info[2] = split(info[1][0], ',');
	info[3] = split(info[1][1], ',');
}

void	Server::execKick(std::vector<std::string> *info, Channel *ref, Client *c, std::string msg) {
	
	for (std::vector<std::string>::iterator r = info[3].begin(); r != info[3].end(); ++r) {
		try {
			Client	*auth = NULL;

			ref->findClient(getClientFromVector(*r)) ? auth = ref->findClient(getClientFromVector(*r)) : throw Server::err(userNotInChannel(c->getNick(), ref->getName()));

			msg = ":" + c->getNick() + "!" + c->getUser() + "@localhost KICK " + ref->getName() + " " + auth->getNick();
			if (info[0].size() == 2)
				msg += " " + info[0][1];
			msg += "\r\n";
			leaveChannel(auth, ref, msg);
		}
		catch (const std::exception& e ) {
			write(c->getSocket(), e.what(), std::strlen(e.what()));
		}
	}
}

void	Server::modeCmd(Client *c, std::string msg) {

	trimCmdName(c->getNick(), msg, "MODE");

	std::vector<std::string>	argv = split(msg, ' ');

	try {
		Channel		*ref = c->getChanFromClientVector(argv[0]);
		bool		sign;

		if (!ref)
			throw Server::err(notOnChannel(c->getNick(), argv[0]));
		std::map<Client*,bool>	chan = ref->getMembers();

		if (!ref->getClientStatus(c))
			throw Server::err(chanOpIsNeeded(c->getNick(), ref->getName()));

		if (argv.size() == 1) {
			c->rpl(ref->channelModeIs(c->getNick(), ref->getName()));
			return ;
		}

		for (int i = 1; argv[1][i]; ++i) {
			try {
				ref->validMode(argv[1], &sign);
				switch (argv[1][i]) {
					case 'i':
						ref->setInviteOnly(sign, c);
						break;
					case 'l':
						if (!sign)	
							ref->delUserLimit(c);
						else
							(argv.size() == 3) ? ref->setUserLimit(argv[2], c) : throw Server::err(needMoreParams(c->getNick(), "MODE"));
						break;
					case 't':
						ref->setTopicRestriction(sign, c);
						break;
					case 'k':
						(sign && argv.size() > 2) || !sign ? ref->setPassword(sign, argv[2], c) : throw Server::err(needMoreParams(c->getNick(), "MODE"));
						break;
					case 'o':
						(argv.size() == 3) ? ref->setOperator(c, sign, argv[2]) : throw Server::err(needMoreParams(c->getNick(), "MODE"));
						break;
					default:
						throw Server::err(unknownMode(argv[1][i], c->getNick()));
				}
			}
			catch (const std::exception& e) {
				write(c->getSocket(), e.what(), std::strlen(e.what()));
			}
		}

	}
	catch (const std::exception& e) {
		write(c->getSocket(), e.what(), std::strlen(e.what()));
	}
}

void	Server::inviteCmd(Client *c, std::string msg) {

	trimCmdName(c->getNick(), msg, "INVITE");

	std::vector<std::string>	argv = split(msg, ' ');

	if (argv.size() < 2)
		throw Server::err(needMoreParams(c->getNick(), "INVITE"));

	try {
		Channel		*ref;
		Client		*auth;
		
		c->getChanFromClientVector(argv[1]) ? ref = c->getChanFromClientVector(argv[1]) : throw Server::err(notOnChannel(c->getNick(), argv[1]));
		getClientFromVector(argv[0]) ? auth = getClientFromVector(argv[0]) : throw Server::err(noSuchNick(c->getNick(), argv[0]));

		std::map<Client*,bool>	chan = ref->getMembers();
		
		if (ref->getInviteOnly())
			for (std::map<Client*,bool>::iterator it = chan.begin(); it != chan.end(); ++it)
				if (it->first->getSocket() == c->getSocket() && !it->second)
					throw Server::err(chanOpIsNeeded(c->getNick(), ref->getName()));

		!ref->findClient(auth) ? : throw Server::err(userOnChannel(auth->getNick(), c->getNick(), ref->getName()));
		
		ref->inviteMember(c, auth);
	}
	catch (const std::exception& e) {
		write(c->getSocket(), e.what(), std::strlen(e.what()));
	}
}

void	Server::sendMsgToAllRecipients(Client *c, std::vector<std::string> &tmp)
{
	std::vector<std::string> recipients = split(tmp.front(), ',');

	while (!recipients.empty())
	{
		try
		{
			trimEndSpace(recipients.front());
			Channel *chan = c->getChanFromClientVector(recipients.front());
			Client	*user = this->getClientFromVector(recipients.front());
			std::string msg;

			if (chan) {
				if (this->_kickerBot.checkBadWords(this, chan, c, tmp.back()))
					return ;
				std::string msg = c->makePrefix("PRIVMSG") + chan->getName() + " :" + tmp.back() + "\r\n";
				chan->sendMsg(c, msg);
			}
			else if (user) {
				std::string msg = c->makePrefix("PRIVMSG") + user->getNick() + " :" + tmp.back() + "\r\n";
				write(user->getSocket(), msg.c_str(), msg.length());
			}
			else
				throw Server::err(noSuchNick(c->getNick(), recipients.front()));
		}
		catch (const std::exception& e) {
			write(c->getSocket(), e.what(), std::strlen(e.what()));
		}
		recipients.erase(recipients.begin());
	}
}

void	Server::privMsgCmd(Client *c, std::string msg) {

	trimCmdName(c->getNick(), msg, "PRIVMSG");
	
	std::string fullPrompt = msg;

	try
	{
		if (msg.empty())
			throw Server::err(noRecipient(c->getNick(), "PRIVMSG"));

		std::vector<std::string> tmp = split(msg, ':');
		if (tmp.size() == 1)
			throw Server::err(noTextToSend(c->getNick()));
		else if (tmp.size() > 2)
			throw Server::err("PRIVMSG: ERR_INCORRECTUSE (!!!)\r\nExample: PRIVMSG #minishell :hello!");
		
		sendMsgToAllRecipients(c, tmp);
	}
	catch(const std::exception& e) {
		write(c->getSocket(), e.what(), std::strlen(e.what()));
	}
}

void	Server::partCmd(Client *c, std::string msg) {


	trimCmdName(c->getNick(), msg, "PART");

	std::vector<std::string>	data = split(msg, ':');
	std::vector<std::string>	channel = split(data[0], ',');

	for (std::vector<std::string>::iterator it = channel.begin(); it != channel.end(); ++it) {

		try {
			Channel	*ref = NULL;
			Client	*auth = NULL;

			trimEndSpace(*it);
			ref = getChannelFromVector(*it);
			if (!ref)
				throw Server::err(noSuchChannel(c->getNick(), *it));

			auth = ref->findClient(c);
			if (!auth)
				throw Server::err(notOnChannel(c->getNick(), ref->getName()));
			
			std::string prompt = c->makePrefix("PART") + ref->getName();
			if (data.size() == 2)
				prompt += " " + data[1];
			prompt +=  "\r\n";
			leaveChannel(auth, ref, prompt);
		}
		catch (const std::exception& e) {
			write(c->getSocket(), e.what(), std::strlen(e.what()));
		}
	}
}

void	Server::quitCmd(Client *leaving, std::string msg) {

	std::string clientMsg = ":" + leaving->getNick() + "!" + leaving->getUser() + "@localhost ";
	clientMsg += msg + "\r\n";

	std::vector<Channel*> clientChans = leaving->getChannels();
	for (std::vector<Channel*>::iterator it = clientChans.begin(); it != clientChans.end(); ++it)
		this->leaveChannel(leaving, *it, clientMsg);

	FD_CLR(leaving->getSocket(), &this->_trackFds);
	eraseClientFromVector(leaving->getSocket());

	delete leaving;
}

void	Server::pingCmd(Client *c, std::string msg) {

	std::string prompt = "PONG localhost\r\n";
	write(c->getSocket(), prompt.c_str(), prompt.length());
	msg.clear();
}

void	Server::topicCmd(Client *c, std::string msg) {

	trimCmdName(c->getNick(), msg, "TOPIC");

	bool	toChangeTopic = false;
	if (msg.find(':') != std::string::npos)
		toChangeTopic = true;
	
	try
	{
		if (!toChangeTopic)
			viewTopic(c, msg);
		else
			changeTopic(c, msg);
	}
	catch (const std::exception& e) {
		write(c->getSocket(), e.what(), std::strlen(e.what()));
	}
}

void	Server::viewTopic(Client *c, std::string &msg)
{
	Channel *servChan;
	Channel *clientChan;

	trimEndSpace(msg);
	servChan = this->getChannelFromVector(msg);
	clientChan = c->getChanFromClientVector(msg);
	if (!servChan)
		throw Server::err(noSuchChannel(c->getNick(), msg));

	else if (servChan && !clientChan)
		throw Server::err(notOnChannel(c->getNick(), msg));

	if (clientChan->getTopic().empty())
		c->rpl(noTopic(c->getNick(), clientChan->getName()));
	else {
		c->rpl(topic(c->getNick(), clientChan->getName(), " :" + clientChan->getTopic()));
		c->rpl(topicWhoTime(c->getNick(), clientChan->getName(), clientChan->getUserTopicChanged(), clientChan->getTimeTopicChanged()));
	}
}

void	Server::changeTopic(Client *c, std::string &msg)
{
	std::vector<std::string>	tmp = split(msg, ':');

	trimEndSpace(tmp.front());
	Channel *servChan = getChannelFromVector(tmp.front());
	Channel *clientChan = c->getChanFromClientVector(tmp.front());

	if (!servChan)
		throw Server::err(noSuchChannel(c->getNick(), tmp.front()));
	else if (servChan && !clientChan)
		throw Server::err(notOnChannel(c->getNick(), tmp.front()));
	else if (clientChan->getTopicRestriction() && clientChan->findClient(c) && !clientChan->getClientStatus(c))
		throw Server::err(chanOpIsNeeded(c->getNick(), tmp.front()));

	if (tmp.size() > 2)
		throw Server::err("TOPIC: ERR_INCORRECTUSE (!!!)\r\n");

	clientChan->setTimeTopic(std::time(NULL));
	clientChan->setUserTopic(c->getNick());

	if (tmp.size() == 1 || tmp.back().empty())
		clientChan->setTopic("");
	else if (tmp.size() == 2)
		clientChan->setTopic(tmp.back());

	std::string prompt = topic(c->getNick(), clientChan->getName(), tmp.back()) + "\r\n";
	clientChan->sendMsgToAllMembers(prompt);
	
	prompt = topicWhoTime(c->getNick(), clientChan->getName(), clientChan->getUserTopicChanged(), clientChan->getTimeTopicChanged()) + "\r\n";
	clientChan->sendMsgToAllMembers(prompt);
}
