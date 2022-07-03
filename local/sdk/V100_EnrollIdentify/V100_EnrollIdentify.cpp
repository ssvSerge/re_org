#include "HFApi.h"
#include <chrono>
#include <thread>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>

#define PERROR(function) printf("Function %s failed with error %d", function, error)

#define VERIFY(function, params) \
    error = function params; \
    if(error != HFERROR_OK) \
    { \
        PERROR(#function); \
        HFTerminate(); \
        return 1; \
    }


std::vector<char> LoadData(const std::string& img)
{
    std::ifstream input(img, std::ios::binary);
    input.seekg(0, input.end);
    int length = input.tellg();
    input.seekg(0, input.beg);
    std::vector<char> buffer;
    buffer.resize(length, 0);
    input.read(&buffer[0], length);
    return buffer;
}

int main()
{
    int32_t error = HFERROR_GENERAL;
    HFOperation operation = HFOPERATION_NONE;
    HFContext context = HFCONTEXT_NONE;
    int32_t status = HFSTATUS_BUSY;
    HFData* result = nullptr;
    HFStringArray* records = nullptr;

    std::vector<char> img = LoadData("res.png");
    HFImage image = { HFIMAGE_ENCODING_PNG, { &img[0], (uint32_t)img.size() } };

    std::vector<char> tmp = LoadData("res.png.bin");
    HFData templ = { &tmp[0], (uint32_t)tmp.size() };

    HFDatabaseRecordHeader header = { "USER1", "", "" };

    HFMatchGallery gallery;

    VERIFY(HFInit, ())

        VERIFY(HFOpenContext, (0, HFALGORITHM_TYPE_ON_HOST, &context))

        VERIFY(HFSetParamInt, (context, HFPARAM_CONTEXT_INT_IMAGE_ENCODING, HFIMAGE_ENCODING_PNG))

        VERIFY(HFAsyncExtractTemplate, (context, &image, HFRESULTVALID_ALL, &operation))

        status = HFSTATUS_BUSY;
    while (status == HFSTATUS_BUSY)
    {
        VERIFY(HFGetIntermediateResult, (operation, HFRESULTVALID_BASIC, HFSEQUENCE_NUMBER_NONE, &result))
            VERIFY(HFParseResultInt, (result, HFRESULT_INT_OPERATION_STATUS, &status))
            HFFree(result);
    }
    VERIFY(HFGetFinalResult, (operation, HFRESULTVALID_ALL, &result))
        VERIFY(HFParseResultInt, (result, HFRESULT_INT_OPERATION_STATUS, &status))
        if (status != HFSTATUS_READY)
        {
            HFTerminate();
            return 1;
        }
    HFFree(result);

    VERIFY(HFAddRecordWithCaptured, (operation, &header, true))
        VERIFY(HFListRecords, ("", &records))
        for (size_t i = 0; i < records->stringsCount; i++)
        {
            printf("Id - %s\n", records->strings[i]);
        }
    HFFree(records);

    VERIFY(HFAsyncIdentifyWithTemplate, (context, "", 0.5, &templ, &operation))

        status = HFSTATUS_BUSY;
    while (status == HFSTATUS_BUSY)
    {
        VERIFY(HFGetIntermediateResult, (operation, HFRESULTVALID_BASIC, HFSEQUENCE_NUMBER_NONE, &result))
            VERIFY(HFParseResultInt, (result, HFRESULT_INT_OPERATION_STATUS, &status))
            HFFree(result);
    }

    VERIFY(HFGetFinalResult, (operation, HFRESULTVALID_ALL, &result))
        VERIFY(HFParseResultInt, (result, HFRESULT_INT_OPERATION_STATUS, &status))
        if (status != HFSTATUS_READY)
        {
            HFTerminate();
            return 1;
        }
    VERIFY(HFParseResultMatchGallery, (result, HFRESULT_MATCHGALLERY_MATCH_GALERY, &gallery))
        for (size_t i = 0; i < gallery.recordsCount; i++)
        {
            printf("Id - %s, score - %f\n", gallery.records[i].header.recordId, gallery.records[i].matchScore);
        }
    HFFree(result);



    VERIFY(HFAsyncExtractTemplate, (context, &image, HFRESULTVALID_ALL, &operation))

        status = HFSTATUS_BUSY;
    while (status == HFSTATUS_BUSY)
    {
        VERIFY(HFGetIntermediateResult, (operation, HFRESULTVALID_BASIC, HFSEQUENCE_NUMBER_NONE, &result))
            VERIFY(HFParseResultInt, (result, HFRESULT_INT_OPERATION_STATUS, &status))
            HFFree(result);
    }
    VERIFY(HFGetFinalResult, (operation, HFRESULTVALID_ALL, &result))
        VERIFY(HFParseResultInt, (result, HFRESULT_INT_OPERATION_STATUS, &status))
        if (status != HFSTATUS_READY)
        {
            HFTerminate();
            return 1;
        }
    HFFree(result);

    VERIFY(HFAsyncIdentifyWithCaptured, (operation, "", 0.5))

        status = HFSTATUS_BUSY;
    while (status == HFSTATUS_BUSY)
    {
        VERIFY(HFGetIntermediateResult, (operation, HFRESULTVALID_BASIC, HFSEQUENCE_NUMBER_NONE, &result))
            VERIFY(HFParseResultInt, (result, HFRESULT_INT_OPERATION_STATUS, &status))
            HFFree(result);
    }

    VERIFY(HFGetFinalResult, (operation, HFRESULTVALID_ALL, &result))
        VERIFY(HFParseResultInt, (result, HFRESULT_INT_OPERATION_STATUS, &status))
        if (status != HFSTATUS_READY)
        {
            HFTerminate();
            return 1;
        }
    VERIFY(HFParseResultMatchGallery, (result, HFRESULT_MATCHGALLERY_MATCH_GALERY, &gallery))
        for (size_t i = 0; i < gallery.recordsCount; i++)
        {
            printf("Id - %s, score - %f\n", gallery.records[i].header.recordId, gallery.records[i].matchScore);
        }
    HFFree(result);

    VERIFY(HFCloseContext, (context))
        VERIFY(HFTerminate, ())
}