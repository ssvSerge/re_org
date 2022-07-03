#pragma once
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string>
#include <cerrno>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <atomic>
#include "usbcmdset.h"

#include <application/const.h>
#include <application/types.h>

class SpinLock {
    public:
        SpinLock() = default;

        explicit SpinLock(const bool lock_on) {
            if (lock_on)
                lock();
        }

        void lock() {
            while (lock_flag_.test_and_set(std::memory_order_acquire)) { std::this_thread::yield(); }
        }
            
        void unlock() {
            lock_flag_.clear(std::memory_order_release);
        }

        ~SpinLock() {
            unlock();
        }

        SpinLock& operator=(SpinLock&) = delete;
        SpinLock& operator=(SpinLock&&) = delete;
        SpinLock(SpinLock&&) = delete;
        SpinLock(const SpinLock&) = delete;

    private:
        std::atomic_flag lock_flag_ = ATOMIC_FLAG_INIT;
};

enum class TransferStates {
    OK,
    GEN_ERR,
    TIMEOUT,
    NOT_STARTED,
    CLOSED,
    WAIT_ACCEPT
};


template <typename Type>
class DataSerialization {
    public:
        static std::vector<unsigned char> Serialize(const Type& value) {
            const auto size_field_len = sizeof(USBCB);
            USBCB header{};
            header.ulCount = sizeof(value);
            std::vector<unsigned char> target(size_field_len + sizeof(value));
            memcpy(target.data(), &header, sizeof(USBCB));
            memcpy(target.data() + size_field_len, &value, sizeof(value));
            return target;
        }

        static std::vector<unsigned char> Serialize(const Type* value, const unsigned data_size) {
            const auto size_field_len = sizeof(USBCB);
            std::vector<unsigned char> target(size_field_len + data_size);
            USBCB header{};
            header.ulCount = data_size;
            memcpy(target.data(), &header, sizeof(USBCB));
            memcpy(target.data(), value, data_size);
            return target;
        }
};

class Logger {
    public:
        Logger(const char* log_file);
};

