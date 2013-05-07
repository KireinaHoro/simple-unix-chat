/*
Copyright (C) 2013 Braden Walters
This file is licensed under the MIT Expat License. See LICENSE.txt.
*/

#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include <list>
#include <exception>
#include <sys/select.h>
#include "Connection.hpp"

class Server;

class ClientConnection : public Connection
{
  public:
    ClientConnection(Server& server, int sock, std::string& nickname);
    bool update();
    const std::string& getNickname() const;
    static int buildFdSet(fd_set* set, ClientConnection** clients);
    static ClientConnection* findBySocket(int sock,
                                          ClientConnection** clients);
  private:
    Server& server;
    int sock;
    std::string nickname;
};

#endif
