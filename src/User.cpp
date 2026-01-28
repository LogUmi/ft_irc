#include <algorithm>
#include <netinet/in.h>
#include "../inc/User.hpp"
#include <iostream>

extern bool	debug;

User::User( int client_fd )
:	fd(client_fd), 
	name(""),
	nick(""),
	buffer_in(""),
	buffer_out(""),
	pass_ok(false),
	forceClose(false),
	shouldClose(false)
{}

User::~User()
{}

void	User::addBufferIn( char* buffer )
{
	this->buffer_in += buffer;
}

void	User::addBufferOut( std::string buffer )
{
	this->buffer_out += buffer;
}

std::string	User::extractLine( void )
{
	int	n = 2;

	size_t pos = this->buffer_in.find("\r\n");
	if (pos == std::string::npos)
	{
		pos = this->buffer_in.find("\n");
		if (pos == std::string::npos)
			return ("");
		n = 1;
	}
	std::string str = this->buffer_in.substr(0, pos);
	this->buffer_in.erase(0, pos + n);
	return (str);
}

std::string	User::getBufferOut( void ) const
{
	return (buffer_out);
}

void	User::flushBufferOut( ssize_t n)
{
	buffer_out.erase(0, n);
}

size_t	User::getBufferInSize( void ) const
{
	return (this->buffer_in.size());
}

size_t	User::getBufferOutSize( void ) const
{
	return (this->buffer_out.size());
}

int	User::getFd( void ) const
{
	return (fd);
}

std::string	User::getName( void ) const
{
	return (this->name);
}

std::string	User::getNickName( void ) const
{
	return (this->nick);
}

bool	User::getPass_ok( void ) const
{
	return (this->pass_ok);
}

bool	User::getForceClose( void ) const
{
	return (this->forceClose);
}

bool	User::getShouldClose( void ) const
{
	return (this->shouldClose);
}

bool	User::hasPendingOut( void ) const
{
	if (buffer_out.size() == 0)
		return (false);
	return (true);
}

bool 	User::is_authenticated() const
{
	if (this->pass_ok == true && !(this->name.empty()) && !(this->nick.empty()))
		return (true);
	return (false);
}

bool	User::lineReady( void ) const
{
	if (buffer_in.empty())
		return (false);
	size_t pos = this->buffer_in.find("\r\n");
	if (pos == std::string::npos)
	{
		size_t pos = this->buffer_in.find("\n");
		if (pos == std::string::npos)
			return (false);
		if (debug) std::cout << "LineReady: '\\n' founded" <<std::endl;
		return (true);
	}
	if (debug) std::cout << "LineReady: '\\r\\n' founded" <<std::endl;
	return (true);
}

std::string	User::prefix( void ) const
{
	return (this->nick + "!" + this->name + "@");
}

void	User::set_name( std::string user )
{
	this->name = user;
}

void	User::set_nick( std::string nname )
{
	this->nick = nname;
}

void	User::set_pass_ok( bool pass )
{
	this->pass_ok = pass;
}

void	User::set_forceClose( bool force )
{
	this->forceClose = force;
}

void	User::set_shouldClose( bool close )
{
	this->shouldClose = close;
}
