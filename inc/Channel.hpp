#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <map>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>

class User;

class Channel 
{


	std::string _name;
	std::string	_password;
	std::string _topic;
	size_t		_maxUsers;
	time_t		_creationTime;

	bool _hasLimit;
	bool _hasPassword;
	bool _inviteOnly;
	bool _topicProtected;

	std::map<User*, int> _users;
	std::vector<User*> _invitedUsers;
	std::vector<User*> _bannedUsers;


	public:
		Channel(const std::string& name);
		Channel(const std::string& name, const std::string& password);
		~Channel();

		const std::string&	getName() const;
		const std::string&	getTopic() const;
		const std::string&	getPassword() const;
		const std::string	getCreationTime() const;
		const std::string	getMaxUsersStr( void ) const;
		const std::map<User*, int>&	getMembers() const;
		size_t	getMaxUsers() const;
		size_t	getUserCount() const;

		bool	hasPassword() const;
		bool	hasLimit(void) const;
		bool	isInviteOnly() const;
		bool	isTopicProtected() const;

		bool	isBanned(User* user) const;
		bool	isInvited(User* user) const;
		bool	isOperator(User* user) const;
		bool	hasUser(User* user) const;
		bool	isFull() const;

		void	addMember(User* user, bool isOp);
		void	removeMember(User* user);
		void	addInvite(User* user);
		void	removeInvite(User* user);
		void	addBan(User* user);
		void	removeBan(User* user);
		void	setTopic(const std::string& topic);
		void	setMode(char mode, bool sign);
		void	setOperator(User* user, bool status);
		void	setLimit(unsigned int max, bool sign);
		void	setPassword(const std::string& password);


};
#endif
