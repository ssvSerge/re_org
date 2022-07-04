#include <signal.h>
#include <cxxopts.hpp>

#include <hid/defs.h>
#include <hid/usb_transport_device.h>

#include <logging.h>
// #define err      printf
// #define info     printf
// #define warn     printf


const char* pEP0Directory       = "/var/data/system/ffs";
const char* pGadgetDirectory    = "/sys/kernel/config/usb_gadget/";

static hid::transport::usb_transport_device_t usb_device;


FILE* open_file(std::string loc) {
    FILE* fFile = fopen(loc.c_str(), "w+");
    return fFile;
}

void signal_callback(int signo) {
    UNUSED(signo);
    err("SIGTERM received. Shutdown application.");
    usb_device.stop();
}

int main(int argc, char** argv) {

    int     verbosity = 7;
    FILE* fLogFD = 0;

    std::string log_location = "/tmp/usb_transceiver.log";

    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, signal_callback);

    cxxopts::Options options("usb_transceiver", "Handles bulk transfers of VCOM protocol messages.");
    options.add_options()
        ("l,log",           "Log file location; stdout to print to console.", cxxopts::value<std::string>())
        ("v,verbosity",     "Verbosity level. 3 = err, 4 = warn, 5 = note, 6 = info, 7 = debug", cxxopts::value<int>())
        ("g,gadget",        "Name of the gadget directory", cxxopts::value<std::string>())
        ("h,help",          "Print usage")
        ;

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (result.count("verbosity")) {
        int vrb = result["verbosity"].as<int>();
        if (vrb < 3) {
            vrb = 3;
        }
        if (vrb > 7) {
            vrb = 7;
        }
        verbosity = vrb;
    }

    if (result.count("log")) {
        log_location = result["log"].as<std::string>();
        if (log_location.compare("stdout") == 0) {
            fLogFD = stdout;
        } else {
            fLogFD = open_file(log_location);
            if (fLogFD == NULL) {
                std::cout << "unable to open log file: \"" << log_location << "\"" << std::endl;
            }
        }
    }

    // set UDC directory
    std::string udcDirectory(pGadgetDirectory);
    if (result.count("gadget")) {
        udcDirectory += result["gadget"].as<std::string>();
    } else {
        udcDirectory += "hidcam";
    }

    set_log_file(fLogFD);
    set_verbosity(verbosity);

    if ( 0 != usb_device.ep0_init(pEP0Directory) ) {
        err("Failed to initialize EP0. Exit from: (%s):(%d)", __FUNCTION__, __LINE__);
        return -1;
    }

    usb_device.init_ep_threads();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    usb_device.join();
    warn("usb_transceiver - Exiting.");
    return 0;
}
