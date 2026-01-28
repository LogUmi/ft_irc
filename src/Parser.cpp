#include "Parser.hpp"
#include <iostream>

void	Parser::print_cmd( const cmd_line& cmd )
{
	std::cout << "Parsed command line: \n\tprefix: <" << cmd.prefix << ">\n";
	std::cout << "\tcommand: <" << cmd.command << ">\n";
	for (size_t i = 0; i < cmd.parameters.size(); i++)
		std::cout << "\tparamater " << i + 1 << ": <" << cmd.parameters[i] << ">\n";
	std::cout << "\thasTrailing: <" << cmd.hasTrailing << ">\n";
	std::cout << "\ttrailing: <" << cmd.trailing << ">\n";
}

void	Parser::split( const std::string& s, std::vector<std::string>& tab )
{
	tab.clear();
	size_t	i = 0;

	while (i < s.size())
	{
		while(i < s.size() && (s[i] == 32 || s[i] == '\t')) 
			i++;
		if (i >= s.size())
			return ;
		size_t	j = i;
		while (i < s.size() && !(s[i] == 32 || s[i] == '\t'))
			i++;
		tab.push_back(s.substr(j, i - j));
	}
}

std::string Parser::toUpper( const std::string& s )
{
    std::string t = s;
    for (size_t i = 0; i < t.size(); i++)
		t[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(t[i])));
    return t;
}

cmd_line	Parser::parsing( const std::string& line )
{
	cmd_line	l;
	std::string s = line;
	size_t		pos = 0;
	size_t		limit = 15;

	l.hasTrailing = false;
	/***** getting prefix if exist *****/
	if(!line.empty() && s[0] == ':')
	{
		pos = s.find(' ');
		if (pos == std::string::npos)
			return (l);
		l.prefix = s.substr(1, pos - 1);
		s.erase(0, ++pos);
	}

	/***** getting trailing if exist *****/
	pos = s.find(" :");
	if (pos != std::string::npos)
	{
		l.trailing = s.substr(pos + 2, s.size());
		l.hasTrailing = true;
		s.erase(pos, s.size());
	}
	else 
	{
		pos = s.find("\t:");
		if (pos != std::string::npos)
		{
			l.trailing = s.substr(pos + 2, s.size());
			l.hasTrailing = true;
			s.erase(pos, s.size());
		}
	}

	/***** getting command and paramaters if exists *****/
	std::vector<std::string>	tab;
	split(s, tab);
	if (tab.empty())
		return (l);
	l.command = toUpper(tab[0]);
	if (l.hasTrailing)
		limit = 14;
	if (limit > tab.size())
		limit = tab.size();
	for (pos = 1; pos != limit; pos++)
		l.parameters.push_back(tab[pos]);
	return (l);
}