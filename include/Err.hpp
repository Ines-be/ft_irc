#ifndef ERR_HPP
# define ERR_HPP

# include <iostream>

inline std::string noSuchNick(const std::string& nick, const std::string& targetNick) {
	return "401 " + nick + " " + targetNick + " :No such nick/channel";
}

inline std::string noSuchChannel(const std::string& nick, const std::string& channelName) {
    return ("403 " + nick + " " + channelName + " :No such channel");
}

inline std::string cannotSendChan(const std::string& nick, const std::string& channel) {
	return "404 " + nick + " " + channel + " :Cannot send to channel";
}

inline std::string noRecipient(const std::string& nick, const std::string& command) {
	return "411 " + nick + " :No recipient given (" + command + ")";
}

inline std::string noTextToSend(const std::string& nick) {
	return "412 " + nick + " :No text to send";
}

inline std::string inputTooLong(const std::string& nick) {
	return "417 " + nick + " :Input line was too long";
}

inline std::string unknownCommand(const std::string& nick, const std::string& command) {
	return "421 " + nick + " " + command + " :Unknown command";
}

inline std::string noNickNameGiven(const std::string& nick = "*") {
	return "431 " + nick + " :No nickname given";
}

inline std::string erroneusNickname(const std::string& nick) {
	return "432 " + nick + " :Erroneous nickname";
}

inline std::string alreadyRegistered(const std::string& nick) {
    return "462 " + nick + " :You may not reregister";
}

inline std::string nickNameInUse(const std::string& nick) {
	return "433 * " + nick + " :Nickname is already in use";
}

inline std::string userNotInChannel(const std::string& nick, const std::string& channel) {
	return "441 " + nick + " " + channel + " :They aren't on that channel";
}

inline std::string notOnChannel(const std::string& nick, const std::string& channel) {
	return "442 " + nick + " " + channel + " :You're not on that channel";
}

inline std::string userOnChannel(const std::string& requester, const std::string& user, const std::string& channel) {
	return "443 " + requester + " " + user + " " + channel + " :is already on channel";
}

inline std::string needMoreParams(const std::string& nick, const std::string& cmd) {
	return "461 " + nick + " " + cmd + " :Not enough parameters";
}

inline std::string passwdMismatch(const std::string& nick) {
    return "464 " + nick + " :Password incorrect";
}

inline std::string channelIsFull(const std::string& nick, const std::string& channel) {
    return "471 " + nick + " " + channel + " :Cannot join channel (+l)";
}

inline std::string unknownMode(char modeChar, const std::string& nick) {
    return "472 " + nick + " " + modeChar + " :is unknown mode char to me";
}

inline std::string inviteOnlyChan(const std::string& nick, const std::string& channel) {
    return "473 " + nick + " " + channel + " :Cannot join channel (+i)";
}

inline std::string badChannelKey(const std::string& nick, const std::string& channel) {
    return "475 " + nick + " " + channel + " :Cannot join channel (+k)";
}

inline std::string badChanMask(const std::string& nick, const std::string& channel) {
    return "476 " + nick + " " + channel + " :Bad Channel Mask";
}

inline std::string chanOpIsNeeded(const std::string& nick, const std::string& channel) {
    return "482 " + nick + " " + channel + " :You're not channel operator";
}

#endif