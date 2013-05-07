/*
Copyright (C) 2013 Braden Walters
This file is licensed under the MIT Expat License. See LICENSE.txt.
*/

#include <iostream>
#include <cstdlib>
#include <signal.h>
#include "Server.hpp"
#include "Connection.hpp"

void serverSignalHandler(int sig)
{
  if (sig == 2)
    exit(0);
}

int main()
{
  signal(SIGINT, serverSignalHandler);
  try
  {
    Server server(5788);
    while (server.isRunning())
      server.update();
  }
  catch (NetworkFailureException e)
  {
    std::cout << e.what() << std::endl;
  }
}
