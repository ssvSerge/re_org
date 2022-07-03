#include "HFApi.h"
#include <chrono>
#include <thread>
#include <cstdio>
#include <string>

#define VERIFY(error) \
    if(error != HFERROR_OK) \
    { \
        HFTerminate(); \
        return 1; \
    }

void Store(const std::string name, const HFImage& image, const HFData& data)
{
    FILE* write_ptr;
    write_ptr = fopen((name + ".png").c_str(), "wb");
    fwrite(image.data.data, image.data.size, 1, write_ptr);
    fclose(write_ptr);
    write_ptr = fopen((name + ".bin").c_str(), "wb");
    fwrite(data.data, data.size, 1, write_ptr);
    fclose(write_ptr);
}

int main()
{
    HFOperation operation = HFOPERATION_NONE;
    HFContext context = HFCONTEXT_NONE;
    int32_t status = HFSTATUS_BUSY;
    HFData* result = nullptr;
    HFData* res = nullptr;
    HFData templ;
    HFImage image;
    char* version = nullptr;
    HFMatchGallery gallery;

    VERIFY(HFInit());

    VERIFY(HFGetParamString(HFCONTEXT_NONE, HFPARAM_GLOBAL_STRING_VERSION, &version));
    printf("Version - %s\n", version);
    HFFree(version);

    VERIFY(HFOpenContext(0, HFALGORITHM_TYPE_ON_DEVICE, &context));
    VERIFY(HFSetParamInt(context, HFPARAM_CONTEXT_INT_IMAGE_ENCODING, HFIMAGE_ENCODING_PNG));

    VERIFY(HFAsyncStartCaptureImage(context, 5000, 0.7, 1, HFRESULTVALID_ALL_NO_IMAGE, HFRESULTVALID_ALL, &operation));
    status = HFSTATUS_BUSY;
    while (status == HFSTATUS_BUSY)
    {
        double quality = 0;
        VERIFY(HFGetIntermediateResult(operation, HFRESULTVALID_ALL, HFSEQUENCE_NUMBER_NONE, &result));
        VERIFY(HFParseResultInt(result, HFRESULT_INT_OPERATION_STATUS, &status));
        if (HFParseResultDouble(result, HFRESULT_DOUBLE_QUALITY, &quality) == HFERROR_OK)
            printf("quality %f\n", quality);
        HFFree(result);
    }

    VERIFY(HFGetFinalResult(operation, HFRESULTVALID_ALL, &result));
    VERIFY(HFParseResultData(result, HFRESULT_DATA_TEMPLATE, &templ));
    VERIFY(HFParseResultImage(result, HFRESULT_IMAGE_IMAGE, &image));
    Store("result", image, templ);

    VERIFY(HFAsyncMatchWithCaptured(operation, &templ));
    status = HFSTATUS_BUSY;
    while (status == HFSTATUS_BUSY)
    {
        VERIFY(HFGetIntermediateResult(operation, HFRESULTVALID_BASIC, HFSEQUENCE_NUMBER_NONE, &res));
        VERIFY(HFParseResultInt(result, HFRESULT_INT_OPERATION_STATUS, &status));
        HFFree(result);
    }
    VERIFY(HFGetFinalResult(operation, HFRESULTVALID_ALL, &res));
    VERIFY(HFParseResultMatchGallery(result, HFRESULT_MATCHGALLERY_MATCH_GALERY, &gallery));
    for (size_t i = 0; i < gallery.recordsCount; i++)
    {
        printf("Match score - %f\n", gallery.records[i].matchScore);
    }
    HFFree(res);

    HFFree(result);

    VERIFY(HFCloseContext(context));
    VERIFY(HFTerminate());
}
