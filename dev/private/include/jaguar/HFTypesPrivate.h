#ifndef __HFTYPES_PRIVATE_H__
#define __HFTYPES_PRIVATE_H__

#include <stdint.h>
#include <stdbool.h>
#include "HFTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Rectangle representing an image or its subset.
     *
     * The X axis goes left-right.
     * The Y axis goes top-down.
     *   X  0  1  2
     * Y
     * 0    *  *  *
     * 1    *  *  *
     * 2    *  *  *
     */
    typedef struct _TAG_HFRect
    {
        /** @brief Upper left corner x. */
        HFPoint upperLeft;
        /** @brief Bottom right corner x. */
        HFPoint bottomRight;
    } HFRect;

    /**
     * @brief Landmarks of a face in an image, size is in pixels.
     *
     * Left and Right are relative to the image, NOT the person's face.
     *
     */
    typedef struct _TAG_HFLandmarks
    {
        /** @brief Left eye point. */
        HFPoint leftEye;
        /** @brief Right eye point. */
        HFPoint rightEye;
        /** @brief Nose point. */
        HFPoint nose;
        /** @brief Mouth left point. */
        HFPoint mouthLeft;
        /** @brief Mouth right point. */
        HFPoint mouthRight;
    } HFLandmarks;

    /**
     * @brief Result accumulated on the device.
     *
     * The vaules and accessible by the HFGetIntermediateResult and HFGetFinalResult functions.
     *
     */
    typedef struct _TAG_HFResult
    {
        uint64_t validFlags;
        HFStatus contextStatus;         // 1 << 1
        int32_t errorCode;              // 1 << 2
        HFImage image;                  // 1 << 3
        uint32_t sequenceNumber;        // 1 << 4
        int32_t facesDetectedCount;     // 1 << 5
        double quality;                 // 1 << 6
        HFData templ;                   // 1 << 7
        HFRect boundBox;                // 1 << 8,9
        double spoofProbability;        // 1 << 10
        HFMatchGallery matches;         // 1 << 11
        int32_t isCaptured;             // 1 << 12
        HFLandmarks landmarks;          // 1 << 13,14,15,16,17
    } HFResult;

    /**
     * @brief System security level.
     */
    typedef enum _TAG_HFSecurityLevel
    {
        /** @brief Low security (~ 0.1% FAR). */
        HFSECURITYLEVEL_LOW = 0,
        /** @brief Below normal security (~ 0.1-0.01% FAR). */
        HFSECURITYLEVEL_BELOW_NORMAL = 1,
        /** @brief Normal security (~ 0.01% FAR). */
        HFSECURITYLEVEL_NORMAL = 2,
        /** @brief Above normal security (~ 0.01-0.001% FAR). */
        HFSECURITYLEVEL_ABOVE_NORMAL = 3,
        /** @brief High security (~ 0.001% FAR). */
        HFSECURITYLEVEL_HIGH = 4
    } HFSecurityLevel;

    //! Camera settings

    //! Complete camera URL which can include parameters as resolution (string)
    #define HF_CAMERA_SETTINGS_URL                  "url"
    //! Name of file to be used for playback instead of camera (string)
    #define HF_CAMERA_SETTINGS_PLAYBACK_FILE        "playbackFile"
    //! Enable/disable playback loop (bool, default false)
    #define HF_CAMERA_SETTINGS_PLAYBACK_LOOP        "playbackLoop"
    //! Playback speed (integer, default read from playback file)
    #define HF_CAMERA_SETTINGS_PLAYBACK_FPS         "playbackFps"
    //! Camera number to be opened (integer, default 0)
    #define HF_CAMERA_SETTINGS_CAM_NUMBER           "camNum"
    //! RealSense camera number to be opened (integer, default 0)
    #define HF_CAMERA_SETTINGS_RS_CAM_NUMBER        "rsCamNum"
    //! Ambarella camera number to be opened (integer, default 0)
    #define HF_CAMERA_SETTINGS_AMBA_CAM_NUMBER      "ambaCamNum"
    //! Ambarella camera playback files (strings)
    #define HF_CAMERA_SETTINGS_AMBA_FILE_COLOR      "ambaFileColor"
    #define HF_CAMERA_SETTINGS_AMBA_FILE_IR         "ambaFileIR"
    #define HF_CAMERA_SETTINGS_AMBA_FILE_PATTERN    "ambaFilePattern"
    //! Empty camera (bool, default false)
    #define HF_CAMERA_SETTINGS_EMPTY_CAMERA         "emptyCam"
    //! Frame/image width (integer, camera default)
    #define HF_CAMERA_SETTINGS_FRAME_WIDTH          "width"
    //! Frame/image height (integer, camera default)
    #define HF_CAMERA_SETTINGS_FRAME_HEIGHT         "height"
    //! Camera FPS (integer, camera default)
    #define HF_CAMERA_SETTINGS_FPS                  "fps"
    //! Depth frame/image width (integer, camera default)
    #define HF_CAMERA_SETTINGS_FRAME_WIDTH_DEPTH    "widthDepth"
    //! Depth frame/image height (integer, camera default)
    #define HF_CAMERA_SETTINGS_FRAME_HEIGHT_DEPTH   "heightDepth"
    //! Depth camera FPS (integer, camera default)
    #define HF_CAMERA_SETTINGS_FPS_DEPTH            "fpsDepth"
    //! Camera fourcc codec (string, camera default)
    #define HF_CAMERA_SETTINGS_FOURCC               "fourcc"
    //! Directory name where recorded images should be stored (string)
    #define HF_CAMERA_RECORD_DIR                    "recordDir"
    //! Recording step. 1 - every image saved, 2 - every 2nd, 3 every 3rd... (integer, default 1)
    #define HF_CAMERA_RECORD_STEP                   "recordStep"
    //! Maximal files in a single directory (integer, default 9999)
    #define HF_CAMERA_RECORD_MAX_DIR_FILES          "recordMaxDirFiles"
    //! JPG quality used for recording (integer, default 0 -> OpenCV default)
    #define HF_CAMERA_RECORD_JPG_QUALITY            "recordJPGQuality"
    //! Time in ms in which camera tries to lower power consumption (integer, default 3000)
    #define HF_CAMERA_IDLE_TIMEOUT                  "idleTimeout"
    //! Enables deep sleep for RealSense camera (bool, default false)
    #define HF_CAMERA_RS_DEEP_SLEEP                 "rsDeepSleep"

    //! AmbaCamera settings

    //! ID of the canvas with pattern frames (integer, default 0)
    #define HF_CAMERA_CANVAS_ID_PATTERN             "canvasIdPattern"
    //! ID of the canvas with RGB frames (integer, default 1)
    #define HF_CAMERA_CANVAS_ID_RGB                 "canvasIdRGB"
    //! ID of the canvas with IR frames, without pattern (integer, default 4)
    #define HF_CAMERA_CANVAS_ID_IR                  "canvasIdIR"


    //! Current camera FPS
    #define HF_PARAMETER_CAMERA_FPS 0
    //! Current camera frame width
    #define HF_PARAMETER_CAMERA_WIDTH 1
    //! Current camera frame height
    #define HF_PARAMETER_CAMERA_HEIGHT 2


    //! FACE ENGINE SETTINGS

    //! FaceEngineGroup settings

    //! Algorithm IDs for detection, recognition and liveness engines,
    //! (int, range: HFAlgorithm, default: see defines HF_FACE_ENGINE_DEFAULT_<engine type>_ALGORITHM below)
    #define HF_FACE_ENGINE_DET_ALGORITHM                    "detAlgorithm"
    #define HF_FACE_ENGINE_REC_ALGORITHM                    "recAlgorithm"
    #define HF_FACE_ENGINE_LIVE_ALGORITHM                   "liveAlgorithm"

    //! Note: default values of the algorithm IDs are different for each platform, each platform might
    //! have a unique combination of the biometric engines
    #define HF_FACE_ENGINE_DEFAULT_DET_ALGORITHM            HF_ALGORITHM_PARAVISION
    #define HF_FACE_ENGINE_DEFAULT_REC_ALGORITHM            HF_ALGORITHM_PARAVISION

    #ifdef _PLATFORM_CV22N_VISION_PLUS
        #define HF_FACE_ENGINE_DEFAULT_LIVE_ALGORITHM       HF_ALGORITHM_HID_PAD
    #else
        #define HF_FACE_ENGINE_DEFAULT_LIVE_ALGORITHM       HF_ALGORITHM_PARAVISION
    #endif

    //! Paths to the folders where the models are located. (string, default: specific to the algorithm,
    //! see HF_FACE_ENGINE_DEFAULT_MODELS_PATH_<algo name>)
    #define HF_FACE_ENGINE_DET_MODELS_PATH                  "detModelsPath"
    #define HF_FACE_ENGINE_REC_MODELS_PATH                  "recModelsPath"
    #define HF_FACE_ENGINE_LIVE_MODELS_PATH                 "liveModelsPath"

    #define HF_FACE_ENGINE_DEFAULT_MODELS_PATH_PARAVISION   "/hid/opt/hidface/models/paravision/"
    #define HF_FACE_ENGINE_DEFAULT_MODELS_PATH_HID_PAD      "/hid/opt/hidface/models/pad/"


    //! Detection settings

    //:TODO: are the quality thresholds still supported?
    //! Quality threshold for enrollment. (double, range: 0 - 1, default: FACE_ENGINE_DEFAULT_QUALITY_THRESHOLD_ENROLL)
    #define HF_FACE_ENGINE_QUALITY_THRES_ENROLL             "qualityThresEnroll"
    //! Quality threshold for verification. (double, range: 0 - 1, default: HF_FACE_ENGINE_DEFAULT_QUALITY_THRES_VERIFY)
    #define HF_FACE_ENGINE_QUALITY_THRES_VERIFY             "qualityThresVerify"
    //! Quality threshold for identification. (double, range: 0 - 1, default: HF_FACE_ENGINE_DEFAULT_QUALITY_THRES_IDENTIFY)
    #define HF_FACE_ENGINE_QUALITY_THRES_IDENTIFY           "qualityThresIdentify"

    #define HF_FACE_ENGINE_DEFAULT_QUALITY_THRES_ENROLL     0.7
    #define HF_FACE_ENGINE_DEFAULT_QUALITY_THRES_VERIFY     0.5
    #define HF_FACE_ENGINE_DEFAULT_QUALITY_THRES_IDENTIFY   0.5


    //! Recognition settings

    //! Security level. (HFSecurityLevel, default: FACE_ENGINE_DEFAULT_SECURITYLEVEL)
    #define HF_FACE_ENGINE_SECURITY_LEVEL                   "securityLevel"
    #define HF_FACE_ENGINE_DEFAULT_SECURITYLEVEL            HFSECURITYLEVEL_NORMAL


    //! Liveness settings

    //! Spoof probability threshold. (double, range: 0 - 1, default: FACE_ENGINE_DEFAULT_SPOOF_THRESHOLD)
    #define HF_FACE_ENGINE_SPOOF_THRESHOLD                  "spoofThreshold"
    //! Minimum distance of the face from the camera. (double, range: 0 - max, default: FACE_ENGINE_DEFAULT_ANTISPOOF_MIN_DISTANCE_M)
    //! Not recommended to set too low - depth camera does not compute the depth for the objects that are too close.
    //! Note: Not supported by FaceEnginePAD
    #define HF_FACE_ENGINE_ANTISPOOF_MIN_DISTANCE_M         "antispoofMinDistanceM"
    //! Maximum distance of the face from the camera. (double, range: 0 - max, default: FACE_ENGINE_DEFAULT_ANTISPOOF_MAX_DISTANCE_M)
    //! Note: Not supported by FaceEnginePAD
    #define HF_FACE_ENGINE_ANTISPOOF_MAX_DISTANCE_M         "antispoofMaxDistanceM"
    //! Accept only faces that are in the center of the image. (bool, default: FACE_ENGINE_DEFAULT_ANTISPOOF_ONLY_CENTERED_FACES)
    #define HF_FACE_ENGINE_ANTISPOOF_ONLY_CENTERED_FACES    "antispoofOnlyCenteredFaces"

    #define HF_FACE_ENGINE_DEFAULT_SPOOF_THRESHOLD                  0.50
    #define HF_FACE_ENGINE_DEFAULT_ANTISPOOF_MIN_DISTANCE_M         0.2
    #define HF_FACE_ENGINE_DEFAULT_ANTISPOOF_MAX_DISTANCE_M         1.0
    #define HF_FACE_ENGINE_DEFAULT_ANTISPOOF_ONLY_CENTERED_FACES    true

#ifdef __cplusplus
}
#endif

#endif // __HFTYPES_PRIVATE_H__
