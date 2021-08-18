/*======================================================================*/
/*                         FUNCTION DECLARATIONS                        */
/*======================================================================*/

extern "C" __declspec(dllexport) HANDLE DDGL_Register(bool bEnglish, const char **descString, const char **version, const char *railProtocols[], const char *feedbackProtocols[]);

extern "C" __declspec(dllexport) bool DDGL_StartServer(DGI_SERVER_PARAMS *serverParams);

extern "C" __declspec(dllexport) bool DDGL_StopServer(DGI_SERVER_PARAMS *serverParams);

extern "C" __declspec(dllexport) bool DDGL_DisplayServer(DGI_SERVER_PARAMS *serverParams);

extern "C" __declspec(dllexport) bool DDGL_GetNextExpectedResponse(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_RCV_DATA genericRcvData);

extern "C" __declspec(dllexport) bool DDGL_GetNextUnExpectedMessage(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_RCV_DATA genericRcvData);

extern "C" __declspec(dllexport) bool DDGL_ResumeOperations(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_PowerOff(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_EmergencyStop(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_StopLoco(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_CommandStationVersionRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_CommandStationStatusRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_DetectorInfoRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_SetAccessoryState(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_SetSpeed(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_SetF0F4(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_SetF5F8(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_SetF9F12(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

/*
 * Not handled functions
 *

extern "C" __declspec(dllexport) bool DDGL_SetF0F12(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_DirectCVRead(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_ServiceModeResultRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_DirectCVWrite(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_OperationModeProgramming(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_LocoInfoRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_AccessoryInfoRequest(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData);

extern "C" __declspec(dllexport) bool DDGL_DisplayError(DGI_SERVER_PARAMS *serverParams);

extern "C" __declspec(dllexport) bool DDGL_DisplayServerStatus(DGI_SERVER_PARAMS *serverParams);

extern "C" __declspec(dllexport) bool DDGL_EnableErrorDisplay(DGI_SERVER_PARAMS *serverParams);

extern "C" __declspec(dllexport) bool DDGL_DisplayStatistics(DGI_SERVER_PARAMS *serverParams);

extern "C" __declspec(dllexport) bool DDGL_DisplayRcvBuffer(DGI_SERVER_PARAMS *serverParams);

extern "C" __declspec(dllexport) bool DDGL_DisplayCmdFifo(DGI_SERVER_PARAMS *serverParams);

extern "C" __declspec(dllexport) bool DDGL_EnterServiceMode(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)

extern "C" __declspec(dllexport) bool DDGL_ExitServiceMode(pDGI_SERVER_PARAMS serverParams, pDGI_GENERIC_DATA genericData)
 */