//
// Created by Mathieu Andrade on 23/05/2021.
//

#include <sstream>
#include "DCCpp_commands.h"
#include "DCCpp_utils.h"

void DCCpp_commands::waitCommand()
{
    char buffer[512];
    bool msgClosed = false, result = false;
    DWORD nbOfBytesRead = 0;
    int i = 0;

    DCCpp_utils::setTimeouts(0);
    result = ReadFile(DCCpp::comPort, &buffer[i], 1, &nbOfBytesRead, nullptr);

    if (result && nbOfBytesRead != 0)
    {
        DCCpp_utils::setTimeouts(50);
        do
        {
            nbOfBytesRead = 0;
            i++;
            result = ReadFile(DCCpp::comPort, &buffer[i], 1, &nbOfBytesRead, nullptr);
            if (buffer[0] != '<')
            {
                break;
            }
            else if (result && buffer[i - 1] == '>')
            {
                msgClosed = true;
            }
        } while (!msgClosed);
    }

    // Convert command char to string and parse it
    if (msgClosed)
    {
        std::string command(reinterpret_cast<char const *>(buffer));
        DCCpp_commands::parse(command);
    }
}

std::string DCCpp_commands::buildCommand(const DCC_CMD_TYPE &cmdType, const CMD_ARG args)
{
    std::stringstream cmd;

    switch (cmdType)
    {
    case POWER_ON:
        cmd << "<1>";
        break;
    case POWER_OFF:
        cmd << "<0>";
        break;
    case CMD_STATION_VERSION_REQUEST:
        cmd << "<s>";
        break;
    case TURNOUT_POSITION:
        // <T 15 1>
        cmd << "<T"
            << " " << args[0] << " " << args[1] << ">";
        break;
    case LOCO_SPEED:
        // <t 1 1 0 1>
        cmd << "<t"
            << " " << args[0] << " " << args[1] << " " << args[2] << " " << args[3] << ">";
        break;
    case LOCO_FUNCTION:
        // <f 3 144>
        cmd << "<f"
            << " " << args[0] << " " << args[1] << ">";
        break;
    case INIT_S88:
        // <Y 8 1>
        cmd << "<Y"
            << " " << args[0] << " " << 1 << ">"; // 1 for hexadecimal format
        break;
    case PING:
        cmd << "<pa>";
        break;
    default:
        cmd << "";
        break;
    }
    return cmd.str();
}

bool DCCpp_commands::sendCommand(const DCC_CMD_TYPE &cmdType, const CMD_ARG args)
{
    int success;
    std::string command = DCCpp_commands::buildCommand(cmdType, args);
    const char *str = command.c_str();

    // success = DCCpp::serial.writeString(str);

    DWORD dwBytesWritten;
    success = WriteFile(DCCpp::comPort, str, strlen(str), &dwBytesWritten, nullptr);

    DCCpp_utils::printDebugMessage("DCCpp_commands::sendCommand : " + command);
    return success == 1;
}

void DCCpp_commands::parse(const std::string &command)
{
    DCCpp_utils::printDebugMessage(command);

    // TODO: Save to stack list

    if (command.rfind("<p0>", 1) == 0)
    {
        DCCpp::handleCommandStationStatus(-1);
    }
    else if (command.rfind("<p1>", 1) == 0)
    {
        DCCpp::handleCommandStationStatus(1);
    }
    else if (command.rfind("<pa>", 1) == 0)
    {
        DCCpp_commands::sendCommand(PING);
    }
    else if (command.rfind("<y", 0) == 0)
    {
        DCCpp::handleDetectorUpdate(const_cast<std::string &>(command));
    }
}
