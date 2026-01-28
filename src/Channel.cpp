#include "../inc/Channel.hpp"
#include "../inc/User.hpp"
#include <algorithm>

Channel::Channel(const std::string& name) :
	_name(name),
	_password(""),
	_topic(""),
	_maxUsers(0),
	_creationTime(std::time(NULL)),
	_hasLimit(false),
	_hasPassword(false),
	_inviteOnly(false),
	_topicProtected(true)
{}

Channel::Channel(const std::string& name, const std::string& password) :
	_name(name),
	_password(password),
	_topic(""),
	_maxUsers(0),
	_creationTime(std::time(NULL)),
	_hasLimit(false),
	_hasPassword(true),
	_inviteOnly(false),
	_topicProtected(true)
{}

Channel::~Channel(void)
{}

const std::string&	Channel::getName(void) const
{
	return (_name);
}

const std::string&	Channel::getTopic(void) const
{
	return (_topic);
}

const std::string&	Channel::getPassword(void) const
{
	return (_password);
}

const std::string	Channel::getCreationTime( void ) const
{
	std::ostringstream ostr;
	ostr << _creationTime;
	return (ostr.str());
}

const std::string	Channel::getMaxUsersStr( void ) const
{
	std::ostringstream ostr;
	ostr << _maxUsers;
	return (ostr.str());
}

const std::map<User*, int>&	Channel::getMembers(void) const
{
	return (_users);
}

size_t	Channel::getMaxUsers(void) const
{
	return (_maxUsers);
}

size_t	Channel::getUserCount(void) const
{
	return (_users.size());
}

bool	Channel::hasPassword(void) const
{
	return (_hasPassword);
}

bool	Channel::hasLimit(void) const
{
	return (_hasLimit);
}

bool	Channel::isInviteOnly(void) const
{
	return (_inviteOnly);
}

bool	Channel::isTopicProtected(void) const
{
	return (_topicProtected);
}

bool	Channel::isBanned(User* user) const
{
	std::vector<User*>::const_iterator it;

	it = std::find(_bannedUsers.begin(), _bannedUsers.end(), user);
	return (it != _bannedUsers.end());
}

bool	Channel::isInvited(User* user) const
{
	std::vector<User*>::const_iterator it;

	it = std::find(_invitedUsers.begin(), _invitedUsers.end(), user);
	return (it != _invitedUsers.end());
}

bool	Channel::isOperator(User* user) const
{
	std::map<User*, int>::const_iterator it = _users.find(user);

	if (it != _users.end())
		return (it->second == 1);
	return (false);
}

bool	Channel::hasUser(User* user) const
{
	return (_users.find(user) != _users.end());
}

bool	Channel::isFull(void) const
{
	if (_maxUsers == 0)
		return (false);
	return (getUserCount() >= static_cast<size_t>(_maxUsers));
}

void	Channel::addMember(User* user, bool isOp)
{
	if (!user)
		return ;

	int	grade = 0;

	if (isOp)
		grade = 1;

	_users.insert(std::make_pair(user, grade));
	this->removeInvite(user);
}

void	Channel::removeMember(User* user)
{
	if (!user)
		return ;

	std::map<User*, int>::iterator it = _users.find(user);

	if (it != _users.end())
		_users.erase(it);
}

void	Channel::addInvite(User* user)
{
	if (!user)
		return ;

	if (this->isInvited(user) == false)
		_invitedUsers.push_back(user);
}

void	Channel::removeInvite(User* user)
{
	if (!user)
		return ;

	std::vector<User*>::iterator it;

	it = std::find(_invitedUsers.begin(), _invitedUsers.end(), user);
	if (it != _invitedUsers.end())
		_invitedUsers.erase(it);
}

void	Channel::addBan(User* user)
{
	if (!user)
		return ;

	if (this->isBanned(user) == false)
		_bannedUsers.push_back(user);
}

void	Channel::removeBan(User* user)
{
	if (!user)
		return ;

	std::vector<User*>::iterator it;

	it = std::find(_bannedUsers.begin(), _bannedUsers.end(), user);
	if (it != _bannedUsers.end())
		_bannedUsers.erase(it);
}

void	Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

void	Channel::setMode(char mode, bool sign)
{
	if (mode == 'i')
		_inviteOnly = sign;
	if (mode == 't')
		_topicProtected = sign;
}

void	Channel::setOperator(User* user, bool status)
{
	if (!user)
		return ;

	std::map<User*, int>::iterator it = _users.find(user);

	if (it != _users.end())
	{
		if (status)
			it->second = 1;
		else
			it->second = 0;
	}
}

void	Channel::setLimit(unsigned int max, bool sign)
{
	if (sign)
	{
		_maxUsers = max;
		_hasLimit = true;
	}
	else
		_hasLimit = false;
}

void	Channel::setPassword(const std::string& password)
{
	_password = password;
	_hasPassword = !password.empty();
}
