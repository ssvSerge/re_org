#include <chrono>
#include <thread>
#include <cstdio>
#include <string>

#include <jaguar/HFApi.h>


#define PERROR(function) printf("Function %s failed with error %d", function, error)

#define VERIFY(function, params) \
    error = function params; \
    if(error != HFERROR_OK) \
    { \
        PERROR(#function); \
        HFTerminate(); \
        return 1; \
    }

void Store(const std::string name, const HFImage& image, const HFData& data)
{
    FILE* write_ptr;
    write_ptr = fopen(name.c_str(), "wb");
    fwrite(image.data.data, image.data.size, 1, write_ptr);
    fclose(write_ptr);
    write_ptr = fopen((name + ".bin").c_str(), "wb");
    fwrite(data.data, data.size, 1, write_ptr);
    fclose(write_ptr);
}

int main()
{
    int32_t error = HFERROR_GENERAL;
    HFOperation operation = HFOPERATION_NONE;
    HFContext context = HFCONTEXT_NONE;
    int32_t status = HFSTATUS_BUSY;
    int64_t sequenceNumber = HFSEQUENCE_NUMBER_NONE;
    HFData* result = nullptr;
    HFData templ;
    HFImage image;
    HFImage* img = nullptr;
    HFStringArray* cameras;
    char* version = nullptr;

    VERIFY(HFInit, ())

        VERIFY(HFGetParamString, (HFCONTEXT_NONE, HFPARAM_GLOBAL_STRING_VERSION, &version))
        printf("Version - %s\n", version);
    HFFree(version);

    VERIFY(HFEnumerateCameras, (&cameras))
        for (uint32_t i = 0; i < cameras->stringsCount; ++i)
            printf("Camera %d - %s\n", i, cameras->strings[i]);
    HFFree(cameras);

    VERIFY(HFOpenContext, (0, HFALGORITHM_TYPE_ON_HOST, &context))

        VERIFY(HFSetParamInt, (context, HFPARAM_CONTEXT_INT_IMAGE_ENCODING, HFIMAGE_ENCODING_PNG))

        VERIFY(HFAsyncStartCaptureImage, (context, 5000, 0.7, 1, HFRESULTVALID_ALL_NO_IMAGE, HFRESULTVALID_ALL, &operation))
        status = HFSTATUS_BUSY;
    while (status == HFSTATUS_BUSY)
    {
        double quality = 0;
        int32_t err = HFGetIntermediateResult(operation, HFRESULTVALID_ALL, (int32_t)sequenceNumber, &result);
        if (err == HFERROR_ALREADY_RETURNED)
            continue;
        else if (err != HFERROR_OK)
        {
            HFTerminate();
            return 1;
        }
        VERIFY(HFParseResultInt, (result, HFRESULT_INT_OPERATION_STATUS, &status))
            if (status != HFSTATUS_ERROR)
            {
                if (HFParseResultDouble(result, HFRESULT_DOUBLE_QUALITY, &quality) == HFERROR_OK)
                {
                    printf("quality %f\n", quality);
                }
                HFParseResultInt(result, HFRESULT_INT_SEQUENCE_NUMBER, reinterpret_cast<int32_t*>(&sequenceNumber));
            }
        HFFree(result);
    }

    VERIFY(HFGetVideoFrame, (context, HFSEQUENCE_NUMBER_NONE, &img, &sequenceNumber))
        Store("frame.png", *img, templ);
    HFFree(img);

    VERIFY(HFGetFinalResult, (operation, HFRESULTVALID_ALL, &result))

        VERIFY(HFParseResultData, (result, HFRESULT_DATA_TEMPLATE, &templ))
        VERIFY(HFParseResultImage, (result, HFRESULT_IMAGE_IMAGE, &image))
        Store("res.png", image, templ);

    HFFree(result);

    VERIFY(HFCloseContext, (context))
        VERIFY(HFTerminate, ())
}