#pragma once

#include <string>

std::string get_timestamp();

extern "C" void _hb_msg(unsigned level, const char *fmt, ...);
extern "C" void set_log_file(FILE* fFile);
extern "C" void set_verbosity(int32_t verbosity);
extern "C" bool is_log_file_set();
extern "C" void close_log_file();

#define die(...)  ( _hb_msg(2, __VA_ARGS__), exit(1) )
#define err(...)    _hb_msg(3, __VA_ARGS__)
#define warn(...)   _hb_msg(4, __VA_ARGS__)
#define note(...)   _hb_msg(5, __VA_ARGS__)
#define info(...)   _hb_msg(6, __VA_ARGS__)
#define debug(...)  _hb_msg(7, __VA_ARGS__)
