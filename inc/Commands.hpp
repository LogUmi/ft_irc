#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "Channel.hpp"
# include "User.hpp"
# include "Parser.hpp"
# include <map>
# include <string>
# include <ctime>

class Server;

class Commands
{
	public:
		Commands( void );
		~Commands( void );

		struct IrcError
		{
    	const int      	code;		// ex: 451
    	const char*		name;		// ERR_NOTREGISTERED
    	const char*		message;	// "You have not registered"
		};
		static const IrcError 	cmd_errors[];
		static const IrcError 	cmd_reply[];

		int	handle_command(Server& server, User& user, const cmd_line& cmd);
		static std::string	fixedString( const std::string& src );

	private:
		typedef int (Commands::*cmdFn)(Server& server, User&, const cmd_line&) const;
		std::map<std::string, cmdFn>	unauth_cmd;
		std::map<std::string, cmdFn>	auth_cmd;

		int checkPassword( Server& server, User& user, const cmd_line& cmd ) const;
		int cmdCap( Server& server, User& user, const cmd_line& cmd ) const;
		int cmdInvite(Server& server, User& user, const cmd_line& cmd) const;
		int cmdJoin( Server& server, User& user, const cmd_line& cmd ) const;
		int cmdKick( Server& server, User& user, const cmd_line& cmd ) const;
		int cmdMode( Server& server, User& user, const cmd_line& cmd ) const;
		int	cmdNotice( Server& server, User& user, const cmd_line& cmd ) const;
		int cmdPart( Server& server, User& user, const cmd_line& cmd ) const;
		int cmdPing( Server& server, User& user, const cmd_line& cmd ) const;
		int cmdPong( Server& server, User& user, const cmd_line& cmd ) const;
		int cmdPrivMsg( Server& server, User& user, const cmd_line& cmd ) const;
		int	cmdQuit( Server& server, User& user, const cmd_line& cmd ) const;
		int	cmdTopic( Server& server, User& user, const cmd_line& cmd ) const;
		int setNick( Server& server, User& user, const cmd_line& cmd ) const;
		int setUser( Server& server, User& user, const cmd_line& cmd ) const;
		int debugOn( Server& server, User& user, const cmd_line& cmd ) const;
		int debugOff( Server& server, User& user, const cmd_line& cmd ) const;
		int serverStop( Server& server, User& user, const cmd_line& cmd ) const;

		static void	buildError( int code, Server& server, User& user,
								std::string err_target, std::string err_arg );
		static void buildReply( int code, Server& server, User& user,
								Channel& ch, const cmd_line& cmd );
		static void			checkWelcome( User& user, Server& server );
		static void			sanitizeString ( std::string& str );
		static void			splitList( const std::string& s, 
							std::vector<std::string>& tab, bool empty );
		static std::string	ft_uitoa( int n );
		static std::string	getCmode(Channel& ch, User& user);
		static bool			isInChannels( Server& server, User& user);
		static bool			isValidCname( const std::string name );
		static bool			isValidKey( const std::string key );
		static bool			isValidLimit( const std::string& limit, size_t& val);
		static bool			isValidNick( const std::string& nick );
		static bool			isValidUser( const std::string& user );
};
#endif //COMMANDS_HPP
