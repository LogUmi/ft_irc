#include "../inc/Server.hpp"
#include "../inc/User.hpp"
#include "../inc/Channel.hpp"
#include <string>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <csignal>

bool	server_stop = false;
bool	debug = false;

void	signalHandler(int signum)
{
	(void)signum;
	server_stop = true;
}

bool	valid_str(std::string& str)
{
	for (size_t i = 0; i < str.length(); ++i)
		if (!std::isalnum(str[i]) && str[i] != '-' && str[i] != '_' && str[i] != '.')
			return (false);
	return (true);
}

int	main(int c, char **v)
{
	//basic args check
	if (c != 3)
	{
		std::cout << "Invalid args\nNeeds ./ft_irc <port> <password>"
			<< std::endl;
		return (0);
	}
	//port arg check
	errno = 0;
	char* end = NULL;
	long port = std::strtol(v[1], &end, 10);
	if (errno == ERANGE || *end != '\0' || port <= 1024 || port > 65535)
	{
		std::cout << "Invalid port >1024 && < 65536" << std::endl;
		return (0);
	}
	//password arg check
	std::string password = v[2];
	if (!valid_str(password))
	{
		std::cout << "Password not valid" << std::endl;
			std::cout << "isalnum or '-' '_' '.'";
		return (1);
	}
	//signal handler
	std::signal(SIGINT, signalHandler);
	std::signal(SIGQUIT, signalHandler);
	//main run
	try
	{
		Server serv(port, password);
		serv.start();
	}
	catch (std::exception &e)
	{
		std::cout << e.what() <<  std::endl;
	}
	return (0);
}
