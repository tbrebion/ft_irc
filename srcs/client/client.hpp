#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <sstream>
# include <vector>
# include <string>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <algorithm>
# include <sys/time.h>
# include "../channel/channel.hpp"
# include "../../includes/utils.hpp"


#include <cstring>
#include <cstdlib>
#include <netdb.h>
#include <arpa/inet.h>


namespace ft_irc
{
	class	Channel;
	class	Server;

	class Client {

		public:

			Client(void);
			Client(Client const &rhs);
			Client(int sockfd, ft_irc::Server *server);

			virtual ~Client(void);

			Client	&operator=(Client const & rhs);
			bool 	operator==(const Client &other) const;

			int							getSockfd(void) const;
			std::string					getNickname(void) const;
			std::string					getUsername(void) const;
			std::string					getRealname(void) const;
			std::string					getPassword(void) const;
			std::string					getHost(void) const;
			std::string					getServername(void) const;
			std::string					getUserLine(void) const;
			std::string					getCommand(void) const;
			time_t						getIdle(void) const;
			time_t						getSignon(void) const;
			std::vector<ft_irc::Channel*>	getChannels(void) const;

			void						setSockfd(int sockfd);
			char						*getBuffer(void);
			void						setBuffer(char *buffer);
			void						setNickname(std::string nickname);
			void						setUsername(std::string username);
			void						setRealname(std::string realname);
			void						setPassword(std::string password);
			void						setHost(std::string host);
			void						setServername(std::string servername);
			void						setUserLine(std::string userLine);
			void						setCommand(std::string userLine);
			void						setIdle(void);
			void						setChannels(std::vector<ft_irc::Channel*> channels);
			void						setIsBot(bool isBot);
			void						setIsSet(bool isSet);
			bool						isBot() const;
			bool						isSet() const;
			void						handleMessage(int serverSockFd, std::string text, Client *bot, Client *receiver);

			std::string					unicorn(void) const;
			std::string					gpt(std::string prompt) const;

			void 						removeChannel(Channel const &channel);
			void 						addChannel(Channel *channel);
			ft_irc::Channel*			getChanPointer(std::string name);

		private:

			ft_irc::Server*				_server;
			bool						_isSet;
			bool						_isBot;
			int							_sockfd;
			std::string					_nickname;
			std::string					_username;
			std::string					_realname;
			std::string					_password;
			std::string					_host;
			std::string					_servername;
			std::string					_userLine;
			std::string					_command;
			time_t						_idle;
			time_t						_signon;
			std::vector<ft_irc::Channel*>	_channels;
	};

std::ostream& operator<<(std::ostream& os, const ft_irc::Client& client);

}

#endif
