/*=======================================================================================
** File Name:  telem_r_app.c
**
** Title:  Function Definitions for TELEM_R Application
**
** $Author:    Austin Cosby
** $Revision: 1.1 $
** $Date:      2018-07-30
**
** Purpose:  This source file contains all necessary function definitions to run TELEM_R
**           application.
**
** Functions Defined:
**    Function X - Brief purpose of function X
**    Function Y - Brief purpose of function Y
**    Function Z - Brief purpose of function Z
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to all functions in the file.
**    2. List the external source(s) and event(s) that can cause the funcs in this
**       file to execute.
**    3. List known limitations that apply to the funcs in this file.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Modification History:
**   Date | Author | Description
**   ---------------------------
**   2018-07-30 | Austin Cosby | Build #: Code Started
**
**=====================================================================================*/

/*
** Pragmas
*/

/*
** Include Files
*/
#include <string.h>

#include "cfe.h"
#include "telem_r_platform_cfg.h"
#include "telem_r_mission_cfg.h"
#include "telem_r_app.h"

#include <wiringPiI2C.h>
#include "lsm9ds1.h"
/*
** Local Defines
*/

/*
** Local Structure Declarations
*/

/*
** External Global Variables
*/

/*
** Global Variables
*/
TELEM_R_AppData_t  g_TELEM_R_AppData;

LSM9DS1_Vector_t mag_data;
LSM9DS1_Vector_t accel_data;
/*
** Local Variables
*/
int thermoDataLen = 4;
int thermoId = 0;
int thermoTemp;
char thermoData[4];
/*
** Local Function Definitions
*/
//Themocoupler init
//sets up the pi to use the thermocoupler plugged in to SPI 0)
//returns -1 if fail;
int SPIThermocouplerInit()
{
	if (wiringPISPISetup(thermoId, 500000) == -1)
	{
		printf ("Unable to initialize the thermocoupler\n");

		return -1;
	}

	
}

int getThermocouplerData()
{
	wiringPiSPIDataRW(thermoId, thermoData, thermoDataLen);
	int temp = (int)thermoData[0];
	temp = (temp << 8);
	temp = temp | thermoData[1];

	temp = temp & 0x7ff0;
	temp = (temp >> 4);
	return temp;
}

/*=====================================================================================
** Name: TELEM_R_InitEvent
**
** Purpose: To initialize and register event table for TELEM_R application
**
** Arguments:
**    None
**
** Returns:
**    int32 iStatus - Status of initialization
**
** Routines Called:
**    CFE_EVS_Register
**    CFE_ES_WriteToSysLog
**
** Called By:
**    TELEM_R_InitApp
**
** Global Inputs/Reads:
**    TBD
**
** Global Outputs/Writes:
**    g_TELEM_R_AppData.EventTbl
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby 
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
int32 TELEM_R_InitEvent()
{
    int32  iStatus=CFE_SUCCESS;

    /* Create the event table */
    memset((void*)g_TELEM_R_AppData.EventTbl, 0x00, sizeof(g_TELEM_R_AppData.EventTbl));

    g_TELEM_R_AppData.EventTbl[0].EventID = TELEM_R_RESERVED_EID;
    g_TELEM_R_AppData.EventTbl[1].EventID = TELEM_R_INF_EID;
    g_TELEM_R_AppData.EventTbl[2].EventID = TELEM_R_INIT_INF_EID;
    g_TELEM_R_AppData.EventTbl[3].EventID = TELEM_R_ILOAD_INF_EID;
    g_TELEM_R_AppData.EventTbl[4].EventID = TELEM_R_CDS_INF_EID;
    g_TELEM_R_AppData.EventTbl[5].EventID = TELEM_R_CMD_INF_EID;

    g_TELEM_R_AppData.EventTbl[ 6].EventID = TELEM_R_ERR_EID;
    g_TELEM_R_AppData.EventTbl[ 7].EventID = TELEM_R_INIT_ERR_EID;
    g_TELEM_R_AppData.EventTbl[ 8].EventID = TELEM_R_ILOAD_ERR_EID;
    g_TELEM_R_AppData.EventTbl[ 9].EventID = TELEM_R_CDS_ERR_EID;
    g_TELEM_R_AppData.EventTbl[10].EventID = TELEM_R_CMD_ERR_EID;
    g_TELEM_R_AppData.EventTbl[11].EventID = TELEM_R_PIPE_ERR_EID;
    g_TELEM_R_AppData.EventTbl[12].EventID = TELEM_R_MSGID_ERR_EID;
    g_TELEM_R_AppData.EventTbl[13].EventID = TELEM_R_MSGLEN_ERR_EID;

    /* Register the table with CFE */
    iStatus = CFE_EVS_Register(g_TELEM_R_AppData.EventTbl,
                               TELEM_R_EVT_CNT, CFE_EVS_BINARY_FILTER);
    if (iStatus != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("TELEM_R - Failed to register with EVS (0x%08X)\n", iStatus);
    }

    return (iStatus);
}
    
/*=====================================================================================
** Name: TELEM_R_InitPipe
**
** Purpose: To initialize all message pipes and subscribe to messages for TELEM_R application
**
** Arguments:
**    None
**
** Returns:
**    int32 iStatus - Status of initialization
**
** Routines Called:
**    CFE_SB_CreatePipe
**    CFE_SB_Subscribe
**    CFE_ES_WriteToSysLog
**
** Called By:
**    TELEM_R_InitApp
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    g_TELEM_R_AppData.usSchPipeDepth
**    g_TELEM_R_AppData.cSchPipeName
**    g_TELEM_R_AppData.SchPipeId
**    g_TELEM_R_AppData.usCmdPipeDepth
**    g_TELEM_R_AppData.cCmdPipeName
**    g_TELEM_R_AppData.CmdPipeId
**    g_TELEM_R_AppData.usTlmPipeDepth
**    g_TELEM_R_AppData.cTlmPipeName
**    g_TELEM_R_AppData.TlmPipeId
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby 
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
int32 TELEM_R_InitPipe()
{
    int32  iStatus=CFE_SUCCESS;

    /* Init schedule pipe */
    g_TELEM_R_AppData.usSchPipeDepth = TELEM_R_SCH_PIPE_DEPTH;
    memset((void*)g_TELEM_R_AppData.cSchPipeName, '\0', sizeof(g_TELEM_R_AppData.cSchPipeName));
    strncpy(g_TELEM_R_AppData.cSchPipeName, "TELEM_R_SCH_PIPE", OS_MAX_API_NAME-1);

    /* Subscribe to Wakeup messages */
    iStatus = CFE_SB_CreatePipe(&g_TELEM_R_AppData.SchPipeId,
                                 g_TELEM_R_AppData.usSchPipeDepth,
                                 g_TELEM_R_AppData.cSchPipeName);
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = CFE_SB_SubscribeEx(TELEM_R_WAKEUP_MID, g_TELEM_R_AppData.SchPipeId, CFE_SB_Default_Qos, 1);

        if (iStatus != CFE_SUCCESS)
        {
            CFE_ES_WriteToSysLog("TELEM_R - Sch Pipe failed to subscribe to TELEM_R_WAKEUP_MID. (0x%08X)\n", iStatus);
            goto TELEM_R_InitPipe_Exit_Tag;
        }
        
    }
    else
    {
        CFE_ES_WriteToSysLog("TELEM_R - Failed to create SCH pipe (0x%08X)\n", iStatus);
        goto TELEM_R_InitPipe_Exit_Tag;
    }

    /* Init command pipe */
    g_TELEM_R_AppData.usCmdPipeDepth = TELEM_R_CMD_PIPE_DEPTH ;
    memset((void*)g_TELEM_R_AppData.cCmdPipeName, '\0', sizeof(g_TELEM_R_AppData.cCmdPipeName));
    strncpy(g_TELEM_R_AppData.cCmdPipeName, "TELEM_R_CMD_PIPE", OS_MAX_API_NAME-1);

    /* Subscribe to command messages */
    iStatus = CFE_SB_CreatePipe(&g_TELEM_R_AppData.CmdPipeId,
                                 g_TELEM_R_AppData.usCmdPipeDepth,
                                 g_TELEM_R_AppData.cCmdPipeName);
    if (iStatus == CFE_SUCCESS)
    {
        /* Subscribe to command messages */
        iStatus = CFE_SB_Subscribe(TELEM_R_CMD_MID, g_TELEM_R_AppData.CmdPipeId);

        if (iStatus != CFE_SUCCESS)
        {
            CFE_ES_WriteToSysLog("TELEM_R - CMD Pipe failed to subscribe to TELEM_R_CMD_MID. (0x%08X)\n", iStatus);
            goto TELEM_R_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_Subscribe(TELEM_R_SEND_HK_MID, g_TELEM_R_AppData.CmdPipeId);

        if (iStatus != CFE_SUCCESS)
        {
            CFE_ES_WriteToSysLog("TELEM_R - CMD Pipe failed to subscribe to TELEM_R_SEND_HK_MID. (0x%08X)\n", iStatus);
            goto TELEM_R_InitPipe_Exit_Tag;
        }
        
    }
    else
    {
        CFE_ES_WriteToSysLog("TELEM_R - Failed to create CMD pipe (0x%08X)\n", iStatus);
        goto TELEM_R_InitPipe_Exit_Tag;
    }

    /* Init telemetry pipe */
    g_TELEM_R_AppData.usTlmPipeDepth = TELEM_R_TLM_PIPE_DEPTH;
    memset((void*)g_TELEM_R_AppData.cTlmPipeName, '\0', sizeof(g_TELEM_R_AppData.cTlmPipeName));
    strncpy(g_TELEM_R_AppData.cTlmPipeName, "TELEM_R_TLM_PIPE", OS_MAX_API_NAME-1);

    /* Subscribe to telemetry messages on the telemetry pipe */
    iStatus = CFE_SB_CreatePipe(&g_TELEM_R_AppData.TlmPipeId,
                                 g_TELEM_R_AppData.usTlmPipeDepth,
                                 g_TELEM_R_AppData.cTlmPipeName);
    if (iStatus == CFE_SUCCESS)
    {
        /* TODO:  Add CFE_SB_Subscribe() calls for other apps' output data here.
        **
        ** Examples:
        **     CFE_SB_Subscribe(GNCEXEC_OUT_DATA_MID, g_TELEM_R_AppData.TlmPipeId);
        */
    }
    else
    {
        CFE_ES_WriteToSysLog("TELEM_R - Failed to create TLM pipe (0x%08X)\n", iStatus);
        goto TELEM_R_InitPipe_Exit_Tag;
    }

TELEM_R_InitPipe_Exit_Tag:
    return (iStatus);
}
    
/*=====================================================================================
** Name: TELEM_R_InitData
**
** Purpose: To initialize global variables used by TELEM_R application
**
** Arguments:
**    None
**
** Returns:
**    int32 iStatus - Status of initialization
**
** Routines Called:
**    CFE_SB_InitMsg
**
** Called By:
**    TELEM_R_InitApp
**
** Global Inputs/Reads:
**    TBD
**
** Global Outputs/Writes:
**    g_TELEM_R_AppData.InData
**    g_TELEM_R_AppData.OutData
**    g_TELEM_R_AppData.HkTlm
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby 
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
int32 TELEM_R_InitData()
{
    int32  iStatus=CFE_SUCCESS;

    /* Init input data */
    memset((void*)&g_TELEM_R_AppData.InData, 0x00, sizeof(g_TELEM_R_AppData.InData));

    /* Init output data */
    memset((void*)&g_TELEM_R_AppData.OutData, 0x00, sizeof(g_TELEM_R_AppData.OutData));
    CFE_SB_InitMsg(&g_TELEM_R_AppData.OutData,
                   TELEM_R_OUT_DATA_MID, sizeof(g_TELEM_R_AppData.OutData), TRUE);

    /* Init housekeeping packet */
    memset((void*)&g_TELEM_R_AppData.HkTlm, 0x00, sizeof(g_TELEM_R_AppData.HkTlm));
    CFE_SB_InitMsg(&g_TELEM_R_AppData.HkTlm,
                   TELEM_R_HK_TLM_MID, sizeof(g_TELEM_R_AppData.HkTlm), TRUE);

    return (iStatus);
}
    
/*=====================================================================================
** Name: TELEM_R_InitApp
**
** Purpose: To initialize all data local to and used by TELEM_R application
**
** Arguments:
**    None
**
** Returns:
**    int32 iStatus - Status of initialization
**
** Routines Called:
**    CFE_ES_RegisterApp
**    CFE_ES_WriteToSysLog
**    CFE_EVS_SendEvent
**    OS_TaskInstallDeleteHandler
**    TELEM_R_InitEvent
**    TELEM_R_InitPipe
**    TELEM_R_InitData
**
** Called By:
**    TELEM_R_AppMain
**
** Global Inputs/Reads:
**    TBD
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby 
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
int32 TELEM_R_InitApp()
{
    int32  iStatus=CFE_SUCCESS;

    g_TELEM_R_AppData.uiRunStatus = CFE_ES_APP_RUN;

    iStatus = CFE_ES_RegisterApp();
    if (iStatus != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("TELEM_R - Failed to register the app (0x%08X)\n", iStatus);
        goto TELEM_R_InitApp_Exit_Tag;
    }

    if ((TELEM_R_InitEvent() != CFE_SUCCESS) || 
        (TELEM_R_InitPipe() != CFE_SUCCESS) || 
        (TELEM_R_InitData() != CFE_SUCCESS))
    {
        iStatus = -1;
        goto TELEM_R_InitApp_Exit_Tag;
    }

    /* Install the cleanup callback */
    OS_TaskInstallDeleteHandler((void*)&TELEM_R_CleanupCallback);

TELEM_R_InitApp_Exit_Tag:
    if (iStatus == CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(TELEM_R_INIT_INF_EID, CFE_EVS_INFORMATION,
                          "TELEM_R - Application initialized");
    }
    else
    {
        CFE_ES_WriteToSysLog("TELEM_R - Application failed to initialize\n");
    }

    return (iStatus);
}
    
/*=====================================================================================
** Name: TELEM_R_CleanupCallback
**
** Purpose: To handle any neccesary cleanup prior to application exit
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    TBD
**
** Called By:
**    TBD
**
** Global Inputs/Reads:
**    TBD
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby 
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void TELEM_R_CleanupCallback()
{
    /* TODO:  Add code to cleanup memory and other cleanup here */
}
    
/*=====================================================================================
** Name: TELEM_R_RcvMsg
**
** Purpose: To receive and process messages for TELEM_R application
**
** Arguments:
**    None
**
** Returns:
**    int32 iStatus - Status of initialization 
**
** Routines Called:
**    CFE_SB_RcvMsg
**    CFE_SB_GetMsgId
**    CFE_EVS_SendEvent
**    CFE_ES_PerfLogEntry
**    CFE_ES_PerfLogExit
**    TELEM_R_ProcessNewCmds
**    TELEM_R_ProcessNewData
**    TELEM_R_SendOutData
**
** Called By:
**    TELEM_R_Main
**
** Global Inputs/Reads:
**    g_TELEM_R_AppData.SchPipeId
**
** Global Outputs/Writes:
**    g_TELEM_R_AppData.uiRunStatus
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby 
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
int32 TELEM_R_RcvMsg(int32 iBlocking)
{
    int32           iStatus=CFE_SUCCESS;
    CFE_SB_Msg_t*   MsgPtr=NULL;
    CFE_SB_MsgId_t  MsgId;

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(TELEM_R_MAIN_TASK_PERF_ID);

    /* Wait for WakeUp messages from scheduler */
    iStatus = CFE_SB_RcvMsg(&MsgPtr, g_TELEM_R_AppData.SchPipeId, iBlocking);

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(TELEM_R_MAIN_TASK_PERF_ID);

    if (iStatus == CFE_SUCCESS)
    {
        MsgId = CFE_SB_GetMsgId(MsgPtr);
        switch (MsgId)
	{
            case TELEM_R_WAKEUP_MID:
                TELEM_R_ProcessNewCmds();
                TELEM_R_ProcessNewData();

                /* TODO:  Add more code here to handle other things when app wakes up */
		
                /* The last thing to do at the end of this Wakeup cycle should be to
                   automatically publish new output. */
		//Adds mag data to send out for other apps to use
		mag_data = LSM9DS1_GetMagData();
		accel_data = LSM9DS1_GetAccelData();
		thermoTemp = getThermocouplerData();


		g_TELEM_R_AppData.OutData.magX = mag_data.x;
		g_TELEM_R_AppData.OutData.magY = mag_data.y;
		g_TELEM_R_AppData.OutData.magZ = mag_data.z;
		g_TELEM_R_AppData.OutData.accelX = accel_data.x;
		g_TELEM_R_AppData.OutData.accelY = accel_data.y;
		g_TELEM_R_AppData.OutData.accelZ = accel_data.z;
		g_TELEM_R_AppData.OutData.thermoTemp = thermoTemp;
		
		

		//Also adds the data to housekeeping for debug at least
		g_TELEM_R_AppData.HkTlm.magX = mag_data.x;
		g_TELEM_R_AppData.HkTlm.magY = mag_data.y;
		g_TELEM_R_AppData.HkTlm.magZ = mag_data.z;
		g_TELEM_R_AppData.HkTlm.accelX = accel_data.x;
		g_TELEM_R_AppData.HkTlm.accelY = accel_data.y;
		g_TELEM_R_AppData.HkTlm.accelZ = accel_data.z;
		g_TELEM_R_AppData.HkTlm.thermoTemp = thermoTemp;
		//mag_data = LSM9DS1_GetMagData();
		//accel_data = LSM9DS1_GetAccelData();
		//g_TELEM_R_AppData.OutData.magX = mag_data.x;
		//g_TELEM_R_AppData.OutData.magY = mag_data.y;
		//g_TELEM_R_AppData.OutData.magZ = mag_data.z;
                TELEM_R_SendOutData();
                break;

            default:
                CFE_EVS_SendEvent(TELEM_R_MSGID_ERR_EID, CFE_EVS_ERROR,
                                  "TELEM_R - Recvd invalid SCH msgId (0x%08X)", MsgId);
        }
    }
    else if (iStatus == CFE_SB_NO_MESSAGE)
    {
        /* If there's no incoming message, you can do something here, or nothing */
    }
    else
    {
        /* This is an example of exiting on an error.
        ** Note that a SB read error is not always going to result in an app quitting.
        */
        CFE_EVS_SendEvent(TELEM_R_PIPE_ERR_EID, CFE_EVS_ERROR,
			  "TELEM_R: SB pipe read error (0x%08X), app will exit", iStatus);
        g_TELEM_R_AppData.uiRunStatus= CFE_ES_APP_ERROR;
    }

    return (iStatus);
}
    
/*=====================================================================================
** Name: TELEM_R_ProcessNewData
**
** Purpose: To process incoming data subscribed by TELEM_R application
**
** Arguments:
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    CFE_SB_RcvMsg
**    CFE_SB_GetMsgId
**    CFE_EVS_SendEvent
**
** Called By:
**    TELEM_R_RcvMsg
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    None
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby 
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void TELEM_R_ProcessNewData()
{
    int iStatus = CFE_SUCCESS;
    CFE_SB_Msg_t*   TlmMsgPtr=NULL;
    CFE_SB_MsgId_t  TlmMsgId;

    /* Process telemetry messages till the pipe is empty */
    while (1)
    {
        iStatus = CFE_SB_RcvMsg(&TlmMsgPtr, g_TELEM_R_AppData.TlmPipeId, CFE_SB_POLL);
        if (iStatus == CFE_SUCCESS)
        {
            TlmMsgId = CFE_SB_GetMsgId(TlmMsgPtr);
            switch (TlmMsgId)
            {
                /* TODO:  Add code to process all subscribed data here 
                **
                ** Example:
                **     case NAV_OUT_DATA_MID:
                **         TELEM_R_ProcessNavData(TlmMsgPtr);
                **         break;
                */

                default:
                    CFE_EVS_SendEvent(TELEM_R_MSGID_ERR_EID, CFE_EVS_ERROR,
                                      "TELEM_R - Recvd invalid TLM msgId (0x%08X)", TlmMsgId);
                    break;
            }
        }
        else if (iStatus == CFE_SB_NO_MESSAGE)
        {
            break;
        }
        else
        {
            CFE_EVS_SendEvent(TELEM_R_PIPE_ERR_EID, CFE_EVS_ERROR,
                  "TELEM_R: CMD pipe read error (0x%08X)", iStatus);
            g_TELEM_R_AppData.uiRunStatus = CFE_ES_APP_ERROR;
            break;
        }
    }
}
    
/*=====================================================================================
** Name: TELEM_R_ProcessNewCmds
**
** Purpose: To process incoming command messages for TELEM_R application
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    CFE_SB_RcvMsg
**    CFE_SB_GetMsgId
**    CFE_EVS_SendEvent
**    TELEM_R_ProcessNewAppCmds
**    TELEM_R_ReportHousekeeping
**
** Called By:
**    TELEM_R_RcvMsg
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    None
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby 
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void TELEM_R_ProcessNewCmds()
{
    int iStatus = CFE_SUCCESS;
    CFE_SB_Msg_t*   CmdMsgPtr=NULL;
    CFE_SB_MsgId_t  CmdMsgId;

    /* Process command messages till the pipe is empty */
    while (1)
    {
        iStatus = CFE_SB_RcvMsg(&CmdMsgPtr, g_TELEM_R_AppData.CmdPipeId, CFE_SB_POLL);
        if(iStatus == CFE_SUCCESS)
        {
            CmdMsgId = CFE_SB_GetMsgId(CmdMsgPtr);
            switch (CmdMsgId)
            {
                case TELEM_R_CMD_MID:
                    TELEM_R_ProcessNewAppCmds(CmdMsgPtr);
                    break;

                case TELEM_R_SEND_HK_MID:
                    TELEM_R_ReportHousekeeping();
                    break;

                /* TODO:  Add code to process other subscribed commands here
                **
                ** Example:
                **     case CFE_TIME_DATA_CMD_MID:
                **         TELEM_R_ProcessTimeDataCmd(CmdMsgPtr);
                **         break;
                */

                default:
                    CFE_EVS_SendEvent(TELEM_R_MSGID_ERR_EID, CFE_EVS_ERROR,
                                      "TELEM_R - Recvd invalid CMD msgId (0x%08X)", CmdMsgId);
                    break;
            }
        }
        else if (iStatus == CFE_SB_NO_MESSAGE)
        {
            break;
        }
        else
        {
            CFE_EVS_SendEvent(TELEM_R_PIPE_ERR_EID, CFE_EVS_ERROR,
                  "TELEM_R: CMD pipe read error (0x%08X)", iStatus);
            g_TELEM_R_AppData.uiRunStatus = CFE_ES_APP_ERROR;
            break;
        }
    }
}
    
/*=====================================================================================
** Name: TELEM_R_ProcessNewAppCmds
**
** Purpose: To process command messages targeting TELEM_R application
**
** Arguments:
**    CFE_SB_Msg_t*  MsgPtr - new command message pointer
**
** Returns:
**    None
**
** Routines Called:
**    CFE_SB_GetCmdCode
**    CFE_EVS_SendEvent
**
** Called By:
**    TELEM_R_ProcessNewCmds
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    g_TELEM_R_AppData.HkTlm.usCmdCnt
**    g_TELEM_R_AppData.HkTlm.usCmdErrCnt
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby 
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void TELEM_R_ProcessNewAppCmds(CFE_SB_Msg_t* MsgPtr)
{
    uint32  uiCmdCode=0;

    if (MsgPtr != NULL)
    {
        uiCmdCode = CFE_SB_GetCmdCode(MsgPtr);
        switch (uiCmdCode)
        {
            case TELEM_R_NOOP_CC:
                g_TELEM_R_AppData.HkTlm.usCmdCnt++;
                CFE_EVS_SendEvent(TELEM_R_CMD_INF_EID, CFE_EVS_INFORMATION,
                                  "TELEM_R - Recvd NOOP cmd (%d)", uiCmdCode);
                break;

            case TELEM_R_RESET_CC:
                g_TELEM_R_AppData.HkTlm.usCmdCnt = 0;
                g_TELEM_R_AppData.HkTlm.usCmdErrCnt = 0;
                CFE_EVS_SendEvent(TELEM_R_CMD_INF_EID, CFE_EVS_INFORMATION,
                                  "TELEM_R - Recvd RESET cmd (%d)", uiCmdCode);
                break;

            /* TODO:  Add code to process the rest of the TELEM_R commands here */

            default:
                g_TELEM_R_AppData.HkTlm.usCmdErrCnt++;
                CFE_EVS_SendEvent(TELEM_R_MSGID_ERR_EID, CFE_EVS_ERROR,
                                  "TELEM_R - Recvd invalid cmdId (%d)", uiCmdCode);
                break;
        }
    }
}
    
/*=====================================================================================
** Name: TELEM_R_ReportHousekeeping
**
** Purpose: To send housekeeping message
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    TBD
**
** Called By:
**    TELEM_R_ProcessNewCmds
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  GSFC, Austin Cosby
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void TELEM_R_ReportHousekeeping()
{
    /* TODO:  Add code to update housekeeping data, if needed, here.  */

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&g_TELEM_R_AppData.HkTlm);
    CFE_SB_SendMsg((CFE_SB_Msg_t*)&g_TELEM_R_AppData.HkTlm);
}
    
/*=====================================================================================
** Name: TELEM_R_SendOutData
**
** Purpose: To publish 1-Wakeup cycle output data
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    TBD
**
** Called By:
**    TELEM_R_RcvMsg
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void TELEM_R_SendOutData()
{
    /* TODO:  Add code to update output data, if needed, here.  */

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&g_TELEM_R_AppData.OutData);
    CFE_SB_SendMsg((CFE_SB_Msg_t*)&g_TELEM_R_AppData.OutData);
}
    
/*=====================================================================================
** Name: TELEM_R_VerifyCmdLength
**
** Purpose: To verify command length for a particular command message
**
** Arguments:
**    CFE_SB_Msg_t*  MsgPtr      - command message pointer
**    uint16         usExpLength - expected command length
**
** Returns:
**    boolean bResult - result of verification
**
** Routines Called:
**    TBD
**
** Called By:
**    TELEM_R_ProcessNewCmds
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby 
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
boolean TELEM_R_VerifyCmdLength(CFE_SB_Msg_t* MsgPtr,
                           uint16 usExpectedLen)
{
    boolean bResult=FALSE;
    uint16  usMsgLen=0;

    if (MsgPtr != NULL)
    {
        usMsgLen = CFE_SB_GetTotalMsgLength(MsgPtr);

        if (usExpectedLen != usMsgLen)
        {
            CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(MsgPtr);
            uint16 usCmdCode = CFE_SB_GetCmdCode(MsgPtr);

            CFE_EVS_SendEvent(TELEM_R_MSGLEN_ERR_EID, CFE_EVS_ERROR,
                              "TELEM_R - Rcvd invalid msgLen: msgId=0x%08X, cmdCode=%d, "
                              "msgLen=%d, expectedLen=%d",
                              MsgId, usCmdCode, usMsgLen, usExpectedLen);
            g_TELEM_R_AppData.HkTlm.usCmdErrCnt++;
        }
    }

    return (bResult);
}
    
/*=====================================================================================
** Name: TELEM_R_AppMain
**
** Purpose: To define TELEM_R application's entry point and main process loop
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    CFE_ES_RegisterApp
**    CFE_ES_RunLoop
**    CFE_ES_PerfLogEntry
**    CFE_ES_PerfLogExit
**    CFE_ES_ExitApp
**    CFE_ES_WaitForStartupSync
**    TELEM_R_InitApp
**    TELEM_R_RcvMsg
**
** Called By:
**    TBD
**
** Global Inputs/Reads:
**    TBD
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  Austin Cosby 
**
** History:  Date Written  2018-07-30
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void TELEM_R_AppMain()
{
    /* Register the application with Executive Services */
    CFE_ES_RegisterApp();

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(TELEM_R_MAIN_TASK_PERF_ID);

    /* Perform application initializations */
    if (TELEM_R_InitApp() != CFE_SUCCESS)
    {
        g_TELEM_R_AppData.uiRunStatus = CFE_ES_APP_ERROR;
    } else {
        /* Do not perform performance monitoring on startup sync */
        CFE_ES_PerfLogExit(TELEM_R_MAIN_TASK_PERF_ID);
        CFE_ES_WaitForStartupSync(TELEM_R_TIMEOUT_MSEC);
        CFE_ES_PerfLogEntry(TELEM_R_MAIN_TASK_PERF_ID);
    }
    
    LSM9DS1_HwInit();
    /* Application main loop */
    while (CFE_ES_RunLoop(&g_TELEM_R_AppData.uiRunStatus) == TRUE)
    {
        TELEM_R_RcvMsg(CFE_SB_PEND_FOREVER);
    }

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(TELEM_R_MAIN_TASK_PERF_ID);

    /* Exit the application */
    CFE_ES_ExitApp(g_TELEM_R_AppData.uiRunStatus);
} 
   /* 
**=====================================================================================*/
    
