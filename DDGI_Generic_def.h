/*======================================================================*/
/*                        Generic definition                            */
/*======================================================================*/

#include <windows.h>

#define DGI_NB_SRV_PARAM_SPARES 8

#define DGI_MAX_DATA_WORDS 5
#define DGI_MAX_DATA_ITEMS 8

#define DLL_NAME "DCCpp"
#define DLL_VERSION "0.0.1"
#define DLL_PROTOCOL "DCC"

typedef enum
{
    DGI_L_FRENCH,
    DGI_L_ENGLISH
} DGI_LANGUAGE;

typedef enum
{
    DGI_COMP_NONE,
    DGI_COMP_OK,
    DGI_COMP_OK_RD,
    DGI_COMP_READ_ERR,
    DGI_COMP_PROT_ERR,
    DGI_COMP_PROG_ERR,
    DGI_COMP_MUDH_ERR,
    DGI_COMP_OPER_ERR,
    DGI_COMP_SEND_ERR,
    DGI_COMP_LOCAL_OK
} DCC_COMPLETION_CODE; // Not Custom but useful

typedef enum
{
    RCV_PC_IF_ERROR, /* 0 */
    RCV_IF_CS_ERROR,
    RCV_UNKNOWN_COMM_ERROR,
    RCV_ACK,
    RCV_NO_CS_TIME_SLOT,
    RCV_IF_BUFFER_OVERFLOW,
    RCV_COMMAND_NOT_UNDERSTOOD,
    RCV_NORMAL_OPER_RESUMED,
    RCV_TRACK_POWER_OFF,
    RCV_EMERGENCY_STOP,
    RCV_SERVICE_MODE_ENTRY, /* 10 */
    RCV_FEEDBACK_BROADCAST,
    RCV_PROG_SHORT_CIRCUIT,
    RCV_PROG_DATA_BYTE_NOT_FOUND,
    RCV_PROG_CMDSTATION_BUSY,
    RCV_PROG_CMDSTATION_READY,
    RCV_SRVC_MODE_RESP_REG_PAGED,
    RCV_SRVC_MODE_RESP_DIRECT_CV,
    RCV_CMDSTATION_SOFT_V_ANSWER_XBUS,
    RCV_CMDSTATION_SOFT_V_ANSWER_XPNET,
    RCV_CMDSTATION_STATUS, /* 20 */
    RCV_TRANSFER_ERRORS,
    RCV_CMDSTATION_BUSY,
    RCV_UNSUPPORTED_COMMAND,
    RCV_ACCESSORY_INFO,
    RCV_DETECTOR_INFO,
    RCV_NORMAL_LOCO_INFO,
    RCV_MULTI_UNIT_LOCO_INFO,
    RCV_MULTI_UNIT_AD_LOCO_INFO,
    RCV_DOUBLE_HEADER_LOCO_INFO,
    RCV_LOCO_OPER_BY_ANOTHER_DEVICE, /* 30 */
    RCV_FUNCTION_STATUS,
    RCV_LOCO_AD_RETRIEVAL,
    RCV_DOUBLE_HEADER_ERROR,
    RCV_MU_DH_ERROR,

    RCV_EVENT_REPORT,
    RCV_LOCO_EVENT_REPORT,
    RCV_DETECTOR_EVENT_REPORT, /* 40 */
    RCV_TURNOUT_EVENT_REPORT,
    RCV_STATUS_EVENT_REPORT,
    RCV_PRE_POWER_OFF_EVENT,

    RCV_NB_DETECTORS,
    RCV_DETECTOR_INITIAL_STATES,
    RCV_DETECTOR_STATES,

    // DCC++ feedbacks
    RCV_SHORT_CIRCUIT_MAIN,
    RCV_SHORT_CIRCUIT_PROG,
    RCV_SENSOR_TRIGGERED,
    RCV_SENSOR_RELEASED, /* 50 */
    RCV_S88_STATE,

    RCV_NB_RCV_COMMANDS,
    RCV_PING,
    RCV_NONE
} RCV_CMD_MNEMONIC; // Not Custom but useful

typedef enum
{
    TAG_NONE,
    TAG_RESUME_OPERATIONS,
    TAG_POWER_OFF,
    TAG_EMERGENCY_STOP,
    TAG_LOCO,
    TAG_ACCESSORY,
    TAG_TURNOUT,
    TAG_TURNOUT_QUERY,
    TAG_SIGNAL,
    TAG_DET_SYNC,
    TAG_FUNCTION,
    TAG_WINDOW,
    TAG_INIT_SERVICE_MODE,
    TAG_CV_RD,
    TAG_CV_WR,
    TAG_CSX_DEBUG,
    TAG_CSX_VERSION,
    TAG_POM
} CMD_TAG_TYPE; // Not Custom but useful

typedef struct S_DCC_CMD_TAG
{
    CMD_TAG_TYPE nCmdTagType;
    void *pObjPtr;
    int nCmdIdx;
    int nAddress; /* used in emulation only */
    int nData;    /* used in emulation only */
    long lTimeTickSchedule;
    long lTimeTickDone;
} DCC_CMD_TAG;

typedef struct S_DGI_SERVER_PARAMS
{
    HWND hMainWindow;
    HANDLE hComPort; /* serial port .... */
    int nComPort;
    void *pSocket;
    int nTCPIPPortNum; /* ... or TCP IP port */

    int nIFType;
    int nBdRateIndex;
    int nAddress;
    bool bSendBreak;
    bool bEmulationMode;
    bool bDumpFile;

    DGI_LANGUAGE nLanguage;

    int nSpare[DGI_NB_SRV_PARAM_SPARES];

    char *sExecDirectory;

} DGI_SERVER_PARAMS; /* CDM-RAIL Digital Generic Server */

typedef struct S_DGI_SERVER_PARAMS *pDGI_SERVER_PARAMS;

typedef struct S_DGI_GENERIC_DATA
{
    DCC_CMD_TAG *pCmdTag;
    long lTimeTick;
    long lNbDelayTicks;
    int nAddress;
    int nAddress2;
    int nData[DGI_MAX_DATA_WORDS];
} DGI_GENERIC_DATA;

typedef struct S_DGI_GENERIC_DATA *pDGI_GENERIC_DATA;

typedef struct
{
    int nAddress;
    unsigned short hData[4];
} DGI_GENERIC_DATA_ITEM; // Not Custom but useful

typedef struct S_DGI_GENERIC_RCV_DATA
{
    DCC_CMD_TAG xCmdTag;
    long lTimeTick;
    DCC_COMPLETION_CODE nCompletionCode;
    RCV_CMD_MNEMONIC nRdCommand;
    int nNbDataItems;
    DGI_GENERIC_DATA_ITEM xDataItem[DGI_MAX_DATA_ITEMS];
} DGI_GENERIC_RCV_DATA;

typedef struct S_DGI_GENERIC_RCV_DATA *pDGI_GENERIC_RCV_DATA;

// Custom

typedef struct
{
    DCC_COMPLETION_CODE nCompletionCode;
    RCV_CMD_MNEMONIC nRdCommand;
    CMD_TAG_TYPE nCmdTagType;
    DGI_GENERIC_DATA_ITEM xDataItem[DGI_MAX_DATA_ITEMS];
    int nCmdIdx;
    int nAddress;
    int nData;
    int nNbDataItems;
    long lTimeTickSchedule;
    long lTimeTickDone;
    void *pObjPtr;
} FEEDBACK_MSG;
typedef std::vector<FEEDBACK_MSG> FEEDBACK_MSG_VECTOR;
typedef FEEDBACK_MSG_VECTOR::iterator FEEDBACK_MSG_IT;

typedef struct {
    int address;
    int speed;
    int direction; // 1 = forward, 0 = backward
    int functions; // Stored in byte
    bool emergencyStopped;
} LOCO_INFOS;
typedef std::vector<LOCO_INFOS> LOCO_INFOS_VECTOR;
typedef LOCO_INFOS_VECTOR::iterator LOCO_INFOS_IT;

typedef int CMD_ARG[5];

typedef enum
{
    POWER_ON,
    POWER_OFF,

    CV_READ,
    CV_WRITE,

    CMD_STATION_VERSION_REQUEST,

    TURNOUT_POSITION,
    ACCESSORY_OPERATION,

    LOCO_INFO_REQUEST,
    LOCO_SPEED,
    LOCO_FUNCTION,

    INIT_S88,
    PING,
} DCC_CMD_TYPE; // Type of commands who can send to command station

typedef enum {
    CMD_POWER_OFF,
    CMD_OPERATIONS_RESUMED,
    CMD_EMERGENCY_STOP,
    CMD_STATION_STATUS,
    CMD_TURNOUT_ACTION,
} STANDALONE_CMD_TYPE; // Type of commands send by CDM-Rail

typedef enum {
    TRACK_POWER_OFF,
    TRACK_POWER_ON,
    TRACK_EMERGENCY_STOP,
    DETECTOR_INFO,
    CMD_STATION_VERSION,
    TURNOUT_EVENT,
    LOCO_SPEED_EVENT,
    LOCO_FUNCTION_EVENT,
} CMD_STATION_FB_TYPE; // Type of commands feedback send by command station

typedef struct {
    DCC_CMD_TYPE cmdType;
    CMD_STATION_FB_TYPE fbCmdType;
    CMD_ARG args;
} CMD_WAITING_RESPONSE;
typedef std::vector<CMD_WAITING_RESPONSE> CMD_WT_RSP_VECTOR;
typedef CMD_WT_RSP_VECTOR::iterator CMD_WT_RSP_IT;