#ifndef PARSER_HPP
# define PARSER_HPP

# include <vector>
# include <string>

struct	cmd_line
{
	std::string					prefix;
	std::string					command;
	std::vector<std::string>	parameters;
	std::string					trailing;
	bool						hasTrailing;

	cmd_line(): hasTrailing( false ){}
};

class Parser
{
	public:
		static cmd_line parsing( const std::string& line );
		static void		print_cmd( const cmd_line& cmd );

	private:
		static void split( const std::string& s, std::vector<std::string>& out );
   		static std::string toUpper (const std::string& s );
};

#endif