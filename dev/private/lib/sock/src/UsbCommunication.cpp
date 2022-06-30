#ifndef _WIN32
    #include "windows_conv.h"
#else
    #include <windows.h>
#endif

#include <iostream>
#include <cassert>
#include <iterator>

#include <UsbCommunication.h>
// #include "V100CommandHandler.h"
// #include "V100Cmd.h"
// #include "V100EncCmd.h"
// #include "IMemMgr.h"

// #include <HFTypesPrivate.h>

// #ifndef __GNUC__
// #include "V100DeviceHandler.h"
// EXTERN_C const GUID GUID_BULKLDI;
// #endif
// 
// static HFResult   g_hfResult;

#define PERIOD               (1)
#define ROUTE_JENGINE        (0)
#define ROUTE_USBTRANSCEIVER (1)

USB_ERROR_CODE V100_Open(V100_DEVICE_TRANSPORT_INFO *pDev) {

    USB_ERROR_CODE rc = USB_ERROR_CODE::GEN_OK;

    #ifdef _WIN32
        #ifndef _WIN32_WCE

            Device* pDevice = new Device();

            rc = V100DeviceHandler::GetV100DeviceHandler()->GetDevice(pDev->DeviceNumber, pDevice);
            if ( USB_ERROR_CODE::GEN_OK != rc && pDev->nBaudRate == 0 ) {
                delete pDevice;
                pDevice = NULL;
                pDev->hInstance = NULL;
                return rc;
            }

            // The Device object is now the hInstance
            pDev->hInstance = pDevice;

        #endif
    #endif

    rc = USB_ERROR_CODE::GEN_OK;
    V100CommandHandler* pVCH = V100CommandHandler::GetCommandHandler(pDev);
    if (0 != pVCH->Initialize(pDev)) {
        pVCH->Close(pDev);
        delete pVCH;
        rc = USB_ERROR_CODE::GEN_ERROR_START;
    }

    #ifdef _WIN32
        #ifndef _WIN32_WCE
            delete pDevice;
            pDevice = NULL;
            V100DeviceHandler::ReleaseV100DeviceHandler();
        #endif
    #endif

    return rc;
}

USB_ERROR_CODE V100_Close(V100_DEVICE_TRANSPORT_INFO *pDev) {
    V100CommandHandler* pVCH = V100CommandHandler::GetCommandHandler(pDev);
    pVCH->Close(pDev);
    delete pVCH;
    return GEN_OK;
}

USB_ERROR_CODE V100_Get_Num_USB_Devices(int* nNumDevices) {
    #ifdef __GNUC__
        *nNumDevices = 1;
    #elif _WIN32_WCE
        *nNumDevices = 1;
    #else
        V100_ERROR_CODE rc = V100DeviceHandler::GetV100DeviceHandler()->GetNumDevices(nNumDevices);
        V100DeviceHandler::ReleaseV100DeviceHandler();
        return rc;
    #endif
    return GEN_OK;
}

const char* GetVCOMErrorStr(uint nEC) {
    switch (nEC) {
        case GEN_OK:                             {return "GEN_OK"; } break;
        case GEN_ENCRYPTION_FAIL:                {return "GEN_ENCRYPTION_FAIL"; } break;
        case GEN_DECRYPTION_FAIL:                {return "GEN_DECRYPTION_FAIL"; } break;
        case GET_PD_INIT_FAIL:                   {return "GET_PD_INIT_FAIL"; } break;
        case PD_HISTOGRAM_FAIL:                  {return "PD_HISTOGRAM_FAIL"; } break;
        case INVALID_ACQ_MODE:                   {return "INVALID_ACQ_MODE"; } break;
        case BURNIN_THREAD_FAIL:                 {return "BURNIN_THREAD_FAIL"; } break;
        case UPDATER_THREAD_FAIL:                {return "UPDATER_THREAD_FAIL"; } break;
        case GEN_ERROR_START:                    {return "GEN_ERROR_START"; } break;
        case GEN_ERROR_PROCESSING:               {return "GEN_ERROR_PROCESSING"; } break;
        case GEN_ERROR_VERIFY:                   {return "GEN_ERROR_VERIFY"; } break;
        case GEN_ERROR_MATCH:                    {return "GEN_ERROR_MATCH"; } break;
        case GEN_ERROR_INTERNAL:                 {return "GEN_ERROR_INTERNAL"; } break;
        case GEN_ERROR_INVALID_CMD:              {return "GEN_ERROR_INVALID_CMD"; } break;
        case GEN_ERROR_PARAMETER:                {return "GEN_ERROR_PARAMETER"; } break;
        case GEN_NOT_SUPPORTED:                  {return "GEN_NOT_SUPPORTED"; } break;
        case GEN_INVALID_ARGUEMENT:              {return "GEN_INVALID_ARGUEMENT"; } break;
        case GEN_ERROR_TIMEOUT:                  {return "GEN_ERROR_TIMEOUT"; } break;
        case GEN_ERROR_LICENSE:                  {return "GEN_ERROR_LICENSE"; } break;
        case GEN_ERROR_COMM_TIMEOUT:             {return "GEN_ERROR_COMM_TIMEOUT"; } break;
        case GEN_FS_ERR_CD:                      {return "GEN_FS_ERR_CD"; } break;
        case GEN_FS_ERR_DELETE:                  {return "GEN_FS_ERR_DELETE"; } break;
        case GEN_FS_ERR_FIND:                    {return "GEN_FS_ERR_FIND"; } break;
        case GEN_FS_ERR_WRITE:                   {return "GEN_FS_ERR_WRITE"; } break;
        case GEN_FS_ERR_READ:                    {return "GEN_FS_ERR_READ"; } break;
        case GEN_FS_ERR_FORMAT:                  {return "GEN_FS_ERR_FORMAT"; } break;
        case GEN_ERROR_MEMORY:                   {return "GEN_ERROR_MEMORY"; } break;
        case GEN_ERROR_RECORD_NOT_FOUND:         {return "GEN_ERROR_RECORD_NOT_FOUND"; } break;
        case GEN_VER_INVALID_RECORD_FORMAT:      {return "GEN_VER_INVALID_RECORD_FORMAT"; } break;
        case GEN_ERROR_DB_FULL:                  {return "GEN_ERROR_DB_FULL"; } break;
        case GEN_ERROR_INVALID_SIZE:             {return "GEN_ERROR_INVALID_SIZE"; } break;
        case GEN_ERROR_TAG_NOT_FOUND:            {return "GEN_ERROR_TAG_NOT_FOUND"; } break;
        case GEN_ERROR_APP_BUSY:                 {return "GEN_ERROR_APP_BUSY"; } break;
        case GEN_ERROR_DEVICE_UNCONFIGURED:      {return "GEN_ERROR_DEVICE_UNCONFIGURED"; } break;
        case GEN_ERROR_TIMEOUT_LATENT:           {return "GEN_ERROR_TIMEOUT_LATENT"; } break;
        case GEN_ERROR_DB_NOT_LOADED:            {return "GEN_ERROR_DB_NOT_LOADED"; } break;
        case GEN_ERROR_DB_DOESNOT_EXIST:         {return "GEN_ERROR_DB_DOESNOT_EXIST"; } break;
        case GEN_ERROR_ENROLLMENT_INCOMPLETE:    {return "GEN_ERROR_ENROLLMENT_INCOMPLETE"; } break;
        case GEN_ERROR_USER_NOT_FOUND:           {return "GEN_ERROR_USER_NOT_FOUND"; } break;
        case GEN_ERROR_DB_USER_FINGERS_FULL:     {return "GEN_ERROR_DB_USER_FINGERS_FULL"; } break;
        case GEN_ERROR_DB_USERS_FULL:            {return "GEN_ERROR_DB_USERS_FULL"; } break;
        case GEN_ERROR_USER_EXISTS:              {return "GEN_ERROR_USER_EXISTS"; } break;
        case GEN_ERROR_DEVICE_NOT_FOUND:         {return "GEN_ERROR_DEVICE_NOT_FOUND"; } break;
        case GEN_ERROR_DEVICE_NOT_READY:         {return "GEN_ERROR_DEVICE_NOT_READY"; } break;
        case GEN_ERROR_PIPE_READ:                {return "GEN_ERROR_PIPE_READ"; } break;
        case GEN_ERROR_PIPE_WRITE:               {return "GEN_ERROR_PIPE_WRITE"; } break;
        case GEN_ERROR_SENGINE_SHUTTING_DOWN:    {return "GEN_ERROR_SENGINE_SHUTTING_DOWN"; } break;
        case GEN_ERROR_SPOOF_DETECTED:           {return "GEN_ERROR_SPOOF_DETECTED"; } break;
        case GEN_ERROR_DATA_UNAVAILABLE:         {return "GEN_ERROR_DATA_UNAVAILABLE"; } break;
        case GEN_ERROR_CRYPTO_FAIL:              {return "GEN_ERROR_CRYPTO_FAIL"; } break;
        case GEN_ERROR_CAPTURE_CANCELLED:        {return "GEN_ERROR_CAPTURE_CANCELLED"; } break;
        case GEN_LAST:                           {return "GEN_LAST"; } break;
        default:                                 {return "** Unknown Error Code **"; } break;
    }

}

const char* GetOpErrorStr(uint nParameter) {
    switch (nParameter) {
        case STATUS_OK:                             {return "STATUS_OK"; } break;
        case ERROR_UID_EXISTS:                      {return "ERROR_UID_EXISTS"; } break;
        case ERROR_ENROLLMENT_QUALIFICATION:        {return "ERROR_ENROLLMENT_QUALIFICATION"; } break;
        case ERROR_UID_DOES_NOT_EXIST:              {return "ERROR_UID_DOES_NOT_EXIST"; } break;
        case ERROR_DB_FULL:                         {return "ERROR_DB_FULL"; } break;
        case ERROR_QUALIFICATION:                   {return "ERROR_QUALIFICATION"; } break;
        case ERROR_DEV_TIMEOUT:                     {return "ERROR_DEV_TIMEOUT"; } break;
        case ERROR_USER_CANCELLED:                  {return "ERROR_USER_CANCELLED"; } break;
        case ERROR_SPOOF_DETECTED:                  {return "ERROR_SPOOF_DETECTED"; } break;
        case ERROR_DB_EXISTS:                       {return "ERROR_DB_EXISTS"; } break;
        case ERROR_DB_DOES_NOT_EXIST:               {return "ERROR_DB_DOES_NOT_EXIST"; } break;
        case ERROR_ID_DB_TOO_LARGE:                 {return "ERROR_ID_DB_TOO_LARGE"; } break;
        case ERROR_ID_DB_EXISTS:                    {return "ERROR_ID_DB_EXISTS"; } break;
        case ERROR_ID_USER_EXISTS:                  {return "ERROR_ID_USER_EXISTS"; } break;
        case ERROR_ID_USER_NOT_FOUND:               {return "ERROR_ID_USER_NOT_FOUND"; } break;
        case STATUS_ID_MATCH:                       {return "STATUS_ID_MATCH"; } break;
        case STATUS_ID_NO_MATCH:                    {return "STATUS_ID_NO_MATCH"; } break;
        case ERROR_ID_PARAMETER:                    {return "ERROR_ID_PARAMETER"; } break;
        case ERROR_ID_GENERAL:                      {return "ERROR_ID_GENERAL"; } break;
        case ERROR_ID_FILE:                         {return "ERROR_ID_FILE"; } break;
        case ERROR_ID_NOT_INITIALIZED:              {return "ERROR_ID_NOT_INITIALIZED"; } break;
        case ERROR_ID_DB_FULL:                      {return "ERROR_ID_DB_FULL"; } break;
        case ERROR_ID_DB_DOESNT_EXIST:              {return "ERROR_ID_DB_DOESNT_EXIST"; } break;
        case ERROR_ID_DB_NOT_LOADED:                {return "ERROR_ID_DB_NOT_LOADED"; } break;
        case ERROR_ID_RECORD_NOT_FOUND:             {return "ERROR_ID_RECORD_NOT_FOUND"; } break;
        case ERROR_ID_FS:                           {return "ERROR_ID_FS"; } break;
        case ERROR_ID_CREATE_FAIL:                  {return "ERROR_ID_CREATE_FAIL"; } break;
        case ERROR_ID_INTERNAL:                     {return "ERROR_ID_INTERNAL"; } break;
        case ERROR_ID_MEM:                          {return "ERROR_ID_MEM"; } break;
        case STATUS_ID_USER_FOUND:                  {return "STATUS_ID_USER_FOUND"; } break;
        case STATUS_ID_USER_NOT_FOUND:              {return "STATUS_ID_USER_NOT_FOUND"; } break;
        case ERROR_ID_USER_FINGERS_FULL:            {return "ERROR_ID_USER_FINGERS_FULL"; } break;
        case ERROR_ID_USER_MULTI_FINGERS_NOT_FOUND: {return "ERROR_ID_USER_MULTI_FINGERS_NOT_FOUND"; } break;
        case ERROR_ID_USERS_FULL:                   {return "ERROR_ID_USERS_FULL"; } break;
        case ERROR_ID_OPERATION_NOT_SUPPORTED:      {return "ERROR_ID_OPERATION_NOT_SUPPORTED"; } break;
        case ERROR_ID_NOT_ENOUGH_SPACE:             {return "ERROR_ID_NOT_ENOUGH_SPACE"; } break;
        case ERROR_ID_DUPLICATE:                    {return "ERROR_ID_DUPLICATE"; } break;
        case ERROR_CAPTURE_TIMEOUT:                 {return "ERROR_CAPTURE_TIMEOUT"; } break;
        case ERROR_CAPTURE_LATENT:                  {return "ERROR_CAPTURE_LATENT"; } break;
        case ERROR_CAPTURE_CANCELLED:               {return "ERROR_CAPTURE_CANCELLED"; } break;
        case ERROR_CAPTURE_INTERNAL:                {return "ERROR_CAPTURE_INTERNAL"; } break;
        case ERROR_UPDATE_MEMORY_ERROR:             {return "ERROR_UPDATE_MEMORY_ERROR"; } break;
        case ERROR_UPDATE_DECRYPTION_FAIL:          {return "ERROR_UPDATE_DECRYPTION_FAIL"; } break;
        case ERROR_UPDATE_FIRMWARE_VERSION_ERROR:   {return "ERROR_UPDATE_FIRMWARE_VERSION_ERROR"; } break;
        case ERROR_UPDATE_FLASH_WRITE_ERROR:        {return "ERROR_UPDATE_FLASH_WRITE_ERROR"; } break;
        case ERROR_UPDATE_INVALID_TYPE:             {return "ERROR_UPDATE_INVALID_TYPE"; } break;
        case ERROR_UPDATE_FORMAT_ERROR:             {return "ERROR_UPDATE_FORMAT_ERROR"; } break;
        case ERROR_UPDATE_FIRMWARE_SIZE_ERROR:      {return "ERROR_UPDATE_FIRMWARE_SIZE_ERROR"; } break;
        case ERROR_UPDATE_RESTORE_FAIL:             {return "ERROR_UPDATE_RESTORE_FAIL"; } break;
        case ERROR_UPDATE_FIRMWARE_INVALID:         {return "ERROR_UPDATE_FIRMWARE_INVALID"; } break;
        case ERROR_CRYPTO_ERROR:                    {return "ERROR_CRYPTO_ERROR"; } break;
        case STATUS_NO_OP:                          {return "STATUS_NO_OP"; } break;
        default:                                    {return "UNKNOWN ERROR"; } break;
    }
//<TODO> update codes
    return "Unknown _V100_OP_STATUS";
}

const char* GetSensorTypeStr(uint nType) {
    switch (nType) {
        case VENUS_V30X:          {return "VENUS V30X"; } break;
        case MERCURY_M30X:        {return "MERCURY M30X"; } break;
        case MERCURY_M31X:        {return "MERCURY M31X"; } break;
        case VENUS_V31X:          {return "VENUS V31X"; } break;
        case VENUS_V371:          {return "VENUS V371"; } break;
        case VENUS_V40X:          {return "VENUS V40X"; } break;
        case VENUS_V42X:          {return "VENUS V42X"; } break;
        case MERCURY_M32X:        {return "MERCURY M32X"; } break;
        case MERCURY_M42X:        {return "MERCURY M42X"; } break;
        case MERCURY_M21X:        {return "MERCURY M21X"; } break;
        case VENUS_V310_10:       {return "VENUS V310_10"; } break;
        case UNKNOWN_LUMI_DEVICE:
        default:                  {return "UNKOWN_LUMI_DEVICE"; } break;
    }
}