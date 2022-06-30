#include <cassert>
#include <stdint.h>
#include <string.h>
#include <HFTypesPrivate.h>
#include <HFApiSimulation.h>
#include <VcomBaseTypes.h>

#define MEMBLOCK_MAGIC1   0x11335577
#define MEMBLOCK_MAGIC2   0x22446688
#define STR_PAD_LEN       3

//-------------------------------------------------------------------------------------------//

enum class MemType {
    MEM_BLOCK_UNKNOWN = 0x0,
    MEM_BLOCK_STR_LIST,
    MEM_BLOCK_IMG,
    MEM_BLOCK_STR,
    MEM_BLOCK_DATA,
    MEM_BLOCK_RESULT,
    MEM_BLOCK_END
};

typedef struct tag_mem_hdr {
    int                 magic1;
    MemType             type;
    int                 magic2;
}   Mem_Hdr;

typedef struct tag_mem_block_str_array {
    Mem_Hdr             hdr;
    HFStringArray       data;
}   Mem_StrArray;

typedef struct tag_mem_block_image {
    Mem_Hdr             hdr;
    HFImage             data;
}   Mem_Image;

typedef struct tag_mem_block_data {
    Mem_Hdr             hdr;
    HFData              data;
}   Mem_Data;

typedef struct tag_mem_string {
    Mem_Hdr             hdr;
    char                data[1];
}   Mem_String;


//-------------------------------------------------------------------------------------------//

uint8_t g_img[] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };
uint8_t g_tpl[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 };

static void* _alloc_block(MemType blockType, size_t blockLen) {

    void*    ret_val    = nullptr;
    size_t   alloc_len  = sizeof(Mem_Hdr) + blockLen + 1;
    uint8_t* alloc_ptr  = nullptr;

    alloc_ptr = new uint8_t[alloc_len];

    memset (alloc_ptr, 0xAA, alloc_len);

    if (nullptr != alloc_ptr) {

        Mem_Hdr* hdr = (Mem_Hdr*) (alloc_ptr);

        hdr->magic1 = MEMBLOCK_MAGIC1;
        hdr->magic2 = MEMBLOCK_MAGIC2;
        hdr->type   = blockType;

        ret_val = alloc_ptr + sizeof(Mem_Hdr);
    }

    return ret_val;
}

static void _fill_res(v100_hfres_t& hfRes) {

    static int val = 100;

    v100_match_rec_t matchRec;

    hfRes.validFlags = 0x0f | val++;
    hfRes.contextStatus = val++;
    hfRes.errorCode = val++;
    hfRes.imageEncoding = val++;

    hfRes.image.assign(g_img, g_img + sizeof(g_img));

    hfRes.sequenceNumber = val++;
    hfRes.facesDetectedCount = val++;
    hfRes.quality = val++;

    hfRes.templ.assign(g_tpl, g_tpl + sizeof(g_tpl));

    hfRes.boundBox.upperLeft.x = val++;
    hfRes.boundBox.upperLeft.y = val++;
    hfRes.boundBox.bottomRight.x = val++;
    hfRes.boundBox.bottomRight.y = val++;
    hfRes.spoofProbability = val++;

    hfRes.matches.clear();

    matchRec.recordId = "recr_00";
    matchRec.customData = "cust_00";
    matchRec.galleryID = "galr_00";
    matchRec.matchScore = 0.2;
    hfRes.matches.push_back(matchRec);

    matchRec.recordId = "recr_00_01";
    matchRec.customData = "cust_00_01";
    matchRec.galleryID = "galr_00_01";
    matchRec.matchScore = 0.3;
    hfRes.matches.push_back(matchRec);

    matchRec.recordId = "recr_00_01_02";
    matchRec.customData = "cust_00_01_02";
    matchRec.galleryID = "galr_00_01_02";
    matchRec.matchScore = 0.4;
    hfRes.matches.push_back(matchRec);

    matchRec.recordId = "recr_00_01_02_03";
    matchRec.customData = "cust_00_01_02_03";
    matchRec.galleryID = "galr_00_01_02_03";
    matchRec.matchScore = 0.5;
    hfRes.matches.push_back(matchRec);

    matchRec.recordId = "recr_00_01_02_03_04";
    matchRec.customData = "cust_00_01_02_03_04";
    matchRec.galleryID = "galr_00_01_02_03_04";
    matchRec.matchScore = 0.6;
    hfRes.matches.push_back(matchRec);

    hfRes.isCaptured = val++;
    hfRes.landmarks.leftEye.x = val++;
    hfRes.landmarks.leftEye.y = val++;
    hfRes.landmarks.rightEye.x = val++;
    hfRes.landmarks.rightEye.y = val++;
    hfRes.landmarks.nose.x = val++;
    hfRes.landmarks.nose.y = val++;
    hfRes.landmarks.mouthLeft.x = val++;
    hfRes.landmarks.mouthLeft.y = val++;
    hfRes.landmarks.mouthRight.x = val++;
    hfRes.landmarks.mouthRight.y = val++;
}

static HFData* _alloc_result(const v100_hfres_t& src) {

    size_t    alloc_len;
    size_t    matches_cnt;
    HFData*   ret_ptr = nullptr;

    matches_cnt = src.matches.size();

    alloc_len = sizeof(HFData);
    alloc_len += sizeof(HFResult);
    alloc_len += sizeof(HFMatchRecord) * matches_cnt;
    alloc_len += src.image.size() + 1;
    alloc_len += src.templ.size() + 1;

    for (size_t i = 0; i < matches_cnt; i++) {
        alloc_len += src.matches[i].recordId.length()   + STR_PAD_LEN;
        alloc_len += src.matches[i].galleryID.length()  + STR_PAD_LEN;
        alloc_len += src.matches[i].customData.length() + STR_PAD_LEN;
    }

    ret_ptr = (HFData*)_alloc_block(MemType::MEM_BLOCK_RESULT, alloc_len);
    ret_ptr->size = static_cast<uint32_t>(alloc_len);
    ret_ptr->data = &ret_ptr[1];

    return ret_ptr;
}

static void _copy_str(uint8_t** dst, const str_data_t& src) {

    if (src.length() > 0) {
        memcpy((*dst), &src[0], src.length() + 1);
        (*dst) += src.length();
    }

    (*dst) += STR_PAD_LEN;
}

static bool _conv_hr_res(const v100_hfres_t& src, HFData** result) {

    size_t          matches_cnt;
    HFData* hf_val = nullptr;
    HFResult* hf_res = nullptr;
    HFMatchRecord* match_ptr = nullptr;
    uint8_t* dst_ptr = nullptr;
    uint8_t* store_ptr = nullptr;

    assert(nullptr != result);

    hf_val = _alloc_result(src);

    if (nullptr == hf_val) {
        return false;
    }

    matches_cnt = src.matches.size();
    hf_res = static_cast<HFResult*> (hf_val->data);
    dst_ptr = static_cast<uint8_t*>  (hf_val->data);
    store_ptr = reinterpret_cast<uint8_t*>(dst_ptr);
    store_ptr += sizeof(HFResult);
    match_ptr = reinterpret_cast<HFMatchRecord*> (store_ptr);
    store_ptr += sizeof(HFMatchRecord) * matches_cnt;

    hf_res->validFlags = src.validFlags;
    hf_res->contextStatus = static_cast<HFStatus> (src.contextStatus);
    hf_res->errorCode = src.errorCode;
    hf_res->isCaptured = src.isCaptured;
    hf_res->quality = src.quality;
    hf_res->sequenceNumber = src.sequenceNumber;
    hf_res->facesDetectedCount = src.facesDetectedCount;
    hf_res->spoofProbability = src.spoofProbability;

    hf_res->landmarks.leftEye.x = src.landmarks.leftEye.x;
    hf_res->landmarks.leftEye.y = src.landmarks.leftEye.y;
    hf_res->landmarks.rightEye.x = src.landmarks.rightEye.x;
    hf_res->landmarks.rightEye.y = src.landmarks.rightEye.y;
    hf_res->landmarks.nose.x = src.landmarks.nose.x;
    hf_res->landmarks.nose.y = src.landmarks.nose.y;
    hf_res->landmarks.mouthLeft.x = src.landmarks.mouthLeft.x;
    hf_res->landmarks.mouthLeft.y = src.landmarks.mouthLeft.y;
    hf_res->landmarks.mouthRight.x = src.landmarks.mouthRight.x;
    hf_res->landmarks.mouthRight.y = src.landmarks.mouthRight.y;
    hf_res->boundBox.upperLeft.x = src.boundBox.upperLeft.x;
    hf_res->boundBox.upperLeft.y = src.boundBox.upperLeft.y;
    hf_res->boundBox.bottomRight.x = src.boundBox.bottomRight.x;
    hf_res->boundBox.bottomRight.y = src.boundBox.bottomRight.y;

    hf_res->image.imageEncoding = static_cast<HFImageEncoding> (src.imageEncoding);

    hf_res->matches.records = nullptr;
    hf_res->templ.data = nullptr;
    hf_res->image.data.data = nullptr;

    hf_res->matches.recordsCount = static_cast<uint32_t> (src.matches.size());
    hf_res->templ.size = static_cast<uint32_t> (src.templ.size());
    hf_res->image.data.size = static_cast<uint32_t> (src.image.size());

    if (hf_res->matches.recordsCount > 0) {

        hf_res->matches.records = reinterpret_cast<HFMatchRecord*> (match_ptr);

        for (uint32_t i = 0; i < hf_res->matches.recordsCount; i++) {

            hf_res->matches.records[i].header.recordId = reinterpret_cast<char*> (store_ptr);
            _copy_str(&store_ptr, src.matches[i].recordId);

            hf_res->matches.records[i].header.galleryID = reinterpret_cast<char*> (store_ptr);
            _copy_str(&store_ptr, src.matches[i].galleryID);

            hf_res->matches.records[i].header.customData = reinterpret_cast<char*> (store_ptr);
            _copy_str(&store_ptr, src.matches[i].customData);

            hf_res->matches.records[i].matchScore = src.matches[i].matchScore;
        }
    }

    if (hf_res->templ.size > 0) {
        hf_res->templ.data = store_ptr;
        memcpy(store_ptr, &src.templ[0], src.templ.size());
        store_ptr += src.templ.size() + 1;
    }

    if (hf_res->image.data.size > 0) {
        hf_res->image.data.data = store_ptr;
        memcpy(store_ptr, &src.image[0], src.image.size());
        store_ptr += src.image.size() + 1;
    }

    (*result) = hf_val;

    return true;
}

//-------------------------------------------------------------------------------------------//

template<typename T>
static int32_t VerifyResult(HFData *result, uint32_t parameter, T *value, HFResult **output) {
    int32_t error = HFERROR_GENERAL;
    // TODO: check correct allocation
    *output = reinterpret_cast<HFResult*>(reinterpret_cast<HFData*>(result)->data);
    error = HFERROR_OK;
    return error;
}

static void VerifyFlags(uint64_t valid, uint64_t flags) {
    assert((valid & flags) == flags);
}

//-------------------------------------------------------------------------------------------//

//-------------------------------------------------------------------------------------------//

//-------------------------------------------------------------------------------------------//

//-------------------------------------------------------------------------------------------//

extern "C" {

int32_t HFInit_() {
    return 0;
}

int32_t HFTerminate_() {
    return 0;
}

int32_t HFEnumerateCameras_(HFStringArray** cameras) {
    return 0;
}

int32_t HFOpenContext_(int32_t cameraId, HFAlgorithmType algorithmType, HFContext* context) {
    return 0;
}

int32_t HFCloseContext_(HFContext context) {
    return 0;
}

int32_t HFGetVideoFrame_(HFContext context, int64_t lastSequenceNumber, HFImage** image, int64_t* imageSequenceNumber) {
    return 0;
}

int32_t HFGetIntermediateResult_(HFOperation operation, uint64_t resultFlags, int32_t lastSequenceNumber, HFData** result) {

    v100_hfres_t v100_res;
    _fill_res(v100_res);
    _conv_hr_res(v100_res, result);

    return 0;
}

int32_t HFGetFinalResult_(HFOperation operation, uint64_t resultFlags, HFData** result) {
    return 0;
}

int32_t HFGetParamInt_(HFContext context, uint32_t parameter, int32_t* value) {
    return 0;
}

int32_t HFSetParamInt_(HFContext context, uint32_t parameter, int32_t value) {
    return 0;
}

int32_t HFGetParamDouble_(HFContext context, uint32_t parameter, double* value) {
    return 0;
}

int32_t HFSetParamDouble_(HFContext context, uint32_t parameter, double value) {
    return 0;
}

int32_t HFGetParamString_(HFContext context, uint32_t parameter, char** value) {
    return 0;
}

int32_t HFSetParamString_(HFContext context, uint32_t parameter, const char* value) {
    return 0;
}

int32_t HFGetParamData_(HFContext context, uint32_t parameter, HFData** value) {
    return 0;
}

int32_t HFSetParamData_(HFContext context, uint32_t parameter, const HFData* value) {
    return 0;
}

int32_t HFStopOperation_(HFOperation operation) {
    return 0;
}

int32_t HFCloseOperation_(HFOperation operation) {
    return 0;
}

int32_t HFAsyncStartCaptureImage_(HFContext context, int32_t timeout, double minimalQuality, double maximalSpoofProbability, uint64_t intermediateResultFlags, uint64_t finalResultFlags, HFOperation* operation) {
    return 0;
}

int32_t HFAsyncExtractTemplate_(HFContext context, const HFImage* image, uint64_t finalResultFlags, HFOperation* operation) {
    return 0;
}

int32_t HFAsyncVerifyWithCaptured_(HFOperation operation, const char* galleryID, const char* id, double minimalMatchScore) {
    return 0;
}

int32_t HFAsyncIdentifyWithCaptured_(HFOperation operation, const char* galleryID, double minimalMatchScore) {
    return 0;
}

int32_t HFAsyncMatchWithCaptured_(HFOperation operation, const HFData* templ) {
    return 0;
}

int32_t HFAsyncVerifyWithTemplate_(HFContext context, const char* galleryID, const char* id, double minimalMatchScore, const HFData* templ, HFOperation* operation) {
    return 0;
}

int32_t HFAsyncIdentifyWithTemplate_(HFContext context, const char* galleryID, double minimalMatchScore, const HFData* templ, HFOperation* operation) {
    return 0;
}

int32_t HFAsyncMatchWithTemplate_(HFContext context, const HFData* templA, const HFData* templB, HFOperation* operation) {
    return 0;
}

int32_t HFAddRecordWithCaptured_(HFOperation operation, const HFDatabaseRecordHeader* header, bool replaceIfExists) {
    return 0;
}

int32_t HFAddRecordWithTemplate_(HFDatabaseRecord* databaseRecord, bool replaceIfExists) {
    return 0;
}

int32_t HFDeleteRecord_(const char* id, const char* galleryID) {
    return 0;
}

int32_t HFGetRecord_(const char* id, const char* galleryID, HFDatabaseRecord** databaseRecord) {
    return 0;
}

int32_t HFListRecords_(const char* galleryID, HFStringArray** ids) {
    return 0;
}

//-------------------------------------------------------------------------------------------//

int32_t HFParseResultInt_ (HFData* result, uint64_t parameter, int32_t* value) {

    int32_t error = HFERROR_GENERAL;

    assert(result != nullptr);
    assert(value != nullptr);

    HFResult* output;
    error = VerifyResult(result, static_cast<uint32_t>(parameter), value, &output);

    if (error == HFERROR_OK) {
        switch (parameter) {
            case HFRESULT_INT_OPERATION_STATUS:
                VerifyFlags(output->validFlags, HFRESULT_INT_OPERATION_STATUS);
                *value = output->contextStatus;
                error = HFERROR_OK;
                break;
            case HFRESULT_INT_CONTEXT_ERROR:
                VerifyFlags(output->validFlags, HFRESULT_INT_CONTEXT_ERROR);
                *value = output->errorCode;
                error = HFERROR_OK;
                break;
            case HFRESULT_INT_SEQUENCE_NUMBER:
                VerifyFlags(output->validFlags, HFRESULT_INT_SEQUENCE_NUMBER);
                *value = output->sequenceNumber;
                error = HFERROR_OK;
                break;
            case HFRESULT_INT_NUMBER_OF_FACES:
                VerifyFlags(output->validFlags, HFRESULT_INT_NUMBER_OF_FACES);
                *value = output->facesDetectedCount;
                error = HFERROR_OK;
                break;
            case HFRESULT_INT_IS_CAPTURED:
                VerifyFlags(output->validFlags, HFRESULT_INT_IS_CAPTURED);
                *value = output->isCaptured;
                error = HFERROR_OK;
                break;
            default:
                error = HFERROR_ARGUMENT_INVALID;
                break;
        }
    }

    return error;
}

int32_t HFParseResultDouble_ (HFData* result, uint64_t parameter, double* value) {

    int32_t error = HFERROR_GENERAL;

    assert(result != nullptr);
    assert(value != nullptr);

    HFResult* output;

    error = VerifyResult(result, static_cast<uint32_t>(parameter), value, &output);
    if (error == HFERROR_OK) {
        switch (parameter) {
        case HFRESULT_DOUBLE_QUALITY:
            VerifyFlags(output->validFlags, HFRESULT_DOUBLE_QUALITY);
            *value = output->quality;
            error = HFERROR_OK;
            break;
        case HFRESULT_DOUBLE_SPOOF_PROBABILITY:
            VerifyFlags(output->validFlags, HFRESULT_DOUBLE_SPOOF_PROBABILITY);
            *value = output->spoofProbability;
            error = HFERROR_OK;
            break;
        default:
            error = HFERROR_ARGUMENT_INVALID;
            break;
        }
    }

    return error;
}

int32_t HFParseResultData_ (HFData* result, uint64_t parameter, HFData* value) {

    int32_t error = HFERROR_GENERAL;

    assert(result != nullptr);
    assert(value != nullptr);

    HFResult* output;
    error = VerifyResult(result, static_cast<uint32_t>(parameter), value, &output);
    if (error == HFERROR_OK) {
        switch (parameter) {
        case HFRESULT_DATA_TEMPLATE:
            VerifyFlags(output->validFlags, HFRESULT_DATA_TEMPLATE);
            *value = output->templ;
            error = HFERROR_OK;
            break;
        default:
            error = HFERROR_ARGUMENT_INVALID;
            break;
        }
    }

    return error;
}

int32_t HFParseResultImage_ (HFData* result, uint64_t parameter, HFImage* value) {

    int32_t error = HFERROR_GENERAL;

    assert(result != nullptr);
    assert(value != nullptr);

    HFResult* output;
    error = VerifyResult(result, static_cast<uint32_t>(parameter), value, &output);
    if (error == HFERROR_OK) {
        switch (parameter) {
        case HFRESULT_IMAGE_IMAGE:
            VerifyFlags(output->validFlags, HFRESULT_IMAGE_IMAGE);
            *value = output->image;
            error = HFERROR_OK;
            break;
        default:
            error = HFERROR_ARGUMENT_INVALID;
            break;
        }
    }

    return error;
}

int32_t HFParseResultPoint_ (HFData* result, uint64_t parameter, HFPoint* value) {

    int32_t error = HFERROR_GENERAL;

    assert(result != nullptr);
    assert(value != nullptr);

    HFResult* output;
    error = VerifyResult(result, static_cast<uint32_t>(parameter), value, &output);
    if (error == HFERROR_OK) {
        switch (parameter) {
            case HFRESULT_POINT_BOUNDING_BOX_UPPER_LEFT:
                VerifyFlags(output->validFlags, HFRESULT_POINT_BOUNDING_BOX_UPPER_LEFT);
                *value = output->boundBox.upperLeft;
                error = HFERROR_OK;
                break;
            case HFRESULT_POINT_BOUNDING_BOX_BOTTOM_RIGHT:
                VerifyFlags(output->validFlags, HFRESULT_POINT_BOUNDING_BOX_BOTTOM_RIGHT);
                *value = output->boundBox.bottomRight;
                error = HFERROR_OK;
                break;
            case HFRESULT_POINT_LEFT_EYE:
                VerifyFlags(output->validFlags, HFRESULT_POINT_LEFT_EYE);
                *value = output->landmarks.leftEye;
                error = HFERROR_OK;
                break;
            case HFRESULT_POINT_RIGHT_EYE:
                VerifyFlags(output->validFlags, HFRESULT_POINT_RIGHT_EYE);
                *value = output->landmarks.rightEye;
                error = HFERROR_OK;
                break;
            case HFRESULT_POINT_NOSE:
                VerifyFlags(output->validFlags, HFRESULT_POINT_NOSE);
                *value = output->landmarks.nose;
                error = HFERROR_OK;
                break;
            case HFRESULT_POINT_MOUTH_LEFT:
                VerifyFlags(output->validFlags, HFRESULT_POINT_MOUTH_LEFT);
                *value = output->landmarks.mouthLeft;
                error = HFERROR_OK;
                break;
            case HFRESULT_POINT_MOUTH_RIGHT:
                VerifyFlags(output->validFlags, HFRESULT_POINT_MOUTH_RIGHT);
                *value = output->landmarks.mouthRight;
                error = HFERROR_OK;
                break;
            default:
                error = HFERROR_ARGUMENT_INVALID;
                break;
        }
    }

    return error;
}

int32_t HFParseResultMatchGallery_ (HFData* result, uint64_t parameter, HFMatchGallery* value) {

    int32_t error = HFERROR_GENERAL;

    assert(result != nullptr);
    assert(value != nullptr);

    HFResult* output;
    error = VerifyResult(result, static_cast<uint32_t>(parameter), value, &output);
    if (error == HFERROR_OK) {
        switch (parameter) {
        case HFRESULT_MATCHGALLERY_MATCH_GALERY:
            VerifyFlags(output->validFlags, HFRESULT_MATCHGALLERY_MATCH_GALERY);
            *value = output->matches;
            error = HFERROR_OK;
            break;
        default:
            error = HFERROR_ARGUMENT_INVALID;
            break;
        }
    }

    return error;
}

//-------------------------------------------------------------------------------------------//

void HFFree_(void* data) {

    uint8_t* free_ptr = static_cast<uint8_t*>(data);

    free_ptr -= sizeof(Mem_Hdr);

    Mem_Hdr* test_ptr = (Mem_Hdr*)(free_ptr);

    assert(test_ptr->magic1 == MEMBLOCK_MAGIC1);
    assert(test_ptr->magic2 == MEMBLOCK_MAGIC2);

    delete free_ptr;
}

//-------------------------------------------------------------------------------------------//

}

