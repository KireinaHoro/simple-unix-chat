/*
Copyright (C) 2013 Braden Walters
This file is licensed under the MIT Expat License. See LICENSE.txt.
*/

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include "Server.hpp"

Server::Server(int port) : listen_sock(0), current_autogen_nickname(1)
{
  //Make sock.
  listen_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sock == -1)
    throw NetworkFailureException("Could not open network socket.");
  //Prepare to bind sock.
  struct sockaddr_in socket_address;
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
  socket_address.sin_port = htons(port);
  //Bind sock.
  if (bind(listen_sock, (struct sockaddr*) &socket_address,
      sizeof(struct sockaddr_in)) == -1)
    throw NetworkFailureException("Could not bind network socket.");
  //Listen for connections.
  if (listen(listen_sock, 5) == -1)
  {
    close(listen_sock);
    throw NetworkFailureException("Could not listen on network socket.");
  }
}

Server::~Server()
{
  if (listen_sock)
    close(listen_sock);
  for (std::list<ClientConnection*>::iterator i = clients.begin();
       i != clients.end(); ++i)
    delete *i;
}

void Server::halt()
{
  running = false;
  for (std::list<ClientConnection*>::iterator i = clients.begin();
       i != clients.end(); ++i)
    (*i)->putLine("HALT!");
}

bool Server::isRunning()
{
  return running;
}

void Server::update()
{
  //Build pointer array of clients.
  ClientConnection** clients_arr = new ClientConnection*[clients.size() + 1];
  {
    unsigned int i = 0;
    for (std::list<ClientConnection*>::iterator j = clients.begin();
         j != clients.end(); ++j)
    {
      clients_arr[i] = *j;
      ++i;
    }
    clients_arr[i] = NULL;
  }

  fd_set socket_set;
  FD_ZERO(&socket_set);
  FD_SET(listen_sock, &socket_set);
  int max_sock = ClientConnection::buildFdSet(&socket_set, clients_arr);
  if (listen_sock > max_sock)
    max_sock = listen_sock;

  if (select(max_sock + 1, &socket_set, NULL, NULL, NULL) == -1)
    throw NetworkFailureException("Could not determine sockets ready to read.");
  for (int i = 0; i <= max_sock; ++i)
  {
    if (FD_ISSET(i, &socket_set))
    {
      //Accept new client.
      if (i == listen_sock)
      {
        struct sockaddr_in client_address;
        socklen_t sockaddr_in_size = sizeof(struct sockaddr_in);
        int clientsock = accept(listen_sock, (struct sockaddr*) &client_address,
                                &sockaddr_in_size);
        std::stringstream nickname;
        nickname << "TEMP";
        nickname.fill('0');
        nickname.width(10);
        nickname << current_autogen_nickname++;
        std::string nick_s = nickname.str();
        clients.push_back(new ClientConnection(*this, clientsock, nick_s));
      }
      //Read on existing client.
      else
      {
        ClientConnection* use_connection =
          ClientConnection::findBySocket(i, clients_arr);
        if (use_connection)
        {
          if (!use_connection->update())
          //Disconnect client.
          {
            clients.remove(use_connection);
            delete use_connection;
            goto outer_loop;
          }
        }
      }
    }
  }
  outer_loop: {}

  delete[] clients_arr;
}

const std::list<ClientConnection*> Server::getClients() const
{
  return clients;
}

void Server::sendMessageToAll(ClientConnection* sender, std::string message)
{
  for (std::list<ClientConnection*>::iterator i = clients.begin();
       i != clients.end(); ++i)
  {
    if (*i != sender)
    {
      (*i)->putLine(std::string("TOLDEVERYONE ") + sender->getNickname() +
                    std::string(" ") + message);
    }
  }
}

void Server::sendMessageTo(ClientConnection* sender, std::string recipient,
                   std::string message)
{
  for (std::list<ClientConnection*>::iterator i = clients.begin();
       i != clients.end(); ++i)
  {
    if ((*i)->getNickname() == recipient)
      (*i)->putLine(std::string("TOLDU ") + sender->getNickname() +
                    std::string(" ") + message);
  }
}

void Server::broadcastJoin(ClientConnection* joined_client)
{
  for (std::list<ClientConnection*>::iterator i = clients.begin();
       i != clients.end(); ++i)
  {
    if (*i != joined_client)
    {
      (*i)->putLine(std::string("OHYAYNEWPERSON ") +
                    joined_client->getNickname());
    }
  }
}

void Server::broadcastQuit(ClientConnection* quit_client)
{
  for (std::list<ClientConnection*>::iterator i = clients.begin();
       i != clients.end(); ++i)
  {
    if (*i != quit_client)
    {
      (*i)->putLine(std::string("IMISSYOUALREADY ") +
                    quit_client->getNickname());
    }
  }
}

void Server::broadcastNickChange(ClientConnection* changed_client,
                                 std::string old_nick, std::string new_nick)
{
  for (std::list<ClientConnection*>::iterator i = clients.begin();
       i != clients.end(); ++i)
  {
    if (*i != changed_client)
    {
      (*i)->putLine(std::string("IZNOWCALLED ") + old_nick + std::string(" ") +
                    new_nick);
    }
  }
}
