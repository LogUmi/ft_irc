#include "../inc/Server.hpp"
#include <sys/socket.h>
#include <string>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>

extern bool	server_stop;
extern bool debug;

Server::Server(int port, const std::string& password) :
	_port(port),
	_password(password),
	_server_name("ft_irc.42.fr")
{
	//server socket init
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0)
		throw std::runtime_error("socket() failed");
	
	//Non-block config
	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(_server_fd);
		throw std::runtime_error("fcntl() failed");
	}

	//re-use port config
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt() failed");

	//binding server socket
	struct sockaddr_in serv_addr;
	std::memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(_port);
	if (bind(_server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		throw std::runtime_error("bind() failed");

	//server socket listening config
	if (listen(_server_fd, SOMAXCONN) < 0)
		throw std::runtime_error("listen() failed");

	//adding server socket to _poll_fds
	struct pollfd server_poll;
	server_poll.fd = _server_fd;
	server_poll.events = POLLIN;
	server_poll.revents = 0;
	_poll_fds.push_back(server_poll);

	//Commands instance
	_cmd = new Commands();
	_bufferOutLimit = 1024 * 128;

}

Server::~Server(void)
{
	for (size_t i = 0; i < _poll_fds.size(); ++i)
		if (_poll_fds[i].fd > 0)
			close(_poll_fds[i].fd);

	std::map<int, User*>::iterator it;
	for (it = _users.begin(); it != _users.end(); ++it)
		delete it->second;
	_users.clear();
	delete _cmd;
}

void	Server::start(void)
{
	std::cout << "Starting server " << _server_name << std::endl;
	while (server_stop == false)
	{

		//looking for fds ready
		if (poll(&_poll_fds[0], _poll_fds.size(), -1) < 0)
		{
			 if (errno == EINTR) // Ctrl+C -> poll interrompu
			 {
                std::cout << "\nServer stopped on manual interruption" << std::endl;
				continue;
			 }
			throw std::runtime_error("poll() failed");
		}
		//looking every _poll_fds

		for (size_t i = 0; i < _poll_fds.size(); ++i)
		{
			//if there's an event
			if (_poll_fds[i].revents & (POLLIN | POLLHUP | POLLERR))
			{
				if (debug) std::cout << "NEW EVENT POLLIN | POLLHUP | POLLERR\n";

				// char	buffer[1024];
				// recv(_poll_fds[i].fd, buffer, 1023, 0);
				// std::cout << buffer << std::endl;


				//from server socker -> client to accept
				if (_poll_fds[i].fd == _server_fd)
					_acceptNewClient();
				else
				{
					//receiving data or client left
					if (!_receiveNewData(i))
					{
						_disconnectedClient(i);
						i--;
						continue;
					}
				}
			}
			if (_poll_fds[i].revents & POLLOUT)
			{
				if (debug) std::cout << "NEW EVENT POLLOUT" << std::endl;
				if (_SendFlush(i) == -1)
				{
					_disconnectedClient(i);
					i--;
					continue;
				}
			}
			if (_checkToClose(i))
			{
				_disconnectedClient(i);
				i--;
				continue;
			}
		}
	}
}

void	Server::_acceptNewClient(void)
{
	std::cout << "NEW CLIENT ACCEPTED - ";
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(_server_fd, (struct sockaddr*)&client_addr, \
			&client_len);
	if (client_fd == -1)
	{
		return ;
	}

	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(client_fd);
		return ;
	}

	struct pollfd newClient;
	newClient.fd = client_fd;
	newClient.events = POLLIN;
	newClient.revents = 0;
	_poll_fds.push_back(newClient);

	User* u = new User(client_fd);
	_users.insert(std::make_pair(client_fd, u));
	if (_password.empty())
	{
		std::cout << "No password required" << std::endl;
		u->set_pass_ok( true );
	}
	else
		std::cout << "Password required" << std::endl;
}

bool	Server::_receiveNewData(int i)
{
	User* current = _users[_poll_fds[i].fd];

	ssize_t	check;
	char	buffer[1024];
	check = recv(_poll_fds[i].fd, buffer, 1023, 0);

	if (check == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (true); //it didn't make poll wakes us
		return (false); //POLLERR
	}
	if (check == 0)
		return (false); //POLLHUP
	buffer[check] = '\0';
	current->addBufferIn(buffer); //User public : add new buffer to User.buffer

	while (current->lineReady()) //User public : check line has "\r\n or \n"
	{
		std::string line = current->extractLine(); //clean User.buffer
		std::cout	<<"NEW received line from "
					<< Commands::fixedString(current->getNickName())
					<< ": " << line << std::endl;
		if (line.size() > 510)
		{
			std::cout 	<< "IRC protocol cmd line issue - "
			<< Commands::fixedString(current->getNickName())
			<< " - Client disconnection" << std::endl;
			return (false);
		}
		if (!_exec_line(*current, line))
			return (false); //no zombie user
	}
	if (current->getBufferInSize() > 510)
	{
		std::cout 	<< "IRC protocol buffering issue - "
					<< Commands::fixedString(current->getNickName())
					<< " - Client disconnection" << std::endl;
		return (false);
	}
	return (true);
}

bool	Server::_exec_line(User& current ,std::string& line)
{
	try
	{
		cmd_line cmd = Parser::parsing(line);
		if (debug) Parser::print_cmd(cmd);
		if (_cmd->handle_command(*this, current, cmd) == -1)
			return (false);
		// 	return (false); //trigger _disconnectedClient(i);
	}
	catch (const std::exception& e) 
	{
    	std::cerr << "Exception in exec_line: " << e.what() << std::endl;
	}
	catch (...) 
	{
    	std::cerr << "Unknown exception in exec_line" << std::endl;
	}
	return (true);
}

void	Server::_disconnectedClient(int i)
{
	int cfd = _poll_fds[i].fd;
	std::map<int, User*>::iterator it = _users.find(cfd);
	if (it != _users.end())
	{
		if (server_stop == true)
		{
			const char* msg = "ERROR :Server shutting down\r\n";
			send(cfd, msg, strlen(msg), MSG_NOSIGNAL);
		}
			std::string nick = it->second->getNickName();
		std::cout << Commands::fixedString(nick) << " : BYE CLIENT" << std::endl;
		_removeFromChannels(it->second);
		if (!nick.empty())
			_nts.erase(nick);
		delete it->second;
		_users.erase(it);
	}
	close(cfd);
	_poll_fds.erase(_poll_fds.begin() + i);
}

int	Server::_SendFlush(int i)
{
	int fd = _poll_fds[i].fd;
  	User* current = _users[fd];
  	if (!current || !current->hasPendingOut()) 
  		return (0);

  	std::string out = current->getBufferOut();

	ssize_t n = send(fd, out.data(), out.size(), 0);
	if (n > 0)
	{
		current->flushBufferOut(n);
		_updatePollEventsForFd(fd);
		std::string str = current->getNickName();
		if (str.empty()) str = "*";
		std::cout << "message sended to " << str << " -> " << out;
	}
	else if (n < 0)
	{
    	if (errno == EAGAIN || errno == EWOULDBLOCK) 
			return (0);
    	// erreur fatale
    	return (-1);
	}
	return (0);
}

void Server::_updatePollEventsForFd(int fd)
{
    std::map<int, User*>::iterator it = _users.find(fd);
    if (it == _users.end())
        return;

    for (size_t i = 0; i < _poll_fds.size(); ++i)
    {
        if (_poll_fds[i].fd == fd)
        {
            _poll_fds[i].events = POLLIN; /* toujours lire */
            if (it->second->hasPendingOut())
                _poll_fds[i].events |= POLLOUT;
            return;
        }
    }
}

void	Server::_removeFromChannels(User* user)
{
	std::vector<std::string> toDelete;
	for (std::map<std::string, Channel*>::iterator it = _channels.begin();
         it != _channels.end(); ++it)
	{
		Channel* ch = it->second;
		if (!ch->hasUser(user))
        	continue;
		ch->removeMember(user);
	/*****  if empty the channel is stored to be removed *****/
		if (ch->getUserCount() == 0)
			toDelete.push_back(it->first);
	}
	/*****  delete the empty channels *****/
	for (size_t i = 0; i < toDelete.size(); ++i)
	{
		delete _channels[toDelete[i]];
    	_channels.erase(toDelete[i]);
	}
}

void	Server::_updateChannels(Channel* ch)
{
	if (!ch)
		return;
	std::string name = ch->getName();
	if (_channels.find(name) != _channels.end())
	{
		if (debug)
            std::cout << "Channel already exists: " << name << std::endl;
        return;
	}
	_channels.insert(std::make_pair(name, ch));
	if (debug) std::cout << "Channel add _channels updated" << std::endl;
}

bool	Server::_checkToClose(int i)
{
	User* current = _users[_poll_fds[i].fd];
	if (current && ((current->getShouldClose() && !current->hasPendingOut()) ||
		current->getForceClose()))
		return (true);
	return (false);
}

const std::string&	Server::get_pass(void) const
{
	return (_password);
}

const std::string&	Server::get_name(void) const
{
	return (_server_name);
}

const std::map<int, User*>&	Server::get_user(void) const
{
	return (_users);
}

const std::map<std::string, int>&	Server::get_nick(void) const
{
	return (_nts);
}

const std::map<std::string, Channel*>&	Server::get_channels(void) const
{
	return (_channels);
}

void	Server::sendTo( User& current, std::string& str )
{
	std::string	nick = current.getNickName();
	if (nick.empty())
		nick = "*";
	
	std::string	cpy = str;
	if (str.size() > 512)
	{
		cpy = str.substr(0, 510);
		cpy += "\r\n";
	}
	if ((current.getBufferOutSize() + cpy.size()) >= _bufferOutLimit)
	{
		current.set_forceClose(true);
		return ;
	}
	current.addBufferOut(cpy);
	_updatePollEventsForFd(current.getFd());
	if (debug) std::cout << "to " << nick << " message queued -> " << cpy;
}

void	Server::broadcastToChannels(User& user, std::string msg, bool includeSelf)
{
	if (_channels.empty())
		return ;
	if (includeSelf)
		sendTo(user, msg);

	std::set<int> sent;
	for (std::map<std::string, Channel*>::const_iterator it = _channels.begin();
		it != _channels.end(); ++it)
	{
		const std::map<User*, int>& inside = it->second->getMembers();
		const std::map<User*, int>::const_iterator itu = inside.find(&user);
		if (itu == inside.end())
			continue ;

		for (std::map<User*, int>::const_iterator itc = inside.begin();
		itc != inside.end(); ++itc)
		{
			if (itc == itu)
				continue ;
			User* msgTo = itc->first;
			if (sent.insert(msgTo->getFd()).second)
				sendTo(*msgTo, msg);
		}
	}
}

void	Server::broadcastToChannel(Channel* ch, User& user, std::string msg,
								   bool includeSelf)
{
	const std::map<User*, int>& inside = ch->getMembers();
	for (std::map<User*, int>::const_iterator itc = inside.begin();
		itc != inside.end(); ++itc)
	{
		User* msgTo = itc->first;
		if( msgTo == &user && !includeSelf)
			continue ;
		sendTo(*msgTo, msg);
	}
}

void Server::updateNick(const std::string& oldNick, const std::string& newNick, int fd)
{
	if (!oldNick.empty())
		_nts.erase(oldNick);
	_nts[newNick] = fd;
	if (debug) std::cout << "Nick _nts updated" << std::endl;
}

User*	Server::whoUser(std::string nick) const
{
	if (nick.empty())
		return (NULL);
	std::map<std::string, int>::const_iterator it = _nts.find(nick);
	if (it == _nts.end())
		return (NULL);
	std::map<int, User*>::const_iterator itu = _users.find(it->second);
	return (itu->second);
}

Channel*	Server::get_channel(std::string ch) const
{
	std::map<std::string, Channel*>::const_iterator it = _channels.find(ch);
	if (it == _channels.end())
		return (NULL);
	return (it->second);
}

Channel*	Server::newChannel(std::string name)
{
	Channel* ch = new Channel(name);
	_updateChannels(ch);
	return (ch);
}

Channel*	Server::newChannel(std::string name, std::string key)
{
	Channel* ch = new Channel(name, key);
	_updateChannels(ch);
	return (ch);
}

void	Server::updateChannels(std::string cname)
{
	if (cname.empty())
		return ;
	std::map<std::string, Channel*>::iterator it = _channels.find(cname);
	if (it ==_channels.end())
		return ;
	Channel* ch = it->second;
	if (ch->getUserCount() != 0)
		return ;
	delete ch;
	_channels.erase(cname);
	if (debug) std::cout << "Channel deleted _channels updated" << std::endl;
}
