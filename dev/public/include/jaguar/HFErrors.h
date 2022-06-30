#ifndef __HFERRORS_H__
#define __HFERRORS_H__

//! HFAPI Error codes

//! General error codes (0..99)
// ----------------------------

//! Success
#define HFERROR_OK                      0
//! Non-specific/general/internal/not yet documented error 
#define HFERROR_GENERAL                 1
//! Busy operation/camera
#define HFERROR_BUSY                    2
//! Resource not found
#define HFERROR_NOT_FOUND               3
//! Resource is already present
#define HFERROR_ALREADY_PRESENT         4
//! HFApi not initialized
#define HFERROR_NOT_INITIALIZED         5
//! HFApi already initialized
#define HFERROR_ALREADY_INITIALIZED     6
//! Argument missing or NULL
#define HFERROR_ARGUMENT_NULL           7
//! Invalid argument value
#define HFERROR_ARGUMENT_INVALID        8
//! Functionality is not yet implemented
#define HFERROR_NOT_IMPLEMENTED         9
//! Access denied
#define HFERROR_ACCESS_DENIED           10
//! HW error
#define HFERROR_HW_ERROR                11
//! Communication/network error
#define HFERROR_COMM_ERROR              12
//! The operation depends on system state or other prerequisites (e.g. configured key)
#define HFERROR_PREREQUISITES_NOT_MET   13
//! Operation terminated by timeout
#define HFERROR_TIMEOUT                 14
//! Operation was aborted/cancelled
#define HFERROR_ABORTED                 15
//! Unknown user requested
#define HFERROR_UNKNOWN_USER            16
//! Memory allocation failed
#define HFERROR_ALLOCATION_FAILED       17
//! Resource was already returned
#define HFERROR_ALREADY_RETURNED        18

//! Biometric error codes (100..199)
// ---------------------------------

//! General biometric error
#define HFERROR_BIO_GENERAL             100
//! More than one record in the identification database is matching given template
#define HFERROR_BIO_MATCH_CONFLICT      101
//! Input image quality is too low, below the set threshold
#define HFERROR_BIO_BAD_QUALITY         102
//! Input biometric data are not matching the target template(s)
#define HFERROR_BIO_MATCH_FAILED        103
//! Biometric algorithm failed
#define HFERROR_BIO_ALGO_FAILED         104
//! Incompatible biometrics (incompatible algorithms used to create them)
#define HFERROR_BIO_ALGO_INCOMPATIBLE   105

//! Camera error codes (200..299)
// ------------------------------

//! General camera error
#define HFERROR_CAMERA_GENERAL          200
//! Unknown camera used
#define HFERROR_CAMERA_UNKNOWN          201

#endif // __HFERRORS_H__