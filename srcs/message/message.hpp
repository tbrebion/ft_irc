/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flcarval <flcarval@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/07 22:16:14 by flcarval          #+#    #+#             */
/*   Updated: 2023/03/07 22:56:50 by flcarval         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGE_HPP
# define MESSAGE_HPP

# include <iostream>
# include "../client/client.hpp"

namespace	ft_irc {

	class	Message {

		public:

			Message(void);
			Message(Message const &rhs);
			Message(std::string message);

			~Message(void);

			Message				&operator=(Message const &rhs);

			ft_irc::Client&		getSender(void) const;
			ft_irc::Client&		getReceiver(void) const;
			std::string			getPayload(void) const;
			void				(*getCallback(void))(ft_irc::Client&, ft_irc::Client&, std::string);

			void				setSender(ft_irc::Client& sender);
			void				setReceiver(ft_irc::Client& receiver);
			void				setPayload(std::string payload);
			void				setCallback(void (*callback)(ft_irc::Client&, ft_irc::Client&, std::string));

		private:

			ft_irc::Client		*_sender;
			ft_irc::Client		*_receiver;
			std::string			_payload;
			void				(*_callback)(ft_irc::Client&, ft_irc::Client&, std::string);
	};

}



#endif