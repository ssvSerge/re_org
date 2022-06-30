#include <chrono>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <epconst.h>
#include <sys/eventfd.h>
#include <linux/usb/functionfs.h>

#include <application/const.h>
#include <application/vcom_tools.h>

#include <epinit.h>

#include <logging.h>
// #define err      printf
// #define info     printf
// #define warn     printf

#ifndef UNUSED
#define UNUSED(x)                   (void)(x)
#endif

#define IOCB_FLAG_RESFD             (1 << 0)

constexpr int UPDATE_DELAY_SEC      =  1;
constexpr int V100_PREFIX_LEN       = 12;

static const char* const names[] = {
    [FUNCTIONFS_BIND]    = "BIND",
    [FUNCTIONFS_UNBIND]  = "UNBIND",
    [FUNCTIONFS_ENABLE]  = "ENABLE",
    [FUNCTIONFS_DISABLE] = "DISABLE",
    [FUNCTIONFS_SETUP]   = "SETUP",
    [FUNCTIONFS_SUSPEND] = "SUSPEND",
    [FUNCTIONFS_RESUME]  = "RESUME",
};

static void print_errno() {
    switch (errno) {
        case EBADF:
            err("\nAn invalid file descriptor was given in one of the sets.");
            break;
        case EINTR:
            err("\nA signal was caught; see signal(7).");
            break;
        case EINVAL:
            err("\nnfds is negative or exceeds the RLIMIT_NOFILE resource or invalid timeout");
            break;
        case ENOMEM:
            err("\nUnable to allocate memory for internal tables.");
            break;
        default:
            err("\nUnknown error, errno = %d", errno);
            break;
    };
}

static void _log_frame(const char* const prefix, USBCB* hdr, uint8_t* data, uint32_t cnt) {

    UNUSED(prefix);
    UNUSED(hdr);
    UNUSED(data);
    UNUSED(cnt);

#if 0
    std::string msg;
    char        hex_buff[32];
    uint8_t*    ptr;
    uint32_t    out_cnt;

    msg = prefix;
    msg += "[ ";

    ptr = (uint8_t*)hdr;
    for (unsigned int i = 0; i < sizeof(USBCB); i++) {
        sprintf(hex_buff, "0x%.2x, ", ptr[i]);
        msg += hex_buff;
    }
    msg += "] [ ";

    ptr     = data;
    out_cnt = cnt;

    if (cnt > 128) {
        out_cnt = 32;
    }
    for (unsigned int i = 0; i < out_cnt; i++) {
        sprintf(hex_buff, "0x%.2x, ", ptr[i]);
        msg += hex_buff;
    }

    if (cnt != out_cnt) {
        msg += " ... ";
        msg += std::to_string(cnt);
        msg += " ... ";
    }

    msg += "]";

    info(msg.c_str());
#endif
}

static void _log_out_frame(USBCB* hdr, uint8_t* data, uint32_t cnt) {
    _log_frame("VCOM TX:", hdr, data, cnt);
}

static void _log_in_frame(USBCB* hdr, uint8_t* data, uint32_t cnt) {
    _log_frame("VCOM RX:", hdr, data, cnt);
}

static int  _file_write(int fout, const void* buf, size_t count) {

    const char* src = (const char*)buf;

    size_t wrCnt = 0;
    while (wrCnt < count) {
        ssize_t ioCnt;
        ioCnt = write(fout, src + wrCnt, count - wrCnt);
        if (ioCnt == -1) {
            return GEN_ERROR_FWUPDATE_FILEWRITE;
        }
        wrCnt += ioCnt;
    }

    return GEN_OK;
}

static void _update_thread(int n) {

    int io_res;

    std::this_thread::sleep_for(std::chrono::seconds(n));
    std::string mangling_string;
    std::string dst_file_name = FW_UPDATE_NAME_FINAL;


    info("FW_UPDATE: Mangling file name: \"%s\" ", FW_UPDATE_NAME_MANGLING );

    std::ifstream magling ( FW_UPDATE_NAME_MANGLING );
    if ( magling ) {
        std::stringstream extra_name;
        extra_name << magling.rdbuf();
        mangling_string += extra_name.str();
        info("FW_UPDATE: Mangling string: \"%s\" ", mangling_string.c_str() );
    } else {
        info("FW_UPDATE: Mangling file not found.");
    }

    dst_file_name  = FW_UPDATE_NAME_FINAL;
    dst_file_name += mangling_string;

    info("FW_UPDATE: Final file name: \"%s\" ", dst_file_name.c_str() );
    info("FW_UPDATE: Renaming file from \"%s\" to \"%s\" ", FW_UPDATE_NAME_TEMP, dst_file_name.c_str() );

    io_res = rename( FW_UPDATE_NAME_TEMP, dst_file_name.c_str() );
    if (io_res == 0) {
        err("FW_UPDATE: Done. Reboot required.");
    } else {
        err("FW_UPDATE: Failed to rename firmware file");
    }
}

epinit::epinit() {
}

void epinit::SetRecoveryMode(int32_t reason_num) {
    m_recovery_flag = true;
    m_recovery_reason = reason_num;
    m_VCOMTransceiver.SetRecoveryMode(reason_num);
}

int32_t epinit::ep0_init(std::string epDirectory) {

    Descriptors descriptors = get_descriptors();
    Strings strings = get_strings();
    int ret = {};

    std::string ep0Path = epDirectory + "/" + "ep0";
    info("%s: writing descriptors (in v2 format)\n", ep0Path.c_str());
    int fd = open(ep0Path.c_str(), O_RDWR | O_NDELAY | O_NONBLOCK);
    if (fd == -1) {
        err("Error opening endpoint (%s):(%d)", __FUNCTION__, __LINE__);
        print_errno();
        close(fd);
        m_error_event.notify_one();
        return -1;
    }

    ret = write(fd, &descriptors, sizeof(descriptors));
    if (ret == -1) {
        err("Endpoint rejected descriptors (%s):(%d)", __FUNCTION__, __LINE__);
        print_errno();
        close(fd);
        m_error_event.notify_one();
        return -1;
    }

    ret = write(fd, &strings, sizeof(strings));
    if (ret == -1) {
        err("Error: Endpoint rejected strings (%s):(%d)", __FUNCTION__, __LINE__);
        print_errno();
        close(fd);
        m_error_event.notify_one();
        return -1;
    }

    m_fs_eps[0] = fd;
    m_epDirectory = epDirectory;

    for (size_t ii = 1; ii < 3; ii++) {
        m_fs_eps[ii] = init_ep(ii);
        if (m_fs_eps[ii] == -1) {
            return -1;
        };
    }

    return 0;
}

int32_t epinit::init_ep(int32_t nEpNo) {
    char buffer[1024];
    sprintf(buffer, "%s/ep%d", m_epDirectory.c_str(), nEpNo);
    int fd = open(buffer, O_RDWR);
    return fd;
}

int32_t epinit::init_ep_threads(std::string udcDirectory) {

    if (m_epDirectory.size() == 0) {
        err("Error Not initialized (%s):(%d)", __FUNCTION__, __LINE__);
        return -1;
    }

    tid_ep0 = std::thread(&epinit::ep_0_thread, this, udcDirectory);
    // Wait for EP0 to come back
    info("Waiting on EP0...");

    while (false == m_ep0_ready_event.wait_for(1000)) {
        info("Waiting on EP0...");
    }

    if (m_error_event.wait_for(1) == true) {
        err("Unable to initialize endpoint. Exiting. (%s):(%d)", __FUNCTION__, __LINE__);
        exit(-1);
        return 0;
    }

    info("EP0 ready.");
    tid_com = std::thread(&epinit::read_ep_2_thread, this, m_fs_eps[0], m_fs_eps[1], m_fs_eps[2]);

    return 0;
}

int32_t epinit::write_udc(std::string udcDirectory) {

    std::vector<std::string> dir_list;

    if (false == list_dir(m_udc_dir, dir_list)) {
        err("Error: Unable to read UDC Directory (%s):(%d)", __FUNCTION__, __LINE__);
        return -1;
    }

    if (dir_list.size() != 1) {
        err("Error: Unexpected entry in %s (%s):(%d)", udcDirectory.c_str(), __FUNCTION__, __LINE__);
        return -1;
    }

    std::string gadget_hdc_path = udcDirectory + "/UDC";
    const char* pStr = gadget_hdc_path.c_str();

    FILE* fFile = fopen(pStr, "w+b");

    if (fFile == NULL) {
        info("Unable to open UDC at %s", udcDirectory.c_str());
        return -1;
    }

    fprintf(fFile, "%s", dir_list[0].c_str());
    fclose(fFile);
    return 0;
}

void epinit::ResetReceiveBufferCallback() {
    info("Called: %s", __FUNCTION__);
}

void epinit::join() {

    m_terminate_event.wait();
    if (tid_com.joinable()) {
        tid_com.join();
    }
    if (tid_ep0.joinable()) {
        m_terminating = true;
        tid_ep0.join();
    }
}

bool epinit::list_dir(std::string path, std::vector<std::string>& items) {
    dirent* entry = {};
    DIR* dir = opendir(path.c_str());
    if (dir == NULL) {
        return false;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR) {
            items.push_back(entry->d_name);
        }
    }

    closedir(dir);
    return true;
}

void epinit::handle_setup(const struct usb_ctrlrequest* setup) {
    info("bRequestType = %d\n", setup->bRequestType);
    info("bRequest     = %d\n", setup->bRequest);
    info("wValue       = %d\n", le16_to_cpu(setup->wValue));
    info("wIndex       = %d\n", le16_to_cpu(setup->wIndex));
    info("wLength      = %d\n", le16_to_cpu(setup->wLength));
}

void epinit::ep_0_thread(std::string udcDirectory) {

    info("ep 0 thread started.");

    size_t buffSize = 4 * sizeof(struct usb_functionfs_event);
    m_ep0Buffer.resize(buffSize);
    do {

        fd_set rfds;
        timeval t = { 0, 50 * 1000 };
        int ret = 0;
        while (ret == 0) {
            FD_ZERO(&rfds);
            FD_SET(m_fs_eps[0], &rfds);
            ret = select(m_fs_eps[0] + 1, &rfds, NULL, NULL, &t);
            if (m_terminating == true) {
                return;
            }
            t = { 0, 50 * 1000 };
        }

        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            err("EINTR received (%s):(%d)", __FUNCTION__, __LINE__);
        }

        int bytes_read = read(m_fs_eps[0], m_ep0Buffer.data(), buffSize);
        if (bytes_read == 0 || bytes_read == -1) {
            warn("Spinning....");
            continue;
        }

        const struct usb_functionfs_event* event = (usb_functionfs_event*)m_ep0Buffer.data();
        size_t n;

        for (n = bytes_read / sizeof(*event); n; --n, ++event) {

            switch (event->type) {
            case FUNCTIONFS_BIND:
                info("EP0:\tFUNCTIONFS_BIND");
                break;

            case FUNCTIONFS_UNBIND:
                info("EP0:\tFUNCTIONFS_UNBIND");
                break;

            case FUNCTIONFS_ENABLE:
                info("EP0:\tFUNCTIONFS_ENABLE");
                m_ep0_ready_event.notify_one();
                break;

            case FUNCTIONFS_DISABLE:
                info("EP0:\tEvent %s", names[event->type]);
                info("Rebooting Unit.");
                m_VCOMTransceiver.ResetUnit();
                break;

            case FUNCTIONFS_SETUP:
            case FUNCTIONFS_SUSPEND:
            case FUNCTIONFS_RESUME:
                info("EP0:\tEvent %s", names[event->type]);
                if (event->type == FUNCTIONFS_SETUP) {
                    handle_setup(&event->u.setup);
                }
                break;

            default:
                info("Event %03u (unknown)", event->type);
            }
        }

    } while (1);

    m_terminate_event.notify_one();
}

int  epinit::read_event(int evfd, fd_set rfds, io_context_t ctx) {

    if (FD_ISSET(evfd, &rfds)) {
        uint64_t ev_cnt;

        int ret = read(evfd, &ev_cnt, sizeof(ev_cnt));
        if (ret < 0) {
            err("unable to read eventfd (%s):(%d)", __FUNCTION__, __LINE__);
            return -1;
        }

        struct io_event e[2];

        ret = io_getevents(ctx, 1, 2, e, NULL);

        for (int i = 0; i < ret; ++i) {
            // debug("ev=%d; ret=%lu", e[i].obj->aio_fildes, e[i].res);
        }
    }
    return 1;
}

int  epinit::wait_and_validate(int eprange, int evfd, fd_set rfds, io_context_t ctx, int nTimeoutMS) {

    FD_ZERO(&rfds);
    FD_SET(evfd, &rfds);

    struct timeval timeout = {};
    timeout.tv_sec  = (nTimeoutMS / 1000);
    timeout.tv_usec = (nTimeoutMS % 1000) * 1000;

    struct timeval* pTimeout = &timeout;

    if (nTimeoutMS == 0) {
        pTimeout = NULL;
    }

    bool bInterrupted = true;
    while (bInterrupted == true) {

        int ret = select(eprange, &rfds, NULL, NULL, pTimeout);

        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            warn("EINTR received.\n");
        }

        bInterrupted = false;
        if (ret == 0) {
            return 0;
        }
    }

    return read_event(evfd, rfds, ctx);
}

int  epinit::queue_read(iocb* iocb_read, int event_fd, io_context_t ctx, int endpoint, void* data, size_t num_bytes) {

    io_prep_pread(iocb_read, endpoint, data, num_bytes, 0);

    iocb_read->u.c.flags |= IOCB_FLAG_RESFD;
    iocb_read->u.c.resfd = event_fd;

    int ret = io_submit(ctx, 1, &iocb_read);

    return (ret > 0) ? 0 : -1;
}

int  epinit::queue_write(iocb* iocb_write, int event_fd, io_context_t ctx, int endpoint, void* data, size_t num_bytes) {

    io_prep_pwrite(iocb_write, endpoint, data, num_bytes, 0);

    iocb_write->u.c.flags |= IOCB_FLAG_RESFD;
    iocb_write->u.c.resfd = event_fd;

    int ret = io_submit(ctx, 1, &iocb_write);

    return (ret > 0) ? 0 : -1;
}

int  epinit::rx_request_enqueue(int ep2, struct iocb& iocb, void* const ptr, uint32_t len) {

    int  retVal = VCOM_USB_ERR_OK;
    int  ioRes;

    for (int errors_cnt = 0; errors_cnt < VCOM_MAX_USBCB_ERR_CNT; errors_cnt++) {

        ioRes = queue_read(&iocb, m_evfd_read, m_ctx, ep2, ptr, len);

        if (ioRes != 0) {
            retVal = VCOM_USB_ERR_IO;
            info("Alert: queue_read failed with code %d; (%s):(%d)", ioRes, __FUNCTION__, __LINE__);
            std::this_thread::sleep_for(std::chrono::milliseconds(VCOM_USB_RETRY_DELAY_MS));
            continue;
        }

        if (retVal != VCOM_USB_ERR_OK) {
            retVal = VCOM_USB_ERR_OK;
            info("queue_read success (%s):(%d)", __FUNCTION__, __LINE__);
        }

        break;
    }

    if (retVal != VCOM_USB_ERR_OK) {
        err("Error: queue_read failed with code: %d (%s):(%d)", ioRes, __FUNCTION__, __LINE__);
    }

    return retVal;
}

int  epinit::rx_request_wait() {

    int  retVal = VCOM_USB_ERR_OK;
    int  ioRes;

    for (int retry_cnt = 0; retry_cnt < VCOM_MAX_USBCB_ERR_CNT; retry_cnt++) {

        ioRes = wait_and_validate(m_nEPRange, m_evfd_read, m_rfds, m_ctx, VCOM_USB_MTU_DELAY_MS);
        if (ioRes == 0) {
            info("Alert: RX Timeout (%s):(%d)", __FUNCTION__, __LINE__);
            retVal = VCOM_USB_ERR_TIMEOUT;
            continue;
        }

        if (ioRes < 0) {
            err("Error: Read Error (%s):(%d)", __FUNCTION__, __LINE__);
            retVal = VCOM_USB_ERR_IO;
            break;
        }

        if (retVal != VCOM_USB_ERR_OK) {
            retVal = VCOM_USB_ERR_OK;
            info ("Data Received (%s):(%d)", __FUNCTION__, __LINE__);
        }

        break;
    }

    return retVal;
}

int  epinit::tx_request_enqueue(int ep1, struct iocb& iocb, const void* const ptr, uint32_t len) {

    int  retVal = VCOM_USB_ERR_OK;
    int  ioRes;

    for (int errors_cnt = 0; errors_cnt < VCOM_MAX_USBCB_ERR_CNT; errors_cnt++) {

        ioRes = queue_write(&iocb, m_evfd_write, m_ctx, ep1, const_cast<void*>(ptr), len);

        if (ioRes != 0) {
            retVal = VCOM_USB_ERR_TIMEOUT;
            info("Alert: queue_write failed with code %d; (%s):(%d)", ioRes, __FUNCTION__, __LINE__);
            std::this_thread::sleep_for(std::chrono::milliseconds(VCOM_USB_RETRY_DELAY_MS));
            continue;
        }

        if (retVal != VCOM_USB_ERR_OK) {
            retVal = VCOM_USB_ERR_OK;
            info("queue_write success (%s):(%d)", __FUNCTION__, __LINE__);
        }

        break;
    }

    if (retVal != VCOM_USB_ERR_OK) {
        err("Error: queue_write failed with code: %d (%s):(%d)", ioRes, __FUNCTION__, __LINE__);
    }

    return VCOM_USB_ERR_OK;
}

int  epinit::tx_request_wait() {

    int  retVal = VCOM_USB_ERR_OK;
    int  ioRes;

    for (int retry_cnt = 0; retry_cnt < VCOM_MAX_USBCB_ERR_CNT; retry_cnt++) {

        ioRes = wait_and_validate(m_nEPRange, m_evfd_write, m_rfds, m_ctx, VCOM_USB_MTU_DELAY_MS);
        if (ioRes == 0) {
            info("Alert: TX Timeout (%s):(%d)", __FUNCTION__, __LINE__);
            retVal = VCOM_USB_ERR_TIMEOUT;
            continue;
        }

        if (ioRes < 0) {
            err("Error: Write Error (%s):(%d)", __FUNCTION__, __LINE__);
            retVal = VCOM_USB_ERR_IO;
            break;
        }

        if (retVal != VCOM_USB_ERR_OK) {
            retVal = VCOM_USB_ERR_OK;
            info("Data Sent (%s):(%d)", __FUNCTION__, __LINE__);
        }

        break;
    }

    return retVal;
}

int  epinit::read_usbcb_hdr(int ep2) {

    int retVal = VCOM_USB_ERR_OK;
    int ioRes;

    ioRes = rx_request_enqueue(ep2, m_iocb_in_hdr, &m_read_hdr, sizeof(USBCB) );
    if (ioRes != 0) {
        err("Error: enqueue_rx_request failed (%s):(%d)", __FUNCTION__, __LINE__);
        return VCOM_USB_ERR_IO;
    }

    for ( ; ; ) {

        if (m_terminating) {
            info("Alert: Terminate requested. (%s):(%d)", __FUNCTION__, __LINE__);
            retVal = VCOM_USB_ERR_SHUTDOWN;
            break;
        }

        ioRes = wait_and_validate(m_nEPRange, m_evfd_read, m_rfds, m_ctx, 500);
        if (ioRes < 0) {
            err("Error: Read error. (%s):(%d)", __FUNCTION__, __LINE__);
            retVal = VCOM_USB_ERR_IO;
            break;
        }

        if (ioRes == 0) {
            // Timeout. Expected state before transaction.
            continue;
        }

        // USB frame received.
        retVal = VCOM_USB_ERR_OK;
        break;
    }

    if (retVal != VCOM_USB_ERR_OK) {
        return retVal;
    }

    if ( ! vcom_hdr_validate(&m_read_hdr) ) {
        err("Invalid Header received. Ignore it. (%s):(%d)", __FUNCTION__, __LINE__);
        return VCOM_USB_ERR_CMD;
    }

    return VCOM_USB_ERR_OK;
}

int  epinit::handle_jengine_cmd(int ep2) {

    int ioRes = 0;

    // Read PAYLOAD
    ioRes = read_usbcb_pkt(ep2);
    if (VCOM_USB_ERR_OK != ioRes) {
        err("Error: Failed to read Payload (%s):(%d)", __FUNCTION__, __LINE__);
        return -1;
    }

    _log_in_frame(&m_read_hdr, m_read_payload.data(), m_read_hdr.ulCount);

    // Handle transaction.
    ioRes = m_VCOMTransceiver.message_arrived(m_read_payload, m_write_payload);

    if (0 != ioRes) {
        err("Error: Message handling failed with code: %d. Transaction terminated. (%s):(%d)", ioRes, __FUNCTION__, __LINE__);
        if (m_write_payload.size() == 0) {
            uint8_t v100_err[] = { 0x0d, 0x56, 0xe0, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            info("Message replaced with V100 error. (%s):(%d)", __FUNCTION__, __LINE__);
            m_write_payload.assign(v100_err, v100_err + sizeof(v100_err));
        }
    }

    _log_out_frame( &m_write_hdr, m_write_payload.data(), m_write_payload.size() );

    return 0;
}

int  epinit::handle_fwupdate_cmd(int ep2) {

    int ret_val = 0;

    info("FW_UPDATE: Started; (%s):(%d)", __FUNCTION__, __LINE__);

    try {

        int fout = -1;

        {   // Open file to store firmware. 
            fout = open(FW_UPDATE_NAME_TEMP, O_WRONLY | O_CREAT | O_TRUNC, 644);
            if (fout == -1) {
                err("FW_UPDATE: Failed to open file: \"%s\"; (%s):(%d)", FW_UPDATE_NAME_TEMP, __FUNCTION__, __LINE__);
            }

        }

        {   // Store Payload to file.
            uint32_t    rx_cnt    = 0;
            uint32_t    data_part = 0;
            std::string tmp_buffer;

            tmp_buffer.resize (VCOM_USB_MTU);

            for ( ; ; ) {

                if ( m_read_hdr.ulCount == rx_cnt ) {
                    break;
                }

                data_part = m_read_hdr.ulCount - rx_cnt;

                if (data_part > VCOM_USB_MTU) {
                    data_part = VCOM_USB_MTU;
                }

                ret_val = rx_request_enqueue(ep2, m_iocb_in_pkt, tmp_buffer.data(), data_part);
                if ( ret_val != VCOM_USB_ERR_OK ) {
                    err ("FW_UPDATE: Failed to enqueue rx_request; (%s):(%d)", __FUNCTION__, __LINE__);
                    break;
                }

                ret_val = rx_request_wait();
                if ( ret_val != VCOM_USB_ERR_OK ) {
                    err("FW_UPDATE: Failed to read; (%s):(%d)", __FUNCTION__, __LINE__);
                    break;
                }

                if ( fout != -1) {

                    int32_t io_res = 0;
                    int32_t store_offset = 0;

                    if ( (rx_cnt == 0) && (data_part > V100_PREFIX_LEN) ) {
                        store_offset = V100_PREFIX_LEN;
                    }

                    io_res = _file_write(fout, tmp_buffer.data()+ store_offset, data_part-store_offset);

                    if ( io_res != GEN_OK ) {
                        err("FW_UPDATE: Failed to write to file; (%s):(%d)", __FUNCTION__, __LINE__);
                        close (fout);
                        fout = -1;
                    }
                }

                rx_cnt += data_part;

                info ("FW_UPDATE: Read %7d from %7d; (%s):(%d)", rx_cnt, m_read_hdr.ulCount, __FUNCTION__, __LINE__);
            }

           
        }

        {   // Close file. 
            close(fout);
            if ( ret_val != VCOM_USB_ERR_OK ) {
                err("FW_UPDATE: Status: %d; Remove temporary file; (%s):(%d)", ret_val, __FUNCTION__, __LINE__);
                unlink (FW_UPDATE_NAME_TEMP);
            }
        }

        {   // Prepare V100 response.
            uint8_t update_status[V100_PREFIX_LEN] = {
                0x0d, 0x56,                   // magic const SOHV
                0x26, 0x01, 0x00, 0x00,       // v100_cmd_id   => 0126 => FW_UPDATE
                0x00, 0x00,                   // v100_err_code => (0 = ERR_NONE) | (191 = ERR_GENERAL) | (193 = ERR_FILEWRITE)
                0x00, 0x00, 0x00, 0x00        // payload_len   => 0 no payload
            };

            uint16_t* err_pos = (uint16_t*)&update_status[6]; // v100_err_code of update_status
            *err_pos = static_cast<uint16_t> (ret_val);

            // Load response.
            m_write_payload.clear();
            m_write_payload.assign(update_status, update_status + sizeof(update_status) );
        }

    } catch (...) {
        ret_val = VCOM_USB_ERR_EXEC;
    }

    return ret_val;
}

int  epinit::read_usbcb_pkt(int ep2) {

    int         retVal      = VCOM_USB_ERR_OK;
    uint32_t    rxCnt       = 0;
    uint32_t    dataPart    = 0;
    int         ret         = 0;

    if (m_read_hdr.ulCount == 0) {
        err ("Zero frame requested (%s):(%d)", __FUNCTION__, __LINE__);
        return VCOM_USB_ERR_EXEC;
    }

    m_read_payload.resize(m_read_hdr.ulCount);

    uint8_t* const dst = static_cast<uint8_t*> (m_read_payload.data() );
    
    while ( rxCnt < m_read_hdr.ulCount ) {

        dataPart = m_read_hdr.ulCount - rxCnt;
        if ( dataPart > VCOM_USB_MTU ) {
            dataPart = VCOM_USB_MTU;
        }

        if (m_terminating) {
            info("Alert: Terminate requested. (%s):(%d)", __FUNCTION__, __LINE__);
            retVal = VCOM_USB_ERR_SHUTDOWN;
            break;
        }

        ret = rx_request_enqueue(ep2, m_iocb_in_pkt, dst+rxCnt, dataPart);
        if ( ret != VCOM_USB_ERR_OK ) {
            err("Error: rx_request_enqueue failed (%s):(%d)", __FUNCTION__, __LINE__);
            retVal = VCOM_USB_ERR_IO;
            break;
        }

        ret = rx_request_wait ();
        if (ret != VCOM_USB_ERR_OK) {
            err("Error: rx_request_wait failed (%s):(%d)", __FUNCTION__, __LINE__);
            retVal = VCOM_USB_ERR_IO;
            break;
        }

        rxCnt += dataPart;
    }

    return retVal;
}

int  epinit::write_usbcb_pkt(int ep1, const void* const ptr, uint32_t tx_len) {

    int         retVal      = VCOM_USB_ERR_OK;
    int         ret         = 0;
    const int   largeTx     = 1024;

    if (tx_len == 0) {
        info ("Response length is ZERO (%s):(%d)", __FUNCTION__, __LINE__);
        return VCOM_USB_ERR_OK;
    }

    #if 1

        ret = tx_request_enqueue(ep1, iocb_out_pay, ptr, tx_len);
        if (ret != VCOM_USB_ERR_OK) {
            err("Error: Failed to enqueue USB write request (%s):(%d)", __FUNCTION__, __LINE__);
            retVal = VCOM_USB_ERR_IO;
        } else {

            if (tx_len > largeTx) {
                info("Start Transfer %d bytes (%s):(%d)", tx_len, __FUNCTION__, __LINE__);
            }

            ret = wait_and_validate(m_nEPRange, m_evfd_write, m_rfds, m_ctx, 15 * 1000);

            if (ret == 0) {
                info("Alert: TX Timeout (%s):(%d)", __FUNCTION__, __LINE__);
                retVal = VCOM_USB_ERR_TIMEOUT;
            } else
            if (ret < 0) {
                err("Error: Write Error (%s):(%d)", __FUNCTION__, __LINE__);
                retVal = VCOM_USB_ERR_IO;
            } else {
                if (tx_len > largeTx) {
                    info("End   Transfer %d bytes (%s):(%d)", tx_len, __FUNCTION__, __LINE__);
                }
                retVal = VCOM_USB_ERR_OK;
            }

        }

    # else

        const uint8_t* const src = static_cast<const uint8_t*> (ptr);
        int dataPart = 0;

        while (txCnt < tx_len) {

            dataPart = tx_len - txCnt;

            if (dataPart > VCOM_USB_MTU) {
                dataPart = VCOM_USB_MTU;
            }

            info ("Sending USB frame %d bytes (%s):(%d)", dataPart, __FUNCTION__, __LINE__ );

            ret = tx_request_enqueue(ep1, iocb_out_pay, src+txCnt, dataPart);
            if ( ret != VCOM_USB_ERR_OK) {
                err ("Error: Failed to enqueue USB write request (%s):(%d)", __FUNCTION__, __LINE__);
                retVal = VCOM_USB_ERR_IO;
                break;
            }

            ret = tx_request_wait();
            if (VCOM_USB_ERR_OK != ret) {
                err("Error: Failed to write the USBCB write request (%s):(%d)", __FUNCTION__, __LINE__);
                retVal = VCOM_USB_ERR_IO;
                break;
            }

            txCnt += dataPart;
        }

    #endif

    return retVal;
}

int  epinit::read_ep_2_thread(int ep0, int ep1, int ep2) {

    int  evfd_read   = 0;
    int  evfd_write  = 0;
    int  ioRes       = 0;
    bool fw_update   = false;

    info("thread %s started.", __FUNCTION__);

    UNUSED(ep0);

    CallbackPrototype callback_func = std::bind(&epinit::ResetReceiveBufferCallback, this);
    m_VCOMTransceiver.SetResetRxBufCallback(callback_func);

    memset( &m_ctx, 0, sizeof(m_ctx) );
    if (io_setup(2, &m_ctx) < 0) {
        err("Error: io_setup failed (%s):(%d)", __FUNCTION__, __LINE__);
        return -1;
    }

    m_evfd_write = eventfd(0, 0);
    m_evfd_read  = eventfd(0, 0);
    if (  (m_evfd_write<0)  ||  (m_evfd_read<0)  ) {
        err("Error: eventfd failed (%s):(%d)", __FUNCTION__, __LINE__);
        return 1;
    }

    if (m_evfd_read > m_evfd_write) {
        m_nEPRange = m_evfd_read  + 1;
    } else {
        m_nEPRange = m_evfd_write + 1;
    }

    for ( ; ; ) {

        ioRes = read_usbcb_hdr(ep2);
        if ( VCOM_USB_ERR_OK != ioRes ) {
            err("Error: Failed to start USB Transaction (%s):(%d)", __FUNCTION__, __LINE__);
            break;
        }

        if ( m_read_hdr.ulData == 0 ) {
            ioRes = handle_jengine_cmd  ( ep2 );
            fw_update = false;
        } else {
            fw_update = true;
            ioRes = handle_fwupdate_cmd ( ep2 );
        }

        vcom_hdr_config(&m_write_hdr, 0, m_write_payload.size(), 0);

        ioRes = write_usbcb_pkt(ep1, &m_write_hdr, sizeof(m_write_hdr));
        if ( VCOM_USB_ERR_OK != ioRes ) {
            err("Error: Failed to write USBCB. (%s):(%d)", __FUNCTION__, __LINE__);
            break;
        }

        ioRes = write_usbcb_pkt(ep1, m_write_payload.data(), m_write_payload.size());
        if ( VCOM_USB_ERR_OK != ioRes ) {
            err("Error: Failed to write USBCB. (%s):(%d)", __FUNCTION__, __LINE__);
            break;
        }

        if ( fw_update ) {
            info("FW_UPDATE: Update thread scheduled. (%s):(%d)", __FUNCTION__, __LINE__);
            std::thread(_update_thread, UPDATE_DELAY_SEC).detach();
        }

    }

    io_destroy(m_ctx);
    close(evfd_read);
    close(evfd_write);

    err("Transaction handler stopped. Exit. (%s):(%d)", __FUNCTION__, __LINE__);

    m_terminate_event.notify_one();

    return 0;
}
