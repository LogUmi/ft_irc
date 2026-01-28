#include "../inc/Commands.hpp"
#include <utility>
#include <iostream>
#include <algorithm>
#include "../inc/Server.hpp"

extern bool	server_stop;
extern bool debug;

const Commands::IrcError Commands::cmd_errors[] =
{
	{401, "ERR_NOSUCHNICK", "No such nick/channel"},
	{403, "ERR_NOSUCHCHANNEL", "No such channel"},
	{404, "ERR_CANNOTSENDTOCHAN", "Cannot send to channel"},
	{411, "ERR_NORECIPIENT", "No recipient given"},
	{412, "ERR_NOTEXTTOSEND", "No text to send"},
	{421, "ERR_UNKNOWNCOMMAND", "Unknown command"},
	{431, "ERR_NONICKNAMEGIVEN", "No nickname given"},
	{432, "ERR_ERRONEUSNICKNAME", "Erroneous nickname"},
	{433, "ERR_NICKNAMEINUSE", "Nickname is already in use"},
	{441, "ERR_USERNOTINCHANNEL", "They aren't on that channel"},
	{442, "ERR_NOTONCHANNEL", "You're not on that channel"},
	{443, "ERR_USERONCHANNEL", "is already on channel"},
	{451, "ERR_NOTREGISTERED", "You have not registered"},
	{461, "ERR_NEEDMOREPARAMS", "Not enough parameters"},
	{462, "ERR_ALREADYREGISTRED", "user is already registred"},
	{464, "ERR_PASSWDMISMATCH", "Password incorrect"},
	{471, "ERR_CHANNELISFULL", "Cannot join channel (+l)"},
	{472, "ERR_UNKNOWNMODE", "is unknown mode char to me"},
	{473, "ERR_INVITEONLYCHAN", "Cannot join channel (+i)"},
	{474, "ERR_BANNEDFROMCHAN", "Cannot join channel (+b)"},
	{475, "ERR_BADCHANNELKEY", "Cannot join channel (+k)"},
	{476, "ERR_BADCHANMASK", "Bad Channel Mask"},
 	{482, "ERR_CHANOPRIVSNEEDED", "You're not channel operator"},
	{0, 0, 0}	// sentinelle
};

const Commands::IrcError Commands::cmd_reply[] =
{
	{324, "RPL_CHANNELMODEIS", ""},
	{329, "RPL_CREATIONTIME", ""},
	{331, "RPL_NOTOPIC", "No topic is set"},
	{332, "RPL_TOPIC", ""},
	{341, "RPL_INVITING", "Inviting"},
	{353, "RPL_NAMREPLY", ""},
	{366, "RPL_ENDOFNAMES", "End of /NAMES list."},
	{0, 0, 0}	// sentinelle
};

Commands::Commands( void )
{
	unauth_cmd.insert(std::make_pair(std::string("PASS"), &Commands::checkPassword));
	unauth_cmd.insert(std::make_pair(std::string("CAP"), &Commands::cmdCap));
	unauth_cmd.insert(std::make_pair(std::string("PING"), &Commands::cmdPing));
	unauth_cmd.insert(std::make_pair(std::string("QUIT"), &Commands::cmdQuit));
	unauth_cmd.insert(std::make_pair(std::string("NICK"), &Commands::setNick));
	unauth_cmd.insert(std::make_pair(std::string("USER"), &Commands::setUser));
	unauth_cmd.insert(std::make_pair(std::string("DEBUGON"), &Commands::debugOn));
	unauth_cmd.insert(std::make_pair(std::string("DEBUGOFF"), &Commands::debugOff));
	unauth_cmd.insert(std::make_pair(std::string("SERVERSTOP"), &Commands::serverStop));
	auth_cmd.insert(std::make_pair(std::string("INVITE"), &Commands::cmdInvite));
	auth_cmd.insert(std::make_pair(std::string("JOIN"), &Commands::cmdJoin));
	auth_cmd.insert(std::make_pair(std::string("MODE"), &Commands::cmdMode));
	auth_cmd.insert(std::make_pair(std::string("KICK"), &Commands::cmdKick));
	auth_cmd.insert(std::make_pair(std::string("PART"), &Commands::cmdPart));
	auth_cmd.insert(std::make_pair(std::string("PRIVMSG"), &Commands::cmdPrivMsg));
	auth_cmd.insert(std::make_pair(std::string("NOTICE"), &Commands::cmdNotice));
	auth_cmd.insert(std::make_pair(std::string("TOPIC"), &Commands::cmdTopic));
	auth_cmd.insert(std::make_pair(std::string("PONG"), &Commands::cmdPong));
}

Commands::~Commands( void )
{}

void	Commands::buildError( int code, Server& server, User& user,
							  std::string err_target, std::string err_arg )
{
	int	i = 0;
	std::string str = ":" + server.get_name();
	while (cmd_errors[i].code != 0 && cmd_errors[i].code != code)
		i++;
	if (cmd_errors[i].code != 0)
		str += " " + ft_uitoa(code);
	if (err_target != "")
		str += " " + err_target;
	if (err_arg != "")
		str += " " + err_arg;
	if (cmd_errors[i].code != 0)
	{
		std::string msg = cmd_errors[i].message;
		str += " :" + msg;
	}
	str += "\r\n";
	server.sendTo(user, str);
}

void Commands::buildReply( int code, Server& server, User& user,
						   Channel& ch, const cmd_line& cmd )
{
	int	i = 0;
	std::string str = ":" + server.get_name();
	while (cmd_reply[i].code != 0 && cmd_reply[i].code != code)
		i++;
	if (cmd_reply[i].code == 0)
		return ;
	str += " " + ft_uitoa(code) + " " + user.getNickName();
	if (code == 341)
		str += " " + cmd.parameters[0];
	if (code == 353)
		str += " =";
	str += " " + ch.getName();
	if (code == 324)
		str += " " + getCmode(ch, user);
	else if (code == 329)
		str += " " + ch.getCreationTime();
	else
	{
		str +=  " :";
		str += cmd_reply[i].message;
	}
	if (code == 353)
	{
		/* send nicks of users in the channel who is operator */
		const std::map<User*, int>& list = ch.getMembers();
		for (std::map<User*, int>::const_iterator it = list.begin();
			it != list.end(); ++it)
		{
			if (str[str.size() - 1] != ':')
				str += " ";
			if (it->second == 1)
				str += "@";
			str += it->first->getNickName();
		}
	}
	if (code == 332)
		str += ch.getTopic();
	str += "\r\n";
	server.sendTo(user, str);
}

void Commands::checkWelcome( User& user, Server& server)
{
	if (!user.is_authenticated())
		return ;
	std::string str = ":" + server.get_name() + " 001 " + user.getNickName();
	str += " :Welcome to the Internet Relay Network ";
	str += user.prefix() + server.get_name() + "\r\n";
	server.sendTo(user, str);
}

std::string	Commands::fixedString( const std::string& src )
{
	std::string	str;

	if (!src.empty())
		str = src;
	else
		str = "*";
	return (str);
}

std::string	Commands::getCmode(Channel& ch, User& user)
{
	std::string limit;
	std::string	key;
	std::string str = "+";
	if (ch.isInviteOnly())
		str += "i";
	if (ch.hasLimit())
	{
		str += "l";
		limit = ch.getMaxUsersStr();
	}
	if (ch.isTopicProtected())
		str += "t";
	if (ch.hasPassword())
	{
		str += "k";
		if (ch.hasUser(&user))
			key = ch.getPassword();
	}
	if (!limit.empty())
		str += " " + limit;
	if (!key.empty())
		str += " " + key;
	return (str);
}

std::string	Commands::ft_uitoa( int n )
{
	int 		i = n;
	int			digit = 0;
	int			k = 0;
	std::string	str;

	if (n == 0) return "0";
	while (i > 0)
	{
		i /= 10;
		digit++;
	}
	while (digit != 0)
	{
		i = n;
		for (k = 1; k < digit; k++)
			i /= 10;
		str += static_cast<char>(i + 48);
		for (k = 1; k < digit; k++)
			i *= 10;
		n -= i;
		digit--; 
	}
	return (str);
}

bool	Commands::isInChannels( Server& server, User& user)
{
	const std::map<std::string, Channel*>& channels = server.get_channels();
	if (channels.empty())
		return (false);
	for (std::map<std::string, Channel*>::const_iterator it = channels.begin();
		it != channels.end(); ++it)
	{
		Channel* ch = it->second;
		if (debug)
			std::cout << "isInChannel " << ch->getName() << "has" << ch->hasUser(&user) << "nb"  << ch->getUserCount() << "\n";
		if (!ch->hasUser(&user))
			continue ;
		if (ch->getUserCount() > 1)
			return (true);
	}
	return (false);
}

bool	Commands::isValidCname( const std::string name )
{
	if (name.empty() || name.size() < 2)
        return (false);
	char c = name[0];
	if (c != '#' )
        return (false);
	for (size_t i = 1; i < name.size(); i++)
	{
		c = name[i];
		if (c <= 32 || c == ',' || c == ':' || c == 127)
			return (false);
    }
    return (true);
}

bool	Commands::isValidKey( const std::string key )
{
	if (key.empty())
        return (false);
	for (size_t i = 0; i < key.size(); i++)
	{
		char c = key[i];
		if (c <= 32 || c == ',' || c == ':' || c == 127)
			return (false);
    }
    return (true);
}

bool	Commands::isValidLimit( const std::string& limit, size_t& val )
{
    if (limit.empty())
        return false;
    for (size_t i = 0; i < limit.size(); ++i)
        if (!isdigit(limit[i]))
            return false;
    long long v = atoll(limit.c_str());
    if (v < 1)
        return false;
    val = static_cast<size_t>(v);
    return true;
}

bool	Commands::isValidNick( const std::string& nick )
{
	if (nick.empty() || nick.size() > 9)
        return (false);

    for (size_t i = 0; i < nick.size(); i++)
	{
		char c = nick[i];
	if (std::isalnum(c))
			continue;
	if (c == '-' || c == '[' || c == ']' || c == '\\' ||
		c == '`' || c == '^' || c == '{' || c == '}')
            continue;
	return (false);
    }
    return (true);
}

bool	Commands::isValidUser( const std::string& user )
{
	if (user.empty())
	    return (false);

	for (size_t i = 0; i < user.size(); i++)
	{
	char c = user[i];
	if (c < 33 || c == 127)
		return (false);
	}
	return (true);
}

void	Commands::sanitizeString( std::string& str )
{
	for (size_t i = 0; i < str.size(); )
	{
		char c = str[i];
		if (c == '\r' || c == '\n')
			str.erase(i, 1);
		else
			i++;
    }
}

void	Commands::splitList( const std::string& s, 
		std::vector<std::string>& tab, bool empty )
{ /* option empty = true renvoie "" entre 2 ',' sinon skip les ',' consecutives */
	tab.clear();
	if (s.empty())
		return ;
	if (empty)
	{
		size_t start = 0;
		while (true)
		{
			size_t pos = s.find(',', start);
			if (pos == std::string::npos)
			{
				tab.push_back(s.substr(start));
				break;
			}
    		tab.push_back(s.substr(start, pos - start));
			start = pos + 1;
		}
	}
	else
	{
		size_t	i = 0;

		while (i < s.size())
		{
			while(i < s.size() && (s[i] == ','))
				i++;
			if (i >= s.size())
				return ;
			size_t	j = i;
			while (i < s.size() && !(s[i] == ','))
				i++;
			tab.push_back(s.substr(j, i - j));
		}
	}
}

int	Commands::handle_command(Server& server, User& user, const cmd_line& cmd)
{
	std::string str;
	std::string	err_target = fixedString(user.getNickName());
	std::string	err_arg = cmd.command;

	std::map<std::string, cmdFn>::const_iterator it = unauth_cmd.find(cmd.command);
	if (it == unauth_cmd.end())
	{
		std::map<std::string, cmdFn>::const_iterator it = auth_cmd.find(cmd.command);
		if (it == auth_cmd.end())
		{
			buildError(421, server, user, err_target, err_arg);
			return (421);
		}
		else if (!user.is_authenticated())
		{
			buildError(451, server, user, err_target, err_arg);
			return (451);
		}
		else return ((this->*(it->second))(server, user, cmd));
	}
	else
		return ((this->*(it->second))(server, user, cmd));
	if (debug) std::cout << "UNKNOWN ERROR in handle_command" << std::endl;
	return (0);
}

int	Commands::checkPassword( Server& server, User& user,const cmd_line& cmd ) const
{
	std::string	pass;
	std::string	str;
	std::string	err_target = fixedString(user.getNickName());
	std::string	err_arg = cmd.command;

	if (user.is_authenticated() == true)
	{
		buildError(462, server, user, err_target, "");
		return (462);
	}

	if (cmd.hasTrailing == true)
	{
		if (cmd.parameters.size() != 0)
		{
			buildError(461, server, user, err_target, err_arg);
			return (461);
		}
		pass = cmd.trailing;
	}
	else if (cmd.parameters.size() == 0 || cmd.parameters.size() > 1)
	{
		buildError(461, server, user, err_target, err_arg);
		return (461);
	}
	else
		pass = cmd.parameters[0];
	if (pass != server.get_pass())
	{
		buildError(464, server, user, err_target, err_arg);
		user.set_shouldClose(true); /* bad password -> disconnection after msg */
		return (464);
	}
	user.set_pass_ok(true);
	std::cout << fixedString(user.getNickName()) << " password Ok" << std::endl;
	checkWelcome(user, server);
	return (0);
}

int Commands::cmdCap(Server& server, User& user, const cmd_line& cmd) const
{
    std::string str;

	if (cmd.parameters.size() >= 1 && cmd.parameters[0] == "LS")
		str =  ":" + server.get_name() + " CAP * LS :\r\n";
	else if (cmd.parameters.size() >= 1 && cmd.parameters[0] == "REQ")
	{
		std::string caps = cmd.hasTrailing ? cmd.trailing : "";
		str = ":" + server.get_name() + " CAP * NAK :" + caps + "\r\n";
	}
	else
		return (0);

	if (!str.empty())
		server.sendTo(user, str);
	return (0);
}

int Commands::cmdInvite(Server& server, User& user, const cmd_line& cmd) const
{
	std::string err_target = fixedString(user.getNickName());
	std::string err_arg = cmd.command;

	if (cmd.parameters.size() != 2 || cmd.hasTrailing)
	{
		buildError(461, server, user, err_target, err_arg);
		return (461);
	}

	std::string targetNick = cmd.parameters[0];
	std::string targetChan = cmd.parameters[1];
	const std::map<std::string, Channel*>& channels = server.get_channels();
	std::map<std::string, Channel*>::const_iterator chan_it;
	chan_it = channels.find(targetChan);
	const std::map<std::string, int>& nicknames = server.get_nick();
	std::map<std::string, int>::const_iterator nick_it;
	nick_it = nicknames.find(targetNick);

	if (targetNick.empty() || nick_it == nicknames.end())
	{
		buildError(401, server, user, err_target, targetNick);
		return (401);
	}
	if (targetChan.empty() || (targetChan[0] != '#' && targetChan[0] != '&') \
			|| chan_it == channels.end())
	{
		buildError(403, server, user, err_target, targetChan);
		return (403);
	}
	if (!chan_it->second->hasUser(&user))
	{
		buildError(442, server, user, err_target, targetChan);
		return (442);
	}
	if (chan_it->second->isInviteOnly())
	{
		if (!chan_it->second->isOperator(&user))
		{
			buildError(482, server, user, err_target, targetChan);
			return (482);
		}
	}
	std::map<int, User*>::const_iterator user_it;
	user_it = server.get_user().find(nick_it->second);
	if (user_it == server.get_user().end())
    	return (0);
	if (chan_it->second->hasUser(user_it->second))
	{
		buildError(443, server, user, err_target, targetNick + " " + targetChan);
		return (443);
	}
	chan_it->second->addInvite(user_it->second);
	buildReply(341, server, user, *chan_it->second, cmd);

	std::string str = ":" + user.getNickName() + "!" + user.getName() + "@";
	str += server.get_name() + " INVITE " + targetNick + " :" + targetChan + "\r\n";
	server.sendTo(*server.whoUser(targetNick), str);
	return (0);
}

int Commands::cmdJoin( Server& server, User& user, const cmd_line& cmd ) const
{
	std::string	err_target = user.getNickName();
	std::string	err_arg = cmd.command;

	if (cmd.parameters.empty())
	{
		buildError(461, server, user, err_target, cmd.command);
		return (461);
	}
	std::string listCh = cmd.parameters[0];
	std::vector<std::string> tname;
	splitList(listCh, tname, true);
	if (tname.empty())
	{
		buildError(461, server, user, err_target, err_arg);
		return (461);
	}
	std::string listKey = "";
	if (cmd.parameters.size() >= 2)
		listKey = cmd.parameters[1];
	std::vector<std::string> tkey;
	splitList(listKey, tkey, true);
	for (size_t pos = 0; pos < tname.size(); pos++)
	{
		std::string	name = tname[pos];
		std::string key = "";
		if (!tkey.empty())
			key = (pos < tkey.size()) ? tkey[pos] : "";
		if (!isValidCname(name))
		{
			buildError(403, server, user, err_target, name);
			continue ;
		}
		if (!key.empty() && !isValidKey(key))
		{
			buildError(475, server, user, err_target, name);
			continue ;
		}
		bool isOp = false;
		Channel*	ch = server.get_channel(name);
		if (!ch)
		{
			if (!key.empty())
				ch = server.newChannel(name, key); /* new key channel and update ._channels*/
			else
				ch = server.newChannel(name); /* new channel and update ._channels*/
			isOp = true;
		}
		if (ch->hasUser(&user))
		{
			buildError(443, server, user, err_target, name);
			continue ;
		}
		if (ch->isInviteOnly() && !ch->isInvited(&user))
		{
			buildError(473, server, user, err_target, name);
			continue ;
		}
		if (ch->isBanned(&user))
		{
			buildError(474, server, user, err_target, name);
			continue ;
		}
		if (!ch->getPassword().empty() && key != ch->getPassword())
		{
			buildError(475, server, user, err_target, name);
			continue ;
		}
		if (ch->hasLimit())
		{
			if (ch->isFull())
			{
				buildError(471, server, user, err_target, name);
				continue ;
			}
		}
		ch->addMember(&user, isOp);

	/***** Broadcasting to the joined channel *****/
		std::string str = ":" + user.getNickName() + "!" + user.getName() + "@";
		str += server.get_name() + " JOIN :" + name + "\r\n";
		server.broadcastToChannel(ch, user, str, true);

	/***** reply to the client who joined channel *****/
		if (ch->getTopic().empty())
			buildReply(331, server, user, *ch, cmd);
		else 
			buildReply(332, server, user, *ch, cmd);
		buildReply(353, server, user, *ch, cmd);
		buildReply(366, server, user, *ch, cmd);
	}
	return (0);
}

int Commands::cmdKick( Server& server, User& user, const cmd_line& cmd ) const
{
	std::string	err_target = user.getNickName();
	std::string	err_arg = cmd.command;

	if (cmd.parameters.size() < 2)
	{
		buildError(461, server, user, err_target, err_arg);
		return (461);
	}
	std::string msg;
	if (cmd.hasTrailing)
		msg = cmd.trailing;
	else if (cmd.parameters.size() >= 3)
		msg = cmd.parameters[2];
	sanitizeString(msg); /* check and erase \r and \n */
	std::string listChan = "";
	listChan = cmd.parameters[0];
	std::vector<std::string> tchan;
	splitList(listChan, tchan, true);
	std::string list = cmd.parameters[1];
	std::vector<std::string> tname;
	splitList(list, tname, true);
	if (tchan.empty() || tname.empty())
	{
		buildError(461, server, user, err_target, err_arg);
		return 461;
	}
	size_t nmax = std::max(tchan.size(), tname.size());
	for (size_t pos = 0; pos < nmax; pos++)
	{
		std::string cname;
		if (tchan.size() == 1)
			cname = tchan[0];
		else if (pos < tchan.size())
			cname = tchan[pos];
		else
        	continue;
        if (cname.empty())
            continue;

		std::string nick;
		if (tname.size() == 1)
			nick = tname[0];
		else if (pos < tname.size())
			nick = tname[pos];
		else
			continue;
		if (nick.empty())
			continue;
	
		Channel* ch = server.get_channel(cname);
		if (!ch)
		{
			buildError(403, server, user, err_target, cname);
			continue ;
		}
		if (!ch->hasUser(&user))
		{
			buildError(442, server, user, err_target, cname);
			continue ;
		}
		if (!ch->isOperator(&user))
		{
			buildError(482, server, user, err_target, cname);
			continue ;
		}
		User* nick_user = server.whoUser(nick);
		if (!nick_user)
		{
			buildError(401, server, user, err_target, nick);
			continue ;
		}
		if (!ch->hasUser(nick_user))
		{
			buildError(441, server, user, err_target, nick + " " + cname );
			continue ;
		}
		std::string str = ":" + user.getNickName() + "!" + user.getName() + "@";
		str += server.get_name() + " KICK " + cname + " " + nick;
		if (!msg.empty())
			str += " :" + msg;
		str += "\r\n";
		server.broadcastToChannel(ch, user, str, true);
		ch->removeMember(nick_user);
		server.updateChannels(cname); /* delete channel if no more users */
	}
	return (0);
}

int Commands::cmdMode( Server& server, User& user, const cmd_line& cmd ) const
{
	std::string	err_target = user.getNickName();
	std::string	err_arg = cmd.command;

	if (cmd.parameters.size() == 1 && !cmd.hasTrailing)
	{
		std::string	cname = cmd.parameters[0];
		Channel* ch = server.get_channel(cname);
		if (!ch)
		{
			buildError(403, server, user, err_target,cname);
			return (403);
		}
		buildReply(324, server, user, *ch, cmd);
		buildReply(329, server, user, *ch, cmd);
		return (0);
	}

	if (cmd.parameters.size() < 2 || cmd.hasTrailing)
	{
		buildError(461, server, user, err_target, err_arg);
		return (461);
	}
	if (cmd.parameters[1].empty() || (cmd.parameters[1][0] != '+' &&
		cmd.parameters[1][0] != '-'))
	{
		buildError(461, server, user, err_target, err_arg);
		return (461);
	}
	std::string	cname = cmd.parameters[0];
	Channel* ch = server.get_channel(cname);
	if (!ch)
	{
		buildError(403, server, user, err_target,cname);
		return (403);
	}
	if (!ch->hasUser(&user))
	{
		buildError(442, server, user, err_target, cname);
		return (442);
	}
	if (!ch->isOperator(&user))
	{
		buildError(482, server, user, err_target, cname);
		return (482);
	}
	size_t indexp = 2;
	std::string reply;
	char c = 0;
	char s = cmd.parameters[1][0];
	char prev = 0;
	for (size_t i = 0; i < cmd.parameters[1].size(); i++)
	{
		c = cmd.parameters[1][i];
		if (c == '+' || c == '-')
		{
			s = c;
			continue ;
		}
		if (c == 'i')	/* set/unset channel access on invite only  */
		{
			if (s == '+')
				ch->setMode('i', true);
			else
				ch->setMode('i', false);
			if (prev != s)
			{
				prev = s;
				reply += s;
			}
			reply += c;
			continue ;
		}
		if (c == 't') 	/* set/unset topic protection */
		{
			if (s == '+')
				ch->setMode('t', true);
			else
				ch->setMode('t', false);
			if (prev != s)
			{
				prev = s;
				reply += s;
			};
			reply += c;
			continue ;
		}
		if (c == 'l')	/* set/unset limit of number of users */
		{
			if (s == '+')
			{
				if (indexp >= cmd.parameters.size())
				{
					buildError(461, server, user, err_target, err_arg);
					break ;
				}
				size_t val = 0;
				if (!isValidLimit(cmd.parameters[indexp], val))
				{
					buildError(461, server, user, err_target, err_arg);
					break ;
				}
				ch->setLimit(val, true);
				indexp++;
			}
			else
				ch->setLimit(0, false);
			if (prev != s)
			{
				prev = s;
				reply += s;
			}
			reply += c;
			continue ;
		}
		if (c == 'k')	/* set/unset channel key */
		{
			if (s == '+')
			{
				if (indexp >= cmd.parameters.size())
				{
					buildError(461, server, user, err_target, err_arg);
					break ;
				}
				std::string key = cmd.parameters[indexp];
				if (!isValidKey(key))
				{
					buildError(475, server, user, err_target, cname);
					break ;
				}
				ch->setPassword(key);
				indexp++;
			}
			else
				ch->setPassword("");
			if (prev != s)
			{
				prev = s;
				reply += s;
			}
			reply += c;
			continue ;
		}
		if (c == 'o')	/* set/unset operator */
		{
			if (s == '+')
			{
				if (indexp >= cmd.parameters.size())
				{
					buildError(461, server, user, err_target, err_arg);
					break ;
				}
				std::string nick = cmd.parameters[indexp];
				User* nick_user = server.whoUser(nick);
				if (!nick_user)
				{
					buildError(401, server, user, err_target,nick);
					break ;
				}
				if (!ch->hasUser(nick_user))
				{
					buildError(441, server, user, err_target, nick + " " + cname );
					break ;
				}
				ch->setOperator(nick_user, true);
			}
			else
			{
				if (indexp >= cmd.parameters.size())
				{
					buildError(461, server, user, err_target, err_arg);
					break ;
				}
				std::string nick = cmd.parameters[indexp];
				User* nick_user = server.whoUser(nick);
				if (!nick_user)
				{
					buildError(401, server, user, err_target, nick);
					break ;
				}
				if (!ch->hasUser(nick_user))
				{
					buildError(441, server, user, err_target, nick + " " + cname );
					break ;
				}
				ch->setOperator(nick_user, false);
			}
			indexp++;
			if (prev != s)
			{
				prev = s;
				reply += s;
			}
			reply += c;
			continue ;
		}
		else 
		{
			std::string str;
			str += c;
			buildError(472, server, user, err_target, str);
			break ;
		}
	}
	if (!reply.empty())
	{
		std::string str = ":" + user.getNickName() + "!" + user.getName() + "@";
		str += server.get_name() + " MODE " + cname + " " + reply;
		for (size_t i = 2; i < indexp; i++)
			str += " " + cmd.parameters[i];
		str += "\r\n";
		server.broadcastToChannel(ch, user,str, true);
	}
	return (0);
}

int	Commands::cmdNotice( Server& server, User& user, const cmd_line& cmd ) const
{
	if (cmd.parameters.empty())
		return (0);
	if (!cmd.hasTrailing && cmd.parameters.size() < 2)
		return (0);
	std::string msg;
	if (!cmd.hasTrailing && cmd.parameters.size() >= 2)
		msg = cmd.parameters[1];
	else if (cmd.hasTrailing)
		msg = cmd.trailing;
	else
		return (0);
	std::string list = cmd.parameters[0];
	std::vector<std::string> tname;
	splitList(list, tname, false);
	if (tname.empty()) /* only ',' in list */
		return (0);
	sanitizeString(msg); /* check and erase \r and \n */
	for (size_t pos = 0; pos < tname.size(); pos++)
	{
		if (tname[pos].empty())
			continue;
		if (tname[pos][0] == '#')
		{
			Channel* ch = server.get_channel(tname[pos]);
			if (!ch)
				continue ;
			if (!ch->hasUser(&user))
    			continue;
			std::string str = ":" + user.getNickName() + "!" + user.getName() + "@";
			str += server.get_name() + " NOTICE " + tname[pos];
			str += " :" + msg + "\r\n";
			server.broadcastToChannel(ch, user, str, false);
		}
		else
		{
			std::string	nick = tname[pos];
			User* nick_user = server.whoUser(nick);
			if (!nick_user)
				continue ;
			std::string str = ":" + user.getNickName() + "!" + user.getName() + "@";
			str += server.get_name() + " NOTICE " + tname[pos];
			str += " :" + msg + "\r\n";
			server.sendTo(*nick_user, str);
		}
	}
	return (0);
}

int Commands::cmdPart( Server& server, User& user, const cmd_line& cmd ) const
{
	std::string	err_target = user.getNickName();
	std::string	err_arg = cmd.command;

	if (cmd.parameters.empty())
	{
		buildError(461, server, user, err_target, err_arg);
		return (461);
	}
	std::string list = cmd.parameters[0];
	std::vector<std::string> cname;
	splitList(list, cname, false);
	if (cname.empty()) /* que des ',' */
	{
		buildError(403, server, user, err_target, list);
		return (403);
	}
	for (size_t pos = 0; pos < cname.size(); pos++)
	{
		Channel* ch = server.get_channel(cname[pos]);
		if (!ch)
		{
			buildError(403, server, user, err_target, cname[pos]);
			continue ;
		}
		if (!ch->hasUser(&user))
		{
			buildError(442, server, user, err_target, cname[pos]);
			continue ;
		}

		std::string str = ":" + user.getNickName() + "!" + user.getName() + "@";
		str += server.get_name() + " PART " + cname[pos];
		std::string msg;
		if (cmd.hasTrailing)
			msg = cmd.trailing;
		else if (cmd.parameters.size() >= 2)
			msg = cmd.parameters[1];
		sanitizeString(msg); /* check and erase \r and \n */
		if (!msg.empty())
			str += " :" + msg;
		str += "\r\n";
		server.broadcastToChannel(ch, user, str, true);
		ch->removeMember(&user);
		server.updateChannels(cname[pos]); /* delete channel if no more users */
	}
	return (0);
}

int Commands::cmdPing(Server& server, User& user, const cmd_line& cmd) const
{
	std::string token;

	if (!cmd.parameters.empty())
		token = cmd.parameters[0];
	else if (cmd.hasTrailing)
		token = cmd.trailing;
	sanitizeString(token); /* check and erase \r and \n */
	std::string str = "PONG " + server.get_name();
	if (!token.empty())
    	str += " :" + token;
	str += "\r\n";

	server.sendTo(user, str);
	return (0);
}

int Commands::cmdPong( Server& server, User& user, const cmd_line& cmd ) const
{
	std::string token;

	if (!cmd.parameters.empty())
		token = cmd.parameters[0];
	else if (cmd.hasTrailing)
		token = cmd.trailing;
	sanitizeString(token); /* check and erase \r and \n */
	std::string str = "PONG " + server.get_name();
	if (!token.empty())
    	str += " :" + token;
	str += "\r\n";

	server.sendTo(user, str);
	return (0);
}

int Commands::cmdPrivMsg( Server& server, User& user, const cmd_line& cmd ) const
{
	std::string	err_target = user.getNickName();
	std::string	err_arg = cmd.command;

	if (cmd.parameters.empty())
	{
		buildError(411, server, user, err_target, err_arg);
		return (411);
	}
	if (!cmd.hasTrailing && cmd.parameters.size() < 2)
	{
		buildError(412, server, user, err_target, "");
		return (412);
	}
	std::string msg;
	if (!cmd.hasTrailing && cmd.parameters.size() >= 2)
		msg = cmd.parameters[1];
	else if (cmd.hasTrailing)
		msg = cmd.trailing;
	else
	{
		buildError(412, server, user, err_target, "");
		return (412);
	}
	std::string list = cmd.parameters[0];
	std::vector<std::string> tname;
	splitList(list, tname, false);
	if (tname.empty()) /* only ',' in list */
	{
		buildError(411, server, user, err_target, err_arg);
		return (411);
	}
	sanitizeString(msg); /* check and erase \r and \n */
	for (size_t pos = 0; pos < tname.size(); pos++)
	{
		if (tname[pos].empty())
			continue;
		if (tname[pos][0] == '#')
		{
			Channel* ch = server.get_channel(tname[pos]);
			if (!ch)
			{
				buildError(403, server, user, err_target, tname[pos]);
				continue ;
			}
			if (!ch->hasUser(&user))
			{
    			buildError(404, server, user, err_target, tname[pos]);
    			continue;
			}
			std::string str = ":" + user.getNickName() + "!" + user.getName() + "@";
			str += server.get_name() + " PRIVMSG " + tname[pos];
			str += " :" + msg + "\r\n";
			server.broadcastToChannel(ch, user, str, false);
		}
		else
		{
			std::string	nick = tname[pos];
			User* nick_user = server.whoUser(nick);
			if (!nick_user)
			{
				buildError(401, server, user, err_target, nick);
				continue ;
			}
			std::string str = ":" + user.getNickName() + "!" + user.getName() + "@";
			str += server.get_name() + " PRIVMSG " + tname[pos];
			str += " :" + msg + "\r\n";
			server.sendTo(*nick_user, str);
		}
	}
	return (0);
}

int Commands::cmdQuit(Server& server, User& user, const cmd_line& cmd) const
{
	if (!user.is_authenticated())
		return (-1);/* no broadcasting so disconnection in this poll loop*/
	if (debug) std::cout << "cmdQuit isInChannel1: " << !isInChannels(server, user) << std::endl;
	if (!isInChannels(server, user))
	{
		if (debug) std::cout << "cmdQuit isInChannel2: " << !isInChannels(server, user) << std::endl;
		return (-1);/* no broadcasting so disconnection in this poll loop*/
	}
	std::string str = ":" + user.getNickName() + "!" + user.getName() +
					  "@" + server.get_name() + " QUIT :";
	std::string msg;
	if (cmd.hasTrailing)
		msg = cmd.trailing;
	else if (cmd.parameters.size() >= 1)
		msg = cmd.parameters[0];
	sanitizeString(msg); /* check and erase \r and \n */
	if (!msg.empty())
		str += msg + "\r\n";
	else
		str += "Client quiting\r\n";
	if (debug) std::cout << "cmdQuit reach before broadcast" << std::endl;
	server.broadcastToChannels(user, str, false);
	user.set_shouldClose(true);
	return (0);
}

int	Commands::cmdTopic( Server& server, User& user, const cmd_line& cmd ) const
{
	std::string	err_target = fixedString(user.getNickName());
	std::string	err_arg = cmd.command;

	if (cmd.parameters.empty() || cmd.parameters.size() > 2)
	{
		buildError(461, server, user, err_target, err_arg);
		return (461);
	}
	std::string cname = cmd.parameters[0];
	Channel* ch = server.get_channel(cname);
	if (!ch)
	{
		buildError(403, server, user, err_target,cname);
		return (403);
	}
	if (!ch->hasUser(&user))
	{
		buildError(442, server, user, err_target, cname);
		return (442);
	}
	if (!cmd.hasTrailing && cmd.parameters.size() == 1)	/* read mode */
	{
		if (ch->getTopic().empty())
			buildReply(331, server, user, *ch, cmd);
		else 
			buildReply(332, server, user, *ch, cmd);
		return (0);
	}
	/* write mode */
	std::string msg;
	if (!cmd.hasTrailing)
		msg = cmd.parameters[1];
	else if (cmd.hasTrailing)
		msg = cmd.trailing;
	if (ch->isTopicProtected() && !ch->isOperator(&user))
	{
		buildError(482, server, user, err_target, cname);
		return (482);
	}
	sanitizeString(msg); /* check and erase \r and \n */
	ch->setTopic(msg);
	std::string str = ":" + user.getNickName() + "!" + user.getName() + "@";
	str += server.get_name() + " TOPIC " + cname + " :" + msg + "\r\n";
	server.broadcastToChannel(ch, user, str, true);	
	return (0);
}

int Commands::setNick( Server& server, User& user, const cmd_line& cmd ) const
{
	bool	anc_reg = user.is_authenticated();
	std::string	str;
	std::string	err_target = fixedString(user.getNickName());
	std::string	err_arg = cmd.command;

	if (cmd.parameters.size() > 1 || (cmd.parameters.size() == 1 && cmd.hasTrailing))
	{
		buildError(461, server, user, err_target, err_arg);
		return (461);
	}
	if (cmd.parameters.size() == 0 && !cmd.hasTrailing)
	{
		buildError(431, server, user, err_target, "");
		return (431);
	}
	std::string	newNick;
	if (cmd.parameters.size() == 1 && !cmd.hasTrailing)
		newNick = cmd.parameters[0];
	else if (cmd.parameters.size() == 0 && cmd.hasTrailing)
		newNick = cmd.trailing;
	if (!isValidNick(newNick))
	{
		buildError(432, server, user, err_target, newNick);
		return (432);
	}
	const std::string oldNick = user.getNickName();
    const std::map<std::string, int>& nts = server.get_nick();
	std::map<std::string, int>::const_iterator it = nts.find(newNick);
	if (it != nts.end() && it->second != user.getFd())
    {
		buildError(433, server, user, err_target, newNick);
		return (433);
	}
	if (oldNick == newNick)
		return (0);
	server.updateNick(oldNick, newNick, user.getFd());
	user.set_nick(newNick);
	if (anc_reg == false)
	{
		checkWelcome(user, server);
		return (0);
	}
	else
	{
		/***** Broadcasting to all channels where user is *****/
		std::string str = ":" + oldNick + "!" + user.getName() + "@";
		str += server.get_name() + " NICK :" + newNick + "\r\n";
		server.broadcastToChannels(user, str, true);
		return (0);
	}
}

int Commands::setUser( Server& server, User& user, const cmd_line& cmd ) const
{
	std::string	str;
	std::string	err_target = fixedString(user.getNickName());
	std::string	err_arg = cmd.command;

	if (user.is_authenticated() == true)
	{
		buildError(462, server, user, err_target, "");
		return (462);
	}
	if (cmd.parameters.size() < 3)
	{
		buildError(461, server, user, err_target, err_arg);
		return (461);
	}
	if (!cmd.hasTrailing && cmd.parameters.size() < 4)
	{
		buildError(461, server, user, err_target, err_arg);
		return 461;
	}

	if (!isValidUser(cmd.parameters[0]))
	{
		buildError(461, server, user, err_target, err_arg);
		return (461);
	}
	user.set_name(cmd.parameters[0]);
	checkWelcome(user, server);
	return (0);
}

int Commands::debugOn( Server& server, User& user, const cmd_line& cmd ) const
{
	(void)server;
	(void)user;
	(void)cmd;
	debug = true;
	return (0);
}

int Commands::debugOff( Server& server, User& user, const cmd_line& cmd ) const
{
	(void)server;
	(void)user;
	(void)cmd;
	debug = false;
	return (0);
}

int Commands::serverStop( Server& server, User& user, const cmd_line& cmd ) const
{
	(void)server;
	(void)user;
	(void)cmd;
	server_stop = true;
	return (0);
}
