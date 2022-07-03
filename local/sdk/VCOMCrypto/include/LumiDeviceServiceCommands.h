/*************************************************************************************************************************
**                                                                                                                      **
** ©Copyright 2017 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                           **
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
#pragma once


typedef enum
{
    LUMI_DS_CMD_GET_NUM_SENSORS      = 0,
    LUMI_DS_CMD_GET_NAMED_PIPE         = 1,
    LUMI_DS_CMD_GET_SENSOR_INFO         = 2,
} LUMI_DEVICE_SERVICE_COMMANDS;


typedef struct
{
  LUMI_DEVICE_SERVICE_COMMANDS Command;                // The command to execute
  unsigned int                 devNumber;            // The device ID assigned by the LDS
                                                    // for LUMI_DS_CMD_GET_SENSOR_INFO it is the index into our device map
  char                         strCommander[256];    // A string to indicate who sent the command
} LumiDevServiceCmdStruct;

