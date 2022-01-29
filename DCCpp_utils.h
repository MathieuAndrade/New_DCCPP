//
// Created by Mathieu Andrade on 23/05/2021.
//

#ifndef DCCPP_UTILS_H
#define DCCPP_UTILS_H

#include <list>
#include "DCCpp.h"
#include "DCCpp_commands.h"

class DCCpp_utils
{
  public:
    static void printDebugMessage(const std::string &msg);
    static int saveFeedbackMsg(S_DCC_CMD_TAG *cmdTag, FEEDBACK_MSG_VECTOR &list);
    static FEEDBACK_MSG_IT findFeedbackMsg(const FEEDBACK_MSG &cmdTag, FEEDBACK_MSG_VECTOR &list);
    static bool removeFeedbackMsg(const FEEDBACK_MSG &cmdTag, FEEDBACK_MSG_VECTOR &list);
    static bool saveCmdWtRsp(DCC_CMD_TYPE cmdType, CMD_STATION_FB_TYPE fbCmdType, const CMD_ARG args = nullptr);
    static bool findCmdWtRsp(CMD_WT_RSP_IT &it, CMD_STATION_FB_TYPE fbType, std::string &cmd, int nbOfArgs, int argsOffset, CMD_ARG args);
    static bool removeCmdWtRsp(const CMD_WT_RSP_IT &cmdWtRsp);
    static void removeCharsFromString(std::string &str, char *charsToRemove);
    static unsigned int ascii2Hex(const unsigned char *str);
    static bool copyFbMsgToGenericData(pDGI_GENERIC_RCV_DATA &genericRcvData, FEEDBACK_MSG_VECTOR &list);
    static int saveLocoInfos(int index, int address, int speed, int absoluteSpeed, int direction, int functions = -1);
    static unsigned int getLocoFuncValue(unsigned int funcValue, unsigned int funcMask);
    static int getLocoInfosIndex(int address);
    static void setTimeouts(DWORD timeout);
    static std::list<std::string> listPorts();
    static void setComParams();
    static void saveAllDCCppParams();
    static void getDCCppParams();
    static void getArgs(std::string &str, CMD_ARG args);
    static int convertLocoFuncValue(int funcValue, int newValue);
};

#endif // DCCPP_UTILS_H
