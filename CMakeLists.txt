cmake_minimum_required(VERSION 2.6)
project(SimpleUnixChat)
find_library(curses ncurses REQUIRED)
include_directories(common)
add_executable(succlient
    client/main.cpp
    client/UserInterface.cpp
    client/ChatRoom.cpp
    common/Connection.cpp
  )
target_link_libraries(succlient ${curses})
add_executable(sucserver
    server/main.cpp
    server/Server.cpp
    server/ClientConnection.cpp
    common/Connection.cpp
  )
