//
// Created by Mathieu Andrade on 23/05/2021.
//

#include <sstream>
#include <regex>
#include "DCCpp_commands.h"
#include "DCCpp_utils.h"

std::list<std::string> DCCpp_commands::cmdToParse;
std::list<std::string> DCCpp_commands::cmdToSend;
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

bool DCCpp_commands::buildCommand(const DCC_CMD_TYPE &cmdType, const CMD_ARG args, bool emergency)
{
    std::stringstream cmd;
    std::string msg;

    switch (cmdType)
    {
    case POWER_ON:
        cmd << "<1>";
        DCCpp_commands::sendCommand(cmd.str());
        break;
    case POWER_OFF:
        cmd << "<0>";
        DCCpp_commands::sendCommand(cmd.str());
        break;
    case CMD_STATION_VERSION_REQUEST:
        cmd << "<s>";
        DCCpp_commands::sendCommand(cmd.str());
        break;
    case ACCESSORY_OPERATION:
        // <X 15 1>
        cmd << "<" << DCCpp::accessoryCmdType
            << " " << args[0] << " " << args[1] << ">";
        // This command need a feedback
        DCCpp_utils::saveCmdWtRsp(ACCESSORY_OPERATION, ACCESSORY_EVENT, args);
        DCCpp_commands::cmdToSend.push_back(cmd.str());
        break;
    case LOCO_SPEED:
        // <t 1 1 0 1>
        cmd << "<t"
            << " " << args[0] << " " << args[1] << " " << args[2] << " " << args[3] << ">";
        // This command need a feedback
        DCCpp_utils::saveCmdWtRsp(LOCO_SPEED, LOCO_SPEED_EVENT, args);
        if(emergency) {
            DCCpp_commands::sendCommand(cmd.str());
        } else {
            DCCpp_commands::cmdToSend.push_back(cmd.str());
        }
        break;
    case LOCO_FUNCTION:
        // <f 3 144>
        cmd << "<f"
            << " " << args[0] << " " << args[1] << ">";
        // This command need a feedback
        DCCpp_utils::saveCmdWtRsp(LOCO_FUNCTION, LOCO_FUNCTION_EVENT, args);
        DCCpp_commands::cmdToSend.push_back(cmd.str());
        break;
    case INIT_S88:
        // <Y 8 1>
        cmd << "<Y"
            << " " << args[0] << " " << 1 << ">"; // 1 for hexadecimal format
        DCCpp_commands::cmdToSend.push_back(cmd.str());
        break;
    case PING:
        cmd << "<g1>";
        DCCpp_commands::sendCommand(cmd.str());
        break;
    default:
        cmd << "";
        break;
    }
    return true;
}

void DCCpp_commands::sendCommand(const std::string &command)
{
    const char *str = command.c_str();

    if (DCCpp::usbMode)
    {
        DWORD dwBytesWritten;
        WriteFile(DCCpp::comPort, str, strlen(str), &dwBytesWritten, nullptr);
    }
    else if (DCCpp::ws->getReadyState() != WebSocket::CLOSED)
    {
        DCCpp::ws->send(str);
    }

    DCCpp_utils::printDebugMessage("DCCpp_commands::sendCommand : " + command);
}

void DCCpp_commands::checkCmdToSend()
{
    if (!DCCpp_commands::cmdToSend.empty() && DCCpp::commandStationStatus == 0)
    {
        std::string command = *DCCpp_commands::cmdToSend.begin();
        DCCpp_commands::cmdToSend.remove(command);
        DCCpp_commands::sendCommand(command);
    }
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
        std::regex_search(command.c_str(), match, reg);
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
            DCCpp_commands::buildCommand(PING);
        }
        else if (command.rfind("<y", 0) == 0)
        {
            DCCpp::handleDetectorUpdate(command);
        }
        else if (command.rfind("<H", 0) == 0)
        {
            DCCpp::handleAccessoryEvent(command);
        }
        else if (command.rfind("<T", 0) == 0 && command.size() > 9)
        {
            DCCpp::handleLocoEvent(command, LOCO_SPEED_EVENT);
        }
        else if (command.rfind("<F", 0) == 0)
        {
            DCCpp::handleLocoEvent(command, LOCO_FUNCTION_EVENT);
        }
        else if (command.rfind("<iDCCpp", 0) == 0)
        {
            DCCpp::handleCommandStationVersion(command);
        }
    }
}
