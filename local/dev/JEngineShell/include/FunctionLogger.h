#ifndef __FUNCTIONLOGGER_H__
#define __FUNCTIONLOGGER_H__

#include <string>
#include <logging.h>


class CFunctionLogger {
    public:
        CFunctionLogger(const char* const function) {
            name_ = function;
            info("%s()", function);
        }

    public:
        void error(const char* const msg) {
            err("%s: %s", name_.c_str(), msg);
        }

        void info(const char* const msg) {
            info("%s: %s", name_.c_str(), msg);
        }

    private:
        std::string  name_;

};

#define ENTER()   CFunctionLogger __l_logger( __FUNCTION__ );
#define ERROR(x)  __l_logger.error(x);
#define INFO(x)   __l_logger.info(x);

#endif
