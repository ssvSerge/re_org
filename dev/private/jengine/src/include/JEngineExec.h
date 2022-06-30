#ifndef __JENGINEEXEC_H__
#define __JENGINEEXEC_H__

#include <vector>
#include <thread>
#include <string>
#include <future>
#include <mutex>
#include <list>

#include <stdint.h>
#include <V100_shared_types.h>
#include <V100_enc_types.h>
#include <HFTypes.h>
#include <HBSEClient.h>

typedef std::list<std::future<CryptExecStatus>>  future_list_t;

class HbClientRes {
    public:
        std::unique_ptr<std::thread>    thread_;
        std::promise<CryptExecStatus>   res_;
};

enum class KeyState {
    KeyStateUnknow,
    KeyStateInvalid,
    KeyStateValid
};

class JEngineSetKeyCtx {
    public:
        JEngineSetKeyCtx();
        ~JEngineSetKeyCtx();

    public:
        bool CtxValidate();

    private:
        void do_cleanup();

    public:
        uint8_t*    ctx;
        int         ctxLen;
        u128        localAnbio;             // ANBIO     [at device].
        u128        remoteAnbio;            // ANBIO     [from request].
        u128        remoteAnsol;            // ANSOL     [from request].
        u16         keySlotCmd;             // Slot      [from V100 command].
        u16         keySlotPack;            // Slot      [from request].
        u16         keyVersion;             // KeyVer    [from request].
        u8          key_kcv[KCV_LENGTH];    //
        u16         keyCryptoLen;           //
        u16         keyMode;                // KeyMode   [from request].
        u2048       keyVal;                 // KeyVal    [from request].
        int         keyLen;                 // Len       [from request].
        u2048       respVal;                // Response  [from request].
        int         respLen;                // Len       [from request].
};

class JEngineExec {

    private:

        JEngineExec();
        ~JEngineExec();

        JEngineExec(JEngineExec& ref)           = delete;
        void operator=(const JEngineExec& ref)  = delete;

    public:
        static JEngineExec* GetInstance();
        static bool Start();
        static void Stop();

    // Crypto API
    public:
        void Delete_Buffer(uint8_t** buffer);
        CryptExecStatus Get_Busy();
        CryptExecStatus Start_Generate_RSA_Key();
        CryptExecStatus Execute_GetRandomBuffer   ( uint8_t* dst_buffer, uint32_t dst_buffer_len );
        CryptExecStatus Execute_Get_Key_Info      ( uint32_t key_slot, KeyInfoStructure& out_key_version );
        CryptExecStatus Execute_Get_RSA_PublicKey ( uint8_t** out_buffer, uint32_t& out_buffer_len );
        CryptExecStatus Execute_Generate_RSA_Key  ( void );
        CryptExecStatus Execute_Get_OP_Status     ( void );
        CryptExecStatus Execute_Set_Key           ( JEngineSetKeyCtx& keyCtx );
        CryptExecStatus Execute_UnlockStorage     ( uint8_t* pPacket, uint iLen );

    private:
        CryptExecStatus do_set_key_ctk            ( JEngineSetKeyCtx& keyCtx );
        CryptExecStatus do_set_key_btk            ( JEngineSetKeyCtx& keyCtx );
        CryptExecStatus do_set_key_vendor         ( JEngineSetKeyCtx& keyCtx );
        CryptExecStatus do_set_key_dsk            ( JEngineSetKeyCtx& keyCtx );
        CryptExecStatus do_set_key_bsk            ( JEngineSetKeyCtx& keyCtx );
        CryptExecStatus do_set_key_common         ( JEngineSetKeyCtx& keyCtx, int iParentKey );
        CryptExecStatus do_check_key_params       ( const u128& remoteAnbio, const u128& localAnbio, const u128& remoteAnsol, const u128& localAnsol, int remoteSlotId, int localSlotId );
        CryptExecStatus do_update_hbse_state      ( void );
        CryptExecStatus do_validate_ctx           ( const JEngineSetKeyCtx& keyCtx );

    public:
        static V100_ERROR_CODE Jag_HFInit();
        static V100_ERROR_CODE Jag_HFTerminate();

        static V100_ERROR_CODE Jag_HFEnumerateCameras(HFStringArray** desc_ptr);

        static V100_ERROR_CODE Jag_HFAsyncOpenContext(int32_t camId, HFAlgorithmType algoType, HFContext* ctx_ptr);
        static V100_ERROR_CODE Jag_HFCloseContext(HFContext ctx);
        static V100_ERROR_CODE Jag_HFAsyncStartCaptureImage(HFContext ctx, int32_t timeout, double minimalQuality, double minimalLivenessScore, uint64_t intermediateResultFlags, uint64_t finalResultFlags, HFOperation* const val_ptr);
        static V100_ERROR_CODE Jag_HFAsyncExtractTemplate(HFContext ctx, const HFImage* const img_ptr, uint64_t finalResultFlags, HFOperation* const val_ptr);
        static V100_ERROR_CODE Jag_HFAsyncVerifyWithCaptured(HFOperation op, const char* const gallery_ptr, const char* const id_ptr, double minScore);
        static V100_ERROR_CODE Jag_HFAsyncMatchWithTemplate(HFContext ctx, const HFData* const templA, const HFData* const templB, HFOperation* const operation);
        static V100_ERROR_CODE Jag_HFAsyncIdentifyWithCaptured(HFOperation op, const char* const galleryID, double minScore);
        static V100_ERROR_CODE Jag_HFAsyncMatchWithCaptured(HFOperation op, const HFData* const val_ptr);
        static V100_ERROR_CODE Jag_HFAsyncVerifyWithTemplate(HFContext ctx, const char* const gal_ptr, const char* const id_ptr, double minimalMatchScore, const HFData* const templ_ptr, HFOperation* const op_ptr);
        static V100_ERROR_CODE Jag_HFAsyncIdentifyWithTemplate(HFContext ctx, const char* const gal_ptr, double minScore, const HFData* const templ_ptr, HFOperation* const op_ptr);
        static V100_ERROR_CODE Jag_HFStopOperation(HFOperation operation);
        static V100_ERROR_CODE Jag_HFCloseOperation(HFOperation operation);

        static V100_ERROR_CODE Jag_HFSetParamInt(int32_t ctx, uint32_t id, uint32_t val);
        static V100_ERROR_CODE Jag_HFGetParamInt(int32_t ctx, uint32_t id, int32_t* const val_ptr);
        static V100_ERROR_CODE Jag_HFSetParamStr(int32_t ctx, uint32_t id, const char* const val_ptr);
        static V100_ERROR_CODE Jag_HFGetParamStr(int32_t ctx, uint32_t id, char** const val_ptr);
        static V100_ERROR_CODE Jag_HFSetParamBin(int32_t ctx, uint32_t id, const HFData* const val_ptr);
        static V100_ERROR_CODE Jag_HFGetParamBin(int32_t ctx, uint32_t id, HFData** const val_ptr);
        static V100_ERROR_CODE Jag_HFSetParamLng(int32_t ctx, uint32_t id, double val);
        static V100_ERROR_CODE Jag_HFGetParamLng(int32_t ctx, uint32_t id, double* const val_ptr);

        static V100_ERROR_CODE Jag_HFGetIntermediateResult(HFOperation op, uint64_t resFlg, int32_t seqId, HFData** const val_ptr);
        static V100_ERROR_CODE Jag_HFGetFinalResult(HFOperation operation, uint64_t resultFlags, HFData** const val_ptr);

        static V100_ERROR_CODE Jag_HFGetVideoFrame(HFContext ctx, int64_t lastSequenceNumber, HFImage** img_ptr, int64_t* seq_ptr);

        static V100_ERROR_CODE Jag_HFAddRecordWithCaptured(HFOperation op, const HFDatabaseRecordHeader* header, bool replaceIfExists);
        static V100_ERROR_CODE Jag_HFAddRecordWithTemplate(HFDatabaseRecord* const databaseRecord, bool replaceIfExists);
        static V100_ERROR_CODE Jag_HFGetRecord(const char* const id, const char* const galleryID, HFDatabaseRecord** const rec_ptr);
        static V100_ERROR_CODE Jag_HFListRecords(const char* galleryID, HFStringArray** ids);
        static V100_ERROR_CODE Jag_HFDelRecords(const char* const id, const char * const galleryID);

        static V100_ERROR_CODE Jag_Free (void* ptr);

    private:
        static V100_ERROR_CODE map_error (const char* const fName, int line, int jag_error_code);
        static CryptExecStatus do_gen_rsa_key(JEngineExec* instance);

    private:
        static JEngineExec*     m_instance_;
        static HBSEClient       m_hbse_client_;
        static std::mutex       m_race_mutex;
        future_list_t           m_pending_cmd_;
        KeyState                m_key_rsa_state_;
        KeyState                m_key_ctx_state_;
        KeyState                m_key_btk_state_;
        u128                    m_anbio_;
};

#endif
