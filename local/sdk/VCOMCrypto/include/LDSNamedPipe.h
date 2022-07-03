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

#include "windows.h"

typedef enum
{
    Server,
    Client
} PipeMode;

typedef unsigned char uchar;
typedef unsigned int  uint;

class LDSNamedPipe
{
public:
    LDSNamedPipe();
    ~LDSNamedPipe(void);
    bool Initialize(PipeMode mode, LPCSTR lpPipeName);
    bool ReadPipe( uchar* pBuffer, uint nSize, uint nTimeoutMS);
    bool WritePipe(uchar* pBuffer, uint nSize, uint nTimeoutMS);
    bool GetPipeName(LPCSTR lpPipeName);
    bool Flush();
    bool Disconnect();

private:
    //
    bool InitServer();
    bool InitClient();
    // Error handling
    bool DisplayLastError();
    PipeMode    m_mode;
    HANDLE        m_hPipe;
    LPCSTR        m_lpPipeName;
    PipeMode    m_pipeMode;



};
