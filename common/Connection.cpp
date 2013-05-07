/*
Copyright (C) 2013 Braden Walters
This file is licensed under the MIT Expat License. See LICENSE.txt.
*/

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include "Connection.hpp"

Connection::Connection(int sock) : sock(sock)
{
  //Non-blocking.
  fcntl(sock, F_SETFL, O_NONBLOCK);
}

Connection::Connection(const std::string& host, int port)
{
  //Make sock.
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1)
    throw NetworkFailureException("Could not open network socket.");
  //Prepare to connect sock.
  struct sockaddr_in socket_address;
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = inet_addr(host.c_str());
  socket_address.sin_port = htons(port);
  //Connect to server.
  if (connect(sock, (struct sockaddr*) &socket_address,
      sizeof(struct sockaddr_in)))
    throw NetworkFailureException("Could not connect to server.");
  //Non-blocking.
  fcntl(sock, F_SETFL, O_NONBLOCK);
}

Connection::~Connection()
{
  close(sock);
}

std::string Connection::getLine()
{
  if (!line_queue.empty() && line_queue.front().second /*Is finished*/)
  {
    std::string ret_val = line_queue.front().first;
    line_queue.pop();
    return ret_val;
  }

  int bytes_read = 0;
  do
  {
    char readbuf[256];
    bytes_read = recv(sock, readbuf, 256, 0);
    if (bytes_read == -1)
    {
      if (errno == EWOULDBLOCK)
        break;
      else
        throw NetworkFailureException("Read error.");
    }

    unsigned int i = 0;
    while (i < bytes_read)
    {
      unsigned int j = i;
      while (readbuf[j] != '\n' && j < bytes_read)
        ++j;

      if (line_queue.empty() || line_queue.back().second /*Is finished*/)
      {
        std::pair<std::string, bool> new_line(std::string(readbuf + i, j),
                                              j != bytes_read ? true : false);
        line_queue.push(new_line);
      }
      else
      {
        std::pair<std::string, bool> cur_line = line_queue.back();
        cur_line.first += std::string(readbuf + i, j);
        cur_line.second = (j != bytes_read);
      }

      i = j + 1;
    }
  } while (bytes_read > 0);

  if (!line_queue.empty() && line_queue.front().second /*Is finished*/)
  {
    std::string ret_val = line_queue.front().first;
    line_queue.pop();
    //HACK: Sometimes '\n' is included, remove it.
    if (ret_val[ret_val.size() - 1] == '\n')
      ret_val.erase(ret_val.begin() + (ret_val.size() - 1));
    return ret_val;
  }

  throw NoLineException();
}

void Connection::putLine(std::string put)
{
  put += '\n';
  send(sock, put.c_str(), put.size(), 0);
}
