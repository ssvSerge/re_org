#include "RemoteExec.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <cassert>
#include "logging.h"

static void reapChild(int unused) {
    while (true)
    {
        auto pid = waitpid(-1, nullptr, WNOHANG);
        if (pid <= 0) break;
    }
}

RemoteExec::RemoteExec()
{
    signal(SIGCHLD, reapChild);
}

RemoteExec::~RemoteExec()
{
    debug("RemoteExec::Destruct!");

    if (tIDExec.joinable())
    {
        debug("RemoteExec: tIDExec.join on destruct");
        tIDExec.join();
    }
    debug("RemoteExec: tIDExec exit!");
    if (tTiming.joinable())
    {
        debug("RemoteExec: tTiming.join on destruct");
        tTiming.join();
    }
    debug("RemoteExec: tTiming exit!");
}

int32_t RemoteExec::get_status()
{
    if (remote_exec_state == RemoteExecStates::Running || remote_exec_state == RemoteExecStates::Not_started)
    {
        return 0;
    }
    if (remote_exec_state == RemoteExecStates::Stopped)
    {
        return 1;
    }
    if (remote_exec_state == RemoteExecStates::Timed_out)
    {
        return -1;
    }
    err("Unknown get_status result is populated!");
    //assert(!"RemoteExec::get_status() invalid state!");
    return -2;
}

void RemoteExec::set_exec_callback(std::function<RemoteExecStatusCallbackPrototype> callback)
{
    callback_ = std::move(callback);
}

int32_t RemoteExec::execute_shell_command(std::vector<std::string> args, int32_t timeout_sec)
{
    remote_exec_state = RemoteExecStates::Not_started;
    is_finished = false;
    is_output_finished = false;
    is_timed_out = false;
    exec_return_code = -1;
    // spin off thread
    tIDExec = std::thread(&RemoteExec::exec_thread, this, args);

    native_handle = tIDExec.native_handle();
    tTiming = std::thread(&RemoteExec::timing_worker, this, timeout_sec);
    return 0;
}
int32_t RemoteExec::get_output(std::string& strVec)
{
    return 0;
}

int32_t RemoteExec::get_return_code()
{
    return exec_return_code.load();
}

void RemoteExec::timing_worker(int32_t timeout_seconds)
{
    auto time_stamp = std::chrono::system_clock::now();

    while (!is_finished)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto new_time = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(new_time - time_stamp).count();
        if (duration > timeout_seconds)
        {
            is_timed_out = true;
            remote_exec_state = RemoteExecStates::Timed_out;
            exec_return_code = -1;
        }
        if (reset_timeout == true)
        {
            time_stamp = std::chrono::system_clock::now();
            reset_timeout = false;
        }
    }
    remote_exec_state = RemoteExecStates::Stopped;
}

void RemoteExec::exec_thread(std::vector<std::string> args)
{
    remote_exec_state = RemoteExecStates::Running;
    int stdout_fds[2];
    pipe(stdout_fds);
    reset_timeout = false;

    bool inc_stderr = true;

    int stderr_fds[2];
    if (!inc_stderr)
    {
        pipe(stderr_fds);
    }

    const pid_t pid = fork();
    if (!pid)
    {
        close(stdout_fds[0]);
        dup2(stdout_fds[1], 1);
        if (inc_stderr)
        {
            dup2(stdout_fds[1], 2);
        }

        close(stdout_fds[1]);

        if (!inc_stderr)
        {
            close(stderr_fds[0]);
            dup2(stderr_fds[1], 2);
            close(stderr_fds[1]);
        }

        std::vector<char*> vc(args.size() + 1, NULL);
        for (size_t i = 0; i < args.size(); ++i)
        {
            vc[i] = const_cast<char*>(args[i].c_str());
        }

        execvp(vc[0], &vc[0]); // this will not return if no failure.
        return;
    }

    close(stdout_fds[1]);

    if (pid < 0)
    {
        err("RemoteExec: PID < 0!!! PID = %d", pid);
        exec_return_code = -1;
        return;
    }

    std::thread th_reader([&] {
        
        fcntl(stdout_fds[0], F_SETFL, fcntl(stdout_fds[0], F_GETFL) | O_NONBLOCK);

        char buf[256]{};
        bool bDone = false;
        while (!is_timed_out && !bDone)
        {
            if (is_finished)
            {
                return;
            }
            auto read_len = read(stdout_fds[0], buf, 128);
            if (read_len > 0)
            {
                reset_timeout = true;
                if (callback_ != nullptr)
                {
                    callback_(std::string(buf));
                }
            }
            else {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    continue;
                }
                else {
                    bDone = true;
                }
            }
        }
        close(stdout_fds[0]);
        if (is_timed_out)
        {
            err("RemoteExec: Thread Read Timed Out!");
            remote_exec_state = RemoteExecStates::Timed_out;
            reapChild(0);
            return;
        }
        });
    int status;
    int wait_result = waitpid(pid, &status, 0);
    if (wait_result < 0)
    {
        err("RemoteExec: fail to wait for process");
        exec_return_code = 0;
    }
    else {
        exec_return_code = WEXITSTATUS(status);
        info("RemoteExec: exec_return_code = %d", exec_return_code.load());
    }

    fprintf(stdout, "\nRetCode = %d", exec_return_code.load());
    remote_exec_state == RemoteExecStates::Timed_out ? remote_exec_state : remote_exec_state = RemoteExecStates::Stopped;
    is_finished = true;
    if (th_reader.joinable())
    {
        debug("RemoteExec: th_reader.join()!");
        th_reader.join();
    }
    debug("RemoteExec: th_reader quit!");
}