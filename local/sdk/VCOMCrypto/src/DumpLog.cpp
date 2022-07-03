/******************************<%BEGIN LICENSE%>******************************/
// (c) Copyright 2013 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
//
// For a list of applicable patents and patents pending, visit www.lumidigm.com/patents/
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
/******************************<%END LICENSE%>******************************/
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>


extern "C"
{
void DumpLogStr    (const char *fileName, const char *str);
void DumpLogFmt    (const char *fileName, const char *fmt...);
void DumpLogBuffer (const char *fileName, const char *str, unsigned char *buffer, int nSz);
}

#ifdef __ANDROID__
#include <android/log.h>
void DumpLogStr (const char *fileName, const char *str)
{
    __android_log_write(ANDROID_LOG_INFO, "[VCOM]", str);
}
#else
void DumpLogStr (const char *fileName, const char *str)
{
    FILE *fp;

    fp = fopen (fileName, "a+");

    if (fp)
        {
        fseek   (fp, 0L, SEEK_END);
        fprintf (fp, "%s", str);
        fclose  (fp);
        }
    else
        printf ("DumpLogStr: error opening %s = %d\n", fileName, errno);
}
#endif

void DumpLogFmt (const char * fileName, const char *fmt...)
{
    va_list argptr;
    va_start(argptr, fmt);

    char line[128];
    int max = sizeof(line) - 1;

    // note that we are disregarding the return code...
    #ifdef WIN32
        _vsnprintf_s(line, sizeof(line), max, fmt, argptr);
    #else
        vsnprintf(line, max, fmt, argptr);
    #endif

    // _always_, _Always_, _ALWAYS_ terminate the string...
    line[max] = '\0';

    va_end(argptr);

    DumpLogStr(fileName, line);
}

void DumpLogBuffer (const char *fileName, const char *str, unsigned char *buffer, int nSz)
{
    int   i, j;
    char  strBuf[100];
    FILE *fp;

    fp = fopen (fileName, "a+");

    if (fp)
        {
        fseek (fp, 0L, SEEK_END);

        fprintf (fp, "%04x, %s: ", nSz, str); 

        for (i=0; i<nSz; i+=32)
            {
            if (i && !(i%32))
                fprintf (fp,"%s", (fileName[23]=='n')? "          " : "                                ");
            for (j=0; j<32 && i+j<nSz; j++)
                sprintf (strBuf+2*j, "%02x", buffer[i+j]);
            strBuf[2*j] = '\0';
            fprintf (fp, "%s\n", strBuf);
            }

        fclose  (fp);
        }
    else
        printf ("DumpLogBuffer: error opening %s = %d\n", fileName, errno);
}
