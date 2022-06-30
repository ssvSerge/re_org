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
#include "V100InternalCmd.h"
#include "V100EncCmd.h"
// #include "EncCmd.h"

#ifdef __GNUC__
// #include "TransportLibUSB.h"
// #include "windows_conv.h"
#else
// #include "windows.h"
// #include "TransportUSB.h"
#endif

#include <map>

ICmd* V100CommandHandler::CreateCommand(_V100_COMMAND_SET cmdSet) {

    ICmd* pGenericCommand = NULL;

    switch (cmdSet) {

        // *** Base Commands *** /
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

        // *** Crypto Commands *** /
        case CMD_ENC_CAPTURE:
            pGenericCommand = new Atomic_Enc_Capture();
            break;
        case CMD_ENC_CLEAR:
            pGenericCommand = new Atomic_Enc_Clear();
            break;
        case CMD_ENC_DECRYPT:
            pGenericCommand = new Atomic_Enc_Decrypt();
            break;
        case CMD_ENC_ENROLL:
            pGenericCommand = new Atomic_Enc_Enroll();
            break;
        case CMD_ENC_FACTORY_SET_KEY:
            pGenericCommand = new Atomic_Enc_Factory_Set_Key();
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
        case CMD_ENC_IDENTIFYMATCH:
            pGenericCommand = nullptr; // new Atomic_Enc_IdentifyMatch();
            break;
        case CMD_ENC_RETURNCAPTUREDBIR:
            pGenericCommand = new Atomic_Enc_ReturnCapturedBIR();
            break;
        case CMD_ENC_RETURNCAPTUREDBIR_IM:
            pGenericCommand = nullptr; // new Atomic_Enc_ReturnCapturedBIR_IM();
            break;
        case CMD_ENC_SET_KEY:
            pGenericCommand = new Atomic_Enc_Set_Key();
            break;
        case CMD_ENC_SET_PARAMETERS:
            pGenericCommand = new Atomic_Enc_Set_Parameters();
            break;
        case CMD_ENC_UNLOCK_KEY:
            pGenericCommand = new Atomic_Enc_Unlock_Key();
            break;
        case CMD_ENC_VERIFYMATCH:
            pGenericCommand = new Atomic_Enc_VerifyMatch();
            break;

        default:
            break;
        }

    return pGenericCommand;
}
