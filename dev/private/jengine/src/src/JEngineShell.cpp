#include "global.h"

TDBG_DEFINE_AREA(jengshl);

#include <assert.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <JEngineShell.h>
#include <JEngineConfig.h>
#include <JEngineExec.h>
#include <logging.h>

#include <application/stuff.h>

#define  RX_TIMEOUT             (1000)
#define  TX_TIMEOUT             (1000)
#define  RX_BUFFER_MAX_LEN      (2048)

#define  LOG_ENABLED            (0)

/////////////////////////////////////////////////////////////////////////////
// Lock state of virtual storage.
//
static std::string FKL_LOCK_PROP_STRING_CP001 = "FKL_LOCK_STATUS_CP001";

/////////////////////////////////////////////////////////////////////////////
// Session key.
//
static std::string DSK_PROP_STRING_CP001 = "DSK_CRYPTOGRAM_CP001";

/////////////////////////////////////////////////////////////////////////////
//
bool                                      JEngineShell::m_running_;                          //
std::mutex                                JEngineShell::m_race_mutex;                        //
SocketServer                              JEngineShell::m_vcom_server_;                      //
SocketServer                              JEngineShell::m_shell_server_;                     //
SocketServer                              JEngineShell::m_shm_server_;                       //
std::vector<std::thread>                  JEngineShell::m_vcom_msg_threads_;                 //
std::vector<std::thread>                  JEngineShell::m_shell_msg_threads_;                //
std::vector<std::thread>                  JEngineShell::m_shm_msg_threads_;                  //
Atomic_Error                              JEngineShell::m_cmd_error_;                        //
Atomic_Add_User                           JEngineShell::m_cmd_add_user_;                     //
Atomic_Arm_Trigger                        JEngineShell::m_cmd_arm_trigger_;                  //
Atomic_Barcode_Get_Data                   JEngineShell::m_cmd_barcode_get_data_;             //
Atomic_Barcode_Get_Details                JEngineShell::m_cmd_barcode_get_details_;          //
Atomic_Cancel_Operation                   JEngineShell::m_cmd_cancel_operation_;             //
Atomic_Config_Comport                     JEngineShell::m_cmd_config_comport_;               //
Atomic_Delete_User                        JEngineShell::m_cmd_delete_user_;                  //
Atomic_Enc_Capture                        JEngineShell::m_cmd_enc_captute_;                  //
Atomic_Enc_Clear                          JEngineShell::m_cmd_enc_clear_;                    //
Atomic_Enc_Decrypt                        JEngineShell::m_cmd_enc_decrypt_;                  //
Atomic_Enc_Enroll                         JEngineShell::m_cmd_enc_enroll_;                   //
Atomic_Enc_Factory_Set_Key                JEngineShell::m_cmd_enc_factory_set_key_;          //
Atomic_Enc_Generate_SessionKey            JEngineShell::m_cmd_enc_generate_session_key_;     //
Atomic_Enc_Get_Key                        JEngineShell::m_cmd_enc_get_key_;                  //
Atomic_Enc_Get_KeyVersion                 JEngineShell::m_cmd_enc_get_get_keyversion_;       //
Atomic_Enc_Get_Rnd_Number                 JEngineShell::m_cmd_enc_get_rand_number_;          //
Atomic_Enc_Get_Serial_Number              JEngineShell::m_cmd_enc_get_serial_number_;        //
Atomic_Enc_Get_Spoof_Score                JEngineShell::m_cmd_enc_get_spoof_score_;          //
Atomic_Enc_ReturnCapturedBIR              JEngineShell::m_cmd_enc_return_capture_bir_;       //
Atomic_Enc_Set_ActiveKey                  JEngineShell::m_cmd_enc_set_active_key_;           //
Atomic_Enc_Set_Key                        JEngineShell::m_cmd_enc_set_key_;                  //
Atomic_Enc_Set_Parameters                 JEngineShell::m_cmd_enc_set_parameters_;           //
Atomic_Enc_VerifyMatch                    JEngineShell::m_cmd_enc_verify_match_;             //
Atomic_File_Delete                        JEngineShell::m_cmd_file_delete_;                  //
Atomic_Format_DB                          JEngineShell::m_cmd_format_db_;                    //
Atomic_Get_Acq_Status                     JEngineShell::m_cmd_get_acq_status_;               //
Atomic_Get_Cmd                            JEngineShell::m_cmd_get_get_cmd_;                  //
Atomic_Get_Composite_Image                JEngineShell::m_cmd_get_composite_image_;          //
Atomic_Get_Config                         JEngineShell::m_cmd_get_get_config_;               //
Atomic_Get_DB_Metrics                     JEngineShell::m_cmd_get_db_metrics_;               //
Atomic_Get_FIR_Image                      JEngineShell::m_cmd_get_fir_image_;                //
Atomic_Get_GPIO                           JEngineShell::m_cmd_get_gpio_;                     //
Atomic_Get_Image                          JEngineShell::m_cmd_get_image_;                    //
Atomic_Get_OP_Status                      JEngineShell::m_cmd_get_op_status_;                //
Atomic_Get_Serial                         JEngineShell::m_cmd_get_serial_;                   //
Atomic_Get_Status                         JEngineShell::m_cmd_get_status_;                   //
Atomic_Get_System_State                   JEngineShell::m_cmd_get_system_state_;             //
Atomic_Get_Tag                            JEngineShell::m_cmd_get_tag_;                      //
Atomic_Get_Template                       JEngineShell::m_cmd_get_template_;                 //
Atomic_Get_User                           JEngineShell::m_cmd_get_user_;                     //
Atomic_Get_Verification_Rules             JEngineShell::m_cmd_get_verification_rules_;       //
Atomic_ID_Delete_User_Record              JEngineShell::m_cmd_id_delete_user_record_;        //
Atomic_ID_Get_DB_Metrics                  JEngineShell::m_cmd_id_db_metics_;                 //
Atomic_ID_Get_Parameters                  JEngineShell::m_cmd_id_get_parameters_;            //
Atomic_ID_Get_Result                      JEngineShell::m_cmd_id_get_result_;                //
Atomic_ID_Get_System_Metrics              JEngineShell::m_cmd_id_system_metrics_;            //
Atomic_ID_Get_User_Record                 JEngineShell::m_cmd_id_get_user_record_;           //
Atomic_ID_Get_User_Record_Header          JEngineShell::m_cmd_id_user_record_header_;        //
Atomic_ID_Set_Parameters                  JEngineShell::m_cmd_id_set_parameters_;            //
Atomic_ID_Set_User_Record                 JEngineShell::m_cmd_id_set_user_record_;           //
Atomic_Set_Cmd                            JEngineShell::m_cmd_set_cmd_;                      //
Atomic_Set_Composite_Image                JEngineShell::m_cmd_set_composite_image_;          //
Atomic_Set_GPIO                           JEngineShell::m_cmd_set_gpio_;                     //
Atomic_Set_Image                          JEngineShell::m_cmd_set_image_;                    //
Atomic_Set_LED                            JEngineShell::m_cmd_set_led_;                      //
Atomic_Set_License_Key                    JEngineShell::m_cmd_set_license_key_;              //
Atomic_Set_Option                         JEngineShell::m_cmd_set_option_;                   //
Atomic_Set_Record                         JEngineShell::m_cmd_set_record_;                   //
Atomic_Set_Tag                            JEngineShell::m_cmd_set_tag_;                      //
Atomic_Set_Template                       JEngineShell::m_cmd_set_template_;                 //
Atomic_Set_Verification_Rules             JEngineShell::m_cmd_set_verificaton_rules_;        //
Atomic_Match_Ex                           JEngineShell::m_cmd_match_ex_;                     //
Atomic_Reset                              JEngineShell::m_cmd_reset_;                        //
Atomic_Spoof_Get_Template                 JEngineShell::m_cmd_spoof_get_template_;           //
Atomic_Write_Flash                        JEngineShell::m_cmd_write_flash_;                  //
Atomic_Enc_Unlock_Key                     JEngineShell::m_cmd_unlock_storage_;               //
Macro_Enroll_User                         JEngineShell::m_cmd_enroll_user_;                  //
Macro_ID_Create_DB                        JEngineShell::m_cmd_macro_id_create_db_;           //
Macro_ID_Delete_DB                        JEngineShell::m_cmd_macro_id_delete_db_;           //
Macro_ID_Enroll_User_Record               JEngineShell::m_cmd_macro_id_enroll_user_record_;  //
Macro_ID_Identify                         JEngineShell::m_cmd_macro_id_identify_;            //
Macro_ID_Verify_User_Record               JEngineShell::m_cmd_macro_id_user_record_;         //
Macro_Match                               JEngineShell::m_cmd_macro_match_;                  //
Macro_Save_Last_Capture                   JEngineShell::m_cmd_macro_save_last_capture_;      //
Macro_Update_Firmware                     JEngineShell::m_cmd_macro_update_firmware_;        //
Macro_Verify_User                         JEngineShell::m_cmd_macro_verify_user_;            //
Macro_Vid_Stream                          JEngineShell::m_cmd_macro_vid_stream_;             //
Macro_Enc_Generate_RSA_Keys               JEngineShell::m_cmd_macro_generate_rsa_key_;       //
Atomic_Hid_Init                           JEngineShell::m_cmd_hid_init;                      // 
Atomic_Hid_Enum_Cams                      JEngineShell::m_cmd_hid_enum_cams;                 // 
Atomic_Hid_Terminate                      JEngineShell::m_cmd_hid_terminate;                 // 
Atomic_Hid_Set_Param_Int                  JEngineShell::m_cmd_hid_set_params_int;            // 
Atomic_Hid_Get_Param_Int                  JEngineShell::m_cmd_hid_get_params_int;            // 
Atomic_Hid_Set_Param_Str                  JEngineShell::m_cmd_hid_set_params_str;            // 
Atomic_Hid_Get_Param_Str                  JEngineShell::m_cmd_hid_get_params_str;            // 
Atomic_Hid_Set_Param_Bin                  JEngineShell::m_cmd_hid_set_param_bin;             // 
Atomic_Hid_Get_Param_Bin                  JEngineShell::m_cmd_hid_get_param_bin;             // 
Atomic_Hid_Set_Param_Long                 JEngineShell::m_cmd_hid_set_param_long;            // 
Atomic_Hid_Get_Param_Long                 JEngineShell::m_cmd_hid_get_param_long;            // 
Atomic_Hid_Capture_Img                    JEngineShell::m_cmd_hid_capture_image;             // 
Atomic_Hid_Open_Context                   JEngineShell::m_cmd_hid_open_context;              // 
Atomic_Hid_Close_Context                  JEngineShell::m_cmd_hid_close_context;             // 
Atomic_Hid_Async_Stop_Operation           JEngineShell::m_cmd_hid_stop_operation;            // 
Atomic_Hid_Async_Extract_Template         JEngineShell::m_cmd_hid_extract_template;          // 
Atomic_Hid_Async_Match_With_Template      JEngineShell::m_cmd_hid_match_with_template;       // 
Atomic_Hid_Async_Match_With_Captured      JEngineShell::m_cmd_hid_match_with_captured;       // 
Atomic_Hid_Async_Identify_With_Template   JEngineShell::m_cmd_hid_identify_with_template;    // 
Atomic_Hid_Async_Identify_With_Captured   JEngineShell::m_cmd_hid_identify_with_captured;    // 
Atomic_Hid_Async_Verify_With_Template     JEngineShell::m_cmd_hid_verify_with_template;      // 
Atomic_Hid_Async_Verify_With_Captured     JEngineShell::m_cmd_hid_verify_with_captured;      // 
Atomic_Hid_Get_Intermediate_Res           JEngineShell::m_cmd_hid_get_int_results;           // 
Atomic_Hid_Get_Final_Res                  JEngineShell::m_cmd_hid_get_final_results;         // 
Atomic_Hid_Parse_Res_Int                  JEngineShell::m_cmd_hid_parse_res_int;             // 
Atomic_Hid_Parse_Res_Double               JEngineShell::m_cmd_hid_parse_res_dbl;             // 
Atomic_Hid_Parse_Res_Data                 JEngineShell::m_cmd_hid_parse_res_data;            // 
Atomic_Hid_Parse_Res_Point                JEngineShell::m_cmd_hid_parse_res_point;           // 
Atomic_Hid_Parse_Res_Image                JEngineShell::m_cmd_hid_parse_res_image;           // 
Atomic_Hid_Parse_Match_Gallery            JEngineShell::m_cmd_hid_parse_res_match_gallery;   // 
Atomic_Hid_Db_Add_Record_With_Captured    JEngineShell::m_cmd_hid_db_add_with_captured;      // 
Atomic_Hid_Db_Add_Record_With_Template    JEngineShell::m_cmd_hid_db_add_with_template;      // 
Atomic_Hid_Db_Get_Record                  JEngineShell::m_cmd_hid_db_get_record;             // 
Atomic_Hid_Db_List_Records                JEngineShell::m_cmd_hid_db_list_records;           // 
Atomic_Hid_Db_Del_Record                  JEngineShell::m_cmd_hid_db_del_record;             //
Atomic_Hid_Get_Video_Frame                JEngineShell::m_cmd_hid_get_video_frame;           //

JEngineShell::JEngineShell () {

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::JEngineShell()" );


        {   // Configuration.
            _V100_INTERFACE_CONFIGURATION_TYPE  devConfig;
            devConfig.Vendor_Id             = JENGINE_DEV_USB_VID;
            devConfig.Product_Id            = JENGINE_DEV_USB_PID;
            devConfig.Device_Cfg_Type       = JENGINE_DEV_DEVICE_CONFIG;
            devConfig.Device_Serial_Number  = JENGINE_DEV_SERIAL_NUMBER_SHORT;
            devConfig.Hardware_Rev          = JENGINE_DEV_HW_REVISION;
            devConfig.Firmware_Rev          = JENGINE_DEV_FW_REVISION;
            devConfig.Spoof_Rev             = JENGINE_DEV_SPOOF_REVISION;
            devConfig.Struct_Size           = JENGINE_DEV_STRUCT_SIZE;
            m_cmd_get_get_config_.SetConfiguration(&devConfig);
        }

        {   // Serial number.
            m_cmd_enc_get_serial_number_.SetSerialNum(JENGINE_DEV_SERIAL_NUMBER_LONG);
            m_cmd_get_serial_.SetSerialNumber(JENGINE_DEV_SERIAL_NUMBER_LONG);
        }

        m_running_ = true;
        m_busy_    = false;

    TTRACE_LEAVE("JEngineShell::JEngineShell() => (void)");
}

JEngineShell* JEngineShell::GetInstance() {

    static JEngineShell* instance = nullptr;

    JEngineShell* retVal = nullptr;

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::GetInstance()");
        {   const std::lock_guard<std::mutex> lock(m_race_mutex);
            if (instance == nullptr) {
                instance = new(JEngineShell);
            }
            assert(instance != nullptr);
            retVal = instance;
        }
    TTRACE_LEAVE("JEngineShell::GetInstance => (JEngineShell*) %p", retVal);

    return retVal;
}

void JEngineShell::StopServers () {

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::StopServers()");

        m_running_ = false;
        JEngineExec::Stop();
        m_vcom_server_.StopServer();
        m_shell_server_.StopServer();
        m_shm_server_.StopServer();

        for (auto& it : m_vcom_msg_threads_) {
            if (it.joinable()) {
                it.detach();
            }
        }

        for (auto& it : m_shell_msg_threads_) {
            if (it.joinable()) {
                it.detach();
            }
        }

        for (auto& it : m_shm_msg_threads_) {
            if (it.joinable()) {
                it.detach();
            }
        }

    TTRACE_LEAVE("JEngineShell::StopServers() => (void)");
}

bool JEngineShell::StartServers () {
    
    bool retVal = false;

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::StartServers()");

        bool init_res;
        char socket_name[128]{};
        std::function<MsgHandleFunctionPrototype> conn_vcom  = std::bind(&JEngineShell::NewConnVcom,  this, std::placeholders::_1);
        std::function<MsgHandleFunctionPrototype> conn_shell = std::bind(&JEngineShell::NewConnShell, this, std::placeholders::_1);
        std::function<MsgHandleFunctionPrototype> conn_shm   = std::bind(&JEngineShell::NewConnShm,   this, std::placeholders::_1);

        {   snprintf(socket_name, 128, "LumiVCOMSocket_%03d_%03d", 0, 0);
            init_res = m_vcom_server_.InitSocket(socket_name, true, true, false);
            if ( ! init_res ) {
                TTRACE(jengshl, TDBG_ERROR, "vcom_server failed to init" );
                StopServers();
                retVal = false;
                goto exit;
            }
        }

        {   snprintf(socket_name, 128, "SEShell_External_%03d_%03d", 0, 0);
            init_res = m_shell_server_.InitSocket(socket_name, true, true, false);
            if (!init_res) {
                TTRACE(jengshl, TDBG_ERROR, "shell_server failed to init");
                StopServers();
                retVal = false;
                goto exit;
            }
        }

        {   snprintf(socket_name, 128, "shm");
            init_res = m_shm_server_.InitSocket(socket_name, true, true, false);
            if (!init_res) {
                TTRACE(jengshl, TDBG_ERROR, "shm_server failed to init");
                StopServers();
                retVal = false;
                goto exit;
            }
        }

        if ( ! JEngineExec::Start() ) {
            TTRACE(jengshl, TDBG_ERROR, "JEngine failed to start");
            StopServers();
            retVal = false;
            goto exit;
        }

        InitHandlers();

        m_vcom_server_.BindConnectionHandleFunction(std::move(conn_vcom));
        m_shell_server_.BindConnectionHandleFunction(std::move(conn_shell));
        m_shm_server_.BindConnectionHandleFunction(std::move(conn_shm));

        m_vcom_server_.StartServer();
        m_shell_server_.StartServer();
        m_shm_server_.StartServer();

        retVal = true;

exit:;
    TTRACE_LEAVE("JEngineShell::StartServers() => (bool) %d", retVal);
    return retVal;
}

void JEngineShell::InitHandlers ( void ) {
}

void JEngineShell::LogFrame ( const USBCB& hdr, const uchar* const out_frame, int out_frame_len, const char* const msgPrefix1, const char* const msgPrefix2 ) {

    #if LOG_ENABLED

        #define  LOG_REDUCED            (1)

        std::string val;
        char        tmp[64];
        uint8_t*    hdp_ptr = (uint8_t*)&hdr;
        int         out_cnt;

        val  = msgPrefix2;
        val += " ";
        val += msgPrefix1;
        val += " [ ";

        for (unsigned int i = 0; i < sizeof(USBCB); i++) {
            sprintf(tmp, "0x%.2x", hdp_ptr[i]);
            if ( i != 0 ) {
                val += ", ";
            }
            val += tmp;
        }

        out_cnt = out_frame_len;

        #if LOG_REDUCED
            if (out_cnt > 128) {
                out_cnt = 128;
            }
        #endif

        val += "] [ ";

        for (int i = 0; i < out_cnt; i++) {
            sprintf(tmp, "0x%.2x", out_frame[i] );
            if ( i != 0 ) {
                val += ", ";
            }
            val += tmp;
        }

        if (out_cnt != out_frame_len) {
            val += " ... ";
        }

        val += " ]";

        info ( val.c_str() );

    #else

        UNUSED(hdr);
        UNUSED(out_frame);
        UNUSED(out_frame_len);
        UNUSED(msgPrefix1);
        UNUSED(msgPrefix2);

    #endif
}

void JEngineShell::LogRxFrame ( const USBCB& hdr, const uchar* const frame_data, int frame_data_len, const char* const msgPrefix ) {
    // Internal logging tool. Must be excluded from the standard logging.
    // Assume the LogFrame is an empty function.
    LogFrame(hdr, frame_data, frame_data_len, msgPrefix, "RX");
}

void JEngineShell::LogTxFrame ( const USBCB& hdr, const uchar* const frame_data, int frame_data_len, const char* const msgPrefix ) {
    // Internal logging tool. Must be excluded from the standard logging.
    // Assume the LogFrame is an empty function.
    LogFrame(hdr, frame_data, frame_data_len, msgPrefix, "TX");
}

bool JEngineShell::ProcessCommand (const uchar* const in_frame, const int in_frame_len, uchar** out_frame, int& out_frame_len) {

    bool retVal = false;

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::ProcessCommand()");

        _V100_COMMAND_SET           cmd;
        _V100_GENERAL_ERROR_CODES   errCode;
        bool                        ioRes;
        ICmd*                       pCmdHandler  = nullptr;
        uchar*                      pSendBuffer  = nullptr;
        uint                        bytesToSend  = 0;

        if ( in_frame_len < 6 ) {
            TTRACE(jengshl, TDBG_ERROR, "JEngineShell::ProcessCommand() Wrong length of frame." );
            retVal = false;
            goto exit;
        }

        memcpy ( &cmd, &in_frame[2], sizeof(cmd) );

        pCmdHandler = MapCmd(cmd);
        assert(pCmdHandler != nullptr);

        ioRes = pCmdHandler->UnpackChallenge(in_frame, in_frame_len);
        if ( false == ioRes ) {
            // Read input parameters.
            m_cmd_error_.SetErrorCode(GEN_INVALID_ARGUEMENT);
            pCmdHandler = reinterpret_cast<ICmd*>(&m_cmd_error_);
        }

        pCmdHandler->Exec();
        errCode = pCmdHandler->GetReturnCode();
        if ( GEN_OK != errCode ) {
            pCmdHandler = reinterpret_cast<ICmd*>(&m_cmd_error_);
            pCmdHandler->SetReturnCode(errCode);
        }

        ioRes = pCmdHandler->PackResponse ( &pSendBuffer, bytesToSend );
        if ( false == ioRes ) {
            pCmdHandler = reinterpret_cast<ICmd*>(&m_cmd_error_);
            pCmdHandler->SetReturnCode(GEN_ERROR_INTERNAL);
            pCmdHandler->PackResponse(&pSendBuffer, bytesToSend);
        }

        *out_frame     = pSendBuffer;
        out_frame_len  = bytesToSend;

        retVal = true;

exit:;
    TTRACE_LEAVE("JEngineShell::ProcessCommand() => (bool) %d", retVal);
    return retVal;
}

bool JEngineShell::HandleVcomCmd ( const uchar* const in_frame, const int in_frame_len, uchar** out_frame, int& out_frame_len ) {

    bool retVal = false;

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::HandleVcomCmd()");
        retVal = ProcessCommand ( in_frame, in_frame_len, out_frame, out_frame_len );
    TTRACE_LEAVE("JEngineShell::HandleVcomCmd() => (bool) %d", retVal);

    return retVal;
}

bool JEngineShell::HandleShellCmd ( const uchar* const in_frame, const int in_frame_len, uchar** out_frame, int& out_frame_len ) {

    bool retVal = false;
    
    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::HandleShellCmd()");
        retVal = ProcessCommand ( in_frame, in_frame_len, out_frame, out_frame_len );
    TTRACE_LEAVE("JEngineShell::HandleShellCmd() => (bool) %d", retVal);
    
    return retVal;
}

bool JEngineShell::HandleShmCmd ( const uchar* const in_frame, const int in_frame_len, uchar** out_frame, int& out_frame_len ) {

    bool retVal = false;

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::HandleShmCmd()");
        retVal = ProcessCommand ( in_frame, in_frame_len, out_frame, out_frame_len );
    TTRACE_LEAVE("JEngineShell::HandleShmCmd() => (bool) %d", retVal);

    return retVal;
}

bool JEngineShell::RxFrame ( SocketServer& srv, int socket, USBCB& hdr, data_buff_t& frame_data, const char* const msgPrefix ) {

    bool retVal = false;

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::RxFrame()");

        data_buff_t     raw_data;
        bool            io_res = false;
        PUSBCB          in_hdr_ptr;
        unsigned        read_len;

        raw_data.resize ( RX_BUFFER_MAX_LEN );
        read_len = sizeof(USBCB);
        io_res = srv.RecvFrame ( socket, (uint8_t*)&raw_data[0], read_len, RX_TIMEOUT );

        if ( ! io_res ) {
            // Connection closed by remote side.
            TTRACE (jengshl, TDBG_ERROR, "Cannot read USBCB for %s", msgPrefix);
            retVal = false;
            goto exit;
        }

        if ( read_len != sizeof(USBCB) ) {
            // Read error. Wrong size of USBCB header received.
            // No idea how to handle error. Let's close connection to prevent processing of invalid data.
            TTRACE(jengshl, TDBG_ERROR, "Wrong length of USBCB frame for %s", msgPrefix);
            retVal = false;
            goto exit;
        }

        in_hdr_ptr    = (PUSBCB) &raw_data[0];

        hdr.ulCommand = in_hdr_ptr->ulCommand;
        hdr.ulData    = in_hdr_ptr->ulData;
        hdr.ulCount   = in_hdr_ptr->ulCount;

        if ( hdr.ulCount > 0 ) {

            frame_data.resize(hdr.ulCount);

            read_len = hdr.ulCount;
            io_res = srv.RecvFrame(socket, &frame_data[0], read_len, RX_TIMEOUT);

            if ( ! io_res ) {
                // Look like connection closed by remote side.
                TTRACE(jengshl, TDBG_ERROR, "Cannot read payload for %s", msgPrefix);
                retVal = false;
                goto exit;
            }

            if (read_len != hdr.ulCount) {
                // Received length is not the same like expected by ulCount.
                // No idea how to handle error. Let's close connection to prevent processing of invalid data.
                TTRACE(jengshl, TDBG_ERROR, "Wrong size of payload for %s", msgPrefix);
                retVal = false;
                goto exit;
            }

        } else {
            frame_data.clear();
        }

        LogRxFrame ( hdr, &frame_data[0], frame_data.size(), msgPrefix );

        retVal = true;

exit:;
    TTRACE_LEAVE("JEngineShell::RxFrame() => (bool) %d", retVal);
    return retVal;
}

bool JEngineShell::TxFrame ( SocketServer& srv, int socket, int return_code, const uchar* const out_frame, int out_frame_len, const char* const pPrefix ) {

    bool retVal = false;

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::TxFrame()");

        USBCB   hdr;
        bool    io_res  = false;

        hdr.ulData  = (uint32_t)return_code;
        hdr.ulCount = out_frame_len;

        io_res = srv.SendFrame(socket, &hdr, sizeof(hdr), TX_TIMEOUT);
        if ( ! io_res ) {
            TTRACE(jengshl, TDBG_ERROR, "Failed to send USBCB for %s", pPrefix);
            retVal = false;
            goto exit;
        }

        if ( hdr.ulCount > 0 ) {
            io_res = srv.SendFrame(socket, out_frame, out_frame_len, TX_TIMEOUT);
            if ( ! io_res ) {
                TTRACE(jengshl, TDBG_ERROR, "Failed to send payload for %s", pPrefix);
                retVal = false;
                goto exit;
            }
        }

        LogTxFrame ( hdr, out_frame, out_frame_len, pPrefix );

        retVal = true;

exit:;
    TTRACE_LEAVE("JEngineShell::TxFrame() => (bool) %d", retVal);
    return retVal;
}

void JEngineShell::RxHandler ( SocketServer& srv, int socket, cmd_handler_t handler, const char* const msgPrefix ) {

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::RxHandler()");

        data_buff_t     raw_in_frame;
        uchar*          raw_out_frame;
        int             raw_out_frame_len;
        int             return_code         = 0;
        USBCB           usbcb_in_header;
        bool            io_res              = false;

        assert (msgPrefix != nullptr);

        raw_in_frame.resize(RX_BUFFER_MAX_LEN);

        while ( m_running_ ) {

            try {

                io_res = RxFrame ( srv, socket, usbcb_in_header, raw_in_frame, msgPrefix );
                if ( ! io_res ) {
                    err( "RxFrame failed for %s", msgPrefix );
                    break;
                }

                io_res = handler( &raw_in_frame[0], raw_in_frame.size(), &raw_out_frame, raw_out_frame_len );

                if ( !io_res ) {
                    err("HandleCommand failed for %s", msgPrefix);
                    break;
                }

                io_res = TxFrame(srv, socket, return_code, raw_out_frame, raw_out_frame_len, msgPrefix);
                if (!io_res) {
                    err("TxFrame failed for %s", msgPrefix);
                    break;
                }

            } catch (const std::exception& /*e*/) {
                err( "rx_manager: Exception for %s", msgPrefix );
            }

        }

    TTRACE_LEAVE("JEngineShell::RxHandler() => (void)" );
}

void JEngineShell::NewConnVcom ( int conn_file_descriptor ) {

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::NewConnVcom()");
        std::thread thrVcom( [this, conn_file_descriptor]
            { RxHandler (m_vcom_server_, conn_file_descriptor, HandleVcomCmd, "VCOM"); }
        );
        m_vcom_msg_threads_.emplace_back(std::move(thrVcom));
    TTRACE_LEAVE("JEngineShell::NewConnVcom() => (void)");
}

void JEngineShell::NewConnShell ( int conn_file_descriptor ) {

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::NewConnShell()");
        std::thread thrShell( [this, conn_file_descriptor]
            { RxHandler(m_shell_server_, conn_file_descriptor, HandleShellCmd, "SHELL"); }
        );
        m_shell_msg_threads_.emplace_back(std::move(thrShell));
    TTRACE_LEAVE("JEngineShell::NewConnShell() => (void)");
}

void JEngineShell::NewConnShm ( int conn_file_descriptor ) {

    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::NewConnShm()");
        std::thread thrShm([this, conn_file_descriptor]
            { RxHandler(m_shm_server_, conn_file_descriptor, HandleShmCmd, "SHM"); }
        );
        m_shm_msg_threads_.emplace_back(std::move(thrShm));
    TTRACE_LEAVE("JEngineShell::NewConnShm() => (void)");
}

ICmd* JEngineShell::MapCmd ( _V100_COMMAND_SET cmd ) {

    ICmd* retVal = nullptr;
    TTRACE_ENTER(jengshl, TDBG_DEBUG, "JEngineShell::MapCmd()");

        switch (cmd) {

            case CMD_MATCH:
                retVal = static_cast<ICmd*>(& m_cmd_macro_match_);
                break;

            case CMD_VID_STREAM:
                retVal = static_cast<ICmd*>(& m_cmd_macro_vid_stream_);
                break;

            case CMD_GET_IMAGE:
                retVal = static_cast<ICmd*>(& m_cmd_get_image_);
                break;

            case CMD_SET_IMAGE:
                retVal = static_cast<ICmd*>(& m_cmd_set_image_);
                break;

            case CMD_GET_COMPOSITE_IMAGE:
                retVal = static_cast<ICmd*>(& m_cmd_get_composite_image_);
                break;

            case CMD_SET_COMPOSITE_IMAGE:
                retVal = static_cast<ICmd*>(&m_cmd_set_composite_image_);
                break;

            case CMD_GET_TEMPLATE:
                retVal = static_cast<ICmd*>(&m_cmd_get_template_);
                break;

            case CMD_SET_TEMPLATE:
                retVal = static_cast<ICmd*>(&m_cmd_set_template_);
                break;

            case CMD_ARM_TRIGGER:
                retVal = static_cast<ICmd*>(&m_cmd_arm_trigger_);
                break;

            case CMD_GET_ACQ_STATUS:
                retVal = static_cast<ICmd*>(&m_cmd_get_acq_status_);
                break;

            case CMD_GET_CONFIG:
                retVal = static_cast<ICmd*>(&m_cmd_get_get_config_);
                break;

            case CMD_GET_STATUS:
                retVal = static_cast<ICmd*>(&m_cmd_get_status_);
                break;

            case CMD_GET_CMD:
                retVal = static_cast<ICmd*>(&m_cmd_get_get_cmd_);
                break;

            case CMD_SET_CMD:
                retVal = static_cast<ICmd*>(&m_cmd_set_cmd_);
                break;

            case CMD_GET_SERIAL_NUMBER:
                retVal = static_cast<ICmd*>(&m_cmd_get_serial_);
                break;

            case CMD_SET_LED:
                retVal = static_cast<ICmd*>(&m_cmd_set_led_);
                break;

            case CMD_CONFIG_COMPORT:
                retVal = static_cast<ICmd*>(&m_cmd_config_comport_);
                break;

            case CMD_SET_OPTION:
                retVal = static_cast<ICmd*>(&m_cmd_set_option_);
                break;

            case CMD_RESET:
                retVal = static_cast<ICmd*>(&m_cmd_reset_);
                break;

            case CMD_MATCH_EX:
                retVal = static_cast<ICmd*>(&m_cmd_match_ex_);
                break;

            case CMD_SPOOF_GET_TEMPLATE:
                retVal = static_cast<ICmd*>(&m_cmd_spoof_get_template_);
                break;

            case CMD_SET_GPIO:
                retVal = static_cast<ICmd*>(&m_cmd_set_gpio_);
                break;

            case CMD_GET_GPIO:
                retVal = static_cast<ICmd*>(&m_cmd_get_gpio_);
                break;

            case CMD_CANCEL_OPERATION:
                retVal = static_cast<ICmd*>(&m_cmd_cancel_operation_);
                break;

            case CMD_GET_FIR_IMAGE:
                retVal = static_cast<ICmd*>(&m_cmd_get_fir_image_);
                break;

            case CMD_SET_VERIFICATION_RULES:
                retVal = static_cast<ICmd*>(&m_cmd_set_verificaton_rules_);
                break;

            case CMD_GET_VERIFICATION_RULES:
                retVal = static_cast<ICmd*>(&m_cmd_get_verification_rules_);
                break;

            case CMD_ENROLL_USER:
                retVal = static_cast<ICmd*>(&m_cmd_enroll_user_);
                break;

            case CMD_VERIFY_USER:
                retVal = static_cast<ICmd*>(&m_cmd_macro_verify_user_);
                break;

            case CMD_DELETE_USER:
                retVal = static_cast<ICmd*>(&m_cmd_delete_user_);
                break;

            case CMD_GET_DB_METRICS:
                retVal = static_cast<ICmd*>(&m_cmd_get_db_metrics_);
                break;

            case CMD_FORMAT_DB:
                retVal = static_cast<ICmd*>(&m_cmd_format_db_);
                break;

            case CMD_GET_USER:
                retVal = static_cast<ICmd*>(&m_cmd_get_user_);
                break;

            case CMD_ADD_USER:
                retVal = static_cast<ICmd*>(&m_cmd_add_user_);
                break;

            case CMD_GET_OP_STATUS:
                retVal = static_cast<ICmd*>(&m_cmd_get_op_status_);
                break;

            case CMD_SET_TAG:
                retVal = static_cast<ICmd*>(&m_cmd_set_tag_);
                break;

            case CMD_GET_TAG:
                retVal = static_cast<ICmd*>(&m_cmd_get_tag_);
                break;

            case CMD_GET_SYSTEM_STATE:
                retVal = static_cast<ICmd*>(&m_cmd_get_system_state_);
                break;

            case CMD_FILE_DELETE:
                retVal = static_cast<ICmd*>(&m_cmd_file_delete_);
                break;

            case CMD_ID_CREATE_DB:
                retVal = static_cast<ICmd*>(&m_cmd_macro_id_create_db_);
                break;

            case CMD_ID_VERIFY_USER_RECORD:
                retVal = static_cast<ICmd*>(&m_cmd_macro_id_user_record_);
                break;

            case CMD_ID_GET_USER_RECORD:
                retVal = static_cast<ICmd*>(&m_cmd_id_get_user_record_);
                break;

            case CMD_ID_GET_USER_RECORD_HEADER:
                retVal = static_cast<ICmd*>(&m_cmd_id_user_record_header_);
                break;

            case CMD_ID_SET_USER_RECORD:
                retVal = static_cast<ICmd*>(&m_cmd_id_set_user_record_);
                break;

            case CMD_ID_ENROLL_USER_RECORD:
                retVal = static_cast<ICmd*>(&m_cmd_macro_id_enroll_user_record_);
                break;

            case CMD_ID_DELETE_DB:
                retVal = static_cast<ICmd*>(&m_cmd_macro_id_delete_db_);
                break;

            case CMD_ID_IDENTIFY:
                retVal = static_cast<ICmd*>(&m_cmd_macro_id_identify_);
                break;

            case CMD_ID_GET_DB_METRICS:
                retVal = static_cast<ICmd*>(&m_cmd_id_db_metics_);
                break;

            case CMD_ID_GET_RESULT:
                retVal = static_cast<ICmd*>(&m_cmd_id_get_result_);
                break;

            case CMD_ID_DELETE_USER_RECORD:
                retVal = static_cast<ICmd*>(&m_cmd_id_delete_user_record_);
                break;

            case CMD_ID_GET_SYSTEM_METRICS:
                retVal = static_cast<ICmd*>(&m_cmd_id_system_metrics_);
                break;

            case CMD_ID_GET_PARAMETERS:
                retVal = static_cast<ICmd*>(&m_cmd_id_get_parameters_);
                break;

            case CMD_ID_SET_PARAMETERS:
                retVal = static_cast<ICmd*>(&m_cmd_id_set_parameters_);
                break;

            case CMD_ID_GET_SPOOF_SCORE:
                retVal = static_cast<ICmd*>(&m_cmd_enc_get_spoof_score_);
                break;

            case CMD_SAVE_LAST_CAPTURE:
                retVal = static_cast<ICmd*>(&m_cmd_macro_save_last_capture_);
                break;

            case CMD_UPDATE_FIRMWARE:
                retVal = static_cast<ICmd*>(&m_cmd_macro_update_firmware_);
                break;

            case CMD_SET_LICENSE_KEY:
                retVal = static_cast<ICmd*>(&m_cmd_set_license_key_);
                break;

            case CMD_SET_RECORD:
                retVal = static_cast<ICmd*>(&m_cmd_set_record_);
                break;

            case CMD_WRITE_FLASH:
                retVal = static_cast<ICmd*>(&m_cmd_write_flash_);
                break;

            case CMD_BARCODE_GET_TEXT:
                retVal = static_cast<ICmd*>(&m_cmd_barcode_get_data_);
                break;

            case CMD_BARCODE_GET_DETAILS:
                retVal = static_cast<ICmd*>(&m_cmd_barcode_get_details_);
                break;

            case CMD_ENC_SET_ACTIVE_KEY:
                retVal = static_cast<ICmd*>(&m_cmd_enc_set_active_key_);
                break;

            case CMD_ENC_GET_SERIAL_NUMBER:
                retVal = static_cast<ICmd*>(&m_cmd_enc_get_serial_number_);
                break;

            case CMD_ENC_GET_RND_NUMBER:
                retVal = static_cast<ICmd*>(&m_cmd_enc_get_rand_number_);
                break;

            case CMD_ENC_DECRYPT:
                retVal = static_cast<ICmd*>(&m_cmd_enc_decrypt_);
                break;

            case CMD_ENC_SET_PARAMETERS:
                retVal = static_cast<ICmd*>(&m_cmd_enc_set_parameters_);
                break;

            case CMD_ENC_VERIFYMATCH:
                retVal = static_cast<ICmd*>(&m_cmd_enc_verify_match_);
                break;

            case CMD_ENC_ENROLL:
                retVal = static_cast<ICmd*>(&m_cmd_enc_enroll_);
                break;

            case CMD_ENC_CAPTURE:
                retVal = static_cast<ICmd*>(&m_cmd_enc_captute_);
                break;

            case CMD_ENC_RETURNCAPTUREDBIR:
                retVal = static_cast<ICmd*>(&m_cmd_enc_return_capture_bir_);
                break;

            case CMD_ENC_FACTORY_SET_KEY:
                retVal = static_cast<ICmd*>(&m_cmd_enc_factory_set_key_);
                break;

            case CMD_ENC_GENERATE_SESSIONKEY:
                retVal = static_cast<ICmd*>(&m_cmd_enc_generate_session_key_);
                break;

            case CMD_ENC_GET_KEY:
                retVal = static_cast<ICmd*>(&m_cmd_enc_get_key_);
                break;

            case CMD_ENC_GET_KEYVERSION:
                retVal = static_cast<ICmd*>(&m_cmd_enc_get_get_keyversion_);
                break;

            case CMD_ENC_SET_KEY:
                retVal = static_cast<ICmd*>(&m_cmd_enc_set_key_);
                break;

            case CMD_ENC_CLEAR:
                retVal = static_cast<ICmd*>(&m_cmd_enc_clear_);
                break;

            case CMD_ENC_GENERATE_RSA_KEYS:
                retVal = static_cast<ICmd*>(&m_cmd_macro_generate_rsa_key_);
                break;

            case CMD_ENC_UNLOCK_KEY:
                retVal = static_cast<ICmd*>(&m_cmd_unlock_storage_);
                break;

            case CMD_HID_INIT:
                retVal = static_cast<ICmd*>(&m_cmd_hid_init);
                break;

            case CMD_HID_ENUM_CAMS:
                retVal = static_cast<ICmd*>(&m_cmd_hid_enum_cams);
                break;

            case CMD_HID_TERMINATE:
                retVal = static_cast<ICmd*>(&m_cmd_hid_terminate);
                break;

            case CMD_HID_SET_PARAM_INT:
                retVal = static_cast<ICmd*>(&m_cmd_hid_set_params_int);
                break;

            case CMD_HID_GET_PARAM_INT:
                retVal = static_cast<ICmd*>(&m_cmd_hid_get_params_int);
                break;

            case CMD_HID_SET_PARAM_STR:
                retVal = static_cast<ICmd*>(&m_cmd_hid_set_params_str);
                break;

            case CMD_HID_GET_PARAM_STR:
                retVal = static_cast<ICmd*>(&m_cmd_hid_get_params_str);
                break;

            case CMD_HID_SET_PARAM_BIN:
                retVal = static_cast<ICmd*>(&m_cmd_hid_set_param_bin);
                break;

            case CMD_HID_GET_PARAM_BIN:
                retVal = static_cast<ICmd*>(&m_cmd_hid_get_param_bin);
                break;

            case CMD_HID_SET_PARAM_LONG:
                retVal = static_cast<ICmd*>(&m_cmd_hid_set_param_long);
                break;

            case CMD_HID_GET_PARAM_LONG:
                retVal = static_cast<ICmd*>(&m_cmd_hid_get_param_long);
                break;

            case CMD_HID_CAPTURE_IMAGE:
                retVal = static_cast<ICmd*>(&m_cmd_hid_capture_image);
                break;

            case CMD_HID_OPEN_CONTEXT:
                retVal = static_cast<ICmd*>(&m_cmd_hid_open_context);
                break;

            case CMD_HID_CLOSE_CONTEXT:
                retVal = static_cast<ICmd*>(&m_cmd_hid_close_context);
                break;

            case CMD_HID_STOP_CMD_ASYNC:
                retVal = static_cast<ICmd*>(&m_cmd_hid_stop_operation);
                break;

            case CMD_HID_ASYNC_EXTRACT_TEMPLATE:
                retVal = static_cast<ICmd*>(&m_cmd_hid_extract_template);
                break;

            case CMD_HID_ASYNC_MATCH_WITH_TEMPLATE:
                retVal = static_cast<ICmd*>(&m_cmd_hid_match_with_template);
                break;

            case CMD_HID_ASYNC_MATCH_WITH_CAPTURED:
                retVal = static_cast<ICmd*>(&m_cmd_hid_match_with_captured);
                break;

            case CMD_HID_ASYNC_IDENTIFY_WITH_TEMPLATE:
                retVal = static_cast<ICmd*>(&m_cmd_hid_identify_with_template);
                break;

            case CMD_HID_ASYNC_IDENTIFY_WITH_CAPTURED:
                retVal = static_cast<ICmd*>(&m_cmd_hid_identify_with_captured);
                break;

            case CMD_HID_ASYNC_VERIFY_WITH_TEMPLATE:
                retVal = static_cast<ICmd*>(&m_cmd_hid_verify_with_template);
                break;

            case CMD_HID_ASYNC_VERIFY_WITH_CAPTURED:
                retVal = static_cast<ICmd*>(&m_cmd_hid_verify_with_captured);
                break;

            case CMD_HID_GET_INTERMEDIATE_RES:
                retVal = static_cast<ICmd*>(&m_cmd_hid_get_int_results);
                break;

            case CMD_HID_GET_FINAL_RES:
                retVal = static_cast<ICmd*>(&m_cmd_hid_get_final_results);
                break;

            case CMD_HID_PARSE_RES_INT:
                retVal = static_cast<ICmd*>(&m_cmd_hid_parse_res_int);
                break;

            case CMD_HID_PARSE_RES_DOUBLE:
                retVal = static_cast<ICmd*>(&m_cmd_hid_parse_res_dbl);
                break;

            case CMD_HID_PARSE_RES_DATA:
                retVal = static_cast<ICmd*>(&m_cmd_hid_parse_res_data);
                break;

            case CMD_HID_PARSE_RES_POINT:
                retVal = static_cast<ICmd*>(&m_cmd_hid_parse_res_point);
                break;

            case CMD_HID_PARSE_RES_IMAGE:
                retVal = static_cast<ICmd*>(&m_cmd_hid_parse_res_image);            
                break;

            case CMD_HID_PARSE_MATCH_GALLERY:
                retVal = static_cast<ICmd*>(&m_cmd_hid_parse_res_match_gallery);            
                break;

            case CMD_HID_DB_ADD_RECORD_WITH_CAPTURED:
                retVal = static_cast<ICmd*>(&m_cmd_hid_db_add_with_captured);
                break;

            case CMD_HID_DB_ADD_RECORD_WITH_TEMPLATE:
                retVal = static_cast<ICmd*>(&m_cmd_hid_db_add_with_template);
                break;

            case CMD_HID_DB_GET_RECORD:
                retVal = static_cast<ICmd*>(&m_cmd_hid_db_get_record);
                break;

            case CMD_HID_DB_LIST_RECORDS:
                retVal = static_cast<ICmd*>(&m_cmd_hid_db_list_records);
                break;

            case CMD_HID_DB_DEL_RECORD:
                retVal = static_cast<ICmd*>(&m_cmd_hid_db_del_record);
                break;

            case CMD_HID_GET_VIDEO_FRAME:
                retVal = static_cast<ICmd*>(&m_cmd_hid_get_video_frame);
                break;

            case CMD_ENC_SET_ACTIVEKEY:
            case CMD_ENC_GET_SPOOF_SCORE:
            case CMD_ENC_RETURNCAPTUREDWSQ:
            case CMD_ID_SET_API_KEY:
            case CMD_LOG:
            case CMD_GET_VERSION:
            case CMD_TRUNCATE_378:
            case CMD_ID_SET_WORKING_DB:
            case CMD_ID_IDENTIFY_378:
            case CMD_ID_VERIFY_378:
            case CMD_VERIFY_378:
            case CMD_ID_GET_TEMPLATE:
            case CMD_ID_GET_IMAGE:
            case CMD_ID_COMMIT:
            case CMD_ID_PURGE_DB_ALL:
            case CMD_ID_VERIFY_MANY:
            case CMD_ENC_CHALLENGE:
            case CMD_ENC_GET_IMAGE:
            case CMD_ENC_GET_NONCE:
            case CMD_ENC_GET_TEMPLATE:
            case CMD_ENC_LOCK_KEY:
            case CMD_ENC_VERIFY:
            case CMD_ENC_GET_DIAG_STATUS:
            case CMD_ENC_GET_PARAMETERS:
            case CMD_ENC_MATCH:
            case CMD_ENC_RETURNCAPTUREDIMAGE:
            case CMD_ENC_RETURNCAPTUREDBIR_IM:
            case CMD_ENC_GET_CAPTURE_STATS:
            case CMD_ENC_VERIFYMATCH_RESULT:
            case CMD_ENC_VERIFYMATCH_MANY:
            case CMD_ENC_GET_SENSOR_INFO:
            case CMD_ENC_RETURNCAPTUREDTEMPLATE:
            case CMD_ENC_GET_KCV:
            case CMD_ENC_SET_SESSION_KEY:
            case CMD_ENC_IDENTIFYMATCH:
            case CMD_ERROR:
            case CMD_NONE:
            case CMD_LAST:
            default:
                err( "JEngineShell::MapCmd() Unrecognized id: %d mapped to cmd_error", cmd );
                m_cmd_error_.SetErrorCode(GEN_NOT_SUPPORTED);
                retVal = static_cast<ICmd*>(&m_cmd_error_);
                break;
        }

    TTRACE_LEAVE("JEngineShell::MapCmd() => (ICmd*) %p", retVal);
    return retVal;
}
