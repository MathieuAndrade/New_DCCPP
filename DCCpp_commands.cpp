//
// Created by Mathieu Andrade on 23/05/2021.
//

#include <sstream>
#include <regex>
#include "DCCpp_commands.h"
#include "DCCpp_utils.h"
#include "DCCpp_emul.h"

std::list<std::string> DCCpp_commands::cmdToParse;
std::list<std::string> DCCpp_commands::cmdToSend;
CMD_WT_RSP_VECTOR DCCpp_commands::listOfCmdWaitingResp;
std::clock_t DCCpp_commands::timeElapsedSinceLastCmd;
bool DCCpp_commands::cmdStationReady = false;

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
        do
        {
            nbOfBytesRead = 0;

            i++;
            result = ReadFile(DCCpp::comPort, &buffer[i], 1, &nbOfBytesRead, nullptr);

            if (result && buffer[i] == '\n')
            {
                // DCCpp_utils::printDebugMessage("DCCpp_commands::waitSerialCommand -> buffer : " + std::string(buffer));

                if (strstr(buffer, "ready") || strstr(buffer, "SERIAL"))
                {
                    DCCpp_utils::printDebugMessage("DCCpp_commands::waitSerialCommand -> ready state found");
                    DCCpp_commands::cmdStationReady = true;
                }

                // Old gen message : <$message>
                // New gen message : -> $message
                if (buffer[0] == '<' || (buffer[0] == '-' && buffer[1] == '>'))
                {
                    msgClosed = true;

                    // Convert command char to string and save to stack
                    std::string command((const char *)buffer);
                    DCCpp_utils::printDebugMessage("DCCpp_commands::waitSerialCommand -> received serial message : " + command);
                    DCCpp_commands::cmdToParse.push_back(command);

                    // Reset buffer
                    strcpy(buffer, "");
                }
                else
                {
                    // Reset buffer
                    strcpy(buffer, "");
                    msgClosed = true;
                }
            }
        } while (!msgClosed);
    }
}

void DCCpp_commands::waitWsCommands(const std::string &message)
{
    // Old gen message : <$message>
    // New gen message : -> $message
    if ((message[0] == '<' || (message[0] == '-' && message[1] == '>')) && message[message.size() - 1] == '>')
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
        if (!args)
        {
            return false;
        }
        // <X 15 1>
        cmd << "<" << DCCpp::accessoryCmdType << " " << args[0] << " " << args[1] << ">";
        // This command need a feedback
        DCCpp_utils::saveCmdWtRsp(ACCESSORY_OPERATION, ACCESSORY_EVENT, args);
        DCCpp_commands::cmdToSend.push_back(cmd.str());
        break;
    case LOCO_SPEED:
        if (!args)
        {
            return false;
        }
        // <t 1 1 0 1>
        cmd << "<t"
            << " " << args[0] << " " << args[1] << " " << args[2] << " " << args[3] << ">";
        // This command need a feedback
        DCCpp_utils::saveCmdWtRsp(LOCO_SPEED, LOCO_SPEED_EVENT, args);
        if (emergency)
        {
            DCCpp_commands::sendCommand(cmd.str());
        }
        else
        {
            DCCpp_commands::cmdToSend.push_back(cmd.str());
        }
        break;
    case LOCO_FUNCTION:
        if (!args)
        {
            return false;
        }
        // <f 3 144>
        cmd << "<f"
            << " " << args[0] << " " << args[1] << ">";
        // This command need a feedback
        DCCpp_utils::saveCmdWtRsp(LOCO_FUNCTION, LOCO_FUNCTION_EVENT, args);
        DCCpp_commands::cmdToSend.push_back(cmd.str());
        break;
    case INIT_S88:
        if (!args)
        {
            return false;
        }
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
    if (DCCpp::emulation)
    {
        DCCpp_emul::emulResponse(command);
        DCCpp_utils::printDebugMessage("DCCpp_commands::sendCommand : " + command);
        return;
    }

    const char *str = command.c_str();

    if (DCCpp::usbMode)
    {
        DWORD dwBytesWritten;
        WriteFile(DCCpp::comPort, str, strlen(str), &dwBytesWritten, nullptr);
        DCCpp_utils::printDebugMessage("DCCpp_commands::sendCommand : " + command);
    }
    else if (DCCpp::ws->getReadyState() != WebSocket::CLOSED)
    {
        DCCpp::ws->send(str);
        DCCpp_utils::printDebugMessage("DCCpp_commands::sendCommand : " + command);
    }
    else if (DCCpp::ws->getReadyState() == WebSocket::CLOSED)
    {
        DCCpp_utils::printDebugMessage("DCCpp_commands::sendCommand : /!\\ Websocket server has been closed ");
    }
}

void DCCpp_commands::checkCmdToSend()
{
    auto elapsed = (std::clock() - DCCpp_commands::timeElapsedSinceLastCmd) / (double)(CLOCKS_PER_SEC / 1000);

    if (!DCCpp_commands::cmdToSend.empty() && DCCpp::commandStationStatus == 0 && elapsed > DCCpp::cmdTimer)
    {
        std::string command = *DCCpp_commands::cmdToSend.begin();
        DCCpp_commands::cmdToSend.remove(command);
        DCCpp_commands::sendCommand(command);

        DCCpp_commands::timeElapsedSinceLastCmd = std::clock();
    }
}

void DCCpp_commands::parse()
{
    if (!DCCpp_commands::cmdToParse.empty())
    {
        const std::regex oldGen(R"(<(.*?)>)");  // Captures only what's inside <...>
        const std::regex newGen(R"(-> (.+))");  // Captures everything after ->

        std::cmatch match;
        std::string command = *DCCpp_commands::cmdToParse.begin();

        // Remove the command from stack
        DCCpp_commands::cmdToParse.remove(command);

        // Try to match the old format first
        if (std::regex_search(command.c_str(), match, oldGen) && match[1].matched) {
            command = match[1].str();  // Capture only the inside of <...>
        } else if (std::regex_search(command.c_str(), match, newGen) && match[1].matched) {
            command = match[1].str();  // Capture everything after ->
        } else {
            // No recognized format, can be ignored or logged
            command.clear();
        }

        //DCCpp_utils::printDebugMessage(command);

        if (command.rfind("p0", 0) == 0)
        {
            DCCpp::handleCommandStationStatus(-1);
        }
        else if (command.rfind("p1", 0) == 0)
        {
            DCCpp::handleCommandStationStatus(1);
        }
        else if (command.rfind("g1", 0) == 0)
        {
            DCCpp_commands::buildCommand(PING);
        }
        else if (command.rfind('y', 0) == 0)
        {
            DCCpp::handleDetectorUpdate(command);
        }
        else if (command.rfind('H', 0) == 0)
        {
            DCCpp::handleAccessoryEvent(command);
        }
        else if (command.rfind('T', 0) == 0 && command.size() < 12)
        {
            DCCpp::handleLocoEvent(command, LOCO_SPEED_EVENT);
        }
        else if (command.rfind('F', 0) == 0)
        {
            DCCpp::handleLocoEvent(command, LOCO_FUNCTION_EVENT);
        }
        else if (command.rfind("iDCCpp", 0) == 0 || command.rfind("iDCC++", 0) == 0 || command.rfind("Station version", 0) == 0)
        {
            DCCpp::handleCommandStationVersion(command);
        }
    }
}
