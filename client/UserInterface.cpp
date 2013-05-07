/*
Copyright (C) 2013 Braden Walters
This file is licensed under the MIT Expat License. See LICENSE.txt.
*/

#include <curses.h>
#include <cstring>
#include "UserInterface.hpp"

UserInterface::UserInterface() : running(true)
{
  //Create null chatroom, the welcome screen.
  chatrooms.push_back(new ChatRoom());
  current_chatroom_index = 0;

  //Setup ncurses
  initscr();
  nodelay(stdscr, TRUE);

  //First draw
  returnCursor();
  draw();
}

UserInterface::~UserInterface()
{
  while (!chatrooms.empty())
  {
    delete chatrooms.front();
    chatrooms.erase(chatrooms.begin());
  }
  endwin();
}

void UserInterface::update()
{
  char current_char = getch();

  //Send message if applicable and update chatroom.
  if (current_char == '\n')
  {
    SendMessageInfo smi =
      chatrooms[current_chatroom_index]->sendMessage(input_line);
    if (smi.new_chatroom)
    {
      chatrooms.push_back(smi.new_chatroom);
      current_chatroom_index = chatrooms.size() - 1;
    }
    else if (smi.quit)
    {
      delete chatrooms[current_chatroom_index];
      chatrooms.erase(chatrooms.begin() + current_chatroom_index);
      if (chatrooms.size() > 0)
      {
        current_chatroom_index = (current_chatroom_index == 0) ?
          chatrooms.size() - 1 : current_chatroom_index - 1;
      }
      else
        running = false;
    }
    else if (smi.switch_chat_dir == -1)
    {
      current_chatroom_index = (current_chatroom_index == 0) ?
        chatrooms.size() - 1 : current_chatroom_index - 1;
    }
    else if (smi.switch_chat_dir == 1)
    {
      current_chatroom_index =
        (current_chatroom_index >= chatrooms.size() - 1) ?
        0 : current_chatroom_index + 1;
    }
    returnCursor();
    draw();
    input_line.clear();
  }
  else if (current_char != ERR)
    input_line += current_char;

  //Update all chatrooms
  for (unsigned int i = 0; i < chatrooms.size(); ++i)
  {
    if (chatrooms[i]->update() && i == current_chatroom_index)
      draw();
  }
}

void UserInterface::draw()
{
  //Get current cursor position so it may be reset later.
  int cur_x, cur_y;
  getyx(stdscr, cur_y, cur_x);

  clear();

  int width, height;
  getmaxyx(stdscr, height, width);

  //Create horizontal divider
  char* h_divider = new char[width + 1];
  memset(h_divider, '=', width);
  h_divider[width] = 0;

  //Divide infobar from chat area; Divide chat area from input area.
  mvprintw(1, 0, h_divider);
  mvprintw(height - 2, 0, h_divider);

  //Print info
  mvprintw(0, 0, "%s",
           chatrooms[current_chatroom_index]->getServerDescription().c_str());

  //Copy visible lines from end of history.
  int num_visible_lines = height - 4;
  std::list<std::string> visible_scrollback;
  const std::list<std::string>& history =
    chatrooms[current_chatroom_index]->getHistory();
  std::list<std::string>::const_reverse_iterator iter = history.rbegin();
  for (unsigned int i = 0; i < num_visible_lines && iter != history.rend(); ++i)
  {
    visible_scrollback.push_front(*iter);
    ++iter;
  }
  //Write visible lines.
  unsigned int use_y = 2;
  for (std::list<std::string>::iterator i = visible_scrollback.begin();
       i != visible_scrollback.end(); ++i)
  {
    move(use_y, 0);
    printw(i->c_str());
    ++use_y;
  }

  refresh();
  delete[] h_divider;

  //Reset cursor position
  move(cur_y, cur_x);
}

void UserInterface::returnCursor(unsigned int x)
{
  int width, height;
  getmaxyx(stdscr, height, width);
  move(height - 1, x);
  deleteln();
  curs_set(1);
}

bool UserInterface::isRunning()
{
  return running;
}
