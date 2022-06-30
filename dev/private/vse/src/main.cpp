#include "HBSETransceiver.h"
#include <signal.h>
#include "cxxopts.hpp"

int kQuitRequest = 0;

void SetQuitNotice(int sig)
{
    kQuitRequest = 1;
}

void StartVSE(bool start_test_mode)
{
    HBSETransceiver tApp;
    if (start_test_mode)
    {
#if ENABLE_DEBUG_MODE
#pragma message("WARNING: DEBUG MODE enabled in this verison. Should not be in the release!")
        tApp.SetDebugMode(start_test_mode);
        fprintf(stdout, "\nInfo: Debug mode enabled.");
#else
        fprintf(stdout, "Warning: This version does not support DEBUG MODE! Please set ENABLE_DEBUG_MODE ");
#endif
    }
    tApp.StartServer();
    while (kQuitRequest == 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    tApp.ExitApp();
}

int main(int argc, char* argv[])
{
    if (signal(SIGINT,SetQuitNotice) == SIG_ERR)
        return -1;
    cxxopts::Options options("VirtualSecureElement", "Virtual secure element for V52x sensor");
    options.add_options()
        ("t,test-mode", "Enable test mode", cxxopts::value<bool>()->default_value("false"))
        ("o,optee-mode", "Switch to op-tee mode", cxxopts::value<bool>()->default_value("false"))\
        ("h,help", "show help", cxxopts::value<bool>());
    const auto parse_result = options.parse(argc, argv);
    bool start_test_mode = false;
    if (parse_result.count("help"))
    {
        options.help();
        return 0;
    }

    if (parse_result.count("test-mode"))
    {
        start_test_mode = parse_result["test-mode"].as<bool>();
    }

    if (parse_result.count("optee-mode"))
    {
        if (parse_result["optee-mode"].as<bool>() == true)
        {
            fprintf(stdout, "Not supporting OPTEE Client now...");
        }
        else {
            StartVSE(start_test_mode);
        }
    }
    else {
        StartVSE(start_test_mode);
    }
    return 0;
}