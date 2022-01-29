//
// Created by Mathieu Andrade on 23/05/2021.
//

#ifndef DDGI_DCCPP_H
#define DDGI_DCCPP_H

#include <ws2tcpip.h>
#include <websockets/wsclient.h>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>
#include "DDGI_Generic_def.h"

class DCCpp
{
  private:
    static HANDLE virtualThread;
    static DWORD virtualThreadId;

  public:
    static HWND wnd;
    static HINSTANCE instance;
    static WebSocket::pointer ws;
    static char comNumber[];
    static char ipAddress[];
    static HANDLE comPort;
    static std::string version;
    static bool usbMode;
    static bool powerOn;
    static unsigned int commandStationStatus;
    static std::string detectorStates;
    static int detectorsModuleCount;
    static FEEDBACK_MSG_VECTOR listOfFeedbackMsg;
    static FEEDBACK_MSG_VECTOR listOfUnexpectedFbMsg;
    static LOCO_INFOS_VECTOR listOfLocoInfos;

    static bool start(DGI_SERVER_PARAMS params);
    static void stop();
    [[noreturn]] static DWORD WINAPI loop(void *);
    static bool connect();
    static bool connectToWebSocketServer();
    static bool disconnect();
    static void initS88();
    // static bool sendLocoInfo(pDGI_GENERIC_DATA genericData);
    static bool setLocoSpeed(pDGI_GENERIC_DATA genericData);
    static bool setLocoFunction(pDGI_GENERIC_DATA genericData, unsigned int funcMask);
    static void emergencyStopAllLocos();
    static void restartAllLocos();

    static void handleStandaloneCommands(STANDALONE_CMD_TYPE cmdType, int feedbackMsgIndex);
    static void handleCommandStationFb(CMD_STATION_FB_TYPE cmdType, int feedbackMsgIndex);
    static void handleCommandStationStatus(int state);
    static void handleCommandStationVersion(std::string &command);
    static void handleDetectorUpdate(std::string &command);
    static void handleTurnoutEvent(std::string &command);
    static void handleLocoEvent(std::string &command, CMD_STATION_FB_TYPE fbCmdType);
};

#endif // DDGI_DCCPP_H
