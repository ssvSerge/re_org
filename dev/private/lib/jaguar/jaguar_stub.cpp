#include <jaguar/HFApi.h>
#include <jaguar/HFTemplatePrivate.h>

#include <hid/HidTypes.h>

extern "C" {


int32_t HFInit() {

    return 0;
}

int32_t HFTerminate() {

    return 0;
}

int32_t HFEnumerateCameras(HFStringArray** cameras) {

    UNUSED(cameras);
    return 0;
}

int32_t HFOpenContext(int32_t cameraId, HFAlgorithmType algorithmType, HFContext* context) {

    UNUSED(cameraId);
    UNUSED(algorithmType);
    UNUSED(context);

    return 0;
}

int32_t HFCloseContext(HFContext context) {

    UNUSED(context);
    return 0;
}

int32_t HFGetVideoFrame(HFContext context, int64_t lastSequenceNumber, HFImage** image, int64_t* imageSequenceNumber) {

    UNUSED(context);
    UNUSED(lastSequenceNumber);
    UNUSED(image);
    UNUSED(imageSequenceNumber);
    return 0;
}

int32_t HFParseResultInt(const HFData* result, uint64_t parameter, int32_t* value) {

    UNUSED(result);
    UNUSED(parameter);
    UNUSED(value);
    return 0;
}

int32_t HFParseResultDouble(const HFData* result, uint64_t parameter, double* value) {

    UNUSED(result);
    UNUSED(parameter);
    UNUSED(value);
    return 0;
}

int32_t HFParseResultData(const HFData* result, uint64_t parameter, HFData* value) {

    UNUSED(result);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFParseResultPoint(const HFData* result, uint64_t parameter, HFPoint* value) {

    UNUSED(result);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFParseResultImage(const HFData* result, uint64_t parameter, HFImage* value) {

    UNUSED(result);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFParseResultMatchGallery(const HFData* result, uint64_t parameter, HFMatchGallery* value) {

    UNUSED(result);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFGetIntermediateResult(HFOperation operation, uint64_t resultFlags, int32_t lastSequenceNumber, HFData** result) {

    UNUSED(operation);
    UNUSED(resultFlags);
    UNUSED(lastSequenceNumber);
    UNUSED(result);

    return 0;
}

int32_t HFGetFinalResult(HFOperation operation, uint64_t resultFlags, HFData** result) {

    UNUSED(operation);
    UNUSED(resultFlags);
    UNUSED(result);

    return 0;
}

int32_t HFGetParamInt(HFContext context, uint32_t parameter, int32_t* value) {

    UNUSED(context);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFSetParamInt(HFContext context, uint32_t parameter, int32_t value) {

    UNUSED(context);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFGetParamDouble(HFContext context, uint32_t parameter, double* value) {

    UNUSED(context);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFSetParamDouble(HFContext context, uint32_t parameter, double value) {

    UNUSED(context);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFGetParamString(HFContext context, uint32_t parameter, char** value) {

    UNUSED(context);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFSetParamString(HFContext context, uint32_t parameter, const char* value) {

    UNUSED(context);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFGetParamData(HFContext context, uint32_t parameter, HFData** value) {

    UNUSED(context);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFSetParamData(HFContext context, uint32_t parameter, const HFData* value) {

    UNUSED(context);
    UNUSED(parameter);
    UNUSED(value);

    return 0;
}

int32_t HFStopOperation(HFOperation operation) {

    UNUSED(operation);

    return 0;
}

int32_t HFCloseOperation(HFOperation operation) {

    UNUSED(operation);

    return 0;
}

int32_t HFAsyncStartCaptureImage(HFContext context, int32_t timeout, double minimalQuality, double maximalSpoofProbability, uint64_t intermediateResultFlags, uint64_t finalResultFlags, HFOperation* operation) {

    UNUSED(context);
    UNUSED(timeout);
    UNUSED(minimalQuality);
    UNUSED(maximalSpoofProbability);
    UNUSED(intermediateResultFlags);
    UNUSED(finalResultFlags);
    UNUSED(operation);

    return 0;
}

int32_t HFAsyncExtractTemplate(HFContext context, const HFImage* image, uint64_t finalResultFlags, HFOperation* operation) {

    UNUSED(context);
    UNUSED(image);
    UNUSED(finalResultFlags);
    UNUSED(operation);

    return 0;
}

int32_t HFAsyncVerifyWithCaptured(HFOperation operation, const char* galleryID, const char* id, double minimalMatchScore) {

    UNUSED(operation);
    UNUSED(galleryID);
    UNUSED(id);
    UNUSED(minimalMatchScore);

    return 0;
}

int32_t HFAsyncIdentifyWithCaptured(HFOperation operation, const char* galleryID, double minimalMatchScore) {

    UNUSED(operation);
    UNUSED(galleryID);
    UNUSED(minimalMatchScore);

    return 0;
}

int32_t HFAsyncMatchWithCaptured(HFOperation operation, const HFData* templ) {

    UNUSED(operation);
    UNUSED(templ);

    return 0;
}

int32_t HFAsyncVerifyWithTemplate(HFContext context, const char* galleryID, const char* id, double minimalMatchScore, const HFData* templ, HFOperation* operation) {

    UNUSED(context);
    UNUSED(galleryID);
    UNUSED(id);
    UNUSED(minimalMatchScore);
    UNUSED(templ);
    UNUSED(operation);

    return 0;
}

int32_t HFAsyncIdentifyWithTemplate(HFContext context, const char* galleryID, double minimalMatchScore, const HFData* templ, HFOperation* operation) {

    UNUSED(context);
    UNUSED(galleryID);
    UNUSED(minimalMatchScore);
    UNUSED(templ);
    UNUSED(operation);

    return 0;
}

int32_t HFAsyncMatchWithTemplate(HFContext context, const HFData* templA, const HFData* templB, HFOperation* operation) {

    UNUSED(context);
    UNUSED(templA);
    UNUSED(templB);
    UNUSED(operation);

    return 0;
}

int32_t HFAddRecordWithCaptured(HFOperation operation, const HFDatabaseRecordHeader* header, bool replaceIfExists) {

    UNUSED(operation);
    UNUSED(header);
    UNUSED(replaceIfExists);

    return 0;
}

int32_t HFAddRecordWithTemplate(HFDatabaseRecord* databaseRecord, bool replaceIfExists) {

    UNUSED(databaseRecord);
    UNUSED(replaceIfExists);

    return 0;
}

int32_t HFDeleteRecord(const char* id, const char* galleryID) {

    UNUSED(id);
    UNUSED(galleryID);

    return 0;
}

int32_t HFGetRecord(const char* id, const char* galleryID, HFDatabaseRecord** databaseRecord) {

    UNUSED(id);
    UNUSED(galleryID);
    UNUSED(databaseRecord);

    return 0;
}

int32_t HFListRecords(const char* galleryID, HFStringArray** ids) {

    UNUSED(galleryID);
    UNUSED(ids);

    return 0;
}

int32_t HFAllocResult(const void* data, HFData** ptr) {

    UNUSED(data);
    UNUSED(ptr);

    return 0;
}

void HFFree(void* data) {

    UNUSED(data);
}

}
