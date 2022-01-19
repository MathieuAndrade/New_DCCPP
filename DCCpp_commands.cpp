//
// Created by Mathieu Andrade on 23/05/2021.
//

#include <sstream>
#include <regex>
#include "DCCpp_commands.h"
#include "DCCpp_utils.h"

std::list<std::string> DCCpp_commands::cmdToParse;
CMD_WT_RSP_VECTOR DCCpp_commands::listOfCmdWaitingResp;

void DCCpp_commands::waitSerialCommand()
{
    char buffer[128];
    bool msgClosed = false, result;
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

            multipleCmdAtSameTime:
            i++;
            result = ReadFile(DCCpp::comPort, &buffer[i], 1, &nbOfBytesRead, nullptr);
            if (buffer[0] != '<')
            {
                break;
            }
            else if (result && buffer[i - 1] == '>')
            {
                msgClosed = true;

                // Convert command char to string and save to stack
                std::string command((char const *)buffer);
                DCCpp_commands::cmdToParse.push_back(command);

                // Reset vars for next routine (eventually)
                memset(buffer, 0, sizeof buffer);
                i = 0;

                // Scan immediately to know if multiple message was sent from command station
                result = ReadFile(DCCpp::comPort, &buffer[i], 1, &nbOfBytesRead, nullptr);
                if (result && buffer[0] == '<')
                {
                    // If multiple message was send from command station, restart routine immediately
                    msgClosed = false;
                    goto multipleCmdAtSameTime;
                }
            }
        } while (!msgClosed);
    }
}

void DCCpp_commands::waitWsCommands(const std::string &message)
{
    if (message[0] == '<' && message[message.size() - 1] == '>')
    {
        DCCpp_commands::cmdToParse.push_back(const_cast<std::string &>(message));
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
        // This command need a feedback
        DCCpp_utils::saveCmdWtRsp(TURNOUT_POSITION, TURNOUT_EVENT, args);
        break;
    case LOCO_SPEED:
        // <t 1 1 0 1>
        cmd << "<t"
            << " " << args[0] << " " << args[1] << " " << args[2] << " " << args[3] << ">";
        // This command need a feedback
        DCCpp_utils::saveCmdWtRsp(LOCO_SPEED, LOCO_SPEED_EVENT, args);
        break;
    case LOCO_FUNCTION:
        // <f 3 144>
        cmd << "<f"
            << " " << args[0] << " " << args[1] << ">";
        // This command need a feedback
        DCCpp_utils::saveCmdWtRsp(LOCO_FUNCTION, LOCO_FUNCTION_EVENT, args);
        break;
    case INIT_S88:
        // <Y 8 1>
        cmd << "<Y"
            << " " << args[0] << " " << 1 << ">"; // 1 for hexadecimal format
        break;
    case PING:
        cmd << "<g1>";
        break;
    default:
        cmd << "";
        break;
    }
    return cmd.str();
}

bool DCCpp_commands::sendCommand(const DCC_CMD_TYPE &cmdType, const CMD_ARG args)
{
    int success = 0;
    std::string command = DCCpp_commands::buildCommand(cmdType, args);
    const char *str = command.c_str();

    if (DCCpp::usbMode)
    {
        DWORD dwBytesWritten;
        success = WriteFile(DCCpp::comPort, str, strlen(str), &dwBytesWritten, nullptr);
    }
    else if (DCCpp::ws->getReadyState() != WebSocket::CLOSED)
    {
        DCCpp::ws->send(str);
        success = 1;
    }

    DCCpp_utils::printDebugMessage("DCCpp_commands::sendCommand : " + command);
    return success == 1;
}

void DCCpp_commands::parse()
{
    if (!DCCpp_commands::cmdToParse.empty())
    {
        const std::regex reg("\\<.*?\\>");
        std::cmatch match;
        std::string command = *DCCpp_commands::cmdToParse.begin();

        // Remove command from stack
        DCCpp_commands::cmdToParse.remove(command);
        // Clean the command to get only important things
        std::regex_search(command.c_str(),match,reg);
        // And reassign it for next steps
        command = match[0].str();

        DCCpp_utils::printDebugMessage(command);

        if (command.rfind("<p0>", 1) == 0)
        {
            DCCpp::handleCommandStationStatus(-1);
        }
        else if (command.rfind("<p1>", 1) == 0)
        {
            DCCpp::handleCommandStationStatus(1);
        }
        else if (command.rfind("<g1>", 1) == 0)
        {
            DCCpp_commands::sendCommand(PING);
        }
        else if (command.rfind("<y", 0) == 0)
        {
            DCCpp::handleDetectorUpdate(command);
        }
        else if (command.rfind("<iDCCpp", 0) == 0)
        {
            DCCpp::handleCommandStationVersion(command);
        }
    }
}
