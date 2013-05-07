/*
Copyright (C) 2013 Braden Walters
This file is licensed under the MIT Expat License. See LICENSE.txt.
*/

#include <string>
#include "UserInterface.hpp"

int main()
{
  UserInterface ui;
  while (ui.isRunning())
    ui.update();
  return 0;
}
