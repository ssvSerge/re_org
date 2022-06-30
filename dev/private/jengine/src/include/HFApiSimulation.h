#ifndef __HFAPISIMULATION_H__
#define __HFAPISIMULATION_H__

#include <HFApi.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t HFInit_ ();
int32_t HFTerminate_ ();
int32_t HFEnumerateCameras_ (HFStringArray **cameras);
int32_t HFOpenContext_ (int32_t cameraId, HFAlgorithmType algorithmType, HFContext *context);
int32_t HFCloseContext_ (HFContext context);
int32_t HFGetVideoFrame_ (HFContext context, int64_t lastSequenceNumber, HFImage **image, int64_t *imageSequenceNumber);
int32_t HFParseResultInt_ (HFData *result, uint64_t parameter, int32_t *value);
int32_t HFParseResultDouble_ (HFData *result, uint64_t parameter, double *value);
int32_t HFParseResultData_ (HFData *result, uint64_t parameter, HFData *value);
int32_t HFParseResultPoint_ (HFData *result, uint64_t parameter, HFPoint *value);
int32_t HFParseResultImage_ (HFData *result, uint64_t parameter, HFImage *value);
int32_t HFParseResultMatchGallery_ (HFData *result, uint64_t parameter, HFMatchGallery *value);
int32_t HFGetIntermediateResult_ (HFOperation operation, uint64_t resultFlags, int32_t lastSequenceNumber, HFData **result);
int32_t HFGetFinalResult_ (HFOperation operation, uint64_t resultFlags, HFData **result);
int32_t HFGetParamInt_ (HFContext context, uint32_t parameter, int32_t *value);
int32_t HFSetParamInt_ (HFContext context, uint32_t parameter, int32_t value);
int32_t HFGetParamDouble_ (HFContext context, uint32_t parameter, double *value);
int32_t HFSetParamDouble_ (HFContext context, uint32_t parameter, double value);
int32_t HFGetParamString_ (HFContext context, uint32_t parameter, char **value);
int32_t HFSetParamString_ (HFContext context, uint32_t parameter, const char *value);
int32_t HFGetParamData_ (HFContext context, uint32_t parameter, HFData **value);
int32_t HFSetParamData_ (HFContext context, uint32_t parameter, const HFData *value);
int32_t HFStopOperation_ (HFOperation operation);
int32_t HFCloseOperation_ (HFOperation operation);
int32_t HFAsyncStartCaptureImage_ ( HFContext context, int32_t timeout, double minimalQuality, double maximalSpoofProbability, uint64_t intermediateResultFlags, uint64_t finalResultFlags, HFOperation *operation);
int32_t HFAsyncExtractTemplate_ (HFContext context, const HFImage *image, uint64_t finalResultFlags, HFOperation *operation);
int32_t HFAsyncVerifyWithCaptured_ (HFOperation operation, const char *galleryID, const char *id, double minimalMatchScore);
int32_t HFAsyncIdentifyWithCaptured_ (HFOperation operation, const char *galleryID, double minimalMatchScore);
int32_t HFAsyncMatchWithCaptured_ (HFOperation operation, const HFData *templ);
int32_t HFAsyncVerifyWithTemplate_ (HFContext context, const char *galleryID, const char *id, double minimalMatchScore, const HFData *templ, HFOperation *operation);
int32_t HFAsyncIdentifyWithTemplate_ (HFContext context, const char *galleryID, double minimalMatchScore, const HFData *templ, HFOperation *operation);
int32_t HFAsyncMatchWithTemplate_ (HFContext context, const HFData *templA, const HFData *templB, HFOperation *operation);
int32_t HFAddRecordWithCaptured_ (HFOperation operation, const HFDatabaseRecordHeader *header, bool replaceIfExists);
int32_t HFAddRecordWithTemplate_ (HFDatabaseRecord *databaseRecord, bool replaceIfExists);
int32_t HFDeleteRecord_ (const char *id, const char *galleryID);
int32_t HFGetRecord_ (const char *id, const char *galleryID, HFDatabaseRecord **databaseRecord);
int32_t HFListRecords_ (const char *galleryID, HFStringArray **ids);
void    HFFree_ (void *data);

#ifdef __cplusplus
}
#endif

#endif
