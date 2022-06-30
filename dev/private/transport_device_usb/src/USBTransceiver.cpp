#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <signal.h>

#include "epinit.h"
#include "cxxopts.hpp"
#include "logging.h"
#include "../../socketComm/include/SocketClient.h"
#include <sys/reboot.h>

/******************** Main **************************************************/

const char* pEP0Directory = "/var/data/system/ffs";
const char* pGadgetDirectory = "/sys/kernel/config/usb_gadget/";
std::string g_sensor_type_str;

FILE* open_file(std::string loc){
   FILE* fFile = fopen(loc.c_str(), "w+");
   return fFile;
}

int main(int argc, char **argv)
{
    int verbosity = 7;
    FILE *fLogFD = 0;
    signal(SIGPIPE, SIG_IGN);
    // Handle command line

    cxxopts::Options options("usb_transceiver", "Handles bulk transfers of VCOM protocol messages.");
    options.add_options()
        ("t,type","Type of the sensor", cxxopts::value<std::string>())
        ("l,log", "Location of log file, or stdout", cxxopts::value<std::string>())
        ("v,verbosity", "3 = err, 4 = warn, 5 = note, 6 = info, 7 = debug", cxxopts::value<int>())
        ("r,recovery","Recovery mode")
        ("c,cause", "Recovery caused reason", cxxopts::value<int32_t>())
        ("g,gadget","Name of the gadget directory", cxxopts::value<std::string>())
        ("h,help", "Print usage")
    ;

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (result.count("type") == 0)
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }else{
        g_sensor_type_str = result["type"].as<std::string>();
    }

    if (result.count("verbosity"))
    {
        int vrb = result["verbosity"].as<int>();
        if (vrb < 2 || vrb > 7)
        {
            std::cout << options.help() << std::endl;
            exit(0);
        }
        verbosity = vrb;
    }

    if (result.count("log"))
    {
        std::string log_location = result["log"].as<std::string>();
        if (log_location.compare("stdout") == 0)
        {
            fLogFD = stdout;
        }
        else
        {
            fLogFD = open_file(log_location);
            if (fLogFD == NULL)
            {
                std::cout << "unable to open log file." << std::endl;
                std::cout << options.help() << std::endl;
                exit(0);
            }
        }
    }
    // Set Log File
    set_log_file(fLogFD);
    set_verbosity(verbosity);
    // Set Verbosity
    epinit tHandler;
    if (result.count("recovery"))
    {
        err("In recovery mode!!! 1");
        if (result["recovery"].as<bool>())
        {
            err("In recovery mode!!! bool result is true! argument provided!");
            if (!result.count("cause"))
            {
                warn("cause not specified!");
                tHandler.SetRecoveryMode(0);
            }
            else {

                tHandler.SetRecoveryMode(result["cause"].as<int32_t>());
            }
        }
    }
    // set UDC directory
    std::string udcDirectory(pGadgetDirectory);
    if(result.count("gadget"))
    {
        udcDirectory += result["gadget"].as<std::string>();
    }
    else
    {
        udcDirectory += "usb_bulk_transceiver";
    }
    if (0 != tHandler.ep0_init(pEP0Directory))
    {
        err("Unable to init EP0 - Exiting.");
        return -1;
    }

    tHandler.init_ep_threads(udcDirectory);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // Should wait until all threads init.

    tHandler.join();
    warn("usb_transceiver - Exiting.");
    return 0;
}
