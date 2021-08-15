//
// Created by Mathieu Andrade on 23/05/2021.
//

#ifndef DCCPP_COMMANDS_H
#define DCCPP_COMMANDS_H

#include "DCCpp.h"

class DCCpp_commands
{
  public:
    static void parse(const std::string &command);
    static std::string buildCommand(const DCC_CMD_TYPE &cmdType, const CMD_ARG args = nullptr);
    static bool sendCommand(const DCC_CMD_TYPE &cmdType, const CMD_ARG args = nullptr);
    static void waitSerialCommand();
    static void waitWsCommands(const std::string &message);
};

#endif // DCCPP_COMMANDS_H
