//
// Created by Mathieu Andrade on 23/05/2021.
//

#ifndef DCCPP_COMMANDS_H
#define DCCPP_COMMANDS_H

#include <list>
#include "DCCpp.h"

class DCCpp_commands
{
  public:
    static std::list<std::string> cmdToParse;
    static std::list<std::string> cmdToSend;
    static CMD_WT_RSP_VECTOR listOfCmdWaitingResp;

    static void parse();
    static bool buildCommand(const DCC_CMD_TYPE &cmdType, const CMD_ARG args = nullptr, bool emergency = false);
    static void sendCommand(const std::string &command);
    static void checkCmdToSend();
    static void waitSerialCommand();
    static void waitWsCommands(const std::string &message);
};

#endif // DCCPP_COMMANDS_H
