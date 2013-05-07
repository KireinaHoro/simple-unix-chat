/*
Copyright (C) 2013 Braden Walters
This file is licensed under the MIT Expat License. See LICENSE.txt.
*/

#include <string>
#include <cstring>
#include "ClientConnection.hpp"
#include "Server.hpp"

ClientConnection::ClientConnection(Server& server, int sock,
  std::string& nickname) : Connection(sock), server(server), sock(sock)
{
  //Set nickname.
  this->nickname = nickname;
  putLine(std::string("UGOTZANICKNAMES ") + nickname);
  //List all members.
  const std::list<ClientConnection*> clients = server.getClients();
  if (clients.size() > 0)
  {
    std::string members = "WEGOTZTHESEPEOPLEHERE";
    for (std::list<ClientConnection*>::const_iterator i = clients.begin();
         i != clients.end(); ++i)
      members += std::string(" ") + (*i)->getNickname();
    putLine(members);
  }
  //Broadcast join.
  server.broadcastJoin(this);
}

bool ClientConnection::update()
{
  try
  {
    std::string line = getLine();

    //Requesting nickname.
    if (line.substr(0, strlen("CANIHAZNICKNAMES? ")) == "CANIHAZNICKNAMES? ")
    {
      std::string nickname = line.substr(strlen("CANIHAZNICKNAMES? "));
      //Must be greater than 0 characters and contain no space.
      if (nickname.size() > 0 && nickname.find(' ') == std::string::npos)
      {
        //Must not already exist.
        bool already_exists = false;
        const std::list<ClientConnection*> clients = server.getClients();
        for (std::list<ClientConnection*>::const_iterator i = clients.begin();
             i != clients.end(); ++i)
        {
          if ((*i)->getNickname() == nickname)
          {
            already_exists = true;
            break;
          }
        }

        if (!already_exists)
        {
          std::string old_nick = this->nickname;
          this->nickname = nickname;
          putLine(std::string("UGOTZANICKNAMES ") + nickname);
          server.broadcastNickChange(this, old_nick, nickname);
        }
        else
          putLine("NOBECAUSENO");
      }
      else
        putLine("NOBECAUSENO");
    }
    //Quiting
    else if (line == "BYEBYE")
    {
      server.broadcastQuit(this);
      return false;
    }
    //Messaging entire server.
    else if (line.substr(0, strlen("TELLEVERYONEZ ")) == "TELLEVERYONEZ ")
    {
      std::string message = line.substr(strlen("TELLEVERYONEZ "));
      if (message.size() > 0)
        server.sendMessageToAll(this, message);
    }
    //Messaging individual user.
    else if (line.substr(0, strlen("TELLSOMEONEZ ")) == "TELLSOMEONEZ ")
    {
      std::string recipient_and_message = line.substr(strlen("TELLSOMEONEZ "));
      int break_char = recipient_and_message.find(' ');
      std::string recipient = recipient_and_message.substr(0, break_char);
      std::string message = recipient_and_message.substr(break_char + 1);
      if (recipient.size() > 0 && message.size() > 0)
        server.sendMessageTo(this, recipient, message);
    }
    //Shutdown server.
    else if (line.substr(0, strlen("STOPSTOPSTOP! ")) == "STOPSTOPSTOP! ")
    {
      std::string password = line.substr(strlen("STOPSTOPSTOP! "));
      if (password == "turtles")
      {
        server.halt();
      }
    }
  }
  catch (NetworkFailureException& e)
  {
    return false;
  }
  catch (NoLineException& e)
  {
  }
  return true;
}

const std::string& ClientConnection::getNickname() const
{
  return nickname;
}

int ClientConnection::buildFdSet(fd_set* set, ClientConnection** clients)
{
  int max_socket = 0;
  for (unsigned int i = 0; clients[i]; ++i)
  {
    FD_SET(clients[i]->sock, set);
    if (clients[i]->sock > max_socket)
      max_socket = clients[i]->sock;
  }
  return max_socket;
}

ClientConnection* ClientConnection::findBySocket(int sock,
  ClientConnection** clients)
{
  ClientConnection* result = NULL;
  for (unsigned int i = 0; clients[i]; ++i)
    if (clients[i]->sock == sock)
      result = clients[i];
  return result;
}

