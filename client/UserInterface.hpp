/*
Copyright (C) 2013 Braden Walters
This file is licensed under the MIT Expat License. See LICENSE.txt.
*/

#ifndef USERINTERFACE_HPP
#define USERINTERFACE_HPP

#include <vector>
#include <string>
#include "ChatRoom.hpp"

class UserInterface
{
  public:
    UserInterface();
    ~UserInterface();
    void update();
    void draw();
    void returnCursor(unsigned int x = 0);
    bool isRunning();
  private:
    bool running;
    std::vector<ChatRoom*> chatrooms;
    int current_chatroom_index;
    std::string input_line;
};

#endif
