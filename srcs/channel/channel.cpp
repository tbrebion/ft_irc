#include "channel.hpp"

/* CONSTRUCTEURS */

ft_irc::Channel::Channel(void) {}

ft_irc::Channel::Channel(Channel const &rhs)
{
	*this = rhs;
}

ft_irc::Channel::Channel(std::string name) : _name(name) {}

/* DESTRUCTEUR */

ft_irc::Channel::~Channel(void) {}

/* OPERATEURS */

ft_irc::Channel &ft_irc::Channel::operator=(Channel const &rhs)
{
	if (this != &rhs)
	{
		this->_name = rhs._name;
		this->_clients = rhs._clients;
		this->_operators = rhs._operators;
		this->_banned_clients = rhs._banned_clients;
	}
	return (*this);
}

/* GETTERS */

std::string ft_irc::Channel::getName(void) const
{
	return (this->_name);
}

std::vector<ft_irc::Client *> const &ft_irc::Channel::getClients() const {
    return this->_clients;
}



std::vector<ft_irc::Client> const	 &ft_irc::Channel::getOperators(void) const
{
	return (this->_operators);
}

std::vector<ft_irc::Client> const &ft_irc::Channel::getBannedClients(void) const
{
	return (this->_banned_clients);
}

/* SETTERS */

void ft_irc::Channel::setName(std::string const &name)
{
	this->_name = name;
}

void ft_irc::Channel::setClients(std::vector<ft_irc::Client *> const &clients)
{
	this->_clients = clients;
}

void ft_irc::Channel::setOperators(std::vector<ft_irc::Client> const &operators)
{
	this->_operators = operators;
}

void ft_irc::Channel::setBannedClients(std::vector<ft_irc::Client> const &banned_clients)
{
	this->_banned_clients = banned_clients;
}

/* METHODES */

void ft_irc::Channel::addClient(Client *client) {
	std::cout << *this << std::endl;
	this->_clients.push_back(client);
	std::cout << *this << std::endl;
}

void ft_irc::Channel::removeClient(Client const &client)
{
	std::vector<Client *>::iterator it = std::find(this->_clients.begin(), this->_clients.end(), &client);
    if (it != this->_clients.end())
    {
        this->_clients.erase(it);
    }
}

void ft_irc::Channel::addOperator(Client &client)
{
	this->_operators.push_back(client);
}

void ft_irc::Channel::removeOperator(Client const &client)
{
    std::vector<Client>::iterator it = std::find(this->_operators.begin(), this->_operators.end(), client);
    if (it != this->_operators.end())
    {
        this->_operators.erase(it);
    }
}

void ft_irc::Channel::addBannedClient(Client &client)
{
	this->_banned_clients.push_back(client);
}

void ft_irc::Channel::removeBannedClient(Client const &client)
{
    std::vector<Client>::iterator it = std::find(this->_banned_clients.begin(), this->_banned_clients.end(), client);
    if (it != this->_banned_clients.end())
    {
        this->_banned_clients.erase(it);
    }
}

int ft_irc::Channel::isClientBanned(Client const &client) const
{
	for (std::vector<Client>::const_iterator it = _banned_clients.begin(); it != _banned_clients.end(); ++it)
	{
		if (client.getNickname() == it->getNickname())
		{
			return 1;
		}
	}
	return 0;
}


std::ostream& ft_irc::operator<<(std::ostream& os, ft_irc::Channel& Channel)
{
	std::ostringstream oss;
	oss << Channel.getClients().size();
	std::string message = "Channel [name : " + Channel.getName() + ", clients size : " + oss.str() + ", users : ";

	std::vector<Client *> clients = Channel.getClients();

	for (std::vector<Client *>::iterator it = clients.begin(); it != clients.end(); ++it) {
		message += (*it)->getNickname();
		if (it + 1!= clients.end())
			message += ", ";
	}

	message += "]";

    os << message;
    return os;
}