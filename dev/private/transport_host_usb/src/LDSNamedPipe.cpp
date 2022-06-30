/***************************************************************************************/
// �Copyright 2020 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.
//
// For a list of applicable patents and patents pending, visit www.hidglobal.com/patents/
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
/***************************************************************************************/
#include "stdafx.h"
#include "windows.h"
#include "LDSNamedPipe.h"
#include "strsafe.h"

//#define m_szPipeName "\\\\.\\Pipe\\MyNamedPipe"
#define BUFFER_SIZE    524288

LDSNamedPipe::LDSNamedPipe()
{
    m_hPipe = 0;
}

LDSNamedPipe::~LDSNamedPipe(void)
{
    CloseHandle(m_hPipe);
}

bool LDSNamedPipe::Initialize(PipeMode mode, LPCSTR lpPipeName)
{
    m_lpPipeName = lpPipeName;
    bool bRet = false;
    m_pipeMode = mode;
    //
    switch(mode)
    {
        case Server:
        {
            bRet = InitServer();
        } break;
        case Client:
            {
            bRet = InitClient();
        } break;
    }
    return bRet;
}
bool LDSNamedPipe::GetPipeName(LPCSTR lpPipeName)
{
    lpPipeName = m_lpPipeName;
    return true;
}
bool LDSNamedPipe::InitServer()
{
    if( m_hPipe ) return false;

    SECURITY_ATTRIBUTES sa;
    sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
    InitializeSecurityDescriptor(sa.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
    // ACL is set as NULL in order to allow all access to the object.
    SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, NULL, FALSE);
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    m_hPipe  = CreateNamedPipeA( m_lpPipeName,          // pipe name
                              PIPE_ACCESS_DUPLEX,       // read/write access
                              PIPE_TYPE_MESSAGE |       // message type pipe
                              PIPE_READMODE_MESSAGE |   // message-read mode
                              PIPE_WAIT,                // blocking mode
                              PIPE_UNLIMITED_INSTANCES, // max. instances
                              BUFFER_SIZE,              // output buffer size
                              BUFFER_SIZE,              // input buffer size
                              NMPWAIT_USE_DEFAULT_WAIT, // client time-out
                              &sa);//NULL);                    // default security attribute

    //Wait for the client to connect
    BOOL bClientConnected = ConnectNamedPipe(m_hPipe, NULL);
    free(sa.lpSecurityDescriptor);
    return (m_hPipe != INVALID_HANDLE_VALUE)?true:false;
}

bool LDSNamedPipe::InitClient()
{
    if( m_hPipe ) return false;
    // Open the pipe
    uint nCtr = 2;
    while(nCtr--)
    {
        m_hPipe = CreateFileA(m_lpPipeName,   // pipe name
            GENERIC_READ |  // read and write access
            GENERIC_WRITE,
            0,              // no sharing
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe
            0,              // default attributes
            NULL);          // no template file

        if (INVALID_HANDLE_VALUE == m_hPipe)
        {
            //printf("\nError occurred while creating the pipe: %d", GetLastError());
            DWORD le = GetLastError();

            if(ERROR_PIPE_BUSY == le)
            {
                if(!WaitNamedPipeA(m_lpPipeName, 200))
                {
                    return false;
                }
            }
            return false;  //Error
        }
        else
            break;
    }

   return (m_hPipe != INVALID_HANDLE_VALUE)?true:false;
}

bool LDSNamedPipe::ReadPipe( uchar* pBuffer, uint nSize, uint nTimeoutMS)
{
    DWORD nSizeRead = 0;
    if( m_hPipe == NULL) return false;
    if(TRUE != ReadFile(m_hPipe, pBuffer, nSize, &nSizeRead, NULL))
    {
        //DisplayLastError();
        //fprintf(stdout,"\n Error reading file: %d", GetLastError());
        DWORD err = GetLastError();
        return false;

    }
    return true;
}

bool LDSNamedPipe::WritePipe(uchar* pBuffer, uint nSize, uint nTimeoutMS)
{
    DWORD nSizeWritten = 0;
    if( m_hPipe == NULL) return false;
    if(TRUE != WriteFile(m_hPipe, pBuffer, nSize, &nSizeWritten, NULL))
    {
        //DisplayLastError();
        //fprintf(stdout,"\n Error writing file: %d", GetLastError());
        return false;
    }
    return true;
}

bool LDSNamedPipe::Flush()
{
    if(TRUE == FlushFileBuffers(m_hPipe))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool LDSNamedPipe::Disconnect()
{
    if(TRUE == DisconnectNamedPipe(m_hPipe))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Last Error

bool LDSNamedPipe::DisplayLastError()
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,(lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("\nError %d: %s"),
        dw, lpMsgBuf);

    wprintf((wchar_t*)lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    return true;
}

