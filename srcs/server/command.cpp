#include "server.hpp"

void ft_irc::Server::initCommands(void) {
	_commands.insert(std::make_pair("CAP", &Server::cap));
	_commands.insert(std::make_pair("PASS", &Server::pass));
	_commands.insert(std::make_pair("NICK", &Server::nick));
	_commands.insert(std::make_pair("USER", &Server::user));

	_commands.insert(std::make_pair("INVITE", &Server::invite));
	_commands.insert(std::make_pair("JOIN", &Server::join));
	_commands.insert(std::make_pair("KICK", &Server::kick));
	_commands.insert(std::make_pair("LIST", &Server::list));
	_commands.insert(std::make_pair("NAMES", &Server::names));
	_commands.insert(std::make_pair("WHOIS", &Server::whois));
	_commands.insert(std::make_pair("PRIVMSG", &Server::privmsg));
	_commands.insert(std::make_pair("PART", &Server::part));
	_commands.insert(std::make_pair("TOPIC", &Server::topic));
	_commands.insert(std::make_pair("MODE", &Server::mode));

	_commands.insert(std::make_pair("PING", &Server::pong));
	_commands.insert(std::make_pair("QUIT", &Server::quit));

	_commands.insert(std::make_pair("info", &Server::info));

	return ;

}

// INITIALISATION CLIENT

void ft_irc::Server::cap(ft_irc::Message* message, const std::string& param) {
    message->getSender()->setIdle();
	ft_irc::Server *server = message->getServer();
    std::string param2  = param;
    std::string::size_type pos = param2.find('\n');
    std::string first;
    std::string rest;

    if (pos != std::string::npos) {
        first = param2.substr(0, pos);
        rest = param2.substr(pos + 1);
    }

    std::istringstream iss(first);
    std::string command, argument;
    iss >> command >> argument;

    if (argument == "LS")
        ft_irc::cout << server->getName() << "" << " => " << first << std::endl;

    if (!rest.empty() && rest.size() > 1 && argument == "LS") {
        pass(message, rest);
    }

	return ;
}

void ft_irc::Server::pass(ft_irc::Message* message, const std::string& param) {
    message->getSender()->setIdle();
    ft_irc::Server *server = message->getServer();
    std::string param2  = param;
    std::string::size_type pos = param2.find('\n');
    std::string first;
    std::string rest;

    if (pos != std::string::npos) {
        first = param2.substr(0, pos);
        rest = param2.substr(pos + 1);
    }

    cleanLine(first);
    removeAllOccurrences(first, "\n");
    ft_irc::cout << server->getName() << "" << " => " << first << std::endl;
    std::string::size_type space_pos = first.find(' ');
    if (space_pos == std::string::npos){
        server->getClientPointerByFd(message->getSender()->getSockfd())->setPassword("");
    }
    else
        server->getClientPointerByFd(message->getSender()->getSockfd())->setPassword(first.substr(space_pos + 1));

    if (!rest.empty() && rest.size() > 1) {
        nick(message, rest);
    }
	return ;
}

void ft_irc::Server::nick(ft_irc::Message* message, const std::string& param) {
    message->getSender()->setIdle();
    ft_irc::Server *server = message->getServer();
    ft_irc::Client *client = server->getClientPointerByFd(message->getSender()->getSockfd());

    std::string param2 = param;
    std::string::size_type pos = param2.find('\n');
    std::string first;
    std::string rest;

    if (pos != std::string::npos) {
        first = param2.substr(0, pos);
        rest = param2.substr(pos + 1);
    }

    cleanLine(first);
    removeAllOccurrences(first, "\n");

    ft_irc::cout << server->getName() << "" << " => " << first << std::endl;

    std::string::size_type space_pos = first.find(' ');
    std::string nickname = first.substr(space_pos + 1);

    if (nickname.empty()) {
        std::string nick_res = ":" + server->getIp() + " 433 * " + nickname + " NICK :Nickname can't be empty. Usage /nick newNickname\r\n";
        send(client->getSockfd(), nick_res.c_str(), nick_res.length(), 0);
        if (!rest.empty() && rest.size() > 1)
            client->setUserLine(rest);
        return;
    }

    if (!server->parsingNickname(client->getSockfd(), nickname)) {
        std::string nick_res = ":" + server->getIp() + " 433 * " + nickname + " NICK :Nickname is already in use. Usage /nick freeNickname\r\n";
        send(client->getSockfd(), nick_res.c_str(), nick_res.length(), 0);
        if (!rest.empty() && rest.size() > 1)
            client->setUserLine(rest);
        return ;
    }

    if (!client->getNickname().empty()) {
        // L'utilisateur a déjà un nickname, nous devons donc mettre à jour son nickname
        std::string oldNickname = client->getNickname();
        client->setNickname(nickname);

        // Envoie un message d'information à tous les clients du serveur
        std::string msg = ":" + oldNickname + "!" + client->getUsername() + "@" + client->getHost() + " NICK :" + nickname + "\r\n";
        server->sendToAllClients(msg);
    } else {
        // L'utilisateur n'a pas encore de nickname, nous devons simplement enregistrer son nickname
        client->setNickname(nickname);
    }

    if (!rest.empty() && rest.size() > 1)
        user(message, rest);
    else if (!client->isSet() && !client->getUserLine().empty())
        user(message, client->getUserLine());
}

void ft_irc::Server::user(ft_irc::Message* message, const std::string& param) {
    message->getSender()->setIdle();
    ft_irc::Server *server = message->getServer();
    std::string line  = param;

    cleanLine(line);
    removeAllOccurrences(line, "\n");
    ft_irc::cout << server->getName() << "" << " => " << line << std::endl;
    std::string::size_type space_pos1 = line.find(' ');
    std::string::size_type space_pos2 = line.find(' ', space_pos1 + 1);
    std::string::size_type space_pos3 = line.find(' ', space_pos2 + 1);
    std::string::size_type colon_pos = line.find(':');
    std::string::size_type end_pos = line.find('\n');

    std::string username = line.substr(space_pos1 + 1, space_pos2 - space_pos1 - 1);
    std::string hostname = line.substr(space_pos2 + 1, space_pos3 - space_pos2 - 1);
    std::string servername = line.substr(space_pos3 + 1, colon_pos - space_pos3 - 2);
    std::string realname = line.substr(colon_pos + 1, end_pos - 1);

    ft_irc::Client *client = server->getClientPointerByFd(message->getSender()->getSockfd());
    client->setUsername(username);
    client->setHost(hostname);
    client->setServername(servername);
    client->setRealname(realname);

    // if (!server->parsingPassword(client->getPassword())) {
    //     // std::string pass_res = ":NOTICE " + client->getNickname() + " :Invalid password. Please try again.\r\n";
    //     // :your.server.name 464 * :Password required
    //     std::string pass_res = ":" + server->getName() + " 464 * :Password required\r\n";
    //     send(client->getSockfd(), pass_res.c_str(), pass_res.length(), 0);

    //     int fd = client->getSockfd();

    //     // Set a -1 le fd ouvert dans la struct pollfd
    //     for (int i = 1; i <= MAX_CLIENTS; i++) {
    //         if (server->getFds()[i].fd == fd)
    //             server->getFds()[i].fd = -1;
    //     }

    //     // Retirer le client de la liste des clients et supprimer son objet de la mémoire
    //     std::vector<ft_irc::Client *> clients = server->getClients();
    //     for (std::vector<ft_irc::Client *>::iterator it = clients.begin(); it != clients.end(); ++it) {
    //         if (*it == client) {
    //             clients.erase(it);
    //             delete client;
    //             break;
    //         }
    //     }

    //     close(fd);
    //     return ;
    // }

    if (client->getNickname().empty()) {
        client->setUserLine(param);
        return ;
    }

    server->sendIrcResponse(client->getSockfd(), client);

    client->setIsSet(true);

    return ;
}

// METHODS
void ft_irc::Server::join(ft_irc::Message* message, const std::string& param) {
    // On récupère le serveur, le canal et la liste des canaux du serveur
    message->getSender()->setIdle();
    ft_irc::Server *server = message->getServer();
    ft_irc::Channel *channel;
    std::vector<Channel*> *channels = server->getChannels();

    // On supprime le caractère '#' au début du paramètre pour récupérer le nom du canal
    std::string param2 = param;
    size_t pos = param2.find(" ");
    param2 = param2.substr(pos + 1);
    cleanLine(param2);
    removeAllOccurrences(param2, "#");

    if (pos == std::string::npos) {
        std::string chan_res = ":" + server->getIp() + " 461 * " + message->getSender()->getNickname() + " JOIN :Channel name missing. Usage: /join #channel\r\n";
        send(message->getSender()->getSockfd(), chan_res.c_str(), chan_res.length(), 0);
        return ;
    }

    std::vector<std::string> vec_channels;
    std::stringstream ss(param2);
    std::string channel_name;
    while (std::getline(ss, channel_name, ',')) {
        if (channel_name.size() >= 1)
            vec_channels.push_back(channel_name);
    }


    for(std::vector<std::string>::const_iterator itb = vec_channels.begin(); itb != vec_channels.end(); ++itb){


        // Si le canal n'existe pas, on le crée et on l'ajoute à la liste des canaux du serveur
        channel = server->getChannelPointer(*itb);
        if (channel && channel->isClientBanned(*(message->getSender())))
            continue ;
        if (!channel) {
            channel = new Channel(*itb);
            channels->push_back(channel);
        }


        // On ajoute l'utilisateur qui a envoyé le message au canal
        channel->addClient(message->getSender());

        message->getSender()->addChannel(channel);

        // On envoie un message de bienvenue à l'utilisateur qui a rejoint le canal
        std::string msg = message->getSender()->getNickname() + ":" + " JOIN #" + channel->getName() + "\r\n";
        if (send(message->getSender()->getSockfd(), msg.c_str(), msg.length(), 0) == -1) {
            std::cerr << "Error SEND" << std::endl;
        }

        // AU client qui rejoint
        // ------------------------------

        // On envoie la liste des noms des clients présents dans le canal à l'utilisateur qui vient de rejoindre le canal
        std::string names_msg = ":" + message->getServer()->getIp() + " 353 " + message->getSender()->getNickname() + " = #" + channel->getName() + " :";
        const std::vector<Client *>& clients = channel->getClients();

        // On parcourt la liste des clients dans le canal et on ajoute leur nom au message
        for (std::vector<Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
            names_msg += " " + (*it)->getNickname();
        }

        names_msg += " \r\n";

        // A tous les clients presents ds le canal
        // ---------------------------------

        // On envoie le message de la liste des noms des clients présents dans le canal à tous les clients du canal
        for (std::vector<Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
            if (send((*it)->getSockfd(), names_msg.c_str(), names_msg.length(), 0) == -1) {
                std::cerr << "2 ERROR SEND" << std::endl;
            }
        }

        // send JOIN message to all clients in the channel
        std::string join_msg = ":" + message->getSender()->getNickname() + "!" + message->getSender()->getUsername() + "@localhost" + " JOIN #" + channel->getName() + "\r\n";
        std::vector<Client *> channel_clients = channel->getClients();
        for (std::vector<Client *>::iterator it = channel_clients.begin(); it != channel_clients.end(); ++it) {
            if ((*it)->getSockfd() != message->getSender()->getSockfd()) {
                if (send((*it)->getSockfd(), join_msg.c_str(), join_msg.length(), 0) == -1) {
                    std::cerr << "3 ERROR SEND" << std::endl;
                }
            }
        }

        std::vector<std::string> const chanMode = channel->getAllMode();

        for (std::vector<std::string>::const_iterator itChan = chanMode.begin(); itChan != chanMode.end(); ++itChan){
            if (send(message->getSender()->getSockfd(), (*itChan).c_str(), (*itChan).length(), 0) == -1)
                std::cerr << "ERROR SEND" << std::endl;

        }
    }
}

void ft_irc::Server::privmsg(ft_irc::Message* message, const std::string& param) {
    // On récupère le serveur, le canal et la liste des canaux du serveur
    message->getSender()->setIdle();
    ft_irc::Server *server = message->getServer();
    ft_irc::Channel *channel;
    std::string channelName;
    std::string messageText;

    std::string test = param;
    cleanLine(test);
    size_t postwo = test.find(' ');
    std::string param2 = test.substr(postwo + 1);

    std::size_t pos = param2.find('#');
    if (pos != std::string::npos) {
        pos++;
        std::size_t endpos = param2.find(" :", pos);
        if (endpos != std::string::npos) {
            channelName = param2.substr(pos, endpos - pos);
        }
    }

    std::size_t messagePos = param2.find(" :");
    if (messagePos != std::string::npos) {
        messageText = param2.substr(messagePos + 2);
    }

    // Si le paramètre commence par un '#', c'est un message à envoyer à un canal
    if (param2[0] == '#') {
        removeAllOccurrences(param2, "#");

        // On récupère le canal correspondant au nom
        // On check si le channel est dans les channels du Sender
        channel = message->getSender()->getChanPointer(channelName);

        if (!channel) {
            // Si le canal n'existe pas, on envoie un message d'erreur à l'utilisateur
            std::string msg = ":" + server->getIp() + " 401 " + message->getSender()->getNickname() + " " + param2 + " :No such channel\r\n";
            if (send(message->getSender()->getSockfd(), msg.c_str(), msg.length(), 0) == -1) {
                std::cerr << "Error SEND" << std::endl;
            }
        } else {
            // On envoie le message à tous les clients du canal, sauf celui qui a envoyé le message
            std::string msg = ":" + message->getSender()->getNickname() + "!" + message->getSender()->getUsername() + "@" + message->getSender()->getHost() + " PRIVMSG #" + channel->getName() + " :" + messageText + "\r\n";
            std::vector<Client *> channel_clients = channel->getClients();
            for (std::vector<Client *>::iterator it = channel_clients.begin(); it != channel_clients.end(); ++it) {
                if ((*it)->getNickname() != message->getSender()->getNickname()) {
                    if (send((*it)->getSockfd(), msg.c_str(), msg.length(), 0) == -1) {
                        std::cerr << "Error SEND" << std::endl;
                    }
                }
            }
        }
    }

    else {
        // Si le paramètre ne commence pas par un '#', c'est un message à envoyer à un client
        std::string nickname;
        std::string contenu;
        std::size_t colonPos = param2.find(" :");
        if (colonPos != std::string::npos) {
            nickname = param2.substr(0, colonPos);
            contenu = param2.substr(colonPos + 2);
        }

        Client *client = server->getClientPointerByNick(nickname);

        if (!client) {
            // Si le client n'existe pas, on envoie un message d'erreur à l'utilisateur
            std::string msg = ":" + server->getIp() + " 401 " + message->getSender()->getNickname() + " " + nickname + " :No such nick/channel\r\n";
            if (send(message->getSender()->getSockfd(), msg.c_str(), msg.length(), 0) == -1) {
                std::cerr << "Error SEND" << std::endl;
            }
        } else if (client->isBot()) {
            // Si le client est un bot, on traite le message avec la méthode handleMessage() de la classe BotClient
            // std::string messageToSend = ":" + message->getSender()->getNickname() + " PRIVMSG " + client->getNickname() + " :" + contenu + "\r\n";
            // send(message->getSender()->getSockfd(), messageToSend.c_str(), messageToSend.length(), 0);
            client->handleMessage(server->getSockfd(), contenu, client, message->getSender());
        } else {
            // Si le client n'est pas un bot, on envoie le message à ce client
            std::string msg = ":" + message->getSender()->getNickname() + "!" + message->getSender()->getUsername() + "@" + message->getSender()->getHost() + " PRIVMSG " + client->getNickname() + " :" + contenu + "\r\n";
            if (send(client->getSockfd(), msg.c_str(), msg.length(), 0) == -1)
                std::cerr << "Error SEND" << std::endl;
        }
    }
}

void ft_irc::Server::kick(ft_irc::Message* message, const std::string& param) {
    //utilisation (in chann only) : /kick <nom d'utilisateur> <raison>
    std::cerr << "[" << param << "]" << std::endl;
    message->getSender()->setIdle();
    ft_irc::Server *server = message->getServer();
    ft_irc::Channel *channel;
    ft_irc::Client *client;

    std::string param2 = param;
    size_t pos = param2.find(" ");
    param2 = param2.substr(pos + 1);
    cleanLine(param2);
    removeAllOccurrences(param2, "#");
    size_t pos2 = param2.find(" ");
    param2 = param2.substr(pos2 + 1);
    std::string userToKick = param2.substr(0, param2.find(' ')); // equal to user to kick
    size_t pos3 = param2.find(":");
    std::string reasonWhy = param2.substr(pos3 + 1); // equal to reason he is kick
    std::string chann;
    size_t hash_pos = param.find('#');
    size_t space_pos = param.find(' ', hash_pos);
    chann = param.substr(hash_pos+1, space_pos-hash_pos-1);
    size_t colon_pos = chann.find(':');
    chann = chann.substr(0, colon_pos); // equal to channel's name

    channel = message->getSender()->getChanPointer(chann);
    client = server->getClientPointerByNick(userToKick);
    if (!client || !channel)
        return ;
    if (channel->isClient(*client) == 0) // Check if client to kick is in the chann or not
        return ;

    if (channel->isClientOp(*message->getSender()) == 0){ /// Check if sender is operator or not
        // :server.name 482 user1 #chan :You're not channel operator
        std::string notOps_msg = ":" + server->getName(); + " 482 " + message->getSender()->getNickname() + " #" + channel->getName() + " :You're not channel operator\r\n";
        if (send(message->getSender()->getSockfd(), notOps_msg.c_str(), notOps_msg.length(), 0) == -1)
            std::cerr << "Error SEND" << std::endl;
        return ;
    }

    if (channel && client && client->getNickname() != message->getSender()->getNickname()){
        std::string kick_message;

        // kick_message = ":" + message->getSender()->getNickname() + " KICK #" + chann + " " + userToKick + "!" + userToKickUsername + "@" + userToKickHostname + " :" + reasonWhy + "\r\n";

        if (reasonWhy.size() >= 1)
            kick_message = ":" + message->getSender()->getNickname() + " " + "KICK" + " #" + chann + " " + client->getNickname() + " :" + reasonWhy + "\r\n";
        else
            kick_message = ":" + message->getSender()->getNickname() + " " + "KICK" + " #" + chann + " " + client->getNickname() + "\r\n";
        for (std::vector<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it) {
            if (send((*it)->getSockfd(), kick_message.c_str(), kick_message.length(), 0) == -1) {
                std::cerr << "Error SEND" << std::endl;
            }
        }

        std::string confirm_msg = "*** You have been kicked from #" + channel->getName() + " by " + message->getSender()->getNickname() + " (" + reasonWhy + ")" + "\r\n";
        if (send(client->getSockfd(), confirm_msg.c_str(), confirm_msg.length(), 0) == -1) {
            std::cerr << "Error SEND" << std::endl;
        }

        channel->removeClient(*client);
        client->removeChannel(*channel);

        std::string names_msg = ":" + message->getServer()->getName() + " 353 " + client->getNickname() + " @ #" + channel->getName() + " :";
        const std::vector<Client *>& clients = channel->getClients();
        std::vector<Client *>::const_iterator it = clients.begin();
        names_msg += (*it)->getNickname();
        ++it;
        for ( ; it != clients.end(); ++it)
            names_msg += " " + (*it)->getNickname();
        names_msg += "\r\n";

        // On envoie le message de la liste des noms des clients présents dans le canal à tous les clients du canal
        for (std::vector<Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
            if (send((*it)->getSockfd(), names_msg.c_str(), names_msg.length(), 0) == -1) {
                std::cerr << "2 ERROR SEND" << std::endl;
            }
        }
    }
	return ;
}

void ft_irc::Server::list(ft_irc::Message* message, const std::string& param) {
    (void)param;
    message->getSender()->setIdle();
    std::string list = message->getServer()->getName() + " => List of channels :\n";
    int fd = message->getSender()->getSockfd();
    std::vector<Channel*> *channels = message->getServer()->getChannels();
    for (std::vector<Channel*>::iterator it = channels->begin(); it != channels->end(); ++it) {
        std::string test = "-- " + (*it)->getName() + "\n";
        list += test;
    }
    send(fd, list.c_str(), list.length(), 0);
    return;
}

void ft_irc::Server::names(ft_irc::Message* message, const std::string& param) {
    message->getSender()->setIdle();
	std::cerr << "NAMES FUNCTION CALLED WITH PARAM = " << param << std::endl;
	return ;
}

void ft_irc::Server::whois(ft_irc::Message* message, const std::string& param) {
    message->getSender()->setIdle();
    ft_irc::Server *server = message->getServer();

    std::string param2 = param;
    size_t pos = param2.find(" ");
    param2 = param2.substr(pos + 1);
    ft_irc::Client  *client = server->getClientPointerByNick(param2);
    if (client){
        std::vector<ft_irc::Channel*> channs = client->getChannels();
        time_t now = time(0);
        time_t idle = now - client->getIdle();
        std::stringstream ss;
        ss << idle;
        std::stringstream ss2;
        ss2 << client->getSignon();
        std::string whois_msg = ":" + server->getIp() + " 311 " + message->getSender()->getNickname() + " " + client->getNickname() + " ~" + client->getNickname() + "@localhost" + " * " + client->getNickname() + " " + client->getRealname() + "\r\n";
        std::string whois_chan_msg = ":" + server->getIp() + " 319 " + message->getSender()->getNickname() + " " + client->getNickname() + " :";
        std::vector<ft_irc::Channel*>::iterator it = channs.begin();
        if (channs.size() > 0){
            whois_chan_msg += "#" + (*it)->getName();
            it++;
            for ( ; it != channs.end(); ++it)
                whois_chan_msg += " #" + (*it)->getName();
        }
        whois_chan_msg += "\r\n";
        std::string whois_time_msg = ":" + server->getIp() + " 317 " + message->getSender()->getNickname() + " " + client->getNickname() + " " +  ss.str() + " " + ss2.str() + " :seconds idle, signon time" + "\r\n";
        std::string whois_end_msg = ":" + server->getIp() + " 318 " + message->getSender()->getNickname() + " " + client->getNickname() + " :End of /WHOIS list\r\n";

        std::cerr << whois_chan_msg << std::endl;
        if (send(message->getSender()->getSockfd(), whois_msg.c_str(), whois_msg.length(), 0) == -1) {
            std::cerr << "Error SEND" << std::endl;
        }
        if (send(message->getSender()->getSockfd(), whois_chan_msg.c_str(), whois_chan_msg.length(), 0) == -1) {
            std::cerr << "Error SEND" << std::endl;
        }
        if (send(message->getSender()->getSockfd(), whois_time_msg.c_str(), whois_time_msg.length(), 0) == -1) {
            std::cerr << "Error SEND" << std::endl;
        }
        if (send(message->getSender()->getSockfd(), whois_end_msg.c_str(), whois_end_msg.length(), 0) == -1) {
            std::cerr << "Error SEND" << std::endl;
        }

    } else{


    }
	return ;
}

void ft_irc::Server::part(ft_irc::Message* message, const std::string& param) {

    // case 1 (out chann) /part <chan> <reasonWhy>
    // case 2 (in chann) /part <reasonWhy>
	std::cerr << "PART FUNCTION CALLED WITH PARAM = " << param << std::endl;
    message->getSender()->setIdle();
    ft_irc::Server *server = message->getServer();
    ft_irc::Channel *channel;
    ft_irc::Client *sender = message->getSender();

    std::string param2 = param;
    size_t pos = param2.find(" ");
    param2 = param2.substr(pos + 1);
    cleanLine(param2);
    removeAllOccurrences(param2, "#");
    param2 = param2.substr(0, param2.find(' ')); // param2 = channel or the list of many channels
    std::vector<std::string> channels;
    std::stringstream ss(param2);
    std::string channel_name;
    while (std::getline(ss, channel_name, ','))
        channels.push_back(channel_name);
    size_t pos2 = param.find(" ");
    std::string param3 = param.substr(pos2 + 1);
    pos2 = param3.find(" ");
    param3 = param3.substr(pos2 + 1);
    removeAllOccurrences(param3, ":"); // param3 = reasonWhy he leave
    if (param.size() <= (5 + param2.size() + 1))
        param3 = "";

    if (param2 == "") {
        std::string chan_res = ":" + server->getIp() + " 461 * " + sender->getNickname() + " PART :Channel name missing. Usage: /part #channel\r\n";
        send(sender->getSockfd(), chan_res.c_str(), chan_res.length(), 0);
        return ;
    }

    for(std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it){

        channel = sender->getChanPointer(*it);
        if (!channel)
            continue ;
        std::string part_msg;
        if (param3.size() >= 1)
            part_msg = ":" + sender->getNickname() + "!"  + sender->getUsername() + "@localhost PART #" + channel->getName() + " :" + param3 + "\r\n";
        else
            part_msg = ":" + sender->getNickname() + "!"  + sender->getUsername() + "@localhost PART #" + channel->getName() + "\r\n";

        for (std::vector<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it) {
            if (send((*it)->getSockfd(), part_msg.c_str(), part_msg.length(), 0) == -1) {
                std::cerr << "Error SEND" << std::endl;
            }
        }
        // :<server>  PART  <channel> :Goodbye

        // Send a message to confirm the client's departure
        // std::string confirm_msg = ":" + server->getName() + " 301 " + sender->getNickname() + " #" + channel->getName() + " :Goodbye!\r\n";
        // std::string confirm_msg = ":" + message->getServer()->getName() + " PART #" + channel->getName() + " :Goodbye\r\n";
        // std::string confirm_msg = ":localhost PART #" + channel->getName() + " :Goodbye\r\n";
        // if (send(sender->getSockfd(), confirm_msg.c_str(), confirm_msg.length(), 0) == -1) {
            // std::cerr << "Error SEND" << std::endl;
        // }
        sender->removeChannel(*channel);
        channel->removeClient(*sender);

        std::string names_msg = ":" + message->getServer()->getIp() + " 301 " + sender->getNickname() + " = #" + channel->getName() + " :";
        const std::vector<Client *>& clients = channel->getClients();

        // On parcourt la liste des clients dans le canal et on ajoute leur nom au message
        for (std::vector<Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it)
                names_msg += " " + (*it)->getNickname();
        names_msg += " \r\n";

        // On envoie le message de la liste des noms des clients présents dans le canal à tous les clients du canal
        for (std::vector<Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
            if (send((*it)->getSockfd(), names_msg.c_str(), names_msg.length(), 0) == -1) {
                std::cerr << "2 ERROR SEND" << std::endl;
            }
        }
        privmsg(message, "PRIVMSG #" + channel->getName() + " ");
    }

}

void	ft_irc::Server::pong(ft_irc::Message* message, const std::string& param) {
	message->getSender()->setIdle();
	ft_irc::Server *server = message->getServer();
	std::string pong_msg = ":" + server->getIp() + " PONG " + server->getIp() + " :" + param + "\r\n";
	if (send(message->getSender()->getSockfd(), pong_msg.c_str(), pong_msg.length(), 0) == -1) {
		std::cerr << "Error SEND" << std::endl;
	}
}

void	ft_irc::Server::quit(ft_irc::Message* message, const std::string& param) {
	message->getSender()->setIdle();
	ft_irc::Server *server = message->getServer();
    std::vector<ft_irc::Channel *> vec = message->getSender()->getChannels();
	for (std::vector<ft_irc::Channel *>::iterator it = vec.begin(); it != vec.end(); ++it) {
		std::cout << "Channel pointer: " << *it << std::endl;
		std::cout << *it << std::endl;
		std::cout << "Channel name: [" << (*it)->getName() << ']' << std::endl;
		ft_irc::Server::part(message, (*it)->getName());
	}
	std::string quit_msg = ":" + message->getSender()->getNickname() + "!" + message->getSender()->getNickname() + "@localhost QUIT :" + param + "\r\n";
	server->sendToAllClients(quit_msg);
	server->closeClient(message->getSender()->getSockfd());
}


void	ft_irc::Server::info(ft_irc::Message* message, const std::string& param){
	(void)param;
    message->getSender()->setIdle();
	ft_irc::Server *server = message->getServer();
	std::stringstream stream;
	stream << "> name: " << server->_name << std::endl;
	stream << "> IP: " << server->_ip << std::endl;
	stream << "> port: " << server->_port << std::endl;
	stream << "> password: " << "**********" << std::endl;
	stream << "> max clients: " << MAX_CLIENTS << std::endl;
	stream << "> uptime: " << server->HRuptime() << std::endl;
	stream << "> channels: " << server->_channels.size() << std::endl;
	stream << "> clients: " << server->_clients.size() << std::endl;
	std::vector<std::string> lines = split(stream.str(), "\n");
	for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
		if (*it != ""){
			std::stringstream ss;
			colors::bold(ss);colors::random(ss);colors::on_blue(ss);
			ss << "    " << format(*it,42);
			colors::reset(ss);
			ss << "\r\n";
			std::string messageToSend = ss.str();
			if (send(message->getSender()->getSockfd(), messageToSend.c_str(), messageToSend.length(), 0) == -1){
                std::cerr << "Error in Server::info()" << std::endl;
            }
		}
	}
    return ;
}

void ft_irc::Server::topic(ft_irc::Message* message, const std::string& param) {

    message->getSender()->setIdle();
    ft_irc::Server *server = message->getServer();
    ft_irc::Channel *channel;

    std::string param2 = param;
    size_t pos = param2.find(" ");
    param2 = param2.substr(pos + 1);
    cleanLine(param2);
    removeAllOccurrences(param2, "#");
    param2 = param2.substr(0, param2.find(' ')); // param2 = channel
    size_t pos2 = param.find(" ");
    std::string param3 = param.substr(pos2 + 1);
    pos2 = param3.find(" ");
    param3 = param3.substr(pos2 + 1);
    removeAllOccurrences(param3, ":"); // param3 = topic


    channel = message->getSender()->getChanPointer(param2);
    if (!channel)
        return ;

    const std::vector<Client *>& clients = channel->getClients();

    if (param.size() <= (6 + param2.size() + 1))
        param3 = "";

    if (param3 == ""){
        std::string topic_noparam;
        if (channel->getTopic() == "")
            topic_noparam = ":" + server->getName() + " 331 " + message->getSender()->getNickname() + "" + channel->getName() + " :No topic is set\r\n";
        else
            topic_noparam = ":" + server->getName() + " 332 " + message->getSender()->getNickname() + "" + channel->getName() + " :" + channel->getTopic() + "\r\n";
        for (std::vector<Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
            if (send((*it)->getSockfd(), topic_noparam.c_str(), topic_noparam.length(), 0) == -1)
                std::cerr << "ERROR SEND" << std::endl;
        }
        return ;
    }

    //change topic

    channel->setTopic(param3);

    std::string topic_msg = ":" + message->getSender()->getNickname() + " TOPIC #" + channel->getName() + " :" + param3 + "\r\n";

    std::cerr << topic_msg << std::endl;

    for (std::vector<Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if (send((*it)->getSockfd(), topic_msg.c_str(), topic_msg.length(), 0) == -1) {
            std::cerr << "ERROR SEND" << std::endl;
        }
    }

	return ;
}

void ft_irc::Server::mode(ft_irc::Message* message, const std::string& param) {

    // :<server> MODE #channel +o user
    message->getSender()->setIdle();
	std::cerr << "MODE FUNCTION CALLED WITH PARAM = " << param << std::endl;
    ft_irc::Server *server = message->getServer();
    ft_irc::Channel *channel;
    ft_irc::Client *client;

    std::string param2 = param;
    size_t pos = param2.find(" ");
    param2 = param2.substr(pos + 1);
    cleanLine(param2);
    removeAllOccurrences(param2, "#");
    param2 = param2.substr(0, param2.find(' '));

    size_t pos2 = param.find(" ");
    std::string param3 = param.substr(pos2 + 1);
    pos2 = param3.find(" ");
    param3 = param3.substr(pos2 + 1);

    size_t pos3 = param3.find(' ');
    std::string param4 = param3.substr(0, pos3);
    std::string param5 = param3.substr(pos3 + 1);

    channel = message->getSender()->getChanPointer(param2);
    client = server->getClientPointerByNick(param5);
    if (!channel || !client)
        return ;

    std::string notOps_msg = ":" + server->getName(); + " 482 " + message->getSender()->getNickname() + " #" + channel->getName() + " :You're not channel operator\r\n";
    if (param4 == "+o")
        channel->addOperator(*client);
    if (param4 == "-o")
        channel->removeOperator(*client);
    if (param4 == "+b"){
        if (channel->isClientOp(*(message->getSender())) == 0){
            if (send(message->getSender()->getSockfd(), notOps_msg.c_str(), notOps_msg.length(), 0) == -1)
                std::cerr << "Error SEND" << std::endl;
            return ;
        }
        channel->addBannedClient(*client);
        kick(message, "KICK #" + channel->getName() + " " + client->getNickname() + "\r\n");
    }
    if (param4 == "-b"){
        if (channel->isClientOp(*(message->getSender())) == 0){
            if (send(message->getSender()->getSockfd(), notOps_msg.c_str(), notOps_msg.length(), 0) == -1)
                std::cerr << "Error SEND" << std::endl;
            return ;
        }
        channel->removeBannedClient(*client);
    }

    std::string mode_msg = ":" + server->getName() + " MODE #" + channel->getName() + " " + param4 + " " + client->getNickname() + "\r\n";

    channel->updateMode(mode_msg);

    const std::vector<Client *>& vec = channel->getClients();
    for (std::vector<Client *>::const_iterator it = vec.begin(); it != vec.end(); ++it){
        if (send((*it)->getSockfd(), mode_msg.c_str(), mode_msg.length(), 0) == -1)
            std::cerr << "ERROR SEND" << std::endl;
    }

	return ;
}

void ft_irc::Server::invite(ft_irc::Message* message, const std::string& param) {
    // /invite user #chan
    // :<server-name> INVITE <nickname> <#channel>

    message->getSender()->setIdle();
    ft_irc::Server *server = message->getServer();
    ft_irc::Channel *channel;
    ft_irc::Client *client;

    std::string userStr = param;
    size_t pos = userStr.find(" ");
    userStr = userStr.substr(pos + 1);
    cleanLine(userStr);
    userStr = userStr.substr(0, userStr.find(' '));
    size_t pos2 = param.find(" ");
    std::string chanStr = param.substr(pos2 + 1);
    pos2 = chanStr.find(" ");
    chanStr = chanStr.substr(pos2 + 1);
    removeAllOccurrences(chanStr, "#");

    channel = message->getSender()->getChanPointer(chanStr);
    client = server->getClientPointerByNick(userStr);

    if (!channel || !client || client->getNickname() == message->getSender()->getNickname())
        return ;
    if (channel->isClientOp(*message->getSender()) == 0) /// Check if sender is operator or not
        return ;

    std::string invite_msg = ":" + message->getSender()->getNickname() + "!" + message->getSender()->getNickname() + "@localhost INVITE " + client->getNickname() + " #" + channel->getName() + "\r\n";
    if (send(client->getSockfd(), invite_msg.c_str(), invite_msg.length(), 0) == -1)
        std::cerr << "ERROR SEND" << std::endl;
    return ;
}
