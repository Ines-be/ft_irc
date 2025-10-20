#include "Server.hpp"

void	signalHandler(int sig)
{
	(void)sig;

	Server::setSignal(true);
	std::cout << "\r\nsignal received. Closing server.\r\n";
}

int	main(int ac, char **av)
{
	std::string test(av[1]);

	if (ac != 3)
		return (errorMsg("Usage: ./ircserv <port> <password>"));

	int port = checkPort(av[1]);
	if (port < 0)
		return (errorMsg("Invalid port. You have to use a free port between 1024-65535"));

	Server	irc(port, av[2]);

	signal(SIGINT, signalHandler);

	if (!irc.run())
		return (errorMsg("There was an issue with the server"));
}
