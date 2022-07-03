#pragma once

#include <stdint.h>
#include <vector>
#include <functional>
#include "SocketClient.h"
#include "TransactionBroker.h"

#include <application/types.h>

using CallbackPrototype = std::function<void()>;

class VCOMTransceiver {

    public:
        VCOMTransceiver();
        VCOMTransceiver(bool recover_flag, int32_t recovery_reason = 0);
        ~VCOMTransceiver();

    public:
        void ResetUnit();
        bool Should_Reboot();
        void SetResetRxBufCallback(CallbackPrototype callback_func);
        void SetRecoveryMode(int32_t reason_num);

        uint32_t message_arrived(const bin_data_t& request, bin_data_t& response);

    private:
        uint32_t connect_engine();
        void     SendSEShutdownMessageToShell();
        void     StopConnection();

    private:
        std::string         vcom_socket_name;
        SocketClient        vcom_socket_client_;
        CallbackPrototype   callback_func_;
        uint32_t            busno                   = 0;
        uint32_t            devno                   = 0;
        TransactionBroker*  m_pTransactionBroker    = nullptr;
        bool                in_macro_mode_          = false;
        bool                m_recovery_flag_        = false;
        bool                m_connected             = false;
        int                 cmd_handle_flags_       = 0;
        int32_t             m_recovery_reason_      = 0;
};