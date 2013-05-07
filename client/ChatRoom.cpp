/*
Copyright (C) 2013 Braden Walters
This file is licensed under the MIT Expat License. See LICENSE.txt.
*/

#include <cstring>
#include <algorithm>
#include "ChatRoom.hpp"

ChatRoom::ChatRoom() : server(NULL), new_chatroom(NULL), nickname("You")
{
  members.push_back(nickname);
}

ChatRoom::ChatRoom(Connection* in_server) : new_chatroom(NULL),
  nickname("You")
{
  members.push_back(nickname);
  server = in_server;
  history.push_back(std::string("Established connection."));
}

ChatRoom::~ChatRoom()
{
  if (new_chatroom)
    delete new_chatroom;
  delete server;
}

SendMessageInfo ChatRoom::sendMessage(std::string& message)
{
  SendMessageInfo out;
  if (message.size() > 0 && message[0] == '/')
  {
    //Connect
    if (message.substr(0, strlen("/connect ")) == "/connect ")
    {
      std::string host = message.substr(strlen("/connect "));
      if (host.size() > 0)
      {
        try
        {
          out.new_chatroom = new ChatRoom(new Connection(host, 5788));
        }
        catch (NetworkFailureException& e)
        {
          history.push_back(std::string("Network failure: ") + e.what());
        }
      }
      else
        history.push_back("Please insert host.");
    }
    //Quit
    else if (message == "/quit")
    {
      out.quit = true;
      if (server)
        server->putLine("BYEBYE");
    }
    //Get list of users
    else if (message == "/who")
    {
      std::string to_history = "Members are: ";
      for (std::list<std::string>::iterator i = members.begin();
           i != members.end(); ++i)
        to_history += (*i) + ", ";
      history.push_back(to_history);
    }
    //View previous chatroom
    else if (message == "/prev")
      out.switch_chat_dir = -1;
    //View next chatroom
    else if (message == "/next")
      out.switch_chat_dir = 1;
    //Change nickname
    else if (message.substr(0, strlen("/name ")) == "/name ")
    {
      std::string nickname = message.substr(strlen("/name "));
      if (nickname.size() > 0 && server)
        server->putLine(std::string("CANIHAZNICKNAMES? ") + nickname);
      else
        history.push_back("Could not set nickname.");
    }
    //Send private message
    else if (message.substr(0, strlen("/msg ")) == "/msg ")
    {
      std::string recipient_and_message = message.substr(strlen("/msg "));
      int break_char = recipient_and_message.find(' ');
      std::string recipient = recipient_and_message.substr(0, break_char);
      std::string message = recipient_and_message.substr(break_char + 1);
      if (recipient.size() > 0 && message.size() > 0 && server)
      {
        server->putLine(std::string("TELLSOMEONEZ ") + recipient +
                        std::string(" ") + message);
        history.push_back(std::string("{{") + recipient + std::string("}} ") +
                          message);
      }
      else
        history.push_back("Cannot send message.");
    }
    //Shutdown server
    else if (message.substr(0, strlen("/shutdown ")) == "/shutdown ")
    {
      std::string password = message.substr(strlen("/shutdown "));
      if (password.size() > 0 && server)
        server->putLine(std::string("STOPSTOPSTOP! ") + password);
    }
    //Invalid command
    else
      history.push_back(std::string("This is not a valid command: ") + message);
  }
  else
  {
    history.push_back(std::string("<") + nickname + std::string("> ") +
                      message);
    if (server)
      server->putLine(std::string("TELLEVERYONEZ ") + message);
  }
  return out;
}

void ChatRoom::addHistory(std::string& to_history)
{
  history.push_back(to_history);
}

const std::list<std::string>& ChatRoom::getHistory() const
{
  return history;
}

bool ChatRoom::update()
{
  if (!server)
    return false;

  try
  {
    std::string line = server->getLine();

    //New nickname
    if (line.substr(0, strlen("UGOTZANICKNAMES ")) == "UGOTZANICKNAMES ")
    {
      std::string nickname = line.substr(strlen("UGOTZANICKNAMES "));
      if (nickname.size() > 0)
      {
        //Replace nickname in member list.
        std::list<std::string>::iterator found = std::find(members.begin(),
          members.end(), this->nickname);
        if (found != members.end())
          *found = nickname;

        this->nickname = nickname;
        history.push_back(std::string("The server assigned you the "
                          "nickname \"") + nickname + std::string("\"."));
      }
      else
      {
        history.push_back("The server provided an invalid nickname. Please "
                          "contact the server administrator.");
      }
    }
    //Member list
    else if (line.substr(0, strlen("WEGOTZTHESEPEOPLEHERE ")) ==
             "WEGOTZTHESEPEOPLEHERE ")
    {
      std::string remaining_members =
        line.substr(strlen("WEGOTZTHESEPEOPLEHERE "));
      while (true)
      {
        int break_char = remaining_members.find(' ');
        std::string member = break_char != std::string::npos ?
                             remaining_members.substr(0, break_char) :
                             remaining_members;
        this->members.push_back(member);
        if (break_char != std::string::npos)
          remaining_members = remaining_members.substr(break_char + 1);
        else
          break;
      }
    }
    //Join
    else if (line.substr(0, strlen("OHYAYNEWPERSON ")) == "OHYAYNEWPERSON ")
    {
      std::string nickname = line.substr(strlen("OHYAYNEWPERSON "));
      if (nickname.size() > 0)
      {
        members.push_back(nickname);
        history.push_back(std::string("User \"") + nickname +
                          std::string("\" has joined."));
      }
    }
    //Quit
    else if (line.substr(0, strlen("IMISSYOUALREADY ")) == "IMISSYOUALREADY ")
    {
      std::string nickname = line.substr(strlen("IMISSYOUALREADY "));
      if (nickname.size() > 0)
      {
        members.remove(nickname);
        history.push_back(std::string("User \"") + nickname +
                          std::string("\" has quit."));
      }
    }
    //User changes nickname
    else if (line.substr(0, strlen("IZNOWCALLED ")) == "IZNOWCALLED ")
    {
      std::string old_and_new = line.substr(strlen("IZNOWCALLED "));
      int break_char = old_and_new.find(' ');
      std::string old_nick = old_and_new.substr(0, break_char);
      std::string new_nick = old_and_new.substr(break_char + 1);
      if (old_nick.size() > 0 && new_nick.size() > 0)
      {
        std::list<std::string>::iterator found = std::find(members.begin(),
          members.end(), old_nick);
        if (found != members.end())
          *found = new_nick;
        history.push_back(std::string("User \"") + old_nick +
                          std::string("\" is now called \"") + new_nick +
                          std::string("\"."));
      }
    }
    //Incoming message to all
    else if (line.substr(0, strlen("TOLDEVERYONE ")) == "TOLDEVERYONE ")
    {
      std::string sender_and_message = line.substr(strlen("TOLDEVERYONE "));
      int break_char = sender_and_message.find(' ');
      std::string sender = sender_and_message.substr(0, break_char);
      std::string message = sender_and_message.substr(break_char + 1);
      if (sender.size() > 0 && message.size() > 0)
        history.push_back(std::string("<") + sender + std::string("> ") +
                          message);
    }
    //Incoming message to you
    else if (line.substr(0, strlen("TOLDU ")) == "TOLDU ")
    {
      std::string sender_and_message = line.substr(strlen("TOLDU "));
      int break_char = sender_and_message.find(' ');
      std::string sender = sender_and_message.substr(0, break_char);
      std::string message = sender_and_message.substr(break_char + 1);
      if (sender.size() > 0 && message.size() > 0)
        history.push_back(std::string("[[") + sender + std::string("]] ") +
                          message);
    }
    //Server shutting down
    else if (line == "HALT!")
    {
      delete server;
      history.push_back("Server halted.");
    }
    return true;
  }
  catch (NetworkFailureException& e)
  {
    history.push_back(std::string("Network failure: ") + e.what());
    history.push_back("Disconnecting. Sorry.");
    delete server;
    return true;
  }
  catch (NoLineException& e)
  {
    return false;
  }
}

std::string ChatRoom::getServerDescription() const
{
  return "Welcome!";
}
