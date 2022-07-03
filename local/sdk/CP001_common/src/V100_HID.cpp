#include <type_traits>
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cstring>

#include <V100EncCmd.h>
#include <HFTypes.h>

#define UNUSED(x)                    (void)(x)

#define PACKET_SIZE_T                ( 4)
#define V100_HEADER_LEN              (12)

#ifndef V100_IMPLEMENTATION
#define V100_IMPLEMENTATION          ( 0)
#endif

#if V100_IMPLEMENTATION
    #include <HFApi.h>
    #include <JEngineExec.h>
#endif

//------------- + Stuff -----------------------------------------------------//

static void _load_array (array_bin_t& storage, const void* const src, uint32_t src_len) {

    storage.clear();

    if ( (nullptr != src) && (src_len > 0) ) {
        const uint8_t* byte_ptr = (uint8_t*)src;
        storage.assign(byte_ptr, byte_ptr+ src_len);
    }
}

//------------- + IHidEncCmd ------------------------------------------------//

template <class T>
hid_types_t IHidEncCmd::do_get_type_fix(const T val) {

    UNUSED(val);

    hid_types_t store_type = hid_types_t::hid_type_err;

    if (std::is_same_v<T, uint64_t>)        { store_type = hid_types_t::hid_uint64_t; } else
    if (std::is_same_v<T, int64_t>)         { store_type = hid_types_t::hid_int64_t;  } else
    if (std::is_same_v<T, uint32_t>)        { store_type = hid_types_t::hid_uint32_t; } else
    if (std::is_same_v<T, int32_t>)         { store_type = hid_types_t::hid_int32_t;  } else
    if (std::is_same_v<T, uint16_t>)        { store_type = hid_types_t::hid_uint16_t; } else
    if (std::is_same_v<T, int16_t>)         { store_type = hid_types_t::hid_int16_t;  } else
    if (std::is_same_v<T, uint8_t>)         { store_type = hid_types_t::hid_uint8_t;  } else
    if (std::is_same_v<T, int8_t>)          { store_type = hid_types_t::hid_int8_t;   } else
    if (std::is_same_v<T, bool>)            { store_type = hid_types_t::hid_uint8_t;  } else
    if (std::is_same_v<T, double>)          { store_type = hid_types_t::hid_double_t; } else
    if (std::is_same_v<T, HFAlgorithmType>) { store_type = hid_types_t::hid_uint32_t; } else
    if (std::is_same_v<T, HFImageEncoding>) { store_type = hid_types_t::hid_uint32_t; }

    assert (hid_types_t::hid_type_err != store_type);
    return store_type;
}

template <class T>
bool IHidEncCmd::StoreFix(const bool prev_cmd_success, const T src) {

    hid_types_t store_type = hid_types_t::hid_type_err;

    if (!prev_cmd_success) {
        assert(false);
        return false;
    }

    store_type = do_get_type_fix(src);

    if (hid_types_t::hid_type_err == store_type) {
        assert(false);
        return false;
    }

    bool ret_val;
    ret_val = do_stream_out( store_type, &src, sizeof(src), hid_store_option_t::hid_store_option_len_fixed);
    return ret_val;
}

bool IHidEncCmd::StoreVar(const bool prev_cmd_success, const array_str_t& src) {

    const char*     src_ptr = nullptr;
    size_t          src_len = 0;

    if (!prev_cmd_success) {
        assert(false);
        return false;
    }

    if (src.length() > 0) {
        src_ptr     = src.c_str();
        src_len     = src.length();
    }

    bool ret_val;
    ret_val = do_stream_out(hid_types_t::hid_char_ptr_t, src_ptr, src_len, hid_store_option_t::hid_store_option_len_variadic);
    return ret_val;
}

bool IHidEncCmd::StoreVar(const bool prev_cmd_success, const array_bin_t& src) {

    const uint8_t*  src_ptr = nullptr;
    size_t          src_len = 0;

    if (!prev_cmd_success) {
        assert(false);
        return false;
    }

    if (src.size() > 0) {
        src_ptr     = &src[0];
        src_len     = src.size();
    }

    bool ret_val;
    ret_val = do_stream_out(hid_types_t::hid_byte_ptr_t, src_ptr, src_len, hid_store_option_t::hid_store_option_len_variadic);
    return ret_val;
}

bool IHidEncCmd::StoreVar(const bool prev_cmd_success, const array_match_t& src) {

    bool success = prev_cmd_success;

    uint32_t src_len = 0;

    if (!prev_cmd_success) {
        assert(false);
        return false;
    }

    src_len = static_cast<uint32_t> (src.size());

    success = StoreArray(success, src_len);

    for (uint32_t i = 0; i < src_len; i++) {
        success = StoreFix(success, src[i].confidence);
        success = StoreVar(success, src[i].id);
    }

    return true;
}

bool IHidEncCmd::StoreArray(const bool prev_cmd_success, uint32_t val) {

    if (!prev_cmd_success) {
        return false;
    }

    bool ret_val;
    ret_val = do_stream_out(hid_types_t::hid_array_t, &val, sizeof(val), hid_store_option_t::hid_store_option_len_fixed);
    return ret_val;
}

template <class T>
bool IHidEncCmd::LoadFix(const bool prev_cmd_success, T& dst) {

    hid_types_t load_type = hid_types_t::hid_type_err;

    if (!prev_cmd_success) {
        return false;
    }

    load_type = do_get_type_fix(dst);

    if (hid_types_t::hid_type_err == load_type) {
        return false;
    }

    bool ret_val;
    ret_val = do_stream_in(load_type, sizeof(dst), &dst);
    return ret_val;

}

bool IHidEncCmd::LoadVar(const bool prev_cmd_success, array_str_t& dst) {
    
    hid_types_t type_in;

    dst.clear();

    if ( ! prev_cmd_success ) {
        return false;
    }

    if ( (m_challenge_offset_ + 1) > m_challenge_len_ ) {
        // No more variables.
        return false;
    }

    // Load TYPE_OF variable stored in.
    type_in = (hid_types_t)m_data_ptr_[m_challenge_offset_];
    m_challenge_offset_++;

    if (hid_types_t::hid_char_ptr_t != type_in) {
        // Type doesn't match.
        return false;
    }

    size_t      in_len = 0;
    uint8_t*    tmp = (uint8_t*) & in_len;

    for (size_t i = 0; i < PACKET_SIZE_T; i++) {
        tmp[i] = m_data_ptr_[m_challenge_offset_];
        m_challenge_offset_++;
    }

    if ((m_challenge_offset_ + in_len) > m_challenge_len_) {
        // Stream isn't long enough.
        return false;
    }

    dst.reserve(in_len + 64);

    dst.assign(                                         // 
        (m_data_ptr_ + m_challenge_offset_),            // Start position
        (m_data_ptr_ + m_challenge_offset_ + in_len)    // End position
    );                                                  // 

    m_challenge_offset_ += in_len;

    return true;
}

bool IHidEncCmd::LoadVar(const bool prev_cmd_success, array_bin_t& dst) {

    hid_types_t type_in;

    dst.clear();

    if (!prev_cmd_success) {
        return false;
    }

    if ((m_challenge_offset_ + 1) > m_challenge_len_) {
        // No more variables.
        return false;
    }

    // Load TYPE_OF variable stored in.
    type_in = (hid_types_t)m_data_ptr_[m_challenge_offset_];
    m_challenge_offset_++;

    if (hid_types_t::hid_byte_ptr_t != type_in) {
        // Type doesn't match.
        return false;
    }

    size_t      in_len = 0;
    uint8_t* tmp = (uint8_t*)&in_len;

    for (size_t i = 0; i < PACKET_SIZE_T; i++) {
        tmp[i] = m_data_ptr_[m_challenge_offset_];
        m_challenge_offset_++;
    }

    if ((m_challenge_offset_ + in_len) > m_challenge_len_) {
        // Stream isn't long enough.
        return false;
    }

    dst.resize(in_len + 64);

    dst.assign(                                         // 
        (m_data_ptr_ + m_challenge_offset_),            // Start position
        (m_data_ptr_ + m_challenge_offset_ + in_len)    // End position
    );                                                  // 

    m_challenge_offset_ += in_len;

    return true;
}

bool IHidEncCmd::LoadVar(const bool prev_cmd_success, array_match_t& dst) {

    UNUSED(prev_cmd_success);
    UNUSED(dst);
    return true;
}

bool IHidEncCmd::LoadArray(const bool prev_cmd_success, uint32_t& val) {

    if (!prev_cmd_success) {
        return false;
    }

    bool ret_val;
    ret_val = do_stream_in(hid_types_t::hid_array_t, sizeof(val), &val);
    return ret_val;
}

std::string IHidEncCmd::to_str(const array_bin_t& arr) {

    std::string ret_val;

    ret_val += "\"";
    for ( size_t i=0; i< arr.size(); i++) {
        ret_val += std::to_string( (int)arr[i] );
        if ( i < (arr.size()-1) ) {
            ret_val += " ";
        }
    }
    ret_val += "\"";
    return ret_val;
}

std::string IHidEncCmd::to_str(const array_str_t& arr) {

    std::string ret_val;

    ret_val += "\"";
    ret_val += arr;
    ret_val += "\"";

    return ret_val;
}

template <class T>
void IHidEncCmd::do_store_val( T val ) {
    const uint8_t* const src = (uint8_t*)&val;
    for (size_t i = 0; i < sizeof(T); i++) {
        m_storage_.push_back(src[i]);
    }
}

template <class T>
void IHidEncCmd::do_load_val( T& val ) {

    if (m_challenge_offset_ + sizeof(T) > m_challenge_len_) {
        return;
    }

    uint8_t* const dst = (uint8_t*)&val;

    for (size_t i = 0; i < sizeof(T); i++) {
        dst[i] = m_data_ptr_[m_challenge_offset_];
        m_challenge_offset_++;
    }
}

bool IHidEncCmd::PackChallenge(uchar** pPacket, uint& nSize) {

    bool ret_val;

    // Check input parameter.
    if (nullptr == pPacket) {
        return false;
    }

    *pPacket = nullptr;
    nSize = 0;

    m_storage_.clear();

    do_prep_header();

    assert(m_nCmd != CMD_NONE);
    assert(m_storage_.size() == V100_HEADER_LEN);

    ret_val = PackChallengeHid();
    if (!ret_val) {
        m_storage_.clear();
        return false;
    }

    do_fix_header();

    *pPacket = static_cast<uchar*> (m_storage_.data());
    nSize = (uint)m_storage_.size();

    return true;
}

bool IHidEncCmd::UnpackChallenge(const uchar* pPacket, uint nSize) {

    if (nSize < V100_HEADER_LEN) {
        return false;
    }

    m_data_ptr_ = pPacket;
    m_challenge_len_ = nSize;
    m_challenge_offset_ = 0;

    do_load_val(m_pack_sohv); //  0 ...  1 
    do_load_val(m_pack_cmd);  //  2 ...  5 
    do_load_val(m_pack_arg);  //  6 ...  7 
    do_load_val(m_pack_len);  //  8 ... 11

    if (m_pack_sohv != SOHV) {
        return false;
    }
    if (m_pack_cmd != m_nCmd) {
        return false;
    }

    assert(m_challenge_offset_ == V100_HEADER_LEN);

    do_cleanup();

    bool retVal = UnpackChallengeHid();

    if ( ! retVal ) {
        do_cleanup();
    }

    return retVal;
}

bool IHidEncCmd::PackResponse(uchar** pPacket, uint& nSize) {

    bool ret_val;

    // Check input parameter.
    if (nullptr == pPacket) {
        return false;
    }

    *pPacket = nullptr;
    nSize = 0;

    m_storage_.clear();

    do_prep_header();

    assert(m_storage_.size() == V100_HEADER_LEN);

    ret_val = PackResponseHid();
    if (!ret_val) {
        m_storage_.clear();
        return false;
    }

    do_fix_header();

    *pPacket = &m_storage_[0];
    nSize = (uint)m_storage_.size();

    return true;
}

bool IHidEncCmd::UnpackResponse(const uchar* pPacket, uint nSize) {

    if (nSize < V100_HEADER_LEN) {
        return false;
    }

    m_data_ptr_ = pPacket;
    m_challenge_len_ = nSize;
    m_challenge_offset_ = 0;

    do_load_val(m_pack_sohv);  //  0 ...  1 
    do_load_val(m_pack_cmd);   //  2 ...  5 
    do_load_val(m_pack_arg);   //  6 ...  7 
    do_load_val(m_pack_len);   //  8 ... 11

    if (m_pack_sohv != SOHV) {
        return false;
    }
    if (m_pack_cmd != m_nCmd) {
        return false;
    }

    do_cleanup();

    bool retVal = UnpackResponseHid();

    if (!retVal) {
        do_cleanup();
    }

    return retVal;
}

bool IHidEncCmd::do_stream_out(hid_types_t type, const void* const data, size_t len, hid_store_option_t len_option) {

    uint8_t* src;

    // Store TYPE
    src = (uint8_t*)&type;
    m_storage_.push_back(src[0]);

    if (hid_store_option_t::hid_store_option_len_variadic == len_option) {
        // Store variadic parameter LEN
        src = (uint8_t*)&len;
        for (int i = 0; i < PACKET_SIZE_T; i++) {
            m_storage_.push_back(src[i]);
        }
    }

    size_t storage_len = m_storage_.size();
    storage_len += len;

    m_storage_.reserve (storage_len);

    // Store DATA
    src = (uint8_t*)data;
    for (size_t i = 0; i < len; i++) {
        m_storage_.push_back(src[i]);
    }

    return true;
}

bool IHidEncCmd::do_stream_in(const hid_types_t type_req, const size_t dst_len, void* dst) {

    hid_types_t type_in;

    if ((m_challenge_offset_ + 1) > m_challenge_len_) {
        // No more variables.
        return false;
    }

    // Load TYPE_OF variable stored in.
    type_in = (hid_types_t)m_data_ptr_[m_challenge_offset_];
    m_challenge_offset_++;

    if (type_req != type_in) {
        // Type doesn't match.
        return false;
    }

    if ((m_challenge_offset_ + dst_len) > m_challenge_len_) {
        // Source packet isn't long enough.
        return false;
    }

    uint8_t* dst_ptr = (uint8_t*)dst;
    for (size_t i = 0; i < dst_len; i++) {
        dst_ptr[i] = m_data_ptr_[m_challenge_offset_];
        m_challenge_offset_++;
    }

    return true;
}

void IHidEncCmd::do_prep_header() {

    m_storage_.clear();

    // Format header (12 bytes length).
    do_store_val((uint16_t)SOHV);     //  0 ...  1 
    do_store_val((uint32_t)m_nCmd);   //  2 ...  5 
    do_store_val((uint16_t)m_nArg);   //  6 ...  7 
    do_store_val((uint32_t)0);        //  8 ... 11

}

void IHidEncCmd::do_fix_header() {

    // Fix the length.
    uint32_t len;

    assert(m_storage_.size() >= V100_HEADER_LEN);

    len = (uint32_t)m_storage_.size();
    len -= V100_HEADER_LEN;

    const uint8_t* const src = (uint8_t*)&len;

    for (int i = 0; i < 4; i++) {
        m_storage_[8 + i] = src[i];
    }
}

bool IHidEncCmd::load_hf_res() {

    bool        read_success = true;
    uint32_t    cnt;

    v100_match_rec_t newRec;

    read_success = LoadFix(read_success, m_hf_res.validFlags);
    read_success = LoadFix(read_success, m_hf_res.contextStatus);
    read_success = LoadFix(read_success, m_hf_res.errorCode);
    read_success = LoadFix(read_success, m_hf_res.imageEncoding);
    read_success = LoadVar(read_success, m_hf_res.image);
    read_success = LoadFix(read_success, m_hf_res.sequenceNumber);
    read_success = LoadFix(read_success, m_hf_res.facesDetectedCount);
    read_success = LoadFix(read_success, m_hf_res.quality);
    read_success = LoadVar(read_success, m_hf_res.templ);
    read_success = LoadFix(read_success, m_hf_res.boundBox.upperLeft.x);
    read_success = LoadFix(read_success, m_hf_res.boundBox.upperLeft.y);
    read_success = LoadFix(read_success, m_hf_res.boundBox.bottomRight.x);
    read_success = LoadFix(read_success, m_hf_res.boundBox.bottomRight.y);
    read_success = LoadFix(read_success, m_hf_res.spoofProbability);
    read_success = LoadFix(read_success, m_hf_res.isCaptured);
    read_success = LoadFix(read_success, m_hf_res.landmarks.leftEye.x);
    read_success = LoadFix(read_success, m_hf_res.landmarks.leftEye.y);
    read_success = LoadFix(read_success, m_hf_res.landmarks.rightEye.x);
    read_success = LoadFix(read_success, m_hf_res.landmarks.rightEye.y);
    read_success = LoadFix(read_success, m_hf_res.landmarks.nose.x);
    read_success = LoadFix(read_success, m_hf_res.landmarks.nose.y);
    read_success = LoadFix(read_success, m_hf_res.landmarks.mouthLeft.x);
    read_success = LoadFix(read_success, m_hf_res.landmarks.mouthLeft.y);
    read_success = LoadFix(read_success, m_hf_res.landmarks.mouthRight.x);
    read_success = LoadFix(read_success, m_hf_res.landmarks.mouthRight.y);

    read_success = LoadArray(read_success, cnt);

    for (uint32_t i = 0; i < cnt; i++) {
        read_success = LoadVar(read_success, newRec.recordId);
        read_success = LoadVar(read_success, newRec.galleryID);
        read_success = LoadVar(read_success, newRec.customData);
        read_success = LoadFix(read_success, newRec.matchScore);

        if (! read_success) {
            break;
        }

        m_hf_res.matches.push_back(newRec);
    }

    return read_success;
}

bool IHidEncCmd::store_hf_res() {

    bool store_success = true;
    uint32_t cnt;

    store_success = StoreFix(store_success, m_hf_res.validFlags);
    store_success = StoreFix(store_success, m_hf_res.contextStatus);
    store_success = StoreFix(store_success, m_hf_res.errorCode);
    store_success = StoreFix(store_success, m_hf_res.imageEncoding);
    store_success = StoreVar(store_success, m_hf_res.image);
    store_success = StoreFix(store_success, m_hf_res.sequenceNumber);
    store_success = StoreFix(store_success, m_hf_res.facesDetectedCount);
    store_success = StoreFix(store_success, m_hf_res.quality);
    store_success = StoreVar(store_success, m_hf_res.templ);
    store_success = StoreFix(store_success, m_hf_res.boundBox.upperLeft.x);
    store_success = StoreFix(store_success, m_hf_res.boundBox.upperLeft.y);
    store_success = StoreFix(store_success, m_hf_res.boundBox.bottomRight.x);
    store_success = StoreFix(store_success, m_hf_res.boundBox.bottomRight.y);
    store_success = StoreFix(store_success, m_hf_res.spoofProbability);
    store_success = StoreFix(store_success, m_hf_res.isCaptured);
    store_success = StoreFix(store_success, m_hf_res.landmarks.leftEye.x);
    store_success = StoreFix(store_success, m_hf_res.landmarks.leftEye.y);
    store_success = StoreFix(store_success, m_hf_res.landmarks.rightEye.x);
    store_success = StoreFix(store_success, m_hf_res.landmarks.rightEye.y);
    store_success = StoreFix(store_success, m_hf_res.landmarks.nose.x);
    store_success = StoreFix(store_success, m_hf_res.landmarks.nose.y);
    store_success = StoreFix(store_success, m_hf_res.landmarks.mouthLeft.x);
    store_success = StoreFix(store_success, m_hf_res.landmarks.mouthLeft.y);
    store_success = StoreFix(store_success, m_hf_res.landmarks.mouthRight.x);
    store_success = StoreFix(store_success, m_hf_res.landmarks.mouthRight.y);

    cnt = static_cast<uint32_t>( m_hf_res.matches.size() );
    
    store_success = StoreArray(store_success, cnt );

    for ( uint32_t i=0; i<cnt; i++ ) {
        store_success = StoreVar(store_success, m_hf_res.matches[i].recordId);
        store_success = StoreVar(store_success, m_hf_res.matches[i].galleryID);
        store_success = StoreVar(store_success, m_hf_res.matches[i].customData);
        store_success = StoreFix(store_success, m_hf_res.matches[i].matchScore);

        if ( ! store_success ) {
            break;
        }
    }

    return store_success;
}

void IHidEncCmd::process_hf_res(const void* const hfData) {

    UNUSED(hfData);

    #if V100_IMPLEMENTATION

        const HFResult* const hfRes = static_cast<const HFResult* const> (hfData);

        m_hf_res.validFlags             = hfRes->validFlags;
        m_hf_res.contextStatus          = hfRes->contextStatus;
        m_hf_res.errorCode              = hfRes->errorCode;
        m_hf_res.imageEncoding          = hfRes->image.imageEncoding;
        m_hf_res.sequenceNumber         = hfRes->sequenceNumber;
        m_hf_res.facesDetectedCount     = hfRes->facesDetectedCount;
        m_hf_res.quality                = hfRes->quality;
        m_hf_res.boundBox.upperLeft.x   = hfRes->boundBox.upperLeft.x;
        m_hf_res.boundBox.upperLeft.y   = hfRes->boundBox.upperLeft.y;
        m_hf_res.boundBox.bottomRight.x = hfRes->boundBox.bottomRight.x;
        m_hf_res.boundBox.bottomRight.y = hfRes->boundBox.bottomRight.y;
        m_hf_res.spoofProbability       = hfRes->spoofProbability;
        m_hf_res.isCaptured             = hfRes->isCaptured;
        m_hf_res.landmarks.leftEye.x    = hfRes->landmarks.leftEye.x;
        m_hf_res.landmarks.leftEye.y    = hfRes->landmarks.leftEye.y;
        m_hf_res.landmarks.rightEye.x   = hfRes->landmarks.rightEye.x;
        m_hf_res.landmarks.rightEye.y   = hfRes->landmarks.rightEye.y;
        m_hf_res.landmarks.nose.x       = hfRes->landmarks.nose.x;
        m_hf_res.landmarks.nose.y       = hfRes->landmarks.nose.y;
        m_hf_res.landmarks.mouthLeft.x  = hfRes->landmarks.mouthLeft.x;
        m_hf_res.landmarks.mouthLeft.y  = hfRes->landmarks.mouthLeft.y;
        m_hf_res.landmarks.mouthRight.x = hfRes->landmarks.mouthRight.x;
        m_hf_res.landmarks.mouthRight.y = hfRes->landmarks.mouthRight.y;

        m_hf_res.image.clear();
        if (hfRes->image.data.size > 0) {
            const uint8_t* const data_start = static_cast<const uint8_t* const> (hfRes->image.data.data);
            const uint8_t* const data_end   = data_start + hfRes->image.data.size;
            m_hf_res.image.assign (data_start, data_end);
        }

        m_hf_res.templ.clear();
        if (hfRes->templ.size > 0) {
            const uint8_t* const data_start = static_cast<const uint8_t* const> (hfRes->templ.data);
            const uint8_t* const data_end   = data_start + hfRes->templ.size;
            m_hf_res.templ.assign(data_start, data_end);
        }

        m_hf_res.matches.clear();
        if (hfRes->matches.recordsCount > 0) {

            v100_match_rec_t machRec;

            for (uint32_t i=0; i< hfRes->matches.recordsCount; i++ ) {

                machRec.customData  = hfRes->matches.records[i].header.customData;
                machRec.galleryID   = hfRes->matches.records[i].header.galleryID;
                machRec.recordId    = hfRes->matches.records[i].header.recordId;
                machRec.matchScore  = hfRes->matches.records[i].matchScore;

                m_hf_res.matches.push_back(machRec);
            }

        }

    #endif    
}

//------------- + Atomic_Hid_Init -------------------------------------------//

Atomic_Hid_Init::Atomic_Hid_Init() {

    m_nCmd = CMD_HID_INIT;
}

Atomic_Hid_Init::~Atomic_Hid_Init() {

}

void Atomic_Hid_Init::SetParam(const char* const param) {

    UNUSED(param);
    do_cleanup();
}

void Atomic_Hid_Init::do_cleanup() {

}

bool Atomic_Hid_Init::PackChallengeHid() {

    return true;
}

bool Atomic_Hid_Init::UnpackChallengeHid() {

    return true;
}

bool Atomic_Hid_Init::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Init::UnpackResponseHid() {

    do_cleanup();
    return true;
}

void Atomic_Hid_Init::dump(std::stringstream& msg) {

    UNUSED(msg);
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Init::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFInit();
    if (GEN_OK != status) {
        m_nRC = status;
    }

}

#else

void Atomic_Hid_Init::Exec() {

}

#endif

//------------- + Atomic_Hid_Enum_Cams --------------------------------------//

Atomic_Hid_Enum_Cams::Atomic_Hid_Enum_Cams() {

    m_nCmd = CMD_HID_ENUM_CAMS;
    m_enum_list_.clear();
}

Atomic_Hid_Enum_Cams::~Atomic_Hid_Enum_Cams() {

}

void Atomic_Hid_Enum_Cams::do_cleanup() {

    m_enum_list_.clear();
}

bool Atomic_Hid_Enum_Cams::PackChallengeHid() {

    return true;
}

bool Atomic_Hid_Enum_Cams::UnpackChallengeHid() {

    return true;
}

bool Atomic_Hid_Enum_Cams::PackResponseHid() {

    bool store_success = true;

    uint32_t items_cnt;

    items_cnt = static_cast<uint32_t>(m_enum_list_.size());

    store_success = StoreArray(store_success, items_cnt);

    if (store_success) {
        for (size_t i = 0; i < items_cnt; i++) {
            store_success = StoreVar( store_success, m_enum_list_[i] );
        }
    }

    return store_success;
}

bool Atomic_Hid_Enum_Cams::UnpackResponseHid() {

    bool        read_success = true;
    uint32_t    strings_cnt  = 0;
    std::string next_str;

    read_success = LoadArray(read_success, strings_cnt);

    if ( read_success ) {
        for (uint32_t i = 0; i < strings_cnt; i++) {
            read_success = LoadVar(read_success, next_str);
            m_enum_list_.push_back(next_str);
        }
    }

    return read_success;
}

void Atomic_Hid_Enum_Cams::GetCamsList(string_list_t& str_list) {

    str_list.clear();
    str_list = m_enum_list_;
}

void Atomic_Hid_Enum_Cams::dump(std::stringstream& msg) {
    msg.str().clear();
    for (size_t i = 0; i < m_enum_list_.size(); i++) {
        msg << m_enum_list_[i] << "; ";
    }
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Enum_Cams::Exec() {

    V100_ERROR_CODE status;

    HFStringArray* desc_ptr = nullptr;

    m_nRC = GEN_OK;
    m_enum_list_.clear();

    status = JEngineExec::GetInstance()->Jag_HFEnumerateCameras(&desc_ptr);
    if (GEN_OK != status) {
        m_nRC = status;
        return;
    }

    if (nullptr != desc_ptr) {
        for (uint32_t i = 0; i < desc_ptr->stringsCount; i++) {
            m_enum_list_.push_back(desc_ptr->strings[i]);
        }
        JEngineExec::GetInstance()->Jag_Free(desc_ptr);
    }

    return;
}

#else 

void Atomic_Hid_Enum_Cams::Exec() {
}

#endif

//------------- + Atomic_Hid_Terminate --------------------------------------//

Atomic_Hid_Terminate::Atomic_Hid_Terminate() {

    m_nCmd = CMD_HID_TERMINATE;
}

Atomic_Hid_Terminate::~Atomic_Hid_Terminate() {

}

void Atomic_Hid_Terminate::do_cleanup(void) {

}

bool Atomic_Hid_Terminate::PackChallengeHid() {

    return true;
}

bool Atomic_Hid_Terminate::UnpackChallengeHid() {

    return true;
}

bool Atomic_Hid_Terminate::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Terminate::UnpackResponseHid() {

    return true;
}

void Atomic_Hid_Terminate::dump(std::stringstream& msg) {

    msg.str().clear();
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Terminate::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFTerminate();
    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Terminate::Exec() {
}

#endif

//------------- + Atomic_Hid_Set_Param_Int ----------------------------------//

Atomic_Hid_Set_Param_Int::Atomic_Hid_Set_Param_Int() {

    m_nCmd = CMD_HID_SET_PARAM_INT;

    do_cleanup();
}

Atomic_Hid_Set_Param_Int::~Atomic_Hid_Set_Param_Int() {

}

void Atomic_Hid_Set_Param_Int::do_cleanup() {
    m_ctx = 0;
    m_id  = 0;
    m_val = 0;
}

void Atomic_Hid_Set_Param_Int::SetParam(int32_t ctx, uint32_t id, int32_t val) {
    m_ctx  = ctx;
    m_id   = id;
    m_val  = val;
}

bool Atomic_Hid_Set_Param_Int::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_id);
    store_success = StoreFix(store_success, m_val);
    return store_success;
}

bool Atomic_Hid_Set_Param_Int::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_id);
    read_success = LoadFix(read_success, m_val);
    return read_success;
}

bool Atomic_Hid_Set_Param_Int::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Set_Param_Int::UnpackResponseHid() {

    return true;
}

void Atomic_Hid_Set_Param_Int::dump(std::stringstream& msg) {
    msg.str().clear();
    msg << "ctx:" << m_ctx << "; id: " << m_id << "; val: " << m_val;
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Set_Param_Int::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFSetParamInt(m_ctx, m_id, m_val);
    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Set_Param_Int::Exec() {
}

#endif

//------------- + Atomic_Hid_Get_Param_Int ----------------------------------//

Atomic_Hid_Get_Param_Int::Atomic_Hid_Get_Param_Int() {

    m_nCmd = CMD_HID_GET_PARAM_INT;
    do_cleanup();
}

Atomic_Hid_Get_Param_Int::~Atomic_Hid_Get_Param_Int() {

}

void Atomic_Hid_Get_Param_Int::do_cleanup() {
    m_ctx = 0;
    m_id  = 0;
    m_val = 0;
}

void Atomic_Hid_Get_Param_Int::SetId(int32_t ctx, uint32_t id) {
    m_ctx = ctx;
    m_id  = id;
}

void Atomic_Hid_Get_Param_Int::GetValue(int32_t& val) {
    val = m_val;
}

bool Atomic_Hid_Get_Param_Int::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_id);
    return store_success;
}

bool Atomic_Hid_Get_Param_Int::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_id);
    return read_success;
}

bool Atomic_Hid_Get_Param_Int::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_val);
    return store_success;
}

bool Atomic_Hid_Get_Param_Int::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_val);
    return read_success;
}

void Atomic_Hid_Get_Param_Int::dump(std::stringstream& msg) {
    msg.str().clear();
    msg << "ctx:" << m_ctx << "; id: " << m_id << "; val: " << m_val;
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Get_Param_Int::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFGetParamInt(m_ctx, m_id, &m_val);
    if (GEN_OK != status) {
        m_nRC = status;
    }

}

#else

void Atomic_Hid_Get_Param_Int::Exec() {
}

#endif

//------------- + Atomic_Hid_Set_Param_Str ----------------------------------//

Atomic_Hid_Set_Param_Str::Atomic_Hid_Set_Param_Str() {
    
    m_nCmd = CMD_HID_SET_PARAM_STR;
}

Atomic_Hid_Set_Param_Str::~Atomic_Hid_Set_Param_Str() {

}

void Atomic_Hid_Set_Param_Str::do_cleanup() {
    m_ctx   = 0;
    m_id    = 0;
    m_val.clear();
}

void Atomic_Hid_Set_Param_Str::SetParam(int32_t ctx, uint32_t id, std::string val) {

    m_ctx = ctx;
    m_id  = id;
    m_val = val;
}

bool Atomic_Hid_Set_Param_Str::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_id);
    store_success = StoreVar(store_success, m_val);
    return store_success;
}

bool Atomic_Hid_Set_Param_Str::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_id);
    read_success = LoadVar(read_success, m_val);
    return read_success;
}

bool Atomic_Hid_Set_Param_Str::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Set_Param_Str::UnpackResponseHid() {

    return true;
}

void Atomic_Hid_Set_Param_Str::dump(std::stringstream& msg) {
    msg.str().clear();
    msg << "ctx:" << m_ctx << "; id: " << m_id << "; val: " << m_val;
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Set_Param_Str::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFSetParamStr( m_ctx, m_id, m_val.c_str() );
    if (GEN_OK != status) {
        m_nRC = status;
    }

}

#else

void Atomic_Hid_Set_Param_Str::Exec() {
}

#endif

//------------- + Atomic_Hid_Get_Param_Str ----------------------------------//

Atomic_Hid_Get_Param_Str::Atomic_Hid_Get_Param_Str() {

    m_nCmd = CMD_HID_GET_PARAM_STR;
}

Atomic_Hid_Get_Param_Str::~Atomic_Hid_Get_Param_Str() {

}

void Atomic_Hid_Get_Param_Str::do_cleanup() {
    m_ctx   = 0;
    m_id    = 0;
    m_val.clear();
}

void Atomic_Hid_Get_Param_Str::SetId(int32_t ctx, uint32_t id) {
    m_ctx = ctx;
    m_id  = id;
}

void Atomic_Hid_Get_Param_Str::GetValue(std::string& val) {
    val = m_val;
}

bool Atomic_Hid_Get_Param_Str::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_id);
    return store_success;
}

bool Atomic_Hid_Get_Param_Str::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_id);
    return read_success;
}

bool Atomic_Hid_Get_Param_Str::PackResponseHid() {
    bool store_success = true;
    store_success = StoreVar(store_success, m_val);
    return store_success;
}

bool Atomic_Hid_Get_Param_Str::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadVar(read_success, m_val);
    return read_success;
}

void Atomic_Hid_Get_Param_Str::dump(std::stringstream& msg) {
    msg.str().clear();
    msg << "ctx:" << m_ctx << "; id: " << m_id << "; val: " << m_val;
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Get_Param_Str::Exec() {

    V100_ERROR_CODE status;
    char* str = nullptr;

    m_nRC = GEN_OK;

    m_val.clear();

    status = JEngineExec::GetInstance()->Jag_HFGetParamStr(m_ctx, m_id, &str ); 
    if (GEN_OK != status) {
        m_nRC = status;
    }

    if (nullptr != str) {
        m_val = str;
        JEngineExec::GetInstance()->Jag_Free(str);
    }
}

#else

void Atomic_Hid_Get_Param_Str::Exec() {
}

#endif

//------------- + Atomic_Hid_Set_Param_Bin ----------------------------------//

Atomic_Hid_Set_Param_Bin::Atomic_Hid_Set_Param_Bin() {

    m_nCmd = CMD_HID_SET_PARAM_BIN;
}

Atomic_Hid_Set_Param_Bin::~Atomic_Hid_Set_Param_Bin() {

}

void Atomic_Hid_Set_Param_Bin::do_cleanup() {

    m_val.clear();
}

void Atomic_Hid_Set_Param_Bin::SetParam(int32_t ctx, uint32_t id, const uint8_t* const val_ptr, uint32_t val_len) {

    do_cleanup();

    m_ctx = ctx;
    m_id  = id;

    m_val.clear();
    if ( (nullptr != val_ptr) && (val_len > 0) ) {
        m_val.assign(val_ptr, val_ptr + val_len);
    }
}

bool Atomic_Hid_Set_Param_Bin::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_id);
    store_success = StoreVar(store_success, m_val);
    return store_success;
}

bool Atomic_Hid_Set_Param_Bin::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_id);
    read_success = LoadVar(read_success, m_val);
    return read_success;
}

bool Atomic_Hid_Set_Param_Bin::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Set_Param_Bin::UnpackResponseHid() {

    return true;
}

void Atomic_Hid_Set_Param_Bin::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string val_str = to_str(m_val);
    msg << "ctx:" << m_ctx << "; id: " << m_id << "; val: " << val_str;
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Set_Param_Bin::Exec() {

    V100_ERROR_CODE status;
    HFData          val;

    m_nRC = GEN_OK;

    val.data = nullptr;
    val.size = 0;

    if (m_val.size() > 0) {
        val.data = (uint8_t*) &m_val[0];
        val.size = (uint32_t) m_val.size();
    }

    status = JEngineExec::GetInstance()->Jag_HFSetParamBin(m_ctx, m_id, &val);

    if (GEN_OK != status) {
        m_nRC = status;
    }

}

#else

void Atomic_Hid_Set_Param_Bin::Exec() {
}

#endif

//------------- + Atomic_Hid_Get_Param_Bin ----------------------------------//

Atomic_Hid_Get_Param_Bin::Atomic_Hid_Get_Param_Bin() {

    m_nCmd = CMD_HID_GET_PARAM_BIN;

    do_cleanup();
}

Atomic_Hid_Get_Param_Bin::~Atomic_Hid_Get_Param_Bin() {

}

void Atomic_Hid_Get_Param_Bin::SetId(int32_t ctx, uint32_t id) {

    m_ctx = ctx;
    m_id  = id;
}

void Atomic_Hid_Get_Param_Bin::GetValue(array_bin_t& val) {

    val = m_val;
}

void Atomic_Hid_Get_Param_Bin::do_cleanup() {

    m_val.clear();
}

bool Atomic_Hid_Get_Param_Bin::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_id);
    return store_success;
}

bool Atomic_Hid_Get_Param_Bin::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_id);
    return read_success;
}

bool Atomic_Hid_Get_Param_Bin::PackResponseHid() {
    bool store_success = true;
    store_success = StoreVar(store_success, m_val);
    return store_success;
}

bool Atomic_Hid_Get_Param_Bin::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadVar(read_success, m_val);
    return read_success;
}

void Atomic_Hid_Get_Param_Bin::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string val_str = to_str(m_val);
    msg << "ctx:" << m_ctx << "; id: " << m_id << "; val: " << val_str;
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Get_Param_Bin::Exec() {

    V100_ERROR_CODE status;
    HFData* val_ptr = nullptr;

    m_nRC = GEN_OK;
    m_val.clear();

    status = JEngineExec::GetInstance()->Jag_HFGetParamBin(m_ctx, m_id, &val_ptr);

    if (GEN_OK != status) {
        m_nRC = status;
        return;
    }

    if (val_ptr) {
        _load_array(m_val, val_ptr->data, val_ptr->size);
        JEngineExec::GetInstance()->Jag_Free(val_ptr);
    }

}

#else

void Atomic_Hid_Get_Param_Bin::Exec() {
}

#endif

//------------- + Atomic_Hid_Set_Param_Long ---------------------------------//

Atomic_Hid_Set_Param_Long::Atomic_Hid_Set_Param_Long() {
    m_nCmd = CMD_HID_SET_PARAM_LONG;
}

Atomic_Hid_Set_Param_Long::~Atomic_Hid_Set_Param_Long() {

}

void Atomic_Hid_Set_Param_Long::do_cleanup() {
    m_ctx = 0;
    m_id  = 0;
    m_val = 0;
}

void Atomic_Hid_Set_Param_Long::SetParam(int32_t ctx, uint32_t id, double val) {
    m_ctx = ctx;
    m_id  = id;
    m_val = val;
}

bool Atomic_Hid_Set_Param_Long::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_id);
    store_success = StoreFix(store_success, m_val);
    return store_success;
}

bool Atomic_Hid_Set_Param_Long::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_id);
    read_success = LoadFix(read_success, m_val);
    return read_success;
}

bool Atomic_Hid_Set_Param_Long::PackResponseHid() {
    return true;
}

bool Atomic_Hid_Set_Param_Long::UnpackResponseHid() {
    return true;
}

void Atomic_Hid_Set_Param_Long::dump(std::stringstream& msg) {
    msg.str().clear();
    msg << "ctx:" << m_ctx << "; id: " << m_id << "; val: " << m_val;
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Set_Param_Long::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFSetParamLng(m_ctx, m_id, m_val);
    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Set_Param_Long::Exec() {

}

#endif

//------------- + Atomic_Hid_Get_Param_Long ---------------------------------//

Atomic_Hid_Get_Param_Long::Atomic_Hid_Get_Param_Long() {

    m_nCmd = CMD_HID_GET_PARAM_LONG;
    do_cleanup();
}

Atomic_Hid_Get_Param_Long::~Atomic_Hid_Get_Param_Long() {

}

void Atomic_Hid_Get_Param_Long::do_cleanup() {
    m_ctx = 0;
    m_id  = 0;
    m_val = 0;
}

void Atomic_Hid_Get_Param_Long::SetId(int32_t ctx, uint32_t id) {
    m_ctx = ctx;
    m_id  = id;
}

void Atomic_Hid_Get_Param_Long::GetValue(double& val) {
    val = m_val;
}

bool Atomic_Hid_Get_Param_Long::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_id);
    return store_success;
}

bool Atomic_Hid_Get_Param_Long::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_id);
    return read_success;
}

bool Atomic_Hid_Get_Param_Long::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_val);
    return store_success;
}

bool Atomic_Hid_Get_Param_Long::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_val);
    return read_success;
}

void Atomic_Hid_Get_Param_Long::dump(std::stringstream& msg) {
    msg.str().clear();
    msg << "ctx:" << m_ctx << "; id: " << m_id << "; val: " << m_val;
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Get_Param_Long::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFGetParamLng(m_ctx, m_id, &m_val);
    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Get_Param_Long::Exec() {
}

#endif

//------------- + Atomic_Hid_Capture_Img ------------------------------------//

Atomic_Hid_Capture_Img::Atomic_Hid_Capture_Img() {
    m_nCmd = CMD_HID_CAPTURE_IMAGE;
}

Atomic_Hid_Capture_Img::~Atomic_Hid_Capture_Img() {

}

void Atomic_Hid_Capture_Img::SetParam(HFContext ctx, int32_t timeout, double minQuality, double minLiveness, uint64_t intFlags, uint64_t finalFlags) {
    m_ctx          = ctx;
    m_timeout      = timeout;
    m_minQuality   = minQuality;
    m_minLiveness  = minLiveness;
    m_intFlags     = intFlags;
    m_finalFlags   = finalFlags;
}

void Atomic_Hid_Capture_Img::GetValue(int32_t& val) {
    val = m_operation;
}

void Atomic_Hid_Capture_Img::do_cleanup() {
    m_ctx          = 0;
    m_timeout      = 0;
    m_minQuality   = 0;
    m_minLiveness  = 0;
    m_intFlags     = 0;
    m_finalFlags   = 0;
    m_operation    = 0;
}

bool Atomic_Hid_Capture_Img::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_timeout);
    store_success = StoreFix(store_success, m_minQuality);
    store_success = StoreFix(store_success, m_minLiveness);
    store_success = StoreFix(store_success, m_intFlags);
    store_success = StoreFix(store_success, m_finalFlags);
    return store_success;
}

bool Atomic_Hid_Capture_Img::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_timeout);
    read_success = LoadFix(read_success, m_minQuality);
    read_success = LoadFix(read_success, m_minLiveness);
    read_success = LoadFix(read_success, m_intFlags);
    read_success = LoadFix(read_success, m_finalFlags);
    return read_success;
}

bool Atomic_Hid_Capture_Img::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_operation);
    return store_success;
}

bool Atomic_Hid_Capture_Img::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_operation);
    return read_success;
}

void Atomic_Hid_Capture_Img::dump(std::stringstream& msg) {
    msg.str().clear();
    msg << "ctx:"       << m_ctx         << "; ";
    msg << "timeout:"   << m_timeout     << "; ";
    msg << "quality:"   << m_minQuality  << "; ";
    msg << "liveness:"  << m_minLiveness << "; ";
    msg << "in_flags:"  << m_intFlags    << "; ";
    msg << "out_flags:" << m_finalFlags  << "; ";
    msg << "operation:" << m_operation   << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Capture_Img::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFAsyncStartCaptureImage(m_ctx, m_timeout, m_minQuality, m_minLiveness, m_intFlags, m_finalFlags, &m_operation);
    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Capture_Img::Exec() {
}

#endif

//------------- + Atomic_Hid_Open_Context -----------------------------------//

Atomic_Hid_Open_Context::Atomic_Hid_Open_Context() {
    m_nCmd = CMD_HID_OPEN_CONTEXT;
}

Atomic_Hid_Open_Context::~Atomic_Hid_Open_Context() {

}

void Atomic_Hid_Open_Context::do_cleanup() {
    m_ctx       = 0;
    m_camId     = 0;
    m_algoType  = HFAlgorithmType::HFALGORITHM_TYPE_DEFAULT;
}

void Atomic_Hid_Open_Context::SetParam(int32_t cam_id, HFAlgorithmType algo_type) {
    m_camId     = cam_id;
    m_algoType  = algo_type;
}

void Atomic_Hid_Open_Context::GetValue(HFContext& ctx) {
    ctx = m_ctx;
}

bool Atomic_Hid_Open_Context::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_camId);
    store_success = StoreFix(store_success, m_algoType);
    return store_success;
}

bool Atomic_Hid_Open_Context::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_camId);
    read_success = LoadFix(read_success, m_algoType);
    return read_success;
}

bool Atomic_Hid_Open_Context::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    return store_success;
}

bool Atomic_Hid_Open_Context::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    return read_success;
}

void Atomic_Hid_Open_Context::dump(std::stringstream& msg) {
    msg.str().clear();
    msg << "cam:"   << m_camId     << "; ";
    msg << "algo:"  << m_algoType  << "; ";
    msg << "ctx:"   << m_ctx       << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Open_Context::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFAsyncOpenContext(m_camId, m_algoType, &m_ctx);
    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Open_Context::Exec() {
}

#endif

//------------- + Atomic_Hid_Close_Context ----------------------------------//

Atomic_Hid_Close_Context::Atomic_Hid_Close_Context() {
    m_nCmd = CMD_HID_CLOSE_CONTEXT;
}

Atomic_Hid_Close_Context::~Atomic_Hid_Close_Context() {

}

void Atomic_Hid_Close_Context::SetId(HFContext id) {

    m_ctx = id;
}

void Atomic_Hid_Close_Context::do_cleanup() {

}

bool Atomic_Hid_Close_Context::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    return store_success;
}

bool Atomic_Hid_Close_Context::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    return read_success;
}

bool Atomic_Hid_Close_Context::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Close_Context::UnpackResponseHid() {

    return true;
}

void Atomic_Hid_Close_Context::dump(std::stringstream& msg) {
    msg.str().clear();
    msg << "ctx:" << m_ctx << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Close_Context::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFCloseContext(m_ctx);
    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Close_Context::Exec() {
}

#endif

//------------- + Atomic_Hid_Async_Stop_Operation ---------------------------//

Atomic_Hid_Async_Stop_Operation::Atomic_Hid_Async_Stop_Operation() {
    m_nCmd = CMD_HID_STOP_CMD_ASYNC;
    do_cleanup();
}

Atomic_Hid_Async_Stop_Operation::~Atomic_Hid_Async_Stop_Operation() {

}

void Atomic_Hid_Async_Stop_Operation::do_cleanup() {

    m_operation = 0;
}

void Atomic_Hid_Async_Stop_Operation::SetId(int32_t operation) {

    m_operation = operation;
}

bool Atomic_Hid_Async_Stop_Operation::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_operation);
    return store_success;
}

bool Atomic_Hid_Async_Stop_Operation::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_operation);
    return read_success;
}

bool Atomic_Hid_Async_Stop_Operation::PackResponseHid() {
    return true;
}

bool Atomic_Hid_Async_Stop_Operation::UnpackResponseHid() {
    return true;
}

void Atomic_Hid_Async_Stop_Operation::dump(std::stringstream& msg) {
    msg.str().clear();
    msg << "operation:" << m_operation << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Async_Stop_Operation::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFStopOperation(m_operation);
    if (GEN_OK != status) {
        m_nRC = status;
    }

}

#else

void Atomic_Hid_Async_Stop_Operation::Exec() {
}

#endif

//------------- + Atomic_Hid_Close_Operation --------------------------------//

Atomic_Hid_Close_Operation::Atomic_Hid_Close_Operation() {
    m_nCmd = CMD_HID_CLOSE_OPERATION;
    do_cleanup();
}

Atomic_Hid_Close_Operation::~Atomic_Hid_Close_Operation() {

}

void Atomic_Hid_Close_Operation::do_cleanup() {

    m_operation = 0;
}

void Atomic_Hid_Close_Operation::SetId(int32_t operation) {

    m_operation = operation;
}

bool Atomic_Hid_Close_Operation::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_operation);
    return store_success;
}

bool Atomic_Hid_Close_Operation::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_operation);
    return read_success;
}

bool Atomic_Hid_Close_Operation::PackResponseHid() {
    return true;
}

bool Atomic_Hid_Close_Operation::UnpackResponseHid() {
    return true;
}

void Atomic_Hid_Close_Operation::dump(std::stringstream& msg) {
    msg.str().clear();
    msg << "operation:" << m_operation << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Close_Operation::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFStopOperation(m_operation);
    if (GEN_OK != status) {
        m_nRC = status;
    }

}

#else

void Atomic_Hid_Close_Operation::Exec() {
}

#endif

//------------- + Atomic_Hid_Async_Extract_Template -------------------------//

Atomic_Hid_Async_Extract_Template::Atomic_Hid_Async_Extract_Template() {
    m_nCmd = CMD_HID_ASYNC_EXTRACT_TEMPLATE;
} 

Atomic_Hid_Async_Extract_Template::~Atomic_Hid_Async_Extract_Template() {

}

void Atomic_Hid_Async_Extract_Template::do_cleanup() {
    m_encoding   = HFImageEncoding::HFIMAGE_ENCODING_MAX;
    m_ctx        = 0;
    m_finalFlags = 0;
    m_res        = 0;
    m_img.clear();
}

void Atomic_Hid_Async_Extract_Template::SetParam(HFContext ctx, HFImageEncoding imgEncoding, const void* const img_ptr, uint32_t img_len, uint64_t flags) {

    m_img.clear();

    m_ctx        = ctx;
    m_encoding   = imgEncoding;
    m_finalFlags = flags;

    if ((nullptr != img_ptr) && (img_len > 0)) {
        const uint8_t* const byte_ptr = (uint8_t*)img_ptr;
        m_img.assign(byte_ptr, byte_ptr+img_len);
    }
}

void Atomic_Hid_Async_Extract_Template::GetValue(int32_t& val) {
    val = m_res;
}

bool Atomic_Hid_Async_Extract_Template::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_encoding);
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_finalFlags);
    store_success = StoreVar(store_success, m_img);
    return store_success;
}

bool Atomic_Hid_Async_Extract_Template::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_encoding);
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_finalFlags);
    read_success = LoadVar(read_success, m_img);
    return read_success;
}

bool Atomic_Hid_Async_Extract_Template::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_res);
    return store_success;
}

bool Atomic_Hid_Async_Extract_Template::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_res);
    return read_success;
}

void Atomic_Hid_Async_Extract_Template::dump(std::stringstream& msg) {

    msg.str().clear();

    msg << "encoding:"    << m_encoding    << "; ";
    msg << "ctx:"         << m_ctx         << "; ";
    msg << "finalFlags:"  << m_finalFlags  << "; ";
    msg << "res:"         << m_res         << "; ";

    std::string val = to_str(m_img);

    msg << "img:" << val << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Async_Extract_Template::Exec() {

    V100_ERROR_CODE status;
    HFImage img;

    m_nRC = GEN_OK;

    img.imageEncoding = m_encoding;
    img.data.data = nullptr;
    img.data.size = 0;

    if (m_img.size() > 0) {
        img.data.data = (void*)    &m_img[0];
        img.data.size = (uint32_t)  m_img.size();
    }

    status = JEngineExec::GetInstance()->Jag_HFAsyncExtractTemplate(m_ctx, &img, m_finalFlags, &m_res);
    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Async_Extract_Template::Exec() {
}

#endif

//------------- + Atomic_Hid_Async_Match_With_Template ----------------------//

Atomic_Hid_Async_Match_With_Template::Atomic_Hid_Async_Match_With_Template() {
    m_nCmd = CMD_HID_ASYNC_MATCH_WITH_TEMPLATE;
}

Atomic_Hid_Async_Match_With_Template::~Atomic_Hid_Async_Match_With_Template() {

}

void Atomic_Hid_Async_Match_With_Template::do_cleanup() {
    m_ctx = 0;
    m_templA.clear();
    m_templB.clear();
    m_operation = 0;
}

void Atomic_Hid_Async_Match_With_Template::SetParam(HFContext ctx, const void* const templA_ptr, uint32_t len_A, const void* const templB_ptr, uint32_t len_B) {
    m_ctx = ctx;
    _load_array(m_templA, templA_ptr, len_A);
    _load_array(m_templB, templB_ptr, len_B);
}

void Atomic_Hid_Async_Match_With_Template::GetValue(HFOperation& val) {
    val = m_operation;
}

bool Atomic_Hid_Async_Match_With_Template::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_operation);
    store_success = StoreVar(store_success, m_templA);
    store_success = StoreVar(store_success, m_templB);
    return store_success;
}

bool Atomic_Hid_Async_Match_With_Template::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_operation);
    read_success = LoadVar(read_success, m_templA);
    read_success = LoadVar(read_success, m_templB);
    return read_success;
}

bool Atomic_Hid_Async_Match_With_Template::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_operation);
    return store_success;
}

bool Atomic_Hid_Async_Match_With_Template::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_operation);
    return read_success;
}

void Atomic_Hid_Async_Match_With_Template::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string valA = to_str(m_templA);
    std::string valB = to_str(m_templB);

    msg << "ctx:"        << m_ctx << "; ";
    msg << "operation:"  << m_operation << "; ";
    msg << "templA:"     << valA << "; ";
    msg << "templB:"     << valB << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Async_Match_With_Template::Exec() {

    V100_ERROR_CODE status;

    HFData templA = {0};
    HFData templB = {0};

    m_nRC = GEN_OK;

    if (m_templA.size() > 0) {
        templA.size = (uint32_t) m_templA.size();
        templA.data = &m_templA[0];
    }

    if (m_templB.size() > 0) {
        templB.size = (uint32_t)m_templB.size();
        templB.data = &m_templB[0];
    }

    status = JEngineExec::GetInstance()->Jag_HFAsyncMatchWithTemplate(m_ctx, &templA, &templB, &m_operation);

    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Async_Match_With_Template::Exec() {
}

#endif

//------------- + Atomic_Hid_Async_Verify_With_Captured ---------------------//

Atomic_Hid_Async_Verify_With_Captured::Atomic_Hid_Async_Verify_With_Captured() {
    m_nCmd = CMD_HID_ASYNC_VERIFY_WITH_CAPTURED;
}

Atomic_Hid_Async_Verify_With_Captured::~Atomic_Hid_Async_Verify_With_Captured() {

}

void Atomic_Hid_Async_Verify_With_Captured::do_cleanup() {
    m_op       = 0;
    m_minScore = 0;
    m_gallery.clear();
    m_id.clear();
}

void Atomic_Hid_Async_Verify_With_Captured::SetParam(HFOperation op, const char* const galery, const char* const id, double minScore) {

    m_op = op;
    m_minScore = minScore;

    m_gallery.clear();
    if (nullptr != galery) {
        m_gallery = galery;
    }

    m_id.clear();
    if (nullptr != id) {
        m_id = id;
    }
}

bool Atomic_Hid_Async_Verify_With_Captured::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_op);
    store_success = StoreFix(store_success, m_minScore);
    store_success = StoreVar(store_success, m_gallery);
    store_success = StoreVar(store_success, m_id);
    return store_success;
}

bool Atomic_Hid_Async_Verify_With_Captured::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_op);
    read_success = LoadFix(read_success, m_minScore);
    read_success = LoadVar(read_success, m_gallery);
    read_success = LoadVar(read_success, m_id);
    return read_success;
}

bool Atomic_Hid_Async_Verify_With_Captured::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Async_Verify_With_Captured::UnpackResponseHid() {

    return true;
}

void Atomic_Hid_Async_Verify_With_Captured::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string galleryStr = to_str(m_gallery);
    std::string idStr      = to_str(m_id);

    msg << "op:"      << m_op       << "; ";
    msg << "score:"   << m_minScore << "; ";
    msg << "gallery:" << galleryStr << "; ";
    msg << "id:"      << idStr      << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Async_Verify_With_Captured::Exec() {

    V100_ERROR_CODE status;

    const char* gallery_ptr = nullptr;
    const char* id_ptr      = nullptr;

    m_nRC = GEN_OK;

    gallery_ptr = m_gallery.c_str();
    id_ptr      = m_id.c_str();

    status = JEngineExec::GetInstance()->Jag_HFAsyncVerifyWithCaptured( m_op, gallery_ptr, id_ptr, m_minScore);

    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Async_Verify_With_Captured::Exec() {
}

#endif

//------------- + Atomic_Hid_Async_Identify_With_Captured -------------------//

Atomic_Hid_Async_Identify_With_Captured::Atomic_Hid_Async_Identify_With_Captured() {
    m_nCmd = CMD_HID_ASYNC_IDENTIFY_WITH_CAPTURED;
}

Atomic_Hid_Async_Identify_With_Captured::~Atomic_Hid_Async_Identify_With_Captured() {

}

void Atomic_Hid_Async_Identify_With_Captured::do_cleanup() {
    m_op       = 0;
    m_minScore = 0;
    m_gallery.clear();
}

void Atomic_Hid_Async_Identify_With_Captured::SetParam(HFOperation op, const char* const gallery, double minScore) {

    m_op = op;
    m_minScore = minScore;

    m_gallery.clear();
    if (nullptr != gallery) {
        m_gallery = gallery;
    }
}

bool Atomic_Hid_Async_Identify_With_Captured::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_op);
    store_success = StoreFix(store_success, m_minScore);
    store_success = StoreVar(store_success, m_gallery);
    return store_success;
}

bool Atomic_Hid_Async_Identify_With_Captured::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_op);
    read_success = LoadFix(read_success, m_minScore);
    read_success = LoadVar(read_success, m_gallery);
    return read_success;
}

bool Atomic_Hid_Async_Identify_With_Captured::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Async_Identify_With_Captured::UnpackResponseHid() {

    return true;
}

void Atomic_Hid_Async_Identify_With_Captured::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string galleryStr = to_str(m_gallery);

    msg << "op:"      << m_op       << "; ";
    msg << "score:"   << m_minScore << "; ";
    msg << "gallery:" << galleryStr << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Async_Identify_With_Captured::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFAsyncIdentifyWithCaptured(m_op, m_gallery.c_str(), m_minScore);

    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Async_Identify_With_Captured::Exec() {
}

#endif

//------------- + Atomic_Hid_Async_Match_With_Captured ----------------------//

Atomic_Hid_Async_Match_With_Captured::Atomic_Hid_Async_Match_With_Captured() {
    m_nCmd = CMD_HID_ASYNC_MATCH_WITH_CAPTURED;
}

Atomic_Hid_Async_Match_With_Captured::~Atomic_Hid_Async_Match_With_Captured() {

}

void Atomic_Hid_Async_Match_With_Captured::do_cleanup() {
}

void Atomic_Hid_Async_Match_With_Captured::SetParam(HFOperation op, const uint8_t* const bin, uint32_t len) {

    m_op = op;
    _load_array(m_data, bin, len);
}

bool Atomic_Hid_Async_Match_With_Captured::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_op);
    store_success = StoreVar(store_success, m_data);
    return store_success;
}

bool Atomic_Hid_Async_Match_With_Captured::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_op);
    read_success = LoadVar(read_success, m_data);
    return read_success;
}

bool Atomic_Hid_Async_Match_With_Captured::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Async_Match_With_Captured::UnpackResponseHid() {

    return true;
}

void Atomic_Hid_Async_Match_With_Captured::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string dataStr = to_str(m_data);

    msg << "op:"   << m_op    << "; ";
    msg << "data:" << dataStr << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Async_Match_With_Captured::Exec() {

    V100_ERROR_CODE status;
    HFData data;

    m_nRC = GEN_OK;

    data.size = 0;
    data.data = nullptr;

    if (m_data.size() > 0) {
        data.size = (uint32_t) m_data.size();
        data.data = &m_data[0];
    }

    status = JEngineExec::GetInstance()->Jag_HFAsyncMatchWithCaptured(m_op, &data);

    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Async_Match_With_Captured::Exec() {
}

#endif

//------------- + Atomic_Hid_Async_Identify_With_Template -------------------//

Atomic_Hid_Async_Identify_With_Template::Atomic_Hid_Async_Identify_With_Template() {
    m_nCmd = CMD_HID_ASYNC_IDENTIFY_WITH_TEMPLATE;
}

Atomic_Hid_Async_Identify_With_Template::~Atomic_Hid_Async_Identify_With_Template() {

}

void Atomic_Hid_Async_Identify_With_Template::SetParams(HFContext ctx, const char* const gal_ptr, double minScore, const uint8_t* const templ_ptr, uint32_t teml_len) {

    m_ctx       = ctx;
    m_minScore  = minScore;
    
    m_gal.clear();
    if (nullptr != gal_ptr) {
        m_gal = gal_ptr;
    }

    _load_array(m_templ, templ_ptr, teml_len);
}

void Atomic_Hid_Async_Identify_With_Template::GetValue(HFOperation& val) {
    val = m_operation;
}

void Atomic_Hid_Async_Identify_With_Template::do_cleanup() {
    m_ctx       = 0;
    m_minScore  = 0;
    m_operation = 0;
    m_templ.clear();
    m_gal.clear();
}

bool Atomic_Hid_Async_Identify_With_Template::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_minScore);
    store_success = StoreVar(store_success, m_gal);
    store_success = StoreVar(store_success, m_templ);
    return store_success;
}

bool Atomic_Hid_Async_Identify_With_Template::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_minScore);
    read_success = LoadVar(read_success, m_gal);
    read_success = LoadVar(read_success, m_templ);
    return read_success;
}

bool Atomic_Hid_Async_Identify_With_Template::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_operation);
    return store_success;
}

bool Atomic_Hid_Async_Identify_With_Template::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_operation);
    return read_success;
}

void Atomic_Hid_Async_Identify_With_Template::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string galleryStr = to_str(m_gal);
    std::string templStr   = to_str(m_templ);

    msg << "ctx:"       << m_ctx        << "; ";
    msg << "score:"     << m_minScore   << "; ";
    msg << "operation:" << m_operation  << "; ";
    msg << "gal:"       << galleryStr   << "; ";
    msg << "template:"  << templStr     << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Async_Identify_With_Template::Exec() {

    V100_ERROR_CODE status;
    HFData data = {0};
    const char* gal_ptr = nullptr;

    m_nRC = GEN_OK;

    gal_ptr = m_gal.c_str();

    if (m_templ.size() > 0) {
        data.data = &m_templ[0];
        data.size = (uint32_t)m_templ.size();
    }

    status = JEngineExec::GetInstance()->Jag_HFAsyncIdentifyWithTemplate(m_ctx, gal_ptr, m_minScore, &data, &m_operation);

    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Async_Identify_With_Template::Exec() {
}

#endif

//------------- + Atomic_Hid_Async_Verify_With_Template ---------------------//

Atomic_Hid_Async_Verify_With_Template::Atomic_Hid_Async_Verify_With_Template() {
    m_nCmd = CMD_HID_ASYNC_VERIFY_WITH_TEMPLATE;
}

Atomic_Hid_Async_Verify_With_Template::~Atomic_Hid_Async_Verify_With_Template() {

}

void Atomic_Hid_Async_Verify_With_Template::do_cleanup() {
    m_ctx       = 0;
    m_minScore  = 0;
    m_operation = 0;
    m_gal.clear();
    m_id.clear();
    m_templ.clear();
}

void Atomic_Hid_Async_Verify_With_Template::SetParams(HFContext ctx, double minScore, const char* const gal_ptr, const char* const id_ptr, const uint8_t* const templ_ptr, uint32_t templ_len) {

    m_ctx = ctx;
    m_minScore = minScore;

    m_gal.clear();
    if (nullptr != gal_ptr) {
        m_gal = gal_ptr;
    }

    m_id.clear();
    if (nullptr != id_ptr) {
        m_id = id_ptr;
    }

    _load_array(m_templ, templ_ptr, templ_len);
}

void Atomic_Hid_Async_Verify_With_Template::GetValue(HFOperation& val) {
    val = m_operation;
}

bool Atomic_Hid_Async_Verify_With_Template::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_minScore);
    store_success = StoreVar(store_success, m_gal);
    store_success = StoreVar(store_success, m_id);
    store_success = StoreVar(store_success, m_templ);
    return store_success;
}

bool Atomic_Hid_Async_Verify_With_Template::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_minScore);
    read_success = LoadVar(read_success, m_gal);
    read_success = LoadVar(read_success, m_id);
    read_success = LoadVar(read_success, m_templ);
    return read_success;
}

bool Atomic_Hid_Async_Verify_With_Template::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_operation);
    return store_success;
}

bool Atomic_Hid_Async_Verify_With_Template::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_operation);
    return read_success;
}

void Atomic_Hid_Async_Verify_With_Template::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string  galStr   = to_str(m_gal);
    std::string  idStr    = to_str(m_id);
    std::string  templStr = to_str(m_templ);

    msg << "ctx:"       << m_ctx        << "; ";
    msg << "score:"     << m_minScore   << "; ";
    msg << "operation:" << m_operation  << "; ";
    msg << "gal:"       << galStr       << "; ";
    msg << "id:"        << idStr        << "; ";
    msg << "template:"  << templStr     << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Async_Verify_With_Template::Exec() {

    V100_ERROR_CODE status;
    HFData data = {0};

    m_nRC = GEN_OK;

    const char* gal_ptr = m_gal.c_str();
    const char* id_ptr  = m_id.c_str();

    if (m_templ.size() > 0) {
        data.size = (uint32_t) m_templ.size();
        data.data = &m_templ[0];
    }

    status = JEngineExec::GetInstance()->Jag_HFAsyncVerifyWithTemplate(m_ctx, gal_ptr, id_ptr, m_minScore, &data, &m_operation);

    if (GEN_OK != status) {
        m_nRC = status;
    }

}

#else

void Atomic_Hid_Async_Verify_With_Template::Exec() {
}

#endif

//------------- + Atomic_Hid_Parse_Res_Int ----------------------------------//

Atomic_Hid_Parse_Res_Int::Atomic_Hid_Parse_Res_Int() {
    m_nCmd = CMD_HID_PARSE_RES_INT;
}

Atomic_Hid_Parse_Res_Int::~Atomic_Hid_Parse_Res_Int() {

}

void Atomic_Hid_Parse_Res_Int::do_cleanup() {

    m_id = 0;
    m_val = 0;
}

void Atomic_Hid_Parse_Res_Int::SetId(uint32_t id) {
    m_id = id;
}

bool Atomic_Hid_Parse_Res_Int::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_id);
    return store_success;
}

bool Atomic_Hid_Parse_Res_Int::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_id);
    return read_success;
}

bool Atomic_Hid_Parse_Res_Int::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_val);
    return store_success;
}

bool Atomic_Hid_Parse_Res_Int::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_val);
    return read_success;
}

void Atomic_Hid_Parse_Res_Int::dump(std::stringstream& msg) {

    msg.str().clear();

    msg << "id:"  << m_id  << "; ";
    msg << "val:" << m_val << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Parse_Res_Int::Exec() {

    m_nRC = GEN_ERROR_JAG_NOT_IMPLEMENTED;
}

#else

void Atomic_Hid_Parse_Res_Int::Exec() {
}

#endif

//------------- + Atomic_Hid_Parse_Res_Double -------------------------------//

Atomic_Hid_Parse_Res_Double::Atomic_Hid_Parse_Res_Double() {
    m_nCmd = CMD_HID_PARSE_RES_DOUBLE;
}

Atomic_Hid_Parse_Res_Double::~Atomic_Hid_Parse_Res_Double() {

}

void Atomic_Hid_Parse_Res_Double::do_cleanup() {

    m_id = 0;
    m_val = 0;
}

void Atomic_Hid_Parse_Res_Double::SetId(uint32_t id) {
    m_id = id;
}

bool Atomic_Hid_Parse_Res_Double::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_id);
    return store_success;
}

bool Atomic_Hid_Parse_Res_Double::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_id);
    return read_success;
}

bool Atomic_Hid_Parse_Res_Double::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_val);
    return store_success;
}

bool Atomic_Hid_Parse_Res_Double::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_val);
    return read_success;
}

void Atomic_Hid_Parse_Res_Double::dump(std::stringstream& msg) {

    msg.str().clear();

    msg << "id:"  << m_id  << "; ";
    msg << "val:" << m_val << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Parse_Res_Double::Exec() {

    m_nRC = GEN_ERROR_JAG_NOT_IMPLEMENTED;
}

#else

void Atomic_Hid_Parse_Res_Double::Exec() {
}

#endif

//------------- + Atomic_Hid_Parse_Res_Data ---------------------------------//

Atomic_Hid_Parse_Res_Data::Atomic_Hid_Parse_Res_Data() {
    m_nCmd = CMD_HID_PARSE_RES_DATA;
}

Atomic_Hid_Parse_Res_Data::~Atomic_Hid_Parse_Res_Data() {

}

void Atomic_Hid_Parse_Res_Data::do_cleanup() {
    m_id = 0;
    m_val.clear();
}

void Atomic_Hid_Parse_Res_Data::SetId(int id) {

    m_id = id;
}

bool Atomic_Hid_Parse_Res_Data::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_id);
    return store_success;
}

bool Atomic_Hid_Parse_Res_Data::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_id);
    return read_success;
}

bool Atomic_Hid_Parse_Res_Data::PackResponseHid() {
    bool store_success = true;
    store_success = StoreVar(store_success, m_val);
    return store_success;
}

bool Atomic_Hid_Parse_Res_Data::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadVar(read_success, m_val);
    return read_success;
}

void Atomic_Hid_Parse_Res_Data::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string valStr = to_str(m_val);

    msg << "id:"  << m_id   << "; ";
    msg << "val:" << valStr << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Parse_Res_Data::Exec() {

    m_nRC = GEN_ERROR_JAG_NOT_IMPLEMENTED;
}

#else

void Atomic_Hid_Parse_Res_Data::Exec() {
}

#endif

//------------- + Atomic_Hid_Parse_Res_Point --------------------------------//

Atomic_Hid_Parse_Res_Point::Atomic_Hid_Parse_Res_Point() {
    m_nCmd = CMD_HID_PARSE_RES_POINT;
}

Atomic_Hid_Parse_Res_Point::~Atomic_Hid_Parse_Res_Point() {

}

void Atomic_Hid_Parse_Res_Point::do_cleanup() {

}

void Atomic_Hid_Parse_Res_Point::SetId(int id) {

    m_id    = id;
    m_x     = 0;
    m_y     = 0;
}

bool Atomic_Hid_Parse_Res_Point::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_id);
    return store_success;
}

bool Atomic_Hid_Parse_Res_Point::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_id);
    return read_success;
}

bool Atomic_Hid_Parse_Res_Point::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_x);
    store_success = StoreFix(store_success, m_y);
    return store_success;
}

bool Atomic_Hid_Parse_Res_Point::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_x);
    read_success = LoadFix(read_success, m_y);
    return read_success;
}

void Atomic_Hid_Parse_Res_Point::dump(std::stringstream& msg) {

    msg.str().clear();

    msg << "id:"  << m_id  << "; ";
    msg << "x:"   << m_x   << "; ";
    msg << "y:"   << m_y   << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Parse_Res_Point::Exec() {

    m_nRC = GEN_ERROR_JAG_NOT_IMPLEMENTED;
}

#else

void Atomic_Hid_Parse_Res_Point::Exec() {
}

#endif

//------------- + Atomic_Hid_Parse_Res_Image --------------------------------//

Atomic_Hid_Parse_Res_Image::Atomic_Hid_Parse_Res_Image() {
    m_nCmd = CMD_HID_PARSE_RES_IMAGE;
}

Atomic_Hid_Parse_Res_Image::~Atomic_Hid_Parse_Res_Image() {

}

void Atomic_Hid_Parse_Res_Image::do_cleanup() {

    m_id = 0;
    m_encoding = HFImageEncoding::HFIMAGE_ENCODING_MAX;
    m_image.clear();
}

void Atomic_Hid_Parse_Res_Image::SetId(int id) {

    m_id = id;
}

bool Atomic_Hid_Parse_Res_Image::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_id);
    return store_success;
}

bool Atomic_Hid_Parse_Res_Image::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_id);
    return read_success;
}

bool Atomic_Hid_Parse_Res_Image::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_encoding);
    store_success = StoreVar(store_success, m_image);
    return store_success;
}

bool Atomic_Hid_Parse_Res_Image::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_encoding);
    read_success = LoadVar(read_success, m_image);
    return read_success;
}

void Atomic_Hid_Parse_Res_Image::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string imageStr = to_str(m_image);

    msg << "id:"        << m_id       << "; ";
    msg << "encoding:"  << m_encoding << "; ";
    msg << "image:"     << imageStr   << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Parse_Res_Image::Exec() {

    m_nRC = GEN_ERROR_JAG_NOT_IMPLEMENTED;
}

#else

void Atomic_Hid_Parse_Res_Image::Exec() {
}

#endif

//------------- + Atomic_Hid_Parse_Match_Gallery ----------------------------//

Atomic_Hid_Parse_Match_Gallery::Atomic_Hid_Parse_Match_Gallery() {
    m_nCmd = CMD_HID_PARSE_MATCH_GALLERY;
}

Atomic_Hid_Parse_Match_Gallery::~Atomic_Hid_Parse_Match_Gallery() {

}

void Atomic_Hid_Parse_Match_Gallery::do_cleanup() {

    m_match_list.clear();
    m_id = 0;
}

void Atomic_Hid_Parse_Match_Gallery::SetId(int id) {

    m_id = id;
}

bool Atomic_Hid_Parse_Match_Gallery::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_id);
    return store_success;
}

bool Atomic_Hid_Parse_Match_Gallery::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_id);
    return read_success;
}

bool Atomic_Hid_Parse_Match_Gallery::PackResponseHid() {

    // Not implemented yet.
    assert(false);
    return true;
}

bool Atomic_Hid_Parse_Match_Gallery::UnpackResponseHid() {

    // Not implemented yet.
    assert(false);
    return true;
}

void Atomic_Hid_Parse_Match_Gallery::dump(std::stringstream& msg) {

    msg.str().clear();

    // not implemented 
    assert(false);
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Parse_Match_Gallery::Exec() {

    m_nRC = GEN_ERROR_JAG_NOT_IMPLEMENTED;
}

#else

void Atomic_Hid_Parse_Match_Gallery::Exec() {
}

#endif

//------------- + Atomic_Hid_Get_Video_Frame --------------------------------//

Atomic_Hid_Get_Video_Frame::Atomic_Hid_Get_Video_Frame() {
    m_nCmd = CMD_HID_GET_VIDEO_FRAME;
}

Atomic_Hid_Get_Video_Frame::~Atomic_Hid_Get_Video_Frame() {

}

void Atomic_Hid_Get_Video_Frame::do_cleanup() {
    m_ctx      = 0;
    m_seqIn    = 0;
    m_seqOut   = 0;
    m_encoding = HFImageEncoding::HFIMAGE_ENCODING_MAX;
    m_image.clear();
}

void Atomic_Hid_Get_Video_Frame::SetParam(HFContext ctx, int64_t sec) {
    m_ctx   = ctx;
    m_seqIn = sec;
}

void Atomic_Hid_Get_Video_Frame::GetValue(int64_t& seq, HFImageEncoding& encoding, array_bin_t& data_bin) {
    seq      = m_seqOut;
    encoding = m_encoding;
    data_bin = m_image;
}

bool Atomic_Hid_Get_Video_Frame::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_ctx);
    store_success = StoreFix(store_success, m_seqIn);
    return store_success;
}

bool Atomic_Hid_Get_Video_Frame::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_ctx);
    read_success = LoadFix(read_success, m_seqIn);
    return read_success;
}

bool Atomic_Hid_Get_Video_Frame::PackResponseHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_seqOut);
    store_success = StoreFix(store_success, m_encoding);
    store_success = StoreVar(store_success, m_image);
    return store_success;
}

bool Atomic_Hid_Get_Video_Frame::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_seqOut);
    read_success = LoadFix(read_success, m_encoding);
    read_success = LoadVar(read_success, m_image);
    return read_success;
}

void Atomic_Hid_Get_Video_Frame::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string imageStr = to_str(m_image);

    msg << "ctx:"        << m_ctx       << "; ";
    msg << "seqIn:"      << m_seqIn     << "; ";
    msg << "seqOut:"     << m_seqOut    << "; ";
    msg << "encoding:"   << m_encoding  << "; ";
    msg << "image:"      << imageStr    << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Get_Video_Frame::Exec() {

    V100_ERROR_CODE status;
    HFImage*        img_ptr = nullptr;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFGetVideoFrame(m_ctx, m_seqIn, &img_ptr, &m_seqOut);

    if (GEN_OK != status) {
        m_nRC = status;
        return;
    }

    if (nullptr != img_ptr) {
        m_encoding = img_ptr->imageEncoding;
        _load_array(m_image, img_ptr->data.data, img_ptr->data.size);
        HFFree(img_ptr);
    }
}

#else

void Atomic_Hid_Get_Video_Frame::Exec() {
}

#endif

//------------- + Atomic_Hid_Get_Intermediate_Res ---------------------------//

Atomic_Hid_Get_Intermediate_Res::Atomic_Hid_Get_Intermediate_Res() {
    m_nCmd = CMD_HID_GET_INTERMEDIATE_RES;
}

Atomic_Hid_Get_Intermediate_Res::~Atomic_Hid_Get_Intermediate_Res() {

}

void Atomic_Hid_Get_Intermediate_Res::do_cleanup() {
    m_op       = 0;
    m_flags    = 0;
    m_sequence = 0;
}

void Atomic_Hid_Get_Intermediate_Res::SetParam(HFOperation operation, uint64_t resultFlags, int32_t lastSequenceNumber) {
    m_op = operation;
    m_flags = resultFlags;
    m_sequence = lastSequenceNumber;
}

void Atomic_Hid_Get_Intermediate_Res::GetValue(v100_hfres_t* result) {

    assert(nullptr != result);

    result->validFlags          = this->m_hf_res.validFlags;
    result->contextStatus       = this->m_hf_res.contextStatus;
    result->errorCode           = this->m_hf_res.errorCode;
    result->imageEncoding       = this->m_hf_res.imageEncoding;
    result->image               = this->m_hf_res.image;
    result->sequenceNumber      = this->m_hf_res.sequenceNumber;
    result->facesDetectedCount  = this->m_hf_res.facesDetectedCount;
    result->quality             = this->m_hf_res.quality;
    result->templ               = this->m_hf_res.templ;
    result->boundBox            = this->m_hf_res.boundBox;
    result->spoofProbability    = this->m_hf_res.spoofProbability;
    result->matches             = this->m_hf_res.matches;
    result->isCaptured          = this->m_hf_res.isCaptured;
    result->landmarks           = this->m_hf_res.landmarks;
}

bool Atomic_Hid_Get_Intermediate_Res::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_op);
    store_success = StoreFix(store_success, m_flags);
    store_success = StoreFix(store_success, m_sequence);
    return store_success;
}

bool Atomic_Hid_Get_Intermediate_Res::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_op);
    read_success = LoadFix(read_success, m_flags);
    read_success = LoadFix(read_success, m_sequence);
    return read_success;
}

bool Atomic_Hid_Get_Intermediate_Res::PackResponseHid() {
    return store_hf_res();
}

bool Atomic_Hid_Get_Intermediate_Res::UnpackResponseHid() {
    return load_hf_res();
}

void Atomic_Hid_Get_Intermediate_Res::dump(std::stringstream& msg) {

    msg.str().clear();

    msg << "op:"       << m_op       << "; ";
    msg << "flags:"    << m_flags    << "; ";
    msg << "sequence:" << m_sequence << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Get_Intermediate_Res::Exec() {

    V100_ERROR_CODE status;
    HFData* val_ptr = nullptr;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFGetIntermediateResult(m_op, m_flags, m_sequence, &val_ptr);

    if (GEN_OK != status) {
        m_nRC = status;
    }

    if (nullptr != val_ptr) {
        if (nullptr != val_ptr->data) {
            process_hf_res(val_ptr->data);
        }
        JEngineExec::GetInstance()->Jag_Free(val_ptr);
    }
}

#else

void Atomic_Hid_Get_Intermediate_Res::Exec() {
}

#endif

//------------- + Atomic_Hid_Get_Final_Res ----------------------------------//

Atomic_Hid_Get_Final_Res::Atomic_Hid_Get_Final_Res() {
    m_nCmd = CMD_HID_GET_FINAL_RES;
}

Atomic_Hid_Get_Final_Res::~Atomic_Hid_Get_Final_Res() {

}

void Atomic_Hid_Get_Final_Res::do_cleanup() {
    m_op    = 0;
    m_flags = 0;
}

void Atomic_Hid_Get_Final_Res::SetParam(HFOperation operation, uint64_t resultFlags) {
    m_op = operation;
    m_flags = resultFlags;
}

void Atomic_Hid_Get_Final_Res::GetValue(v100_hfres_t* result) {

    assert(nullptr != result);

    result->validFlags = this->m_hf_res.validFlags;
    result->contextStatus = this->m_hf_res.contextStatus;
    result->errorCode = this->m_hf_res.errorCode;
    result->imageEncoding = this->m_hf_res.imageEncoding;
    result->image = this->m_hf_res.image;
    result->sequenceNumber = this->m_hf_res.sequenceNumber;
    result->facesDetectedCount = this->m_hf_res.facesDetectedCount;
    result->quality = this->m_hf_res.quality;
    result->templ = this->m_hf_res.templ;
    result->boundBox = this->m_hf_res.boundBox;
    result->spoofProbability = this->m_hf_res.spoofProbability;
    result->matches = this->m_hf_res.matches;
    result->isCaptured = this->m_hf_res.isCaptured;
    result->landmarks = this->m_hf_res.landmarks;
}

bool Atomic_Hid_Get_Final_Res::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_op);
    store_success = StoreFix(store_success, m_flags);
    return store_success;
}

bool Atomic_Hid_Get_Final_Res::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_op);
    read_success = LoadFix(read_success, m_flags);
    return read_success;
}

bool Atomic_Hid_Get_Final_Res::PackResponseHid() {
    return store_hf_res();
}

bool Atomic_Hid_Get_Final_Res::UnpackResponseHid() {
    return load_hf_res();
}

void Atomic_Hid_Get_Final_Res::dump(std::stringstream& msg) {

    msg.str().clear();

    msg << "op:"    << m_op    << "; ";
    msg << "flags:" << m_flags << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Get_Final_Res::Exec() {

    V100_ERROR_CODE status;
    HFData* val_ptr = nullptr;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFGetFinalResult(m_op, m_flags, &val_ptr);

    if (GEN_OK != status) {
        m_nRC = status;
    }

    if (nullptr != val_ptr) {
        if (nullptr != val_ptr->data) {
            process_hf_res(val_ptr->data);
        }
        JEngineExec::GetInstance()->Jag_Free(val_ptr);
    }
}

#else

void Atomic_Hid_Get_Final_Res::Exec() {
}

#endif

//------------- + Atomic_Hid_Db_Add_Record_With_Captured --------------------//

Atomic_Hid_Db_Add_Record_With_Captured::Atomic_Hid_Db_Add_Record_With_Captured() {
    m_nCmd = CMD_HID_DB_ADD_RECORD_WITH_CAPTURED;
}

Atomic_Hid_Db_Add_Record_With_Captured::~Atomic_Hid_Db_Add_Record_With_Captured() {

}

void Atomic_Hid_Db_Add_Record_With_Captured::do_cleanup() {
    m_op = 0;
    m_replace = false;
    m_id.clear();
    m_gallery.clear();
    m_custom.clear();
}

void Atomic_Hid_Db_Add_Record_With_Captured::SetParam(HFOperation op, bool replace, const char* const id, const char* const gallery, const char* const custom) {

    do_cleanup();

    m_op = op;
    m_replace = replace;

    if ( nullptr != id) {
        m_id = id;
    }

    if (nullptr != gallery) {
        m_gallery = gallery;
    }

    if (nullptr != custom) {
        m_custom = custom;
    }
}

bool Atomic_Hid_Db_Add_Record_With_Captured::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_op);
    store_success = StoreFix(store_success, m_replace);
    store_success = StoreVar(store_success, m_id);
    store_success = StoreVar(store_success, m_gallery);
    store_success = StoreVar(store_success, m_custom);
    return store_success;
}

bool Atomic_Hid_Db_Add_Record_With_Captured::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_op);
    read_success = LoadFix(read_success, m_replace);
    read_success = LoadVar(read_success, m_id);
    read_success = LoadVar(read_success, m_gallery);
    read_success = LoadVar(read_success, m_custom);
    return read_success;
}

bool Atomic_Hid_Db_Add_Record_With_Captured::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Db_Add_Record_With_Captured::UnpackResponseHid() {

    return true;
}

void Atomic_Hid_Db_Add_Record_With_Captured::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string idStr       = to_str(m_id);
    std::string galleryStr  = to_str(m_gallery);
    std::string customStr   = to_str(m_custom);

    msg << "operation:"  << m_op       << "; ";
    msg << "replace:"    << m_replace  << "; ";
    msg << "id:"         << idStr      << "; ";
    msg << "gallery:"    << galleryStr << "; ";
    msg << "custom:"     << customStr  << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Db_Add_Record_With_Captured::Exec() {

    V100_ERROR_CODE status;
    HFDatabaseRecordHeader dbRec;

    dbRec.recordId   = m_id.c_str();
    dbRec.galleryID  = m_gallery.c_str();
    dbRec.customData = m_custom.c_str();

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFAddRecordWithCaptured(m_op, &dbRec, m_replace);

    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Db_Add_Record_With_Captured::Exec() {
}

#endif

//------------- + Atomic_Hid_Db_Add_Record_With_Template --------------------//

Atomic_Hid_Db_Add_Record_With_Template::Atomic_Hid_Db_Add_Record_With_Template() {
    m_nCmd = CMD_HID_DB_ADD_RECORD_WITH_TEMPLATE;
}

Atomic_Hid_Db_Add_Record_With_Template::~Atomic_Hid_Db_Add_Record_With_Template() {

}

void Atomic_Hid_Db_Add_Record_With_Template::do_cleanup() {
    m_replace = false;
    m_id.clear();
    m_gallery.clear();
    m_custom.clear();
    m_image.clear();
}

void Atomic_Hid_Db_Add_Record_With_Template::SetParam(bool replace, const char* const id, const char* const gallery, const char* const custom, const uint8_t* const data_ptr, uint32_t data_len) {

    do_cleanup();

    m_replace = replace;

    if (nullptr != id) {
        m_id = id;
    }

    if (nullptr != gallery) {
        m_gallery = gallery;
    }

    if (nullptr != custom) {
        m_custom = custom;
    }

    _load_array (m_image, data_ptr, data_len);
}

bool Atomic_Hid_Db_Add_Record_With_Template::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreFix(store_success, m_replace);
    store_success = StoreVar(store_success, m_id);
    store_success = StoreVar(store_success, m_gallery);
    store_success = StoreVar(store_success, m_custom);
    store_success = StoreVar(store_success, m_image);
    return store_success;
}

bool Atomic_Hid_Db_Add_Record_With_Template::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadFix(read_success, m_replace);
    read_success = LoadVar(read_success, m_id);
    read_success = LoadVar(read_success, m_gallery);
    read_success = LoadVar(read_success, m_custom);
    read_success = LoadVar(read_success, m_image);
    return read_success;
}

bool Atomic_Hid_Db_Add_Record_With_Template::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Db_Add_Record_With_Template::UnpackResponseHid() {

    return true;
}

void Atomic_Hid_Db_Add_Record_With_Template::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string idStr       = to_str(m_id);
    std::string galleryStr  = to_str(m_gallery);
    std::string customStr   = to_str(m_custom);
    std::string imageStr    = to_str(m_image);

    msg << "replace:"    << m_replace   << "; ";
    msg << "gallery:"    << galleryStr  << "; ";
    msg << "custom:"     << customStr   << "; ";
    msg << "image:"      << imageStr    << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Db_Add_Record_With_Template::Exec() {

    V100_ERROR_CODE   status;
    HFDatabaseRecord  dbRec;

    m_nRC = GEN_OK;

    dbRec.header.recordId   = m_id.c_str();
    dbRec.header.galleryID  = m_gallery.c_str();
    dbRec.header.customData = m_custom.c_str();
    dbRec.templ.data        = nullptr;
    dbRec.templ.size        = 0;

    if (m_image.size() > 0) {
        dbRec.templ.data = &m_image[0];
        dbRec.templ.size = (uint32_t) m_image.size();
    }

    status = JEngineExec::GetInstance()->Jag_HFAddRecordWithTemplate( &dbRec, m_replace);

    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Db_Add_Record_With_Template::Exec() {
}

#endif

//------------- + Atomic_Hid_Get_Record -------------------------------------//

Atomic_Hid_Db_Get_Record::Atomic_Hid_Db_Get_Record() {
    m_nCmd = CMD_HID_DB_GET_RECORD;
}

Atomic_Hid_Db_Get_Record::~Atomic_Hid_Db_Get_Record() {

}

void Atomic_Hid_Db_Get_Record::do_cleanup() {
    m_id.clear();
    m_gallery.clear();
    m_custom.clear();
    m_data.clear();
}

void Atomic_Hid_Db_Get_Record::SetParam(const char* const id, const char* const gallery) {

    m_id.clear();
    if (nullptr != id) {
        m_id = id;
    }

    m_gallery.clear();
    if (nullptr != gallery) {
        m_gallery = gallery;
    }
}

void Atomic_Hid_Db_Get_Record::GetValue(array_bin_t& data, array_str_t& custom) {
    data   = m_data;
    custom = m_custom;
}

bool Atomic_Hid_Db_Get_Record::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreVar(store_success, m_id);
    store_success = StoreVar(store_success, m_gallery);
    return store_success;
}

bool Atomic_Hid_Db_Get_Record::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadVar(read_success, m_id);
    read_success = LoadVar(read_success, m_gallery);
    return read_success;
}

bool Atomic_Hid_Db_Get_Record::PackResponseHid() {
    bool store_success = true;
    store_success = StoreVar(store_success, m_data);
    return store_success;
}

bool Atomic_Hid_Db_Get_Record::UnpackResponseHid() {
    bool read_success = true;
    read_success = LoadVar(read_success, m_data);
    return read_success;
}

void Atomic_Hid_Db_Get_Record::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string idStr      = to_str(m_id);
    std::string galleryStr = to_str(m_gallery);
    std::string customStr  = to_str(m_custom);
    std::string dataStr    = to_str(m_data);

    msg << "id:"      << idStr       << "; ";
    msg << "gallery:" << galleryStr  << "; ";
    msg << "custom:"  << customStr   << "; ";
    msg << "data:"    << dataStr     << "; ";
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Db_Get_Record::Exec() {

    V100_ERROR_CODE status;
    HFDatabaseRecord* rec_ptr = nullptr;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFGetRecord(m_id.c_str(), m_gallery.c_str(), &rec_ptr);

    if (GEN_OK != status) {
        m_nRC = status;
        return;
    }

    do_cleanup();

    if (nullptr != rec_ptr) {
        
        if (nullptr != rec_ptr->header.recordId) {
            m_id = rec_ptr->header.recordId;
        }

        if (nullptr != rec_ptr->header.galleryID) {
            m_gallery = rec_ptr->header.galleryID;
        }

        if (nullptr != rec_ptr->header.customData) {
            m_custom = rec_ptr->header.customData;
        }

        if (nullptr != rec_ptr->templ.data) {
            _load_array(m_data, rec_ptr->templ.data, rec_ptr->templ.size);
        }

        JEngineExec::GetInstance()->Jag_Free(rec_ptr);
    }

}

#else

void Atomic_Hid_Db_Get_Record::Exec() {
}

#endif

//------------- + Atomic_Hid_Del_Record -------------------------------------//

Atomic_Hid_Db_Del_Record::Atomic_Hid_Db_Del_Record() {
    m_nCmd = CMD_HID_DB_DEL_RECORD;
}

Atomic_Hid_Db_Del_Record::~Atomic_Hid_Db_Del_Record() {

}

void Atomic_Hid_Db_Del_Record::do_cleanup() {
    m_id.clear();
    m_gallery.clear();
}

void Atomic_Hid_Db_Del_Record::SetId(const char* const id, const char* const gallery) {
    m_id = id;
    m_gallery = gallery;
}

bool Atomic_Hid_Db_Del_Record::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreVar(store_success, m_id);
    store_success = StoreVar(store_success, m_gallery);
    return store_success;
}

bool Atomic_Hid_Db_Del_Record::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadVar(read_success, m_id);
    read_success = LoadVar(read_success, m_gallery);
    return read_success;
}

bool Atomic_Hid_Db_Del_Record::PackResponseHid() {

    return true;
}

bool Atomic_Hid_Db_Del_Record::UnpackResponseHid() {

    return true;
}

void Atomic_Hid_Db_Del_Record::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string idStr      = to_str(m_id);
    std::string galleryStr = to_str(m_gallery);

    msg << "id:"      << idStr       << "; ";
    msg << "gallery:" << galleryStr  << "; ";

}

#if V100_IMPLEMENTATION

void Atomic_Hid_Db_Del_Record::Exec() {

    V100_ERROR_CODE status;

    m_nRC = GEN_OK;

    status = JEngineExec::GetInstance()->Jag_HFDelRecords(m_id.c_str(), m_gallery.c_str());

    if (GEN_OK != status) {
        m_nRC = status;
    }
}

#else

void Atomic_Hid_Db_Del_Record::Exec() {
}

#endif

//------------- + Atomic_Hid_List_Records -----------------------------------//

Atomic_Hid_Db_List_Records::Atomic_Hid_Db_List_Records() {
    m_nCmd = CMD_HID_DB_LIST_RECORDS;
}

Atomic_Hid_Db_List_Records::~Atomic_Hid_Db_List_Records() {

}

void Atomic_Hid_Db_List_Records::do_cleanup() {
    m_gallery.clear();
    m_list.clear();
}

void Atomic_Hid_Db_List_Records::SetId(const char* const gallery) {

    m_gallery = gallery;
}

void Atomic_Hid_Db_List_Records::GetParams(string_list_t& list) {

    list = m_list;
}

bool Atomic_Hid_Db_List_Records::PackChallengeHid() {
    bool store_success = true;
    store_success = StoreVar(store_success, m_gallery);
    return store_success;
}

bool Atomic_Hid_Db_List_Records::UnpackChallengeHid() {
    bool read_success = true;
    read_success = LoadVar(read_success, m_gallery);
    return read_success;
}

bool Atomic_Hid_Db_List_Records::PackResponseHid() {

    uint32_t cnt;
    bool store_success = true;

    cnt = (uint32_t) m_list.size();

    store_success = StoreArray(store_success, cnt);

    for (uint32_t i = 0; i < cnt; i++) {
        store_success = StoreVar(store_success, m_list[i]);
    }

    return store_success;
}

bool Atomic_Hid_Db_List_Records::UnpackResponseHid() {

    bool        load_success = true;
    uint32_t    items_cnt;
    array_str_t db_item;

    m_list.clear();

    load_success = LoadArray(load_success, items_cnt);

    if (load_success) {
        for (uint32_t i = 0; i < items_cnt; i++) {
            load_success = LoadVar(load_success, db_item);
            m_list.push_back(db_item);
        }
    }

    return true;
}

void Atomic_Hid_Db_List_Records::dump(std::stringstream& msg) {

    msg.str().clear();

    std::string galleryStr = to_str(m_gallery);

    msg << "gallery:" << galleryStr << "; ";

    for (size_t i=0; i< m_list.size(); i++) {
        msg << "id:"  << i << "; val: " << m_list[i] << "; ";
    }
}

#if V100_IMPLEMENTATION

void Atomic_Hid_Db_List_Records::Exec() {

    V100_ERROR_CODE status = GEN_ERROR_PROCESSING;
    HFStringArray* ids_list = nullptr;

    m_nRC = GEN_OK;
    m_list.clear();

    status = JEngineExec::GetInstance()->Jag_HFListRecords(m_gallery.c_str(), &ids_list);

    if (GEN_OK != status) {
        m_nRC = status;
        return;
    }

    if (nullptr != ids_list) {
        for (uint32_t i = 0; i < ids_list->stringsCount; i++) {
            char* name = ids_list->strings[i];
            if (nullptr != name) {
                m_list.push_back(name);
            }
        }
        JEngineExec::GetInstance()->Jag_Free(ids_list);
    }
}

#else

void Atomic_Hid_Db_List_Records::Exec() {
}

#endif


//------------- + Atomic_Hid_FwUpdate ---------------------------------------//

Atomic_Hid_FwUpdate::Atomic_Hid_FwUpdate() {
    m_nCmd = CMD_HID_FW_UPDATE;
}

Atomic_Hid_FwUpdate::~Atomic_Hid_FwUpdate() {

}

bool Atomic_Hid_FwUpdate::SetFileName(const char* const name) {

    bool ret_val = false;
    size_t fw_file_size;

    std::ifstream fw_file( name, std::ios::binary);

    if ( fw_file ) {

        fw_file.seekg(0, std::ios::end);
        fw_file_size = fw_file.tellg();
        fw_file.seekg(0, std::ios::beg);

        m_text.reserve (fw_file_size);

        m_text.assign(std::istreambuf_iterator<char>(fw_file), std::istreambuf_iterator<char>());
        ret_val = true;
    }

    return ret_val;
}

void Atomic_Hid_FwUpdate::do_cleanup() {
    m_text.clear();
}

bool Atomic_Hid_FwUpdate::PackChallengeHid() {

    bool ret_val = false;

    try {

        const size_t store_pos = m_storage_.size();

        m_storage_.resize(m_storage_.size() + m_text.size());

        std::memcpy(&m_storage_[store_pos], m_text.data(), m_text.size());

        ret_val = true;

    } catch (...) {

    }

    return ret_val;
}

bool Atomic_Hid_FwUpdate::UnpackChallengeHid() {
    // do noting. 
    return true;
}

bool Atomic_Hid_FwUpdate::PackResponseHid() {
    // do nothing.
    return true;
}

bool Atomic_Hid_FwUpdate::UnpackResponseHid() {
    // do nothing.
    return true;
}

void Atomic_Hid_FwUpdate::Exec() {
    // do nothing.
    return;
}

void Atomic_Hid_FwUpdate::dump(std::stringstream& msg) {
    // do nothing.
    return;
}

//------------- + End_of_file -----------------------------------------------//
