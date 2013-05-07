/*
Copyright (C) 2013 Braden Walters
This file is licensed under the MIT Expat License. See LICENSE.txt.
*/

#ifndef SERVER_HPP
#define SERVER_HPP

#include <exception>
#include <list>
#include <string>
#include "ClientConnection.hpp"

class Server
{
  public:
    Server(int port);
    ~Server();
    void halt();
    bool isRunning();
    void update();
    const std::list<ClientConnection*> getClients() const;
    void sendMessageToAll(ClientConnection* sender, std::string message);
    void sendMessageTo(ClientConnection* sender, std::string recipient,
                       std::string message);
    void broadcastJoin(ClientConnection* joined_client);
    void broadcastQuit(ClientConnection* quit_client);
    void broadcastNickChange(ClientConnection* changed_client,
                             std::string old_nick, std::string new_nick);
  private:
    bool running;
    int listen_sock;
    std::list<ClientConnection*> clients;
    unsigned short current_autogen_nickname;
};

#endif
