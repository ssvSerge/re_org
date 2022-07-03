#ifndef __VCOMBASETYPES_H__
#define __VCOMBASETYPES_H__

#include <vector>
#include <string>

typedef std::vector<std::string>  string_list_t;

typedef std::string               str_data_t;
typedef std::vector<uint8_t>      bin_data_t;

typedef struct _tag_v100_point {
    int32_t             x;
    int32_t             y;
}   v100_point_t;

typedef struct tag_v100_rect {
    v100_point_t        upperLeft;
    v100_point_t        bottomRight;
}   v100_rect_t;

typedef struct _tag_v100_match_rec {
    str_data_t          recordId;
    str_data_t          galleryID;
    str_data_t          customData;
    double              matchScore;
}   v100_match_rec_t;

typedef struct _tag_v100_landmarks {
    v100_point_t        leftEye;
    v100_point_t        rightEye;
    v100_point_t        nose;
    v100_point_t        mouthLeft;
    v100_point_t        mouthRight;
}   v100_landmarks_t;

typedef std::vector<v100_match_rec_t> v100_match_list_t;

typedef struct _tag_v100_hf_res {
    uint64_t            validFlags;
    int32_t             contextStatus;
    int32_t             errorCode;
    int32_t             imageEncoding;
    bin_data_t          image;
    uint32_t            sequenceNumber;
    int32_t             facesDetectedCount;
    double              quality;
    bin_data_t          templ;
    v100_rect_t         boundBox;
    double              spoofProbability;
    v100_match_list_t   matches;
    int32_t             isCaptured;             
    v100_landmarks_t    landmarks;
}   v100_hfres_t;



#endif

