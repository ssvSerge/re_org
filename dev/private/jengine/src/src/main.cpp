#include "global.h"

TDBG_DEFINE_AREA(jengmain);

#include <stdio.h>
#include <signal.h>
#include <chrono>

#include <CriticalErrorLog.h>
#include <JEngineShell.h>
#include <logging.h>
#include "cxxopts.hpp"

int main ( int argc, char* argv[] ) {

    signal(SIGPIPE, SIG_IGN);

    TTRACE_ENTER(jengmain, TDBG_DEBUG, "main()");

        bool init_res;

        init_res = JEngineShell::GetInstance()->StartServers();
        if ( ! init_res ) {
            TTRACE(jengmain, TDBG_ERROR, "JEngineShell failed to start. Exit.");
            goto exit;
        }

        while (1) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        JEngineShell::GetInstance()->StopServers();

exit:;
    TTRACE_LEAVE("main => (void)" );
    return 0;
}
