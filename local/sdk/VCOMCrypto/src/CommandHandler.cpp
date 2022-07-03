/*************************************************************************************************************************
**                                                                                                                      **
** ©Copyright 2019 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                           **
**                                                                                                                      **
** For a list of applicable patents and patents pending, visit www.hidglobal.com/patents                                **
**                                                                                                                      **
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                                           **
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS                                     **
** FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR                                       **
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER                                       **
** IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN                                              **
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                           **
**                                                                                                                      **
*************************************************************************************************************************/

#include "stdio.h"
#include "V100CommandHandler.h"
#include "V100Cmd.h"
#include "V100EncCmd.h"
#include "ITransport.h"
#include "IMemMgr.h"
#include "Device.h"

#include <application/types.h>


#ifdef __GNUC__
    #include "TransportLibUSB.h"
    #include "windows_conv.h"
#else
    #include "windows.h"
    #include "TransportUSB.h"
#endif

#include <map>

ICmd* V100CommandHandler::CreateCommand(_V100_COMMAND_SET cmdSet) {

    ICmd* pGenericCommand = nullptr;

    switch(cmdSet) {
        case CMD_CANCEL_OPERATION: 
            pGenericCommand = new Atomic_Cancel_Operation();
            break;

        case CMD_ERROR:
            pGenericCommand = new Atomic_Error();
            break;

        case CMD_GET_ACQ_STATUS:
            pGenericCommand = new Atomic_Get_Acq_Status();
            break;

        case CMD_GET_CONFIG:
            pGenericCommand = new Atomic_Get_Config();
            break;

        case CMD_GET_CMD:
            pGenericCommand = new Atomic_Get_Cmd();
            break;

        case CMD_GET_OP_STATUS:
            pGenericCommand = new Atomic_Get_OP_Status();
            break;

        case CMD_GET_SERIAL_NUMBER:
            pGenericCommand = new Atomic_Get_Serial();
            break;

        case CMD_GET_STATUS:
            pGenericCommand = new Atomic_Get_Status();
            break;

        case CMD_GET_SYSTEM_STATE:
            pGenericCommand = new Atomic_Get_System_State();
            break;

        case CMD_GET_TAG:
            pGenericCommand = new Atomic_Get_Tag();
            break;

        case CMD_RESET:
            pGenericCommand = new Atomic_Reset();
            break;

        case CMD_SET_TAG:
            pGenericCommand = new Atomic_Set_Tag();
            break;

        case CMD_UPDATE_FIRMWARE:
            pGenericCommand = new Macro_Update_Firmware();
            break;

        case CMD_ENC_GENERATE_RSA_KEYS:
            pGenericCommand = new Macro_Enc_Generate_RSA_Keys();
            break;

        case CMD_ENC_GET_KEY:
            pGenericCommand = new Atomic_Enc_Get_Key();
            break;

        case CMD_ENC_GET_KEYVERSION:
            pGenericCommand = new Atomic_Enc_Get_KeyVersion();
            break;

        case CMD_ENC_GET_RND_NUMBER:
            pGenericCommand = new Atomic_Enc_Get_Rnd_Number();
            break;

        case CMD_ENC_GET_SERIAL_NUMBER:
            pGenericCommand = new Atomic_Enc_Get_Serial_Number();
            break;

        case CMD_ENC_SET_KEY:
            pGenericCommand = new Atomic_Enc_Set_Key();
            break;

        case CMD_ENC_UNLOCK_KEY:
            pGenericCommand = new Atomic_Enc_Unlock_Key();
            break;

        case CMD_HID_INIT:
            pGenericCommand = new Atomic_Hid_Init();
            break;

        case CMD_HID_ENUM_CAMS:
            pGenericCommand = new Atomic_Hid_Enum_Cams();
            break;

        case CMD_HID_TERMINATE:
            pGenericCommand = new Atomic_Hid_Terminate();
            break;

        case CMD_HID_SET_PARAM_INT:
            pGenericCommand = new Atomic_Hid_Set_Param_Int();
            break;

        case CMD_HID_GET_PARAM_INT:
            pGenericCommand = new Atomic_Hid_Get_Param_Int();
            break;

        case CMD_HID_SET_PARAM_STR:
            pGenericCommand = new Atomic_Hid_Set_Param_Str();
            break;

        case CMD_HID_GET_PARAM_STR:
            pGenericCommand = new Atomic_Hid_Get_Param_Str();
            break;

        case CMD_HID_SET_PARAM_BIN:
            pGenericCommand = new Atomic_Hid_Set_Param_Bin();
            break;

        case CMD_HID_GET_PARAM_BIN:
            pGenericCommand = new Atomic_Hid_Get_Param_Bin();
            break;

        case CMD_HID_SET_PARAM_LONG:
            pGenericCommand = new Atomic_Hid_Set_Param_Long();
            break;

        case CMD_HID_GET_PARAM_LONG:
            pGenericCommand = new Atomic_Hid_Get_Param_Long();
            break;

        case CMD_HID_CAPTURE_IMAGE:
            pGenericCommand = new Atomic_Hid_Capture_Img();
            break;

        case CMD_HID_OPEN_CONTEXT:
            pGenericCommand = new Atomic_Hid_Open_Context();
            break;

        case CMD_HID_CLOSE_CONTEXT:
            pGenericCommand = new Atomic_Hid_Close_Context();
            break;

        case CMD_HID_STOP_CMD_ASYNC:
            pGenericCommand = new Atomic_Hid_Async_Stop_Operation();
            break;

        case CMD_HID_GET_VIDEO_FRAME:
            pGenericCommand = new Atomic_Hid_Get_Video_Frame();
            break;

        case CMD_HID_ASYNC_EXTRACT_TEMPLATE:
            pGenericCommand = new Atomic_Hid_Async_Extract_Template();
            break;

        case CMD_HID_ASYNC_IDENTIFY_WITH_TEMPLATE:
            pGenericCommand = new Atomic_Hid_Async_Identify_With_Template();
            break;

        case CMD_HID_ASYNC_MATCH_WITH_TEMPLATE:
            pGenericCommand = new Atomic_Hid_Async_Match_With_Template();
            break;

        case CMD_HID_ASYNC_MATCH_WITH_CAPTURED:
            pGenericCommand = new Atomic_Hid_Async_Match_With_Captured(); 
            break;

        case CMD_HID_ASYNC_VERIFY_WITH_CAPTURED:
            pGenericCommand = new Atomic_Hid_Async_Verify_With_Captured();
            break;

        case CMD_HID_ASYNC_IDENTIFY_WITH_CAPTURED:
            pGenericCommand = new Atomic_Hid_Async_Identify_With_Captured();
            break;

        case CMD_HID_ASYNC_VERIFY_WITH_TEMPLATE:
            pGenericCommand = new Atomic_Hid_Async_Verify_With_Template();
            break;

        case CMD_HID_GET_INTERMEDIATE_RES:
            pGenericCommand = new Atomic_Hid_Get_Intermediate_Res();
            break;

        case CMD_HID_GET_FINAL_RES: 
            pGenericCommand = new Atomic_Hid_Get_Final_Res(); 
            break;

        case CMD_HID_PARSE_RES_INT: 
            pGenericCommand = new Atomic_Hid_Parse_Res_Int(); 
            break;

        case CMD_HID_PARSE_RES_DOUBLE: 
            pGenericCommand = new Atomic_Hid_Parse_Res_Double(); 
            break;

        case CMD_HID_PARSE_RES_DATA: 
            pGenericCommand = new Atomic_Hid_Parse_Res_Data();
            break;

        case CMD_HID_PARSE_RES_POINT: 
            pGenericCommand = new Atomic_Hid_Parse_Res_Point();
            break;

        case CMD_HID_PARSE_RES_IMAGE: 
            pGenericCommand = new Atomic_Hid_Parse_Res_Image(); 
            break;

        case CMD_HID_PARSE_MATCH_GALLERY: 
            pGenericCommand = new Atomic_Hid_Parse_Match_Gallery(); 
            break;

        case CMD_HID_DB_ADD_RECORD_WITH_CAPTURED: 
            pGenericCommand = new Atomic_Hid_Db_Add_Record_With_Captured();
            break;

        case CMD_HID_DB_ADD_RECORD_WITH_TEMPLATE: 
            pGenericCommand = new Atomic_Hid_Db_Add_Record_With_Template(); 
            break;

        case CMD_HID_DB_GET_RECORD: 
            pGenericCommand = new Atomic_Hid_Db_Get_Record(); 
            break;

        case CMD_HID_DB_LIST_RECORDS: 
            pGenericCommand = new Atomic_Hid_Db_List_Records(); 
            break;

        case CMD_HID_DB_DEL_RECORD: 
            pGenericCommand = new Atomic_Hid_Db_Del_Record(); 
            break;

        case CMD_HID_FW_UPDATE:
            pGenericCommand = new Atomic_Hid_FwUpdate();
            break;


        default: 
            pGenericCommand = nullptr; 
            break;
    }

    return pGenericCommand;
}
