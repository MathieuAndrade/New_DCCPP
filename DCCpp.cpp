//
// Created by Mathieu Andrade on 23/05/2021.
//

#include <regex>
#include "DCCpp.h"
#include <dialogs/DCCpp_dlg.cpp>

// Public vars
HWND DCCpp::wnd;
HINSTANCE DCCpp::instance;
WebSocket::pointer DCCpp::ws = nullptr;
char DCCpp::comNumber[5];
char DCCpp::ipAddress[20];
char DCCpp::accessoryCmdType[1];
HANDLE DCCpp::comPort;
std::string DCCpp::version;
bool DCCpp::usbMode = true;
bool DCCpp::powerOn;
bool DCCpp::emulation = false;
unsigned int DCCpp::commandStationStatus;
std::string DCCpp::detectorStates;
int DCCpp::detectorsModuleCount = 8; // 64 detectors = modulesCount * 8 (8 * 8) || 16 nibbles = modulesCount * 8 (2 * 8) || By default set to 64 detectors
FEEDBACK_MSG_VECTOR DCCpp::listOfFeedbackMsg;
FEEDBACK_MSG_VECTOR DCCpp::listOfUnexpectedFbMsg;
LOCO_INFOS_VECTOR DCCpp::listOfLocoInfos;
int DCCpp::cmdTimer = 50;

// Private vars
HANDLE DCCpp::virtualThread;
DWORD DCCpp::virtualThreadId;

bool DCCpp::start(DGI_SERVER_PARAMS params)
{
    bool success;
    DCCpp::wnd = params.hMainWindow;
    DCCpp_utils::getDCCppParams();
    DCCpp::emulation = params.bEmulationMode;

    // Turnout command is by default to turnout mode
    // https://github.com/MathieuAndrade/New_DCCPP/commit/312dddc75dbd694bda2098f2cda9f76b614085c1
    DCCpp::accessoryCmdType[0] = 'T';

    powerOn = false;
    virtualThreadId = 0;

    if (DCCpp::emulation)
    {
        virtualThread = CreateThread(nullptr, 0, loop, nullptr, 0, &virtualThreadId);
        DCCpp_commands::buildCommand(CMD_STATION_VERSION_REQUEST);
        DCCpp::detectorsModuleCount = 36; // For 288 detectors, Enough for testing :)

        MessageBox(DCCpp::wnd, "Mode emulation", "DCCpp serveur emulation", MB_APPLMODAL | MB_OK | MB_ICONINFORMATION);
        return true;
    }

tryToConnect:
    success = DCCpp_dlg::runParamDlg();
    if (success)
    {
        success = DCCpp::connect();
        virtualThread = CreateThread(nullptr, 0, loop, nullptr, 0, &virtualThreadId);

        if (success)
        {
            DCCpp_dlg::runProgressDlg();

            // In usb mode some boards take longer to start
            // So you have to wait until they have finished their initialization
            if(DCCpp::usbMode)
            {
                auto start = std::clock();
                auto elapsed = 0.0;

                while (!DCCpp_commands::cmdStationReady && elapsed < 16)
                {
                    // Check if one second is passed
                    if (elapsed < ((std::clock() - start) / (double)CLOCKS_PER_SEC))
                    {
                        DCCpp_utils::printDebugMessage("DCCpp_commands::start -> waiting to command station get ready");
                        elapsed += 1;
                        DCCpp_dlg::setTimerOfProgressDlg(16 - int(elapsed));
                    }
                }
            }

            int attempts = 0;
            do
            {
                DCCpp_utils::printDebugMessage("DCCpp_commands::start -> try getting command station info");
                DCCpp_commands::buildCommand(CMD_STATION_VERSION_REQUEST);
                attempts++;
                DCCpp_utils::printDebugMessage("DCCpp_commands::start -> command station info requested, waiting 200ms");
                Sleep(200);
            } while (attempts < 6 && DCCpp::version.empty());

            DCCpp_dlg::closeProgressDlg();

            if (attempts >= 6 && DCCpp::version.empty())
            {
                success = false;
            }
        }

        if (!success)
        {
            TerminateThread(virtualThread, 0);
            virtualThread = nullptr;
            DCCpp::closeAnyConnection();
            std::string msg = "Impossible d'etablir la connexion avec la centrale\nSouhaitez-vous reessayer ?";
            int res = MessageBox(DCCpp::wnd, msg.c_str(), " Connexion echoue", MB_APPLMODAL | MB_RETRYCANCEL | MB_ICONWARNING);

            if (res == IDRETRY)
            {
                goto tryToConnect;
            }
        }
    }

    if (success)
    {
        DCCpp_utils::saveAllDCCppParams();
    }
    return success;
}

void DCCpp::stop()
{
    if (virtualThread)
    {
        TerminateThread(virtualThread, 0);
        virtualThread = nullptr;
    }
    DCCpp::disconnect();
}

[[noreturn]] DWORD WINAPI DCCpp::loop(void *)
{
    while (true)
    {
        if (DCCpp::usbMode)
        {
            DCCpp_commands::waitSerialCommand();
        }
        else if (DCCpp::ws != nullptr && DCCpp::ws->getReadyState() != WebSocket::CLOSED)
        {
            DCCpp::ws->poll();
            DCCpp::ws->dispatch(DCCpp_commands::waitWsCommands);
        }

        DCCpp_commands::checkCmdToSend();
        DCCpp_commands::parse();
    }
}

bool DCCpp::connect()
{
    if (DCCpp::usbMode)
    {
        DCCpp::comPort = CreateFile((LPCSTR)&DCCpp::comNumber, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

        if (DCCpp::comPort != INVALID_HANDLE_VALUE)
        {
            DCCpp_utils::setComParams();
            DCCpp_utils::setTimeouts(50);
            Sleep(2000);
            return true;
        }
    }
    else
    {
        return DCCpp::connectToWebSocketServer();
    }
    return false;
}

bool DCCpp::disconnect()
{
    bool success;
    if (DCCpp::comPort != INVALID_HANDLE_VALUE || DCCpp::comPort != nullptr || DCCpp::ws != nullptr)
    {
        success = DCCpp_commands::buildCommand(POWER_OFF);

        if (success)
        {
            DCCpp::closeAnyConnection();
        }
    }
    else
    {
        success = true;
    }
    return success;
}

void DCCpp::closeAnyConnection()
{
    if (DCCpp::usbMode)
    {
        CloseHandle(DCCpp::comPort);
    }
    else
    {
        DCCpp::ws->close();
        DCCpp::ws = nullptr;
        WSACleanup();
    }

    DCCpp::version.clear();
    DCCpp_commands::cmdStationReady = false;
}

bool DCCpp::connectToWebSocketServer()
{
    int result;
    WSADATA wsaData;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        DCCpp_utils::printDebugMessage("WSAStartup Failed.\n");
        return false;
    }

    std::stringstream url;
    url << "ws://" << DCCpp::ipAddress;

    // Check common WS server at first time
    DCCpp::ws = WebSocket::from_url(url.str() += ":81");

    if (DCCpp::ws == nullptr || DCCpp::ws->getReadyState() != WebSocket::OPEN)
    {
        // If common WS server fail, check new url
        DCCpp::ws = WebSocket::from_url(url.str() += "/ws");
        if (DCCpp::ws == nullptr || DCCpp::ws->getReadyState() != WebSocket::OPEN)
        {
            return false;
        }
    }

    assert(DCCpp::ws);
    return true;
}

void DCCpp::initS88()
{
    CMD_ARG args;
    args[0] = DCCpp::detectorsModuleCount;

    DCCpp_utils::clearDetectorStates();

    DCCpp_commands::buildCommand(INIT_S88, args);
}

/*
bool DCCpp::sendLocoInfo(pDGI_GENERIC_DATA genericData)
{
    bool result = false;
    int fbMsgIndex, locoIndex;

    locoIndex = DCCpp_utils::getLocoInfosIndex(genericData->nAddress);

    if (DCCpp::listOfLocoInfos[locoIndex].dataUpdated)
    {
        // Create new feedback message
        fbMsgIndex = DCCpp_utils::saveFeedbackMsg(genericData->pCmdTag, DCCpp::listOfFeedbackMsg);
        DCCpp::handleStandaloneCommands(CMD_LOCO_INFO_REQUEST, fbMsgIndex);

        DCCpp::listOfFeedbackMsg[fbMsgIndex].xDataItem[0].nAddress = DCCpp::listOfLocoInfos[locoIndex].address;
        DCCpp::listOfFeedbackMsg[fbMsgIndex].xDataItem[0].hData[0] = DCCpp::listOfLocoInfos[locoIndex].speed;
        DCCpp::listOfFeedbackMsg[fbMsgIndex].xDataItem[0].hData[1] = DCCpp::listOfLocoInfos[locoIndex].functions;
        DCCpp::listOfFeedbackMsg[fbMsgIndex].xDataItem[0].hData[2] = DCCpp::listOfLocoInfos[locoIndex].functions;
        DCCpp::listOfFeedbackMsg[fbMsgIndex].xDataItem[0].hData[3] = 4095; // Not working for the moment

        DCCpp::listOfLocoInfos[locoIndex].dataUpdated = false;
        result = true;
    }

    return result;
}
 */

bool DCCpp::setLocoSpeed(pDGI_GENERIC_DATA genericData)
{
    bool success = false;
    int index;
    CMD_ARG args;

    if (DCCpp::powerOn)
    {
        args[1] = genericData->nAddress;         // Loco address
        args[2] = genericData->nData[1];         // Loco speed
        args[3] = genericData->nData[3] ? 1 : 0; // Loco forward/backward

        args[2] = args[2] < 0 ? -args[2] : args[2]; // Absolute value for loco speed

        index = DCCpp_utils::getLocoInfosIndex(args[1]);

        if (index != -1)
        {
            // If loco is present in list, check if speed or direction has changed
            if (DCCpp::listOfLocoInfos[index].speed != args[2] || DCCpp::listOfLocoInfos[index].direction != args[3])
            {
                args[0] = index + 1; // Register number used by command station start at 1, so, this is index of loco in list + 1

                DCCpp_utils::saveLocoInfos(index, args[1], args[2], args[3]);
                success = DCCpp_commands::buildCommand(LOCO_SPEED, args);
            }
        }
        else
        {
            // Save loco info and get register number used by command station if is not already present in list
            args[0] = DCCpp_utils::saveLocoInfos(-1, args[1], args[2], args[3]);

            success = DCCpp_commands::buildCommand(LOCO_SPEED, args);
        }
    }
    return success;
}

void DCCpp::emergencyStopAllLocos()
{
    bool success;
    LOCO_INFOS_IT it;
    CMD_ARG args;

    for (it = DCCpp::listOfLocoInfos.begin(); it != DCCpp::listOfLocoInfos.end(); ++it)
    {
        args[0] = (it - DCCpp::listOfLocoInfos.begin()) + 1; // Register number used by command station start at 1, so, this is index of loco in list + 1
        args[1] = it->address;                               // Loco address
        args[2] = -1;                                        // Loco speed
        args[3] = it->direction;                             // Loco forward/backward
        success = DCCpp_commands::buildCommand(LOCO_SPEED, args, true);

        if (success)
        {
            it->emergencyStopped = true;
        }
    }
}

void DCCpp::restartAllLocos()
{
    bool success;
    LOCO_INFOS_IT it;
    CMD_ARG args;

    for (it = DCCpp::listOfLocoInfos.begin(); it != DCCpp::listOfLocoInfos.end(); ++it)
    {
        args[0] = (it - DCCpp::listOfLocoInfos.begin()) + 1; // Register number used by command station start at 1, so, this is index of loco in list + 1
        args[1] = it->address;                               // Loco address
        args[2] = it->speed;                                 // Loco speed
        args[3] = it->direction;                             // Loco forward/backward
        success = DCCpp_commands::buildCommand(LOCO_SPEED, args);

        if (success)
        {
            it->emergencyStopped = false;
        }
    }
}

bool DCCpp::setLocoFunction(pDGI_GENERIC_DATA genericData, unsigned int funcMask)
{
    bool success;
    int index;
    CMD_ARG args;

    args[0] = genericData->nAddress;
    index = DCCpp_utils::getLocoInfosIndex(genericData->nAddress);

    if (index != -1)
    {
        if (DCCpp::listOfLocoInfos[index].functions == -1)
        {
            args[1] = (int)DCCpp_utils::getLocoFuncValue(0, funcMask);
        }
        else
        {
            args[1] = (int)DCCpp_utils::getLocoFuncValue(genericData->nData[0], funcMask);
        }

        DCCpp_utils::saveLocoInfos(index, DCCpp::listOfLocoInfos[index].address, DCCpp::listOfLocoInfos[index].speed, DCCpp::listOfLocoInfos[index].direction, args[1]);
    }
    else
    {
        args[1] = (int)DCCpp_utils::getLocoFuncValue(0, funcMask);
        DCCpp_utils::saveLocoInfos(-1, genericData->nAddress, 0, 1, args[1]);
    }

    success = DCCpp_commands::buildCommand(LOCO_FUNCTION, args);
    return success;
}

/*
 * Standalone commands are requests from CDM-Rail which do not require a direct response from the command station.
 */
void DCCpp::handleStandaloneCommands(STANDALONE_CMD_TYPE cmdType, int feedbackMsgIndex)
{
    switch (cmdType)
    {
    case CMD_OPERATIONS_RESUMED:
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nRdCommand = RCV_NORMAL_OPER_RESUMED;
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nCompletionCode = DGI_COMP_OK_RD;
        break;
    case CMD_POWER_OFF:
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nRdCommand = RCV_TRACK_POWER_OFF;
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nCompletionCode = DGI_COMP_OK_RD;
    case CMD_EMERGENCY_STOP:
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nRdCommand = RCV_EMERGENCY_STOP;
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nCompletionCode = DGI_COMP_OK_RD;
        break;
    case CMD_STATION_STATUS:
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nRdCommand = RCV_CMDSTATION_STATUS;
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nCompletionCode = DGI_COMP_OK_RD;
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].xDataItem[0].hData[0] = DCCpp::commandStationStatus;
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nNbDataItems = 1;
        break;
    case CMD_ACCESSORY_ACTION:
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nRdCommand = RCV_ACCESSORY_INFO;
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nCompletionCode = DGI_COMP_OK;
        break;
    case CMD_LOCO_INFO_REQUEST:
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nRdCommand = RCV_NORMAL_LOCO_INFO;
        DCCpp::listOfFeedbackMsg[feedbackMsgIndex].nCompletionCode = DGI_COMP_OK_RD;
        break;
    default:
        break;
    }
}

void DCCpp::handleCommandStationFb(CMD_STATION_FB_TYPE cmdType, int feedbackMsgIndex)
{
    DCCpp::listOfUnexpectedFbMsg[feedbackMsgIndex].nCompletionCode = DGI_COMP_OK; // By default, is ok

    switch (cmdType)
    {
    case TRACK_POWER_ON:
        DCCpp::listOfUnexpectedFbMsg[feedbackMsgIndex].nRdCommand = RCV_NORMAL_OPER_RESUMED;
        break;
    case TRACK_POWER_OFF:
    case TRACK_EMERGENCY_STOP:
        DCCpp::listOfUnexpectedFbMsg[feedbackMsgIndex].nRdCommand = RCV_TRACK_POWER_OFF;
        break;
    case DETECTOR_INFO:
        DCCpp::listOfUnexpectedFbMsg[feedbackMsgIndex].nRdCommand = RCV_FEEDBACK_BROADCAST;
        break;
    case LOCO_SPEED_EVENT:
    case LOCO_FUNCTION_EVENT:
        DCCpp::listOfUnexpectedFbMsg[feedbackMsgIndex].nRdCommand = RCV_LOCO_OPER_BY_ANOTHER_DEVICE;
        break;
    default:
        break;
    }
}

void DCCpp::handleCommandStationStatus(int state)
{
    int index;
    DCC_CMD_TAG feedbackMsg;

    switch (state)
    {
    case -1:
        // -1 for emergency
        if (DCCpp::powerOn)
        {
            DCCpp::emergencyStopAllLocos();
            feedbackMsg.nCmdTagType = TAG_EMERGENCY_STOP;
            index = DCCpp_utils::saveFeedbackMsg(&feedbackMsg, DCCpp::listOfUnexpectedFbMsg);
            DCCpp::handleCommandStationFb(TRACK_EMERGENCY_STOP, index);

            DCCpp::commandStationStatus += (1 << 0);
            DCCpp::powerOn = false;
        }
        break;
    case 0:
        // 0 for power of
        if (!DCCpp::powerOn)
        {
            feedbackMsg.nCmdTagType = TAG_POWER_OFF;
            index = DCCpp_utils::saveFeedbackMsg(&feedbackMsg, DCCpp::listOfUnexpectedFbMsg);
            DCCpp::handleCommandStationFb(TRACK_POWER_OFF, index);

            DCCpp::commandStationStatus += (1 << 1);
            DCCpp::powerOn = false;
        }
        break;
    case 1:
        // 1 for power on
        // No feedback for power on
        DCCpp::commandStationStatus = 0;
        DCCpp::powerOn = true;
        break;
    default:
        // set emergency stop
        feedbackMsg.nCmdTagType = TAG_EMERGENCY_STOP;
        index = DCCpp_utils::saveFeedbackMsg(&feedbackMsg, DCCpp::listOfUnexpectedFbMsg);
        DCCpp::handleCommandStationFb(TRACK_EMERGENCY_STOP, index);

        DCCpp::commandStationStatus += (1 << 0);
        DCCpp::powerOn = false;
        break;
    }
}

void DCCpp::handleCommandStationVersion(std::string &command)
{
    const std::regex reg("\\V.*?\\ ");
    std::cmatch match;
    std::regex_search(command.c_str(), match, reg);
    DCCpp::version = match[0].str();
}

void DCCpp::handleDetectorUpdate(std::string &command)
{
    int FbIndex, nbOfChanges = 0, nbOfReports = 1, offset = 0;
    int posOfs88NibblesChanges[128]; // 128 nibbles = 512 detectors
    DCC_CMD_TAG feedbackMsg;

    DCCpp_utils::removeCharsFromString(command, (char *)"<y >");

    for (int i = 0; i < (DCCpp::detectorsModuleCount * 2); ++i)
    {
        if (DCCpp::detectorStates[i] != command[i])
        {
            DCCpp::detectorStates[i] = command[i];
            posOfs88NibblesChanges[nbOfChanges] = i;
            nbOfChanges++;
        }
    }

    if (nbOfChanges > 0)
    {
        feedbackMsg.nCmdTagType = TAG_NONE;

        // If nb of s88 changes is higher of max items accepted by CDM
        // calculate the necessary reports count
        if (nbOfChanges > DGI_MAX_DATA_ITEMS)
        {
            nbOfReports = nbOfChanges / DGI_MAX_DATA_ITEMS;
        }

        // Loop until necessary reports are completed
        while (nbOfReports != 0)
        {
            FbIndex = DCCpp_utils::saveFeedbackMsg(&feedbackMsg, DCCpp::listOfUnexpectedFbMsg);
            DCCpp::handleCommandStationFb(DETECTOR_INFO, FbIndex);

            // Set nb of changes to iterate if the nb of S88 changes is too high
            int changes = (nbOfChanges > DGI_MAX_DATA_ITEMS) ? DGI_MAX_DATA_ITEMS : nbOfChanges;

            for (int i = 0; i < changes; i++)
            {
                // Get position of nibble changed accorded with offset,
                // Offset is used if the nb of S88 changes is too high
                int nibbleIndex = posOfs88NibblesChanges[i + offset];

                // Get ascii state in list and convert it to hex
                unsigned char asciiState = DCCpp::detectorStates[nibbleIndex];
                unsigned int hexState = DCCpp_utils::ascii2Hex(&asciiState);

                // Report data
                DCCpp::listOfUnexpectedFbMsg[FbIndex].xDataItem[i].nAddress = 4 * nibbleIndex;
                DCCpp::listOfUnexpectedFbMsg[FbIndex].xDataItem[i].hData[0] = hexState;
            }

            // Finally, set number of changes
            DCCpp::listOfUnexpectedFbMsg[FbIndex].nNbDataItems = changes;
            // Decreases the nb of changes states according to nb of changes reported
            nbOfChanges = nbOfChanges - changes;
            // Increment offset for next loop
            offset = offset + DGI_MAX_DATA_ITEMS;
            // Decreases the nb of reports for next loop
            nbOfReports--;
        }
    }
}

void DCCpp::handleAccessoryEvent(std::string &command)
{
    bool found;
    CMD_WT_RSP_IT it;
    CMD_ARG args;

    if ((std::strcmp(DCCpp::accessoryCmdType, "a") == 0))
    {
        // Simple accessory mode
        DCCpp_utils::removeCharsFromString(command, (char *)"<A/:>");
    }
    else if ((std::strcmp(DCCpp::accessoryCmdType, "X") == 0))
    {
        // Extended accessory mode
        DCCpp_utils::removeCharsFromString(command, (char *)"<x:>");
    }
    else
    {
        // Turnout mode
        DCCpp_utils::removeCharsFromString(command, (char *)"<H>");
    }

    found = DCCpp_utils::findCmdWtRsp(it, ACCESSORY_EVENT, command, 2, 0, args);

    if (found)
    {
        DCCpp_utils::removeCmdWtRsp(it);
    }
    // else is not a command fired by CDM-Rail,
    // so he needs a feedback to handle this event fired by another controller
    // but for the moment, this type of feedback is not implemented in CDM-Rail
}

void DCCpp::handleLocoEvent(std::string &command, CMD_STATION_FB_TYPE fbCmdType)
{
    bool found;
    // int locoAddress, locoIndex;
    // int fbMsgIndex;
    CMD_WT_RSP_IT it;
    CMD_ARG args;
    // DCC_CMD_TAG feedbackMsg;

    if (fbCmdType == LOCO_SPEED_EVENT)
    {
        DCCpp_utils::removeCharsFromString(command, (char *)"<T>");
        found = DCCpp_utils::findCmdWtRsp(it, LOCO_SPEED_EVENT, command, 3, 1, args);
    }
    else
    {
        DCCpp_utils::removeCharsFromString(command, (char *)"<F>");
        found = DCCpp_utils::findCmdWtRsp(it, LOCO_FUNCTION_EVENT, command, 3, 1, args);
    }

    if (found)
    {
        DCCpp_utils::removeCmdWtRsp(it);
    }
    // else is not a command fired by CDM-Rail,
    // so he needs a feedback to handle this event fired by another controller
    // Work in progress
}
