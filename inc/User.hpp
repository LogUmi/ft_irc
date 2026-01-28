#ifndef USER_HPP
#define USER_HPP
# include <string>
# include <sys/types.h>
# include <iostream>

class User
{
	public:
		explicit	User( int client_fd );
					~User( void );

		void		addBufferIn( char* buffer );
		void		addBufferOut(std::string buffer);
		std::string	extractLine( void );
		void		flushBufferOut( ssize_t n );
		std::string	getBufferOut( void ) const;
		size_t		getBufferInSize ( void ) const;
		size_t		getBufferOutSize ( void ) const;
		int			getFd( void ) const;
		std::string	getName ( void ) const;
		std::string	getNickName( void ) const;
		bool		getPass_ok( void ) const;
		bool		getForceClose( void ) const;
		bool		getShouldClose( void ) const;
		bool		hasPendingOut( void ) const;
		bool		is_authenticated( void ) const;
		bool		lineReady( void ) const;
		std::string	prefix( void ) const;
		void		set_name ( std::string user );
		void		set_nick ( std::string nname );
		void		set_pass_ok( bool pass );
		void		set_forceClose( bool force );
		void		set_shouldClose( bool close );

	private:
		int			fd; // socket file descriptor of the user
		std::string name; // username of the user : name for verification/security
		std::string nick; // nickname of the user :  name display
		std::string buffer_in;
		std::string buffer_out;
		bool		pass_ok;
		bool		forceClose;
		bool		shouldClose;
};

#endif //USER_HPP
