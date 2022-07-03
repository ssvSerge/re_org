#pragma once

#include <stdint.h>
#include <string>
#include <thread>
#include <vector>
#include <condition_variable>
#include <multitask.h>
#include <libaio.h>

#include <application/const.h>
#include <application/types.h>

#include <VCOMTransceiver.h>

struct usb_ctrlrequest;

class epinit {

    public:
        epinit();
        int32_t ep0_init(std::string epDirectory);
        int32_t init_ep_threads(std::string udcDirectory);
        int32_t write_udc(std::string udcDirectory);
        void    join();
        void    SetRecoveryMode(int32_t reason_num);
        void    ResetReceiveBufferCallback();

    private:
        int32_t init_ep(int32_t nEpNo);
        int     handle_jengine_cmd(int ep2);
        int     handle_fwupdate_cmd(int ep2);
        void    ep0_handler();
        void    ep_0_thread(std::string udcDirectory);
        void    write_ep_1_thread();
        int     read_ep_2_thread(int ep0, int ep1, int ep2);
        void    handle_setup(const struct usb_ctrlrequest* setup);
        bool    list_dir(std::string path, std::vector< std::string >& items);
        int     read_message(USBCB* readHdr, uint8_t* pRxBuff);
        int     read_event(int evfd, fd_set rfds, io_context_t ctx);
        int     wait_and_validate(int eprange, int evfd, fd_set rfds, io_context_t ctx, int nTimeoutMS);
        int     queue_read(iocb* iocb_read, int event_fd, io_context_t ctx, int endpoint, void* data, size_t num_bytes);
        int     queue_write(iocb* iocb_write, int event_fd, io_context_t ctx, int endpoint, void* data, size_t num_bytes);

    private:
        int     rx_request_enqueue(int ep2, struct iocb& iocb, void* const ptr, uint32_t len);
        int     rx_request_wait();
        int     tx_request_enqueue(int ep1, struct iocb& iocb, const void* const ptr, uint32_t len);
        int     tx_request_wait();

        int     read_usbcb_hdr(int ep2);
        int     read_usbcb_pkt(int ep2);
        int     write_usbcb_pkt(int ep1, const void* const ptr, uint32_t tx_len);

    private:
        const int                   nTimeoutReadMS      = 5000;
        const int                   nTimeoutWriteMS     = 1000;
        const std::string           m_udc_dir           = "/sys/class/udc";
        const std::string           m_gadget_dir        = "/home/root/ffs";

        std::atomic_bool            m_terminating       = ATOMIC_VAR_INIT(false);
        bool                        m_recovery_flag     = false;
        int32_t                     m_recovery_reason   = 0;

        std::vector<std::thread>    threadVec;

        std::string                 m_epDirectory;
        std::thread                 tid_ep0;
        std::thread                 tid_com;
        int32_t                     m_fs_eps[3];
        bin_data_t                  m_ep0Buffer;
        std::mutex                  m_write_mutex;
        std::condition_variable     m_condVarWrite;
        utils::thread_event         m_ep0_ready_event;
        utils::thread_event         m_terminate_event;
        utils::thread_event         m_error_event;
        VCOMTransceiver             m_VCOMTransceiver;

        USBCB                       m_read_hdr;
        bin_data_t                  m_read_payload;
        USBCB                       m_write_hdr;
        bin_data_t                  m_write_payload;
        USBCB                       m_err_hdr;
        bin_data_t                  m_err_payload;
        uint32_t                    m_resp_length;


        io_context_t                m_ctx;
        int                         m_evfd_read;
        int                         m_evfd_write;
        int                         m_nEPRange;
        fd_set                      m_rfds;

        struct iocb                 m_iocb_in_hdr;
        struct iocb                 m_iocb_in_pkt;
        struct iocb                 iocb_out_hdr;
        struct iocb                 iocb_out_pay;
};