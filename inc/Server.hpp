#ifndef SERVER_HPP
#define SERVER_HPP
#include <map>
#include <set>
#include <string>
#include "poll.h"
#include <fcntl.h>
#include <cerrno>
#include "Channel.hpp"
#include "User.hpp"
#include "Parser.hpp"
#include "Commands.hpp"

class Server 
{
	private:
		int								_port;
		std::string						_password;
		std::string						_server_name;
		int								_server_fd; // first of _poll_fds
		std::map<int, User*>			_users; //socket to user
		std::map<std::string, int>		_nts; //nick to socket
		std::map<std::string, Channel*>	_channels; //name to channel
		std::vector<struct pollfd>		_poll_fds; //fds listened
		Commands*						_cmd; // commands instance
		size_t							_bufferOutLimit; // max size before disconnect

		void	_acceptNewClient();
		bool	_receiveNewData(int i);
		bool	_exec_line(User& current, std::string& line);
		void	_disconnectedClient(int i);
		int		_SendFlush(int i);
		void 	_updatePollEventsForFd(int fd);
		void	_removeFromChannels(User* user);
		void	_updateChannels(Channel* channel);
		bool	_checkToClose(int i);

	public:
		Server(int port, const std::string& password);
		~Server();
	
		const std::string& 						get_pass() const;
		const std::string& 						get_name() const;
		const std::map<int, User*>&				get_user() const;
		const std::map<std::string, int>& 		get_nick() const;
		const std::map<std::string, Channel*>& 	get_channels() const;
		void	start();
		void	sendTo(User& current, std::string& str);
		void	broadcastToChannels(User& user, std::string msg, bool includeSelf);
		void	broadcastToChannel(Channel* ch, User& user, std::string msg, bool includeSelf);
		void	updateNick(const std::string& oldNick, const std::string& newNick, int fd);
		void	updateChannels(std::string cname);
		User*	whoUser(std::string nick) const;
		Channel*	get_channel(std::string ch) const;
		Channel*	newChannel(std::string name);
		Channel*	newChannel(std::string name, std::string key);
};

#endif //SERVER_HPP
