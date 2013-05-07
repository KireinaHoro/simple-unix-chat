/*
Copyright (C) 2013 Braden Walters
This file is licensed under the MIT Expat License. See LICENSE.txt.
*/

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <exception>
#include <queue>
#include <string>

class NetworkFailureException : public std::exception
{
  public:
    NetworkFailureException(const char* reason) : reason(reason)
    {
    }

    virtual const char* what() const throw()
    {
      return reason;
    }
  private:
    const char* reason;
};

class NoLineException : public std::exception
{
};

class Connection
{
  public:
    Connection(int sock);
    Connection(const std::string& host, int port);
    ~Connection();
    std::string getLine();
    void putLine(std::string put);
  private:
    int sock;
    std::queue<std::pair<std::string, bool /*Is complete*/> > line_queue;
};

#endif
