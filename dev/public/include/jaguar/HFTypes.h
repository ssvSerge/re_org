#ifndef __HFTYPES_H__
#define __HFTYPES_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Flags for filtering the result values
     * 
     * Used to indicate what data should be calculated, retained and returned.
     * Used also by \c HFParseResult... functions to extract the actual values from a result
     * obtained by \c HFGetIntermediateResult and \c HFGetFinalResult.
     */
    typedef enum _TAG_HFResultFlags
    {
        /**
         * @brief The status of the context.
         * Values of the Intermediate and final results, see \c HFStatus enum.
         */
        HFRESULT_INT_OPERATION_STATUS = 0x00001,
        /**
         * @brief The error encountered during the operation
         * Values of the Intermediate and final results, see \c HFErrors.h
         * Always check if \c HFRESULT_INT_OPERATION_STATUS is \c HFSTATUS_ERROR .
         */
        HFRESULT_INT_CONTEXT_ERROR = 0x00002,
        /** @brief The captured image. */
        HFRESULT_IMAGE_IMAGE = 0x00004,
        /** @brief The unique sequence number of the result. */
        HFRESULT_INT_SEQUENCE_NUMBER = 0x00008,
        /** @brief The number of faces detected in the image. */
        HFRESULT_INT_NUMBER_OF_FACES = 0x00010,
        /** @brief The image quality for biometric operations (values between 0-1, the higher the better). */
        HFRESULT_DOUBLE_QUALITY = 0x00020,
        /** @brief The biometric template extracted from the \c HFRESULT_IMAGE_IMAGE . */
        HFRESULT_DATA_TEMPLATE = 0x00040,
        /** @brief The bounding box of the face represented by the \c HFRESULT_DATA_TEMPLATE , upper left corner. */
        HFRESULT_POINT_BOUNDING_BOX_UPPER_LEFT = 0x00080,
        /** @brief The bounding box of the face represented by the \c HFRESULT_DATA_TEMPLATE , bottom right corner. */
        HFRESULT_POINT_BOUNDING_BOX_BOTTOM_RIGHT = 0x00100,
        /** @brief The probability that the face on the image is a fake (values between 0-1, the higher the more probable). */
        HFRESULT_DOUBLE_SPOOF_PROBABILITY = 0x00200,
        /** @brief List of all matches found by the last biometric operation (Match, IDentify, Verify). */
        HFRESULT_MATCHGALLERY_MATCH_GALERY = 0x00400,
        /** @brief Flag indicating if the result is based on an image captured by the camera. */
        HFRESULT_INT_IS_CAPTURED = 0x00800,
        /** @brief Left eye position. */
        HFRESULT_POINT_LEFT_EYE = 0x01000,
        /** @brief Right eye position. */
        HFRESULT_POINT_RIGHT_EYE = 0x02000,
        /** @brief Nose position. */
        HFRESULT_POINT_NOSE = 0x04000,
        /** @brief Left mouth corner position. */
        HFRESULT_POINT_MOUTH_LEFT = 0x08000,
        /** @brief Right mouth corner position. */
        HFRESULT_POINT_MOUTH_RIGHT = 0x10000
    } HFResultFlags;

    /** @brief Produce and store all available values. */
    #define HFRESULTVALID_ALL 0xffffffffffffffff
    /** @brief Store all values besides image and template, usually used for intermediate results. */
    #define HFRESULTVALID_ALL_NO_IMAGE (HFRESULTVALID_ALL & ~HFRESULT_IMAGE_IMAGE & ~HFRESULT_DATA_TEMPLATE)
    /** @brief Store only the most basic information, mainly the context status. */
    #define HFRESULTVALID_BASIC (HFRESULT_INT_OPERATION_STATUS | HFRESULT_INT_CONTEXT_ERROR)

    /**
     * @brief Supported parameter identifiers.
     */
    typedef enum _TAG_HFParam
    {
        /** @brief Library version.
         * Only as read only global parameter */
        HFPARAM_GLOBAL_STRING_VERSION = 1,
        /**
         * @brief Context-wide image encoding
         * Encoding used for all images captured and stored with the context.
         * Available only for contexts with a camera */
        HFPARAM_CONTEXT_INT_IMAGE_ENCODING = 2
    } HFParam;

    /**
     * @brief Infinite timeout for continuous operations.
     */
    #define HF_INFINITE_TIMEOUT -1

    /**
     * @brief Nonexistent or empty context.
     */
    #define HFCONTEXT_NONE -1

    /**
     * @brief Nonexistent or empty operation.
     */
    #define HFOPERATION_NONE -1

    /**
     * @brief No camera, used in HFOpenContext to create a context without a camera.
     */
    #define HF_CAMERA_NONE -1

    /**
     * @brief Empty sequence number
     * Can be used to skip sequence checks in \c HFGetIntermediateResult and \c HFGetVideoFrame .
     */
    #define HFSEQUENCE_NUMBER_NONE -1

    /**
     * @brief Hid Face context.
     * 
     * An ID of a context opened and closed by \c HFOpenContext and \c HFCloseContext
     * Opened context combines a camera handler and a biometric algorithm,
     * both may be empty if not needed.
     */
    typedef int32_t HFContext;
    
    /**
     * @brief Background operation.
     * 
     * An ID of an operation running on background initiated by an \c HFAsync... function.
     * The operation status can be obtained by \c HFGetIntermediateResult to get result
     * and \c HFParseResultInt with \c HFRESULT_INT_OPERATION_STATUS flag to get the value.
     */
    typedef int32_t HFOperation;

    /**
     * @brief Operation status codes.
     */
    typedef enum _TAG_HFStatus
    {
        /** @brief Unknown status. */
        HFSTATUS_UNKNOWN = 1,
        /** @brief Camera and algorithm are ready - inactive. */
        HFSTATUS_READY = 2,
        /** @brief Camera or algorithm are running a \c HFAsync... function. */
        HFSTATUS_BUSY = 3,
        /** @brief Camera or algorithm error, see \c HFRESULT_INT_CONTEXT_ERROR for details. */
        HFSTATUS_ERROR = 4,
        /** @brief Max value */
        HFSTATUS_MAX
    } HFStatus;

    /**
     * @brief Image encodings available for export.
     */
    typedef enum _TAG_HFImageEncoding
    {
        /** @brief JPEG compression */
        HFIMAGE_ENCODING_JPEG = 0,
        /** @brief PNG compression */
        HFIMAGE_ENCODING_PNG = 1,
        /** @brief Max value */
        HFIMAGE_ENCODING_MAX
    } HFImageEncoding;

    /**
     * @brief Algorithm types
     * The actual algorithm vendor and type depends on the SDK and connected device.
     */
    typedef enum _TAG_HFAlgorithmType
    {
        /** @brief Default algorithm ( \c HFALGORITHM_TYPE_ON_DEVICE if available ) */
        HFALGORITHM_TYPE_DEFAULT = 0,
        /** @brief On device algorithm, requires the HID Face Camera */
        HFALGORITHM_TYPE_ON_DEVICE = 1,
        /** @brief On host algorithm, all biometric operations are done on host */
        HFALGORITHM_TYPE_ON_HOST = 2,
        /** @brief No algorithm, context will not support any biometric operations */
        HFALGORITHM_TYPE_NONE = 3,
        /** @brief Max value */
        HFALGORITHM_TYPE_MAX
    } HFAlgorithmType;

    /**
     * @brief Point in an image, size is in pixels.
     *
     * The X axis goes left-right.
     * The Y axis goes top-down.
     */
    typedef struct _TAG_HFPoint
    {
        /** @brief x coordinate. */
        int32_t x;
        /** @brief y coordinate. */
        int32_t y;
    } HFPoint;

    /**
     * @brief Generic binary buffer.
     */
    typedef struct _TAG_HFData
    {
        /** @brief Pointer to the beginning of the binary buffer. */
        void *data;
        /** @brief Size of the binary buffer. */
        uint32_t size;
    } HFData;

    /**
     * @brief Captured Image.
     */
    typedef struct _TAG_HFImage
    {
        /** @brief Encoding of the image. */
        HFImageEncoding imageEncoding;
        /** @brief Binary data of the encoded image. */
        HFData data;
    } HFImage;

    /**
     * @brief Database record header.     * 
     * Empty strings ("") usualy denotes "all" for \p recordId and \p galleryID
     */
    typedef struct _TAG_HFDatabaseRecordHeader
    {
        /** @brief Unique identifier of the database record. */
        const char *recordId;
        /** @brief Identifier of a gallery the record is in. */
        const char *galleryID;
        /** @brief Custom data assigned by caller. */
        const char *customData;
    } HFDatabaseRecordHeader;

    /**
     * @brief Database record stored in the internal database.
     */
    typedef struct _TAG_HFDatabaseRecord
    {
        /** @brief Record header. */
        HFDatabaseRecordHeader header;
        /** @brief Binary data of the template used to identify this record. */
        HFData templ;
    } HFDatabaseRecord;

    /**
     * @brief Result of all matching operations (Match, Verify, Identify).
     */
    typedef struct _TAG_HFMatchRecord
    {
        /** @brief Identification of the matched record. */
        HFDatabaseRecordHeader header;
        /** @brief The reulting score of the match operation. */
        double matchScore;
    } HFMatchRecord;

    /**
     * @brief List of the match results. 
     */
    typedef struct _TAG_HFMatchGallery
    {
        /** @brief Simple C array of the match results. */
        HFMatchRecord *records;
        /** @brief Number of stored results. */
        uint32_t recordsCount;
    } HFMatchGallery;

    /**
     * @brief List of strings. 
     */
    typedef struct _TAG_HFStringArray
    {
        /** @brief Simple C array of a strings. */
        char **strings;
        /** @brief Number of stored strings. */
        uint32_t stringsCount;
    } HFStringArray;

#ifdef __cplusplus
}
#endif

#endif // __HFTYPES_H__
