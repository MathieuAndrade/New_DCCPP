//
// Created by Mathieu Andrade on 23/05/2021.
//

#ifndef DCCPP_UTILS_H
#define DCCPP_UTILS_H

#include "DCCpp.h"
#include <list>

class DCCpp_utils
{
  public:
    static void printDebugMessage(const std::string &msg);
    static int saveFeedbackMsg(S_DCC_CMD_TAG *cmdTag, FEEDBACK_MSG_VECTOR &list);
    static FEEDBACK_MSG_IT findFeedbackMsg(const FEEDBACK_MSG &cmdTag, FEEDBACK_MSG_VECTOR &list);
    static bool removeFeedbackMsg(const FEEDBACK_MSG &cmdTag, FEEDBACK_MSG_VECTOR &list);
    static void removeCharsFromString(std::string &str, char *charsToRemove);
    static unsigned int ascii2Hex(const unsigned char *str);
    static bool copyFbMsgToGenericData(pDGI_GENERIC_RCV_DATA &genericRcvData, FEEDBACK_MSG_VECTOR &list);
    static int saveLocoInfos(int index, int address, int speed, int direction, int functions = -1);
    static unsigned int getLocoFuncValue(unsigned int funcValue, unsigned int funcMask);
    static int getLocoInfosIndex(int address);
    static void setTimeouts(DWORD timeout);
    static std::list<std::string> listPorts();
    static void setComParams();
    static void saveAllDCCppParams();
    static void getDCCppParams();
};

#endif // DCCPP_UTILS_H
