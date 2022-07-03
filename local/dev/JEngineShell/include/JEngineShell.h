#ifndef __JENGINESHELL_H__
#define __JENGINESHELL_H__

#include <stdint.h>
#include <atomic>
#include <thread>
#include <tuple>
#include <mutex>
#include <vector>

#include <CryptoMgrCP001.h>
#include <SocketServer.h>

#include <V100Cmd.h>
#include <V100EncCmd.h>
#include <V100IDCmd.h>
#include <V100InternalCmd.h>

enum class CryptoRes {
    CM_ERROR_NONE               = 0,
    CM_ERROR_KEY_NOT_EXISTS     = 1,
    CM_ERROR_NOT_SUPPORTED      = 2,
    CM_ERROR_BAD_PARAM          = 3,
    CM_ERROR_BUSY               = 4,
    CM_ERROR_GENERAL            = 5
};

typedef std::vector<uint8_t> data_buff_t;

typedef bool (*cmd_handler_t) (const uchar* const in_frame, const int in_frame_len, uchar** out_frame, int& out_frame_len);

class JEngineShell {

    private:
        JEngineShell();
       ~JEngineShell();

    public:
        bool StartServers();
        void StopServers();

        static JEngineShell* GetInstance();

    private:
        void NewConnVcom    ( int conn_file_descriptor );
        void NewConnShell   ( int conn_file_descriptor );
        void NewConnShm     ( int conn_file_descriptor );
        bool RxFrame        ( SocketServer& srv, int socket, USBCB& hdr, data_buff_t& frame_data, const char* const msgPrefix );
        bool TxFrame        ( SocketServer& srv, int socket, int return_code, const uchar* const out_frame, int out_frame_len, const char* const pPrefix );
        void RxHandler      ( SocketServer& srv, int socket, cmd_handler_t handler, const char* const msgPrefix );
        void InitHandlers   ( void );
        void LogFrame       ( const USBCB& hdr, const uchar* const frame_data, int frame_data_len, const char* const msgPrefix1, const char* const msgPrefix2 );
        void LogRxFrame     ( const USBCB& hdr, const uchar* const frame_data, int frame_data_len, const char* const msgPrefix );
        void LogTxFrame     ( const USBCB& hdr, const uchar* const frame_data, int frame_data_len, const char* const msgPrefix );
        ICmd* GetCommand    ( _V100_COMMAND_SET cmd );

    private:
        CryptoRes InvalidateDSK();
        CryptoRes InvalidateBSK();

    private:
        static bool     ProcessCommand ( const uchar* const in_frame, const int in_frame_len, uchar** out_frame, int& out_frame_len );
        static bool     HandleVcomCmd  ( const uchar* const in_frame, const int in_frame_len, uchar** out_frame, int& out_frame_len );
        static bool     HandleShellCmd ( const uchar* const in_frame, const int in_frame_len, uchar** out_frame, int& out_frame_len );
        static bool     HandleShmCmd   ( const uchar* const in_frame, const int in_frame_len, uchar** out_frame, int& out_frame_len );
        static ICmd*    MapCmd         ( const _V100_COMMAND_SET cmd );

    private:
        bool                                            m_busy_;                             // std::atomic_flag lock = ATOMIC_FLAG_INIT;
        u256                                            m_ANBIO;                             //
        u16                                             m_ActiveSlot;                        //
        _V100_ENC_KEY_TYPE                              m_nActiveKeyType;                    //

    private:
        static Atomic_Error                             m_cmd_error_;                        //
        static Atomic_Add_User                          m_cmd_add_user_;                     //
        static Atomic_Arm_Trigger                       m_cmd_arm_trigger_;                  // ->
        static Atomic_Barcode_Get_Data                  m_cmd_barcode_get_data_;             //
        static Atomic_Barcode_Get_Details               m_cmd_barcode_get_details_;          //
        static Atomic_Cancel_Operation                  m_cmd_cancel_operation_;             //
        static Atomic_Config_Comport                    m_cmd_config_comport_;               //
        static Atomic_Delete_User                       m_cmd_delete_user_;                  // -> OK
        static Atomic_Enc_Capture                       m_cmd_enc_captute_;                  //
        static Atomic_Enc_Clear                         m_cmd_enc_clear_;                    //
        static Atomic_Enc_Decrypt                       m_cmd_enc_decrypt_;                  //
        static Atomic_Enc_Enroll                        m_cmd_enc_enroll_;                   //
        static Atomic_Enc_Factory_Set_Key               m_cmd_enc_factory_set_key_;          //
        static Atomic_Enc_Generate_SessionKey           m_cmd_enc_generate_session_key_;     //
        static Atomic_Enc_Get_Key                       m_cmd_enc_get_key_;                  //
        static Atomic_Enc_Get_KeyVersion                m_cmd_enc_get_get_keyversion_;       //
        static Atomic_Enc_Get_Rnd_Number                m_cmd_enc_get_rand_number_;          //
        static Atomic_Enc_Get_Serial_Number             m_cmd_enc_get_serial_number_;        //
        static Atomic_Enc_Get_Spoof_Score               m_cmd_enc_get_spoof_score_;          //
        static Atomic_Enc_ReturnCapturedBIR             m_cmd_enc_return_capture_bir_;       //
        static Atomic_Enc_Set_ActiveKey                 m_cmd_enc_set_active_key_;           //
        static Atomic_Enc_Set_Key                       m_cmd_enc_set_key_;                  //
        static Atomic_Enc_Set_Parameters                m_cmd_enc_set_parameters_;           //
        static Atomic_Enc_VerifyMatch                   m_cmd_enc_verify_match_;             //
        static Atomic_Enc_Unlock_Key                    m_cmd_unlock_storage_;               //
        static Atomic_File_Delete                       m_cmd_file_delete_;                  //
        static Atomic_Format_DB                         m_cmd_format_db_;                    //
        static Atomic_Get_Acq_Status                    m_cmd_get_acq_status_;               //
        static Atomic_Get_Cmd                           m_cmd_get_get_cmd_;                  //
        static Atomic_Get_Composite_Image               m_cmd_get_composite_image_;          //
        static Atomic_Get_Config                        m_cmd_get_get_config_;               //
        static Atomic_Get_DB_Metrics                    m_cmd_get_db_metrics_;               //
        static Atomic_Get_FIR_Image                     m_cmd_get_fir_image_;                //
        static Atomic_Get_GPIO                          m_cmd_get_gpio_;                     //
        static Atomic_Get_Image                         m_cmd_get_image_;                    //
        static Atomic_Get_OP_Status                     m_cmd_get_op_status_;                //
        static Atomic_Get_Serial                        m_cmd_get_serial_;                   //
        static Atomic_Get_Status                        m_cmd_get_status_;                   //
        static Atomic_Get_System_State                  m_cmd_get_system_state_;             //
        static Atomic_Get_Tag                           m_cmd_get_tag_;                      //
        static Atomic_Get_Template                      m_cmd_get_template_;                 //
        static Atomic_Get_User                          m_cmd_get_user_;                     //
        static Atomic_Get_Verification_Rules            m_cmd_get_verification_rules_;       //
        static Atomic_ID_Delete_User_Record             m_cmd_id_delete_user_record_;        //
        static Atomic_ID_Get_DB_Metrics                 m_cmd_id_db_metics_;                 //
        static Atomic_ID_Get_Parameters                 m_cmd_id_get_parameters_;            //
        static Atomic_ID_Get_Result                     m_cmd_id_get_result_;                //
        static Atomic_ID_Get_System_Metrics             m_cmd_id_system_metrics_;            //
        static Atomic_ID_Get_User_Record                m_cmd_id_get_user_record_;           //
        static Atomic_ID_Get_User_Record_Header         m_cmd_id_user_record_header_;        //
        static Atomic_ID_Set_Parameters                 m_cmd_id_set_parameters_;            //
        static Atomic_ID_Set_User_Record                m_cmd_id_set_user_record_;           //
        static Atomic_Set_Cmd                           m_cmd_set_cmd_;                      //
        static Atomic_Set_Composite_Image               m_cmd_set_composite_image_;          //
        static Atomic_Set_GPIO                          m_cmd_set_gpio_;                     //
        static Atomic_Set_Image                         m_cmd_set_image_;                    //
        static Atomic_Set_LED                           m_cmd_set_led_;                      //
        static Atomic_Set_License_Key                   m_cmd_set_license_key_;              //
        static Atomic_Set_Option                        m_cmd_set_option_;                   //
        static Atomic_Set_Record                        m_cmd_set_record_;                   //
        static Atomic_Set_Tag                           m_cmd_set_tag_;                      //
        static Atomic_Set_Template                      m_cmd_set_template_;                 //
        static Atomic_Set_Verification_Rules            m_cmd_set_verificaton_rules_;        //
        static Atomic_Match_Ex                          m_cmd_match_ex_;                     //
        static Atomic_Reset                             m_cmd_reset_;                        //
        static Atomic_Spoof_Get_Template                m_cmd_spoof_get_template_;           //
        static Atomic_Write_Flash                       m_cmd_write_flash_;                  //
        static Macro_Enroll_User                        m_cmd_enroll_user_;                  //
        static Macro_ID_Create_DB                       m_cmd_macro_id_create_db_;           //
        static Macro_ID_Delete_DB                       m_cmd_macro_id_delete_db_;           //
        static Macro_ID_Enroll_User_Record              m_cmd_macro_id_enroll_user_record_;  //
        static Macro_ID_Identify                        m_cmd_macro_id_identify_;            //
        static Macro_ID_Verify_User_Record              m_cmd_macro_id_user_record_;         //
        static Macro_Match                              m_cmd_macro_match_;                  //
        static Macro_Save_Last_Capture                  m_cmd_macro_save_last_capture_;      //
        static Macro_Update_Firmware                    m_cmd_macro_update_firmware_;        //
        static Macro_Verify_User                        m_cmd_macro_verify_user_;            //
        static Macro_Vid_Stream                         m_cmd_macro_vid_stream_;             //
        static Macro_Enc_Generate_RSA_Keys              m_cmd_macro_generate_rsa_key_;       //
        static Atomic_Hid_Init                          m_cmd_hid_init;                      // 
        static Atomic_Hid_Enum_Cams                     m_cmd_hid_enum_cams;                 // 
        static Atomic_Hid_Terminate                     m_cmd_hid_terminate;                 // 
        static Atomic_Hid_Set_Param_Int                 m_cmd_hid_set_params_int;            // 
        static Atomic_Hid_Get_Param_Int                 m_cmd_hid_get_params_int;            // 
        static Atomic_Hid_Set_Param_Str                 m_cmd_hid_set_params_str;            // 
        static Atomic_Hid_Get_Param_Str                 m_cmd_hid_get_params_str;            // 
        static Atomic_Hid_Set_Param_Bin                 m_cmd_hid_set_param_bin;             // 
        static Atomic_Hid_Get_Param_Bin                 m_cmd_hid_get_param_bin;             // 
        static Atomic_Hid_Set_Param_Long                m_cmd_hid_set_param_long;            // 
        static Atomic_Hid_Get_Param_Long                m_cmd_hid_get_param_long;            // 
        static Atomic_Hid_Capture_Img                   m_cmd_hid_capture_image;             // 
        static Atomic_Hid_Open_Context                  m_cmd_hid_open_context;              // 
        static Atomic_Hid_Close_Context                 m_cmd_hid_close_context;             // 
        static Atomic_Hid_Async_Stop_Operation          m_cmd_hid_stop_operation;            // 
        static Atomic_Hid_Async_Extract_Template        m_cmd_hid_extract_template;          // 
        static Atomic_Hid_Async_Match_With_Template     m_cmd_hid_match_with_template;       // 
        static Atomic_Hid_Async_Match_With_Captured     m_cmd_hid_match_with_captured;       // 
        static Atomic_Hid_Async_Identify_With_Template  m_cmd_hid_identify_with_template;    // 
        static Atomic_Hid_Async_Identify_With_Captured  m_cmd_hid_identify_with_captured;    // 
        static Atomic_Hid_Async_Verify_With_Template    m_cmd_hid_verify_with_template;      // 
        static Atomic_Hid_Async_Verify_With_Captured    m_cmd_hid_verify_with_captured;      // 
        static Atomic_Hid_Get_Intermediate_Res          m_cmd_hid_get_int_results;           // 
        static Atomic_Hid_Get_Final_Res                 m_cmd_hid_get_final_results;         // 
        static Atomic_Hid_Parse_Res_Int                 m_cmd_hid_parse_res_int;             // 
        static Atomic_Hid_Parse_Res_Double              m_cmd_hid_parse_res_dbl;             // 
        static Atomic_Hid_Parse_Res_Data                m_cmd_hid_parse_res_data;            // 
        static Atomic_Hid_Parse_Res_Point               m_cmd_hid_parse_res_point;           // 
        static Atomic_Hid_Parse_Res_Image               m_cmd_hid_parse_res_image;           // 
        static Atomic_Hid_Parse_Match_Gallery           m_cmd_hid_parse_res_match_gallery;   // 
        static Atomic_Hid_Db_Add_Record_With_Captured   m_cmd_hid_db_add_with_captured;      // 
        static Atomic_Hid_Db_Add_Record_With_Template   m_cmd_hid_db_add_with_template;      // 
        static Atomic_Hid_Db_Get_Record                 m_cmd_hid_db_get_record;             // 
        static Atomic_Hid_Db_List_Records               m_cmd_hid_db_list_records;           // 
        static Atomic_Hid_Db_Del_Record                 m_cmd_hid_db_del_record;             // 
        static Atomic_Hid_Get_Video_Frame               m_cmd_hid_get_video_frame;           //

    private:
        static bool                                     m_running_;
        static std::mutex                               m_race_mutex;
        static SocketServer                             m_vcom_server_;
        static SocketServer                             m_shell_server_;
        static SocketServer                             m_shm_server_;
        static std::vector<std::thread>                 m_vcom_msg_threads_;
        static std::vector<std::thread>                 m_shell_msg_threads_;
        static std::vector<std::thread>                 m_shm_msg_threads_;
};

#endif
