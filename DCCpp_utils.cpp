//
// Created by Mathieu Andrade on 23/05/2021.
//

#include <windows.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include "DCCpp_utils.h"

void DCCpp_utils::printDebugMessage(const std::string &msg)
{
    std::stringstream debugMessage;
    SYSTEMTIME st;
    GetSystemTime(&st);

    // Build debug message with time and header
    debugMessage << st.wHour << ':' << st.wMinute << ':' << st.wSecond << ':' << st.wMilliseconds << " -> CDM-Rail DCCPP Debug message : " << msg << '\n';

    // copying the contents of the string to char array
    char char_array[debugMessage.str().length() + 1];
    strcpy(char_array, debugMessage.str().c_str());

    OutputDebugStringA(char_array); // For Legacy support
    std::cout << char_array;
}

int DCCpp_utils::saveFeedbackMsg(DCC_CMD_TAG *cmdTag, FEEDBACK_MSG_VECTOR &list)
{
    FEEDBACK_MSG newFeedbackMsg;

    if (cmdTag != nullptr)
    {
        newFeedbackMsg.nCmdTagType = cmdTag->nCmdTagType;
        newFeedbackMsg.lTimeTickSchedule = cmdTag->lTimeTickSchedule;
        newFeedbackMsg.lTimeTickDone = cmdTag->lTimeTickDone;
        newFeedbackMsg.nCmdIdx = cmdTag->nCmdIdx;
        newFeedbackMsg.nData = cmdTag->nData;
        newFeedbackMsg.nAddress = cmdTag->nAddress;
        newFeedbackMsg.pObjPtr = cmdTag->pObjPtr;
    }

    list.push_back(newFeedbackMsg);
    return static_cast<int>(list.size()) - 1;
}

bool DCCpp_utils::copyFbMsgToGenericData(pDGI_GENERIC_RCV_DATA &genericRcvData, FEEDBACK_MSG_VECTOR &list)
{
    genericRcvData->nCompletionCode = list[0].nCompletionCode;
    genericRcvData->nRdCommand = list[0].nRdCommand;
    genericRcvData->xCmdTag.nCmdTagType = list[0].nCmdTagType;
    genericRcvData->xCmdTag.lTimeTickSchedule = list[0].lTimeTickSchedule;
    genericRcvData->xCmdTag.lTimeTickDone = list[0].lTimeTickDone;
    genericRcvData->xCmdTag.nCmdIdx = list[0].nCmdIdx;
    genericRcvData->xCmdTag.nData = list[0].nData;
    genericRcvData->xCmdTag.nAddress = list[0].nAddress;
    genericRcvData->xCmdTag.pObjPtr = list[0].pObjPtr;
    genericRcvData->nNbDataItems = list[0].nNbDataItems;

    for (int i = 0; i < DGI_MAX_DATA_ITEMS; ++i)
    {
        genericRcvData->xDataItem[i].nAddress = list[0].xDataItem[i].nAddress;
        genericRcvData->xDataItem[i].hData[0] = list[0].xDataItem[i].hData[0];
        genericRcvData->xDataItem[i].hData[1] = list[0].xDataItem[i].hData[1];
        genericRcvData->xDataItem[i].hData[2] = list[0].xDataItem[i].hData[2];
        genericRcvData->xDataItem[i].hData[3] = list[0].xDataItem[i].hData[3];
    }

    return true;
}

bool DCCpp_utils::removeFeedbackMsg(const FEEDBACK_MSG &cmdTag, FEEDBACK_MSG_VECTOR &list)
{
    list.erase(findFeedbackMsg(cmdTag, list));
    return true;
}

FEEDBACK_MSG_IT DCCpp_utils::findFeedbackMsg(const FEEDBACK_MSG &cmdTag, FEEDBACK_MSG_VECTOR &list)
{
    FEEDBACK_MSG_IT it;

    for (it = list.begin(); it != list.end(); ++it)
    {
        if (it->nCmdTagType == cmdTag.nCmdTagType && it->pObjPtr == cmdTag.pObjPtr)
        {
            break; // Element found, stop loop
        }
    }
    return it;
}

void DCCpp_utils::removeCharsFromString(std::string &str, char *charsToRemove)
{
    for (unsigned int i = 0; i < strlen(charsToRemove); ++i)
    {
        str.erase(remove(str.begin(), str.end(), charsToRemove[i]), str.end());
    }
}

unsigned int DCCpp_utils::ascii2Hex(const unsigned char *str)
{
    unsigned int value;

    value = 0;

    value <<= 4;
    if (str[0] >= '0' && str[0] <= '9')
    {
        value += (str[0] - '0');
    }
    else if (str[0] >= 'a' && str[0] <= 'f')
    {
        value += (str[0] - 'a' + 10);
    }
    else if (str[0] >= 'A' && str[0] <= 'F')
    {
        value += (str[0] - 'A' + 10);
    }

    return value;
}

int DCCpp_utils::saveLocoInfos(int index, int address, int speed, int direction, int functions)
{
    if (index == -1)
    {
        DCCpp::listOfLocoInfos.push_back({address, speed, direction, functions});
        index = (int)DCCpp::listOfLocoInfos.size() - 1; // Get last index of list
    }
    else
    {
        DCCpp::listOfLocoInfos[index].address = address;
        DCCpp::listOfLocoInfos[index].speed = speed;
        DCCpp::listOfLocoInfos[index].direction = direction;
        DCCpp::listOfLocoInfos[index].functions = functions;
    }

    return index + 1; // Return register number (register number start at 1, so this is index in list + 1)
}

int DCCpp_utils::getLocoInfosIndex(const int address)
{
    LOCO_INFOS_IT it;
    int index = -1;

    for (it = DCCpp::listOfLocoInfos.begin(); it != DCCpp::listOfLocoInfos.end(); ++it)
    {
        if (it->address == address)
        {
            index = it - DCCpp::listOfLocoInfos.begin();
            break; // Element found, stop loop
        }
    }
    return index;
}

unsigned int DCCpp_utils::getLocoFuncValue(unsigned int funcValue, unsigned int funcMask)
{
    unsigned int value;
    bool F0State;

    switch (funcMask)
    {
    case 0x001F:
        value = funcValue & funcMask;
        F0State = (value & 1) != 0;
        value >>= 1;
        if (F0State)
        {
            value |= 0x10;
        }
        value += 0x80;
        break;

    case 0x01E0:
        value = funcValue & funcMask;
        value >>= 5;
        value += 0xB0;
        break;

    case 0x1E00:
        value = funcValue & funcMask;
        value >>= 9;
        value += 0xA0;
        break;

    default:
        value = 0;
        break;
    }

    return value;
}

int DCCpp_utils::convertLocoFuncValue(int funcValue, int newValue)
{
    int value = newValue;

    if (newValue > 127 && newValue < 160)
    {
        value = value - 128;
        value <<= 1;
        if((value & 32) != 0)
        {
            value -= 31;
        }
    }
    else if (newValue > 175 && newValue < 192)
    {
        value = value - 176;
        value = value * 32;
    }
    else if (newValue > 159 && newValue < 176)
    {
        value = value - 160;
        value = value * 512;
    }

    return funcValue ^ value;
}

void DCCpp_utils::setTimeouts(DWORD timeout)
{
    COMMTIMEOUTS timeouts;

    GetCommTimeouts(DCCpp::comPort, &timeouts);

    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = timeout;
    timeouts.ReadTotalTimeoutMultiplier = 50;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 50;

    SetCommTimeouts(DCCpp::comPort, &timeouts);
}

void DCCpp_utils::setComParams()
{
    DCB dcbSerialParams;
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    // Write the parameters
    GetCommState(DCCpp::comPort, &dcbSerialParams);
    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    SetCommState(DCCpp::comPort, &dcbSerialParams);
}

std::list<std::string> DCCpp_utils::listPorts()
{
    char lpTargetPath[5000]; // buffer to store the path of the COM PORTS
    std::list<std::string> portList;

    for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
    {
        std::string str = "COM" + std::to_string(i); // converting to COM0, COM1, COM2
        DWORD result = QueryDosDeviceA(str.c_str(), lpTargetPath, 5000);

        // Test the return value and error if any
        if (result != 0) // QueryDosDevice returns zero if it didn't find an object
        {
            portList.push_back(str);
        }
    }
    return portList;
}

void DCCpp_utils::saveAllDCCppParams()
{
    WritePrivateProfileString("Default", "com", DCCpp::comNumber, "dccpp_config.ini");
    WritePrivateProfileString("Default", "ip", DCCpp::ipAddress, "dccpp_config.ini");

    WritePrivateProfileString("Default", "mode", DCCpp::usbMode ? "true" : "false", "dccpp_config.ini");
    WritePrivateProfileString("Default", "accessory", DCCpp::accessoryCmdType, "dccpp_config.ini");

    std::string detectors = std::to_string(DCCpp::detectorsModuleCount);
    WritePrivateProfileString("Default", "s88_modules", detectors.c_str(), "dccpp_config.ini");

    std::string timer = std::to_string(DCCpp::cmdTimer);
    WritePrivateProfileString("Default", "timer", timer.c_str(), "dccpp_config.ini");
}

void DCCpp_utils::getDCCppParams()
{
    char temp[6];
    GetPrivateProfileString("Default", "mode", "true", temp, 6, "dccpp_config.ini");
    DCCpp::usbMode = (std::strcmp(temp, "true") == 0);

    GetPrivateProfileString("Default", "com", "", DCCpp::comNumber, 5, "dccpp_config.ini");
    GetPrivateProfileString("Default", "ip", "", DCCpp::ipAddress, 20, "dccpp_config.ini");
    // GetPrivateProfileString("Default", "accessory", "T", DCCpp::accessoryCmdType, 2, "dccpp_config.ini");

    DCCpp::detectorsModuleCount = GetPrivateProfileInt("Default", "s88_modules", 8, "dccpp_config.ini");
    DCCpp::cmdTimer = GetPrivateProfileInt("Default", "timer", 100, "dccpp_config.ini");
}

bool DCCpp_utils::saveCmdWtRsp(const DCC_CMD_TYPE cmdType, const CMD_STATION_FB_TYPE fbCmdType, const CMD_ARG args)
{
    if (args != nullptr)
    {
        DCCpp_commands::listOfCmdWaitingResp.push_back({cmdType, fbCmdType, {args[0], args[1], args[2], args[3], args[4]}});
    }
    else
    {
        DCCpp_commands::listOfCmdWaitingResp.push_back({cmdType, fbCmdType});
    }
    return true;
}

bool DCCpp_utils::findCmdWtRsp(CMD_WT_RSP_IT &it, CMD_STATION_FB_TYPE fbType, std::string &cmd, int nbOfArgs, int argsOffset, CMD_ARG args)
{
    bool found = false;
    DCCpp_utils::getArgs(cmd, args);

    for (it = DCCpp_commands::listOfCmdWaitingResp.begin(); it != DCCpp_commands::listOfCmdWaitingResp.end(); ++it)
    {
        if (it->fbCmdType == fbType)
        {
            // Command type found
            // Check if args corresponding to target
            for (int i = argsOffset; i < nbOfArgs; i++)
            {
                if (it->args[i] == args[i])
                {
                    found = true; // If args corresponding to target, is good
                }
                else
                {
                    found = false; // If args not corresponding to target
                    break;         // stop loop
                }
            }

            if (found)
            {
                break; // Element found, stop loop
            }
        }
    }

    return found;
}

bool DCCpp_utils::removeCmdWtRsp(const CMD_WT_RSP_IT &cmdWtRsp)
{
    DCCpp_commands::listOfCmdWaitingResp.erase(cmdWtRsp);
    return true;
}

void DCCpp_utils::getArgs(std::string &str, CMD_ARG args)
{
    int index = 0;
    char charStr[str.length() + 1];

    // copying the contents of the
    // string to char array
    strcpy(charStr, str.c_str());

    char *token = std::strtok(charStr, " ");

    // Keep printing tokens while one of the
    // delimiters present in str[].
    while (token != nullptr)
    {
        args[index] = std::stoi(token);
        token = strtok(nullptr, " ");
        index++;
    }
}

void DCCpp_utils::clearDetectorStates() {
    // Clear states to prevent a multiple call of resume operations
    DCCpp::detectorStates = "";

    // Init all nibbles states to 0
    for (int i = 0; i < (DCCpp::detectorsModuleCount * 2); ++i)
    {
        DCCpp::detectorStates.push_back('0');
    }
}

// https://stackoverflow.com/questions/4308503/how-to-enable-visual-styles-without-a-manifest
// NOTE: It is recommended that you delay-load ComCtl32.dll (/DelayLoad:ComCtl32.dll)
// and that you ensure this code runs before GUI components are loaded.
// Otherwise, you may get weird issues, like black backgrounds in icons in image lists.
ULONG_PTR DCCpp_utils::enableVisualStyles()
{
    TCHAR dir[MAX_PATH];
    ULONG_PTR ulpActivationCookie = FALSE;
    ACTCTX actCtx =
        {
            sizeof(actCtx),
            ACTCTX_FLAG_RESOURCE_NAME_VALID
                | ACTCTX_FLAG_SET_PROCESS_DEFAULT
                | ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID,
            TEXT("shell32.dll"), 0, 0, dir, (LPCTSTR)124
        };
    UINT cch = GetSystemDirectory(dir, sizeof(dir) / sizeof(*dir));
    if (cch >= sizeof(dir) / sizeof(*dir)) { return FALSE; /*shouldn't happen*/ }
    dir[cch] = TEXT('\0');
    ActivateActCtx(CreateActCtx(&actCtx), &ulpActivationCookie);
    return ulpActivationCookie;
}