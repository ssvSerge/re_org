#ifndef __HFAPI_H__
#define __HFAPI_H__

#include <stdint.h>
#include <stdbool.h>

#include "HFVersion.h"
#include "HFErrors.h"
#include "HFTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif
    // GENERAL FUNCTIONS

    /**
     * @brief Initialize the library.
     * 
     * Any global parameters can be set using the HFSetParam... functions, 
     * with the context param set to \c NULL .
     * This function is blocking.
     * 
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFInit();

    /**
     * @brief Terminate the library.
     * 
     * All running operations are stopped and all existing contexts 
     * are closed before returning.
     * This function is blocking.
     * Memory allocated by HFApi functions is deallocated here,
     * it must not be used afterwards.
     * 
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFTerminate();

    /**
     * @brief List all available cameras.
     * 
     * This function is blocking.
     * 
     * @param cameras [out] Pointer to a string array allocated by this function (deallocate by HFFree)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFEnumerateCameras(HFStringArray **cameras);

    /**
     * @brief Open a context.
     * 
     * Create a new context with a camera and an algorithm, 
     * both can be empty if not needed. The camera and algorithm
     * are configured using the HFSetParam... functions.
     * This function is blocking.
     * 
     * @param cameraId [in] Index of the camera in the HFEnumerateCameras
     * @param algorithmType [in] The type of the selected algorithm
     * @param context [out] The context created by the operation
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFOpenContext(
        int32_t cameraId,
        HFAlgorithmType algorithmType,
        HFContext *context);

    /**
     * @brief Close a context.
     * 
     * All running operations on the context are terminated.
     * This function is blocking.
     * 
     * @param context [in] A context to be closed
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFCloseContext(HFContext context);
    
    /**
     * @brief Get the last frame of a video stream.
     * 
     * The last captured image is encoded and returned. 
     * The frame is returned only if it has different sequence number
     * than the \p lastSequenceNumber , error \c HFERROR_ALREADY_RETURNED is returned otherwise. 
     * Use \c HFSEQUENCE_NUMBER_NONE to ignore the sequence number.
     * 
     * @param context [in] Context used for capturing (must have a non-empty camera)
     * @param lastSequenceNumber [in] The sequence number of the last frame received
     * @param image [out] The last RGB image captured
     * @param imageSequenceNumber [out] The sequence number of the returned image
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFGetVideoFrame(HFContext context, int64_t lastSequenceNumber, HFImage **image, int64_t *imageSequenceNumber);

    /**
     * @brief Get a value of the selected parameter from the result.
     * 
     * In case of HFData, HFImage and HFMatchGallery, the returned \p value is valid
     * only until the \p result is deallocated by HFFree, it is NOT a deep copy.
     * Accessing internal data of a returned \p value after HFFree has undefined behavior.
     * 
     * @param result [in] The result obtained by the HFGetIntermediateResult or HFGetFinalResult
     * @param parameter [in] The parameter identifier, see \c HFResultFlags enum in HFTypes.h
     * @param value [out] Pointer to the value in the \p result
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFParseResultInt(const HFData *result, uint64_t parameter, int32_t *value);
    int32_t HFParseResultDouble(const HFData *result, uint64_t parameter, double *value);
    int32_t HFParseResultData(const HFData *result, uint64_t parameter, HFData *value);
    int32_t HFParseResultPoint(const HFData *result, uint64_t parameter, HFPoint *value);
    int32_t HFParseResultImage(const HFData *result, uint64_t parameter, HFImage *value);
    int32_t HFParseResultMatchGallery(const HFData *result, uint64_t parameter, HFMatchGallery *value);

    /**
     * @brief Get the intermediate result of an async operation.
     * 
     * The \p result will contain only values valid in both the actual result and in \p resultFlags ,
     * use \c HFRESULTVALID_ALL to get all available values (see \c HFResultFlags enum in HFTypes.h)
     * The result is set only if its sequence number differs from the \p lastSequenceNumber ,
     * error \c HFERROR_ALREADY_RETURNED is returned otherwise. 
     * Use \c HFSEQUENCE_NUMBER_NONE to ignore the sequence number. 
     * The values in the result can be accessed by the \c HFParseResult... functions.
     * 
     * @param operation [in] A non-closed operation
     * @param resultFlags [in] A mask of the required values
     * @param lastSequenceNumber [in] The sequence number of the last result received
     * @param result [out] Pointer to the result allocated by this function (deallocate with HFFree)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFGetIntermediateResult(HFOperation operation, uint64_t resultFlags, int32_t lastSequenceNumber, HFData **result);

    /**
     * @brief Get the final result of a completed async operation.
     * 
     * Returns \c HFERROR_BUSY if the operation is still running.
     * The \p result will contain values valid in both the actual result and in \p resultFlags ,
     * use \c HFRESULTVALID_ALL to get all available values (see \c HFResultFlags enum in HFTypes.h)
     * The result is set only if its sequence number differs the \p lastSequenceNumber ,
     * error \c HFERROR_ALREADY_RETURNED is returned otherwise. 
     * Use \c HFSEQUENCE_NUMBER_NONE to ignore the sequence number. 
     * The values in the result can be accessed by the \c HFParseResult... functions.
     * 
     * @param operation [in] A non-closed operation
     * @param resultFlags [in] A mask of required values
     * @param result [out] Pointer to the result allocated by this function (deallocate with HFFree)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFGetFinalResult(HFOperation operation, uint64_t resultFlags, HFData **result);

    /**
     * @brief Get the value of an int parameter for specific context.
     * 
     * The global parameters valid for the entire HFApi, the have the GLOBAL prefix (see \c HFParam enum).
     * The context-specific parameters are valid only for a specified context.
     * All parameters start with a default value.
     * 
     * @param context [in] Context id for the context-specific parameters, HFCONTEXT_NONE for the global parameters
     * @param parameter [in] Identifier of the parameter (see \c HFParam enum)
     * @param value [out] Pointer to store value of the parameter to
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFGetParamInt(HFContext context, uint32_t parameter, int32_t *value);

    /**
     * @brief Set the value of an int parameter for specific context.
     * 
     * The global parameters valid for the entire HFApi, the have the GLOBAL prefix (see \c HFParam enum).
     * The context-specific parameters are valid only for a specified context.
     * All parameters start with a default value.
     * 
     * @param context [in] Context id for the context-specific parameters, HFCONTEXT_NONE for the global parameters
     * @param parameter [in] Identifier of the parameter (see \c HFParam enum)
     * @param value [int] The new value of the parameter
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFSetParamInt(HFContext context, uint32_t parameter, int32_t value);

    // Similar to the int variant
    int32_t HFGetParamDouble(HFContext context, uint32_t parameter, double *value);
    int32_t HFSetParamDouble(HFContext context, uint32_t parameter, double value);
    int32_t HFGetParamString(HFContext context, uint32_t parameter, char **value);
    int32_t HFSetParamString(HFContext context, uint32_t parameter, const char *value);
    int32_t HFGetParamData(HFContext context, uint32_t parameter, HFData **value);
    int32_t HFSetParamData(HFContext context, uint32_t parameter, const HFData *value);

    /**
     * @brief Stop a running operation.
     * 
     * Stopped operation fails and no final result is available.
     * Error \c HFERROR_ARGUMENT_INVALID is returned if the operation is not running.
     * 
     * @param operation [in] An operation
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFStopOperation(HFOperation operation);

    /**
     * @brief Stop and remove operation from list of existing operations.
     * 
     * @param operation [in] An operation
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFCloseOperation(HFOperation operation);

    // TEAMPLATE CAPTURE

    /**
     * @brief Capture an image of a sufficient quality.
     * 
     * The operation can be optimized by limiting the 
     * \p intermediateResultFlags and \p finalResultFlags .
     * Values that are disabled by the flags will not be stored or calculated.
     * Returns \c HFERROR_ARGUMENT_INVALID without any further action for out of bound parameters.
     * 
     * @param context [in] The context, neither camera nor algorithm can be empty
     * @param timeout [in] The operation is canceled if not completed within this time [ms]
     * @param minimalQuality [in] Minimal quality of the image (value between 0-1)
     * @param maximalSpoofProbability [in] Maximal probability that the image is a spoof (value between 0-1)
     * @param intermediateResultFlags [in] Filter of values captured in the intermediate result
     * @param finalResultFlags [in] Filter of values calculated in the final result
     * @param operation [out] Pointer to the operation that will be created by this function (use \c HFGet...Result to get result)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFAsyncStartCaptureImage(
        HFContext context, 
        int32_t timeout,
        double minimalQuality,
        double maximalSpoofProbability,
        uint64_t intermediateResultFlags,
        uint64_t finalResultFlags,
        HFOperation *operation);

    /**
     * @brief Extract a template from an image.
     * 
     * @param context [in] The context, algorithm must not be empty
     * @param image [in] The source image
     * @param finalResultFlags [in] Filter of values to be calculated in the final result
     * @param operation [out] Pointer to the operation that will be created by this function (use \c HFGet...Result to get result)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFAsyncExtractTemplate(HFContext context, const HFImage *image, uint64_t finalResultFlags, HFOperation *operation);

    // CAPTURED BIOMETRIC FUNCTIONS

    /**
     * @brief Verify a user using a captured template.
     * 
     * A 1:1 match of the captured result against the user specified by its identifier.
     * Check \c HFRESULT_INT_OPERATION_STATUS and \c HFRESULT_INT_CONTEXT_ERROR for result
     * using \p HFGetIntermediateResult , \p HFGetFinalResult and \p HFParseResultInt .
     * Returns \c HFERROR_UNKNOWN_USER if no user is found for verification or if id is empty string ("")
     * 
     * @param operation [in] The operation that has produced a template (see \p HFAsyncStartCaptureImage or \p HFAsyncExtractTemplate )
     * @param galleryID [in] The galleries used to search for the user, empty string for entire database
     * @param id [in] The identifier of the verified user
     * @param minimalMatchScore [in] The minimal match score allowed for the operation to succeed (value between 0-1)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFAsyncVerifyWithCaptured(HFOperation operation, const char *galleryID, const char *id, double minimalMatchScore);

    /**
     * @brief Identify a user using a captured template.
     * 
     * This is a 1:N match of the captured result against all users in the database.
     * Check \c HFRESULT_INT_OPERATION_STATUS and \c HFRESULT_INT_CONTEXT_ERROR for result
     * using \p HFGetIntermediateResult , \p HFGetFinalResult and \p HFParseResultInt .
     * 
     * @param operation [in] The operation that has produced a template (see \p HFAsyncStartCaptureImage or \p HFAsyncExtractTemplate )
     * @param galleryID [in] The gallery used to search for the user, empty string for entire database
     * @param minimalMatchScore [in] The minimal match score allowed for the operation to succeed (value between 0-1)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFAsyncIdentifyWithCaptured(HFOperation operation, const char *galleryID, double minimalMatchScore);

    /**
     * @brief Match a template against a captured template.
     * 
     * This is a 1:1 match.
     * Check \c HFRESULT_INT_OPERATION_STATUS and \c HFRESULT_INT_CONTEXT_ERROR for result
     * using \p HFGetIntermediateResult , \p HFGetFinalResult and \p HFParseResultInt .
     * 
     * @param operation [in] The operation that has produced a template (see \p HFAsyncStartCaptureImage or \p HFAsyncExtractTemplate )
     * @param templ [in] The template to match
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFAsyncMatchWithCaptured(HFOperation operation, const HFData *templ);

    // TEMPLATE BIOMETRIC FUNCTIONS

    /**
     * @brief Verify a user using a template.
     * 
     * This is a 1:1 match of the captured result against the user specified by its identifier.
     * Check \c HFRESULT_INT_OPERATION_STATUS and \c HFRESULT_INT_CONTEXT_ERROR for result
     * using \p HFGetIntermediateResult , \p HFGetFinalResult and \p HFParseResultInt .
     * Returns \c HFERROR_UNKNOWN_USER if no user is found for verification or if id is empty string ("").
     * 
     * @param context [in] The context, algorithm must not be empty
     * @param galleryID [in] The gallery used to search for the user, empty string for entire database
     * @param id [in] The identifier of the verified user
     * @param minimalMatchScore [in] The minimal match score allowed for the operation to succeed (value between 0-1)
     * @param templ [in] The template to match user with
     * @param operation [out] Pointer to the operation that will be created by this function (use \p HFGet...Result to get result)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFAsyncVerifyWithTemplate(HFContext context, const char *galleryID, const char *id, double minimalMatchScore, const HFData *templ, HFOperation *operation);

    /**
     * @brief Identify a user using a template.
     * 
     * This is a 1:N match of the captured result against all users in the database.
     * Check \c HFRESULT_INT_OPERATION_STATUS and \c HFRESULT_INT_CONTEXT_ERROR for result
     * using \p HFGetIntermediateResult , \p HFGetFinalResult and \p HFParseResultInt .
     * 
     * @param context [in] The context, algorithm must not be empty
     * @param galleryID [in] The gallery used to search for the user, empty string ("") for entire database
     * @param minimalMatchScore [in] The minimal match score allowed for the operation to succeed (value between 0-1)
     * @param templ [in] The template to identify user with
     * @param operation [out] Pointer to the operation that will be created by this function (use \p HFGet...Result to get result)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFAsyncIdentifyWithTemplate(HFContext context, const char *galleryID, double minimalMatchScore, const HFData *templ, HFOperation *operation);

    /**
     * @brief Match a template against another template.
     * 
     * This is a 1:1 match.
     * Check \c HFRESULT_INT_OPERATION_STATUS and \c HFRESULT_INT_CONTEXT_ERROR for result
     * using \p HFGetIntermediateResult , \p HFGetFinalResult and \p HFParseResultInt .
     * 
     * @param context [in] The context, algorithm must not be empty
     * @param templA [in] The template to match
     * @param templB [in] The template to match
     * @param operation [out] Pointer to the operation that will be created by this function (use \p HFGet...Result to get result)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFAsyncMatchWithTemplate(HFContext context, const HFData *templA, const HFData *templB, HFOperation *operation);

    // DATABASE FUNCTIONS

    /**
     * @brief Insert a record into the database with a template captured in operation.
     * 
     * The record must have a unique identifier or the \p replaceIfExists must be true,
     * returns \c HFERROR_ALREADY_PRESENT otherwise.
     * Use \c HFGetIntermediateResult , \c HFGetFinalResult after 
     * \c HFAsyncStartCaptureImage or \c HFAsyncExtractTemplate
     * to ensure that there is a valid template.
     * 
     * @param operation [in] The operation that has produced a template (see \p HFAsyncStartCaptureImage or \p HFAsyncExtractTemplate )
     * @param header [in] Holds the identification of the record ( \c recordId ) and additional data.
     * @param replaceIfExists [in] Replace an existing record with the same identifier if \c true ( \c galleryID  isn't checked but is rewritten).
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFAddRecordWithCaptured(HFOperation operation, const HFDatabaseRecordHeader *header, bool replaceIfExists);

    /**
     * @brief Insert a record with a template into the database.
     * 
     * The record must have a unique identifier or the \p replaceIfExists must be true,
     * returns \c HFERROR_ALREADY_PRESENT otherwise.
     * 
     * @param databaseRecord [in] Holds the identification ( \c recordId ) and template ( \c templ ) of the record.
     * @param replaceIfExists [in] Replace an existing record with the same identifier if \c true ( \c galleryID  isn't checked but is rewritten).
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFAddRecordWithTemplate(HFDatabaseRecord *databaseRecord, bool replaceIfExists);

    /**
     * @brief Delete a record from the database.
     * 
     * Returns \c HFERROR_UNKNOWN_USER if user with specified gallery is not found
     * or if \p id is empty string ("")
     * 
     * @param id [in] The identifier of the record
     * @param galleryID [in] The gallery used to search for the user, empty string ("") for entire database
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFDeleteRecord(const char *id, const char *galleryID);

    /**
     * @brief Get a specified record.
     * 
     * Returns \c HFERROR_UNKNOWN_USER if the user is not found or if more then one user is found
     * 
     * @param id [in] The identifier of the record, empty string ("") will look for all users (fails)
     * @param galleryID [in] The galleries used to search for the user, empty string ("") for entire database
     * @param databaseRecord [out] Pointer to pointer a database record belonging to the identifier will be allocated (free with HFFree)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFGetRecord(const char *id, const char *galleryID, HFDatabaseRecord **databaseRecord);

    /**
     * @brief List all records in the database.
     * 
     * @param galleryID [in] The gallery used to search for the user, empty string ("") for entire database
     * @param ids [out] Pointer to pointer a list of all known identifiers will be allocated to (free with HFFree)
     * @return int32_t Hid Face error code (see HFErrors.h)
     */
    int32_t HFListRecords(const char *galleryID, HFStringArray **ids);

    // MISC FUNCTIONS

    /**
     * @brief Release memory allocated by HFApi functions.
     * 
     * The function internally releases any recursively allocated memory (strings inside HFStringArray).
     * It can't free memory allocated outside of HFApi (new, malloc, ...).
     * Nullptr isn't a problem (will cause TRACE level trace in standard error output).
     * Set pointer to nullptr after or assure it won't be used or freed again.
     * 
     * @param data [in] Data allocated by an HFApi function
     */
    void HFFree(void *data);

    //namespace HFTest
    //{
        /**
         * @brief Allocate memory using internal memory register for tests
         * 
         * @param data [in] Data to be copied into new memory spot 
         * @param ptr [out] Pointer for newly allocated memory, free with HFFree
         * @return int32_t Hid Face error code (see ...)
         */
        /*
        int32_t HFAllocString(const std::string& data, char** ptr);
        int32_t HFAllocData(const std::vector<uint8_t>& data, HFData** ptr);
        int32_t HFAllocImage(const EncodedImage& data, HFImage** ptr);
        int32_t HFAllocDatabaseRecord(const DatabaseRecord& data, HFDatabaseRecord** ptr);
        int32_t HFAllocStringArray(const std::vector<std::string>& data, HFStringArray** ptr);
        */
        int32_t HFAllocResult(const void* data, HFData** ptr);
    //} // namespace HFTest

#ifdef __cplusplus
}
#endif

#endif // __HFAPI_H__
