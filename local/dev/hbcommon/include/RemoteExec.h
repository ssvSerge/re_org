#pragma once
#include "multitask.h"
#include <thread>
#include <atomic>
#include <pthread.h>
#include <functional>

enum class RemoteExecStates : int
{
    Running = -100,
    Stopped,
    Timed_out,
    Not_started,

};

using RemoteExecStatusCallbackPrototype = void(const std::string&);

class RemoteExec
{
public:
    RemoteExec();
    ~RemoteExec();
    int32_t execute_shell_command(std::vector<std::string> args, int32_t timeout_sec = 30);
    int32_t get_output(std::string& strVec);
    void set_exec_callback(std::function<RemoteExecStatusCallbackPrototype> callback);
    int32_t get_status();
    int32_t get_return_code();
private:
    // Remove Execution Thread
    void exec_thread(std::vector<std::string> args);
    void timing_worker(int32_t timeout_sec);
    std::thread tIDExec;
    std::thread tTiming;
    std::thread::native_handle_type native_handle;
    std::atomic<bool> is_finished {false};
    utils::safe_queue<std::string> safe_queue;
    std::atomic<int> exec_return_code;
    std::atomic<bool> is_output_finished {false};
    RemoteExecStates remote_exec_state = RemoteExecStates::Not_started;
    std::vector<std::string> exec_state_buffer;
    long long exec_state_line;
    std::atomic<bool> is_timed_out;
    std::atomic<bool> reset_timeout;

    std::function<RemoteExecStatusCallbackPrototype> callback_ = nullptr;
};