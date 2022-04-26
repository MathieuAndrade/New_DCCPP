//
// Created by Mathieu Andrade on 23/05/2021.
//

#include <windows.h>
#include <iostream>
#include "DCCpp.h"
#include "DDGI_Generic_lib_def.h"
#include "DCCpp_utils.h"
#include "DCCpp_commands.h"

/*======================================================================*/
/*                            GLOBAL DLL MAIN                           */
/*======================================================================*/

bool WINAPI DllMain(HINSTANCE instance, DWORD fdwReason, LPVOID lpvReserved)
{
    /* The return value is used for successful DLL_PROCESS_ATTACH */
    DCCpp::instance = instance;
    return true;
}

/*======================================================================*/
/*                          EXPORTABLE FUNCTIONS                        */
/*======================================================================*/

__declspec(dllexport) HANDLE DDGL_Register(bool bEnglish, const char **descString, const char **version, const char *railProtocols[], const char *feedbackProtocols[])
{
    *descString = DLL_NAME;
    *version = DLL_VERSION;
    railProtocols[0] = DLL_PROTOCOL;
    railProtocols[1] = nullptr;
    feedbackProtocols[0] = nullptr;
    return nullptr;
}

__declspec(dllexport) bool DDGL_StartServer(DGI_SERVER_PARAMS *serverParams)
{
    return DCCpp::start(*serverParams);
}

__declspec(dllexport) bool DDGL_StopServer(DGI_SERVER_PARAMS *serverParams)
{
    DCCpp::stop();
    return true;
}

__declspec(dllexport) bool DDGL_DisplayServer(DGI_SERVER_PARAMS *serverParams)
{
    std::stringstream msg;

    if (DCCpp::comPort != INVALID_HANDLE_VALUE || DCCpp::comPort != nullptr || DCCpp::ws != nullptr)
    {
        msg << "Mode : " << (DCCpp::usbMode ? "USB" : "WIFI") << '\n';
        msg << " " << '\n';
        msg << (DCCpp::usbMode ? "COM port : " : "Adress IP : ") << (DCCpp::usbMode ? DCCpp::comNumber : DCCpp::ipAddress) << '\n';
        msg << " " << '\n';
        msg << "Version : " << DCCpp::version << '\n';
        msg << " " << '\n';
        msg << "Nombre de modules S88 : " << DCCpp::detectorsModuleCount  << '\n';

        MessageBox(DCCpp::wnd, msg.str().c_str(), "Interface DCCPP", MB_APPLMODAL | MB_OK | MB_ICONINFORMATION);
    }

    return true;
}

__declspec(dllexport) bool DDGL_GetNextExpectedResponse(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_RCV_DATA genericRcvData)
{
    bool result = false;

    if (!DCCpp::listOfFeedbackMsg.empty())
    {
        // Wait until message was copied
        result = DCCpp_utils::copyFbMsgToGenericData(genericRcvData, DCCpp::listOfFeedbackMsg);

        if(result)
        {
            // Wait until message was removed
            result = DCCpp_utils::removeFeedbackMsg(DCCpp::listOfFeedbackMsg[0], DCCpp::listOfFeedbackMsg);
        }
    }

    return result;
}

__declspec(dllexport) bool DDGL_GetNextUnExpectedMessage(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_RCV_DATA genericRcvData)
{
    bool result = false;

    if (!DCCpp::listOfUnexpectedFbMsg.empty())
    {
        // Wait until message was copied
        result = DCCpp_utils::copyFbMsgToGenericData(genericRcvData, DCCpp::listOfUnexpectedFbMsg); //1977903376

        if(result)
        {
            // Wait until message was removed
            result = DCCpp_utils::removeFeedbackMsg(DCCpp::listOfUnexpectedFbMsg[0], DCCpp::listOfUnexpectedFbMsg);
        }
    }

    return result;
}

__declspec(dllexport) bool DDGL_ResumeOperations(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    bool success;
    int index;

    success = DCCpp_commands::buildCommand(POWER_ON);

    if(success) {
        DCCpp::initS88();
        // Wait for command station before send locos commands
        // If locos speed are send immediately, command station not handle it
        Sleep(1000);
        DCCpp::restartAllLocos();
        index = DCCpp_utils::saveFeedbackMsg(genericData->pCmdTag, DCCpp::listOfFeedbackMsg);
        DCCpp::handleStandaloneCommands(CMD_OPERATIONS_RESUMED, index);
    }

    return success;
}

__declspec(dllexport) bool DDGL_PowerOff(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    int index;
    bool success;

    success = DCCpp_commands::buildCommand(POWER_OFF);

    if(success) {
        index = DCCpp_utils::saveFeedbackMsg(genericData->pCmdTag, DCCpp::listOfFeedbackMsg);
        DCCpp::handleStandaloneCommands(CMD_POWER_OFF, index);
    }

    return success;
}

__declspec(dllexport) bool DDGL_EmergencyStop(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    int index;
    bool success;

    DCCpp::emergencyStopAllLocos();
    success = DCCpp_commands::buildCommand(POWER_OFF);

    if(success) {
        index = DCCpp_utils::saveFeedbackMsg(genericData->pCmdTag, DCCpp::listOfFeedbackMsg);
        DCCpp::handleStandaloneCommands(CMD_EMERGENCY_STOP, index);
    }

    return success;
}

__declspec(dllexport) bool DDGL_StopLoco(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    genericData->nData[1] = 0;
    return DCCpp::setLocoSpeed(genericData);
}

__declspec(dllexport) bool DDGL_CommandStationVersionRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    DCCpp_commands::buildCommand(CMD_STATION_VERSION_REQUEST);
    return true;
}

__declspec(dllexport) bool DDGL_CommandStationStatusRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    int index;

    index = DCCpp_utils::saveFeedbackMsg(genericData->pCmdTag, DCCpp::listOfFeedbackMsg);
    DCCpp::handleStandaloneCommands(CMD_STATION_STATUS, index);
    return true;
}

__declspec(dllexport) bool DDGL_DetectorInfoRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    // int index;

    // index = DCCpp_utils::saveFeedbackMsg(genericData->pCmdTag);
    // DCCpp::handleStandaloneCommands(DETECTOR_INFO, index);
    // TODO: Investigate on this function
    // DCCpp_commands::sendCommand(INIT_S88);
    return true;
}

__declspec(dllexport) bool DDGL_SetAccessoryState(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    int index;
    bool success = false;
    CMD_ARG args;

    args[0] = genericData->nAddress; // Accessory address
    args[1] = genericData->nData[0] != 0; // Accessory state

    success = DCCpp_commands::buildCommand(TURNOUT_POSITION, args);

    if(success) {
        index = DCCpp_utils::saveFeedbackMsg(genericData->pCmdTag, DCCpp::listOfFeedbackMsg);
        DCCpp::handleStandaloneCommands(CMD_TURNOUT_ACTION, index);
    }

    return success;
}

__declspec(dllexport) bool DDGL_LocoInfoRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    // return DCCpp::sendLocoInfo(genericData);
    // Work in progress
    return true;
}

__declspec(dllexport) bool DDGL_SetSpeed(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    return DCCpp::setLocoSpeed(genericData);
}

__declspec(dllexport) bool DDGL_SetF0F4(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    return DCCpp::setLocoFunction(genericData, 0x1F);
}

__declspec(dllexport) bool DDGL_SetF5F8(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    return DCCpp::setLocoFunction(genericData, 0x01E0);
}

__declspec(dllexport) bool DDGL_SetF9F12(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    return DCCpp::setLocoFunction(genericData, 0x1E00);
}

/*
 * Not handled functions
 *

__declspec(dllexport) bool DDGL_SetF0F12(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    return true;
}

__declspec(dllexport) bool DDGL_DisplayError(DGI_SERVER_PARAMS *serverParams)
{
    return true;
}

__declspec(dllexport) bool DDGL_DisplayServerStatus(DGI_SERVER_PARAMS *serverParams)
{
    return true;
}

__declspec(dllexport) bool DDGL_EnableErrorDisplay(DGI_SERVER_PARAMS *serverParams)
{
    return true;
}

__declspec(dllexport) bool DDGL_DisplayStatistics(DGI_SERVER_PARAMS *serverParams)
{
    return true;
}

__declspec(dllexport) bool DDGL_DisplayRcvBuffer(DGI_SERVER_PARAMS *serverParams)
{
    return true;
}

__declspec(dllexport) bool DDGL_DisplayCmdFifo(DGI_SERVER_PARAMS *serverParams)
{
    return true;
}

 __declspec(dllexport) bool DDGL_EnterServiceMode(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    return true;
}

__declspec(dllexport) bool DDGL_DirectCVRead(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    return true;
}

__declspec(dllexport) bool DDGL_ServiceModeResultRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    return true;
}

__declspec(dllexport) bool DDGL_DirectCVWrite(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    return true;
}

__declspec(dllexport) bool DDGL_ExitServiceMode(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    return true;
}

__declspec(dllexport) bool DDGL_AccessoryInfoRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
{
    return true;
}

 */