/*************************************************************************************************************************
**                                                                                                                      **
** ©Copyright 2019 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                           **
**                                                                                                                      **
** For a list of applicable patents and patents pending, visit www.hidglobal.com/patents                                **
**                                                                                                                      **
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                                           **
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS                                     **
** FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR                                       **
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER                                       **
** IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN                                              **
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                           **
**                                                                                                                      **
*************************************************************************************************************************/

#pragma once


/***********************************************************************************************************************

VERSION: Generic
-----------------
For more information on this API please reference the "VCOMCrypto Integration Kit Overview.pdf" document provided
with this code. The sections specified in the comments for each function map directly to this version of the document.

************************************************************************************************************************/


#ifdef __cplusplus
#define _C_ "C"
#else
#define _C_
#endif

#ifdef    VCOMCORE_EXPORTS
#define VCOM_CORE_EXPORT extern _C_ __declspec(dllexport)
#define _STDCALL
#else
#define VCOM_CORE_EXPORT extern _C_   // for static lib
#define _STDCALL
#endif

#include <vector>
#include <string>
#include <sstream>
#include "V100_shared_types.h"
#include "V100_enc_types.h"
#include "IEncCmd.h"

#include <HFTypes.h>

#include <VcomBaseTypes.h>

typedef _V100_GENERAL_ERROR_CODES V100_ERROR_CODE;
typedef int(*StatusCallBack)(int);

/************************************************************************************************************************

V100_Cancel Operation
Cancels capture related commands

V100_ERROR_CODE V100_Cancel_Operation(const V100_DEVICE_TRANSPORT_INFO* pDev)

Parameters
pDev                Pointer to Device handle

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
Use this command to cancel a capture operation initiated by V100_Enc_Capture. It returns GEN_OK or GEN_ERROR_APP_BUSY if
the system is busy. In either case the user must poll for completion using V100_Get_Acq_Status for acquisition status
of the active capture operation.

See also
V100_Enc_Capture, V100_Get_Acq_Status

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Cancel_Operation(const V100_DEVICE_TRANSPORT_INFO* pDev);

/***********************************************************************************************************************

V100_Close
Closes communication to device

V100_ERROR_CODE V100_Close (V100_DEVICE_TRANSPORT_INFO* pDev)

Parameters
pDev                Pointer to Device handle

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
Applications must call this function to close communication with a device before exiting and if reestablishing a connection
to the Device if the previous handle connection is lost or has timed out.

See also
V100_Open

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Close(V100_DEVICE_TRANSPORT_INFO* pDev);

/***********************************************************************************************************************

V100_Get_Acq_Status
Returns the status of the current acquisition

V100_ERROR_CODE V100_Get_Acq_Status (const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ACQ_STATUS_TYPE* pACQ_Status)

Parameters
pDev                Pointer to device handle
pACQ_Status         Pointer to the status structure returned

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful

Remarks
See _V100_ACQ_STATUS_TYPE structure in Appendix A for available acquisition status codes returned.

See also
V100_Enc_Capture

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Get_Acq_Status(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ACQ_STATUS_TYPE* pACQ_Status);

/***********************************************************************************************************************

V100_Get_Config
Returns device configuration structure

V100_ERROR_CODE V100_Get_Config (const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_INTERFACE_CONFIGURATION_TYPE * ICT)

Parameters
pDev                Pointer to device handle
ICT                 Pointer to configuration structure to be returned

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
See _V100_INTERFACE_CONFIGURATION_TYPE in Appendix A structure for information on configuration structure.

This is a handy call to ensure that proper communication is established with the sensor.

See also
None

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Get_Config(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_INTERFACE_CONFIGURATION_TYPE* ICT);

/***********************************************************************************************************************

7.7    V100_Get_Sensor_Type
Get the type of Lumidigm fingerprint sensor connected

V100_ERROR_CODE V100_Get_Sensor_Type(const V100_DEVICE_TRANSPORT_INFO *pDev, _V100_SENSOR_TYPE& sensorType);

Parameters
pDev                Pointer to device handle
sensorType            Type of sensor that the device handle is communicating with

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful

Remarks
This is a “composite function” that does not implement one specific VCOM Command but rather uses multiple commands
to determine the sensor type connected.

See also
None

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Get_Sensor_Type(const V100_DEVICE_TRANSPORT_INFO *pDev, _V100_SENSOR_TYPE& sensorType);

/***********************************************************************************************************************

V100_Get_Num_USB_Devices
Returns the number of Devices attached to the host system

V100_ERROR_CODE V100_Get_Num_USB_Devices (u32 * nNumDevices)

Parameters
nNumDevices         Number of devices attached to system

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful

Remarks
None

See also
None

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE _STDCALL V100_Get_Num_USB_Devices(int* nNumDevices);

/***********************************************************************************************************************

V100_Get_OP_Status
Retrieves the current status of a firmware update operation

V100_ERROR_CODE V100_Get_OP_Status (const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_OP_STATUS* opStatus)

Parameters
pDev                Pointer to device handle
opStatus            OP Status structure returned upon success. See table in this section

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful

Remarks
See _V100_OP_STATUS and _V100_OP_MODE structures in Appendix A for available operation status and mode codes returned.

Only applies to a V100_UpdateFirmware operation.

See also
V100_UpdateFirmware

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Get_OP_Status(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_OP_STATUS& opStatus);

/***********************************************************************************************************************

V100_Get_Serial
Returns device serial number

V100_ERROR_CODE V100_Get_Serial(const V100_DEVICE_TRANSPORT_INFO* pDev, u32* pSerialNumber)

Parameters
pDev                Pointer to device handle
pSerialNumber        Pointer to serial number to be returned

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
Overrides the method of obtaining the serial number of the Device from the ‘Device_Serial_Number?and ‘Device_Serial_Number_Ex?
fields in the _V100_INTERFACE_CONFIGURATION_TYPE structure. This method of obtaining the serial number is preferred, but
they are equivalent.

See Appendix A for detailed definition of the _V100_INTERFACE_CONFIGURATION_TYPE structure.

See also
None

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Get_Serial(const V100_DEVICE_TRANSPORT_INFO* pDev, u32* pSerialNumber);

/***********************************************************************************************************************

V100_Get_Status
Returns device Status structure which contains all device error codes, conditions and health monitoring data

V100_ERROR_CODE V100_Get_Status(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_INTERFACE_STATUS_TYPE* pIST)

Parameters
pDev                Pointer to device handle
ICT                 pointer to Status structure to be returned

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
See _V100_INTERFACE_STATUS_TYPE in Appendix A structure for information on Status structure..

See also
None

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Get_Status(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_INTERFACE_STATUS_TYPE* pIST);

/***********************************************************************************************************************

V100_GetSystemDiagnostics
Returns device system diagnostics structure which contains Device health monitoring data

V100_ERROR_CODE V100_GetSystemDiagnostics(V100_DEVICE_TRANSPORT_INFO *pDev, _V100_SYSTEM_DIAGNOSTICS* pSysDiag)

Parameters
pDev                Pointer to device handle
pSysDiag            pointer to System diagnostics structure to be returned

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
None

See also
None

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_GetSystemDiagnostics(V100_DEVICE_TRANSPORT_INFO *pDev, _V100_SYSTEM_DIAGNOSTICS* pSysDiag);

/***********************************************************************************************************************

V100_Get_Tag
Retrieves tag data set by V100_Set_Tag

V100_ERROR_CODE V100_Get_Tag(const V100_DEVICE_TRANSPORT_INFO* pDev, u8* pTag, u16& nTagLength)

Parameters
pDev                Pointer to Device handle
pTag                Data buffer which stores the tag, upon success
nTagLenth            Size of the data returned in bytes

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
If tag size is unknown create output buffer with maximum length of 255 bytes.

See Also
V100_Set_Tag

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Get_Tag(const V100_DEVICE_TRANSPORT_INFO* pDev, u8* pTag, u16& nTagLength);

/***********************************************************************************************************************

V100_Open
Opens a connection to the device using USB interface

V100_ERROR_CODE V100_Open (V100_DEVICE_TRANSPORT_INFO* pDev)

Parameters
pDev                Pointer to Device handle

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
An application must call this function to begin communication with a device.

V40x sensors have watchdogs that force the device to reboot itself in the event of a terminal USB communication event or
an unrecoverable system event. Client applications must handle these reboot events appropriately to maintain communication
with the device once it has recovered from the reboot and make this call again after calling V100_Close with the previous
handle connection.

V40x devices only support USB communication at this time.

See also
V100_Close

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE _STDCALL V100_Open(V100_DEVICE_TRANSPORT_INFO* pDev);

/***********************************************************************************************************************

V100_Reset
Reboots the device

V100_ERROR_CODE V100_Reset (const V100_DEVICE_TRANSPORT_INFO* pDev)

Parameters
pDev                Pointer to device handle

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
Issues a System Reset (Reboot) command to the device.

See also
None

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Reset(const V100_DEVICE_TRANSPORT_INFO* pDev);

/***********************************************************************************************************************

V100_Release_Memory
Releases Memory allocated by the VCOM library

V100_ERROR_CODE V100_Release_Memory(const V100_DEVICE_TRANSPORT_INFO* pDev, void* pMem)

Parameters
pDev                Pointer to device handle
pMem                 pointer to Memory to release

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
None

See also
None

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Release_Memory(const V100_DEVICE_TRANSPORT_INFO* pDev, void* pMem);

/***********************************************************************************************************************

V100_Set_Tag
Sets custom tag data that persists on the file system for future use. Can be retrieved using V100_Get_Tag

V100_ERROR_CODE V100_Set_Tag(const V100_DEVICE_TRANSPORT_INFO* pDev, u8* pTagData, u16 nTagDataSize)

Parameters
pDev                 Pointer to Device handle
pTagData            An input data packet containing the tag data to set
nTagDataSize        The size of the input data packet


Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK                 Indicates operation was successful.

Remarks
The pTagData is a populated buffer of custom tag data where the maximum length is 255 bytes

See also
V100_Set_Tag

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Set_Tag(const V100_DEVICE_TRANSPORT_INFO* pDev, u8* pTagData, u16 nTagDataSize);

/***********************************************************************************************************************

V100_Enc_Clear
Clears all data from the last capture

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Clear(const V100_DEVICE_TRANSPORT_INFO* pDev)

Parameters
None

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
None

See also
<TODO> Clean this up<>V100_Enc_Capture, V100_Enc_ReturnCapturedTemplate, V100_Enc_ReturnCaptured_WSQ, V100_Enc_Get_Spoof_Score, V100_Enc_Enroll,
V100_Enc_Verify

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Clear(const V100_DEVICE_TRANSPORT_INFO* pDev);

/***********************************************************************************************************************

V100_Enc_Generate_RSA_Keys
Generate Device asymmetric keys (macro operation)

V100_ERROR_CODE V100_Enc_Generate_RSA_Keys (const V100_DEVICE_TRANSPORT_INFO* pDev)

Parameters
pDev                 Pointer to device handle

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK                 Indicates operation was successful.

Remarks
This call generates 2048 bit RSA key pairs.

Client must poll for completion using V100_Get_OP_Status.

This call is only available when the Device is in Base Configuration.

See also
V100_Get_OP_Status

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Generate_RSA_Keys(const V100_DEVICE_TRANSPORT_INFO* pDev);

/***********************************************************************************************************************

V100_Enc_Get_Key
Get the specified key

V100_ERROR_CODE V100_Enc_Get_Key (const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ENC_KEY_TYPE nKeyType, u2048 pKey,
                                 u16& nKeyVersion, u8* pKCV, u16& nKeySize, u16& nKeyMode)

Parameters
pDev                 Pointer to device handle
nKeyType            The key (slot) to retrieve
pKey                The specified key
nKeyVersion            The version of the returned key
pKCV                The key check value of the returned key
nKeySize            The size of the returned key
nKeyMode            The key mode of the returned key


Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK                 Indicates operation was successful.

Remarks
When the Device is in the Base Configuration the only key slot available to request is the Device’s Public RSA Key
(KDevPublic in slot KT_EXTKEY_DEVICE_PUBLIC). The KCV is returned as a SHA-256 Hash.

See also
V100_Enc_Generate_RSA_Keys

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Get_Key(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ENC_KEY_TYPE nKeyType, u2048 pKey, u16& nKeyVersion, u8* pKCV, u16& nKeySize, u16& nKeyMode);

/***********************************************************************************************************************

V100_Enc_Get_KeyVersion
Get the Key Version and Key Check Value (KCV) of the specified key slot

V100_ERROR_CODE V100_Enc_Get_KeyVersion (const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ENC_KEY_TYPE nKeySlot, u16& nKeyVersion,
                                         u8* pKCV, u16& nKeyMode)

Parameters
pDev                 Pointer to device handle
nKeySlot            The key slot to check
nKeyVersion            The version of the specified key
pKCV                The key check value of the specified key
nKeyMode            The key mode of the specified key


Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK                 Indicates operation was successful.

Remarks
When the Device is in the Base Configuration the only key slot available to request is the KCTK.
When the Device is in the MSK01 Configuration all loaded key slots can be requested.

See also
V100_Enc_Set_Key

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Get_KeyVersion(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ENC_KEY_TYPE nKeySlot, u16& nKeyVersion, u8* pKCV, u16& nKeyMode);

/***********************************************************************************************************************

V100_Enc_Get_Rnd_Number
Gets a random number from the device

V100_ERROR_CODE V100_Enc_Get_Rnd_Number(const V100_DEVICE_TRANSPORT_INFO* pDev, u256 rndNumber, int nBytesRequested)

Parameters
pDev                Pointer to device handle
rndNumber           A Nonce from the device (ANBIO)
nBytesRequested        Size of random number (up to 32 bytes)


Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
ANBIO is the Nonce (of ANBIO_LENGTH bytes) generated by the Device.

See also
None

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Get_Rnd_Number (const V100_DEVICE_TRANSPORT_INFO* pDev, u256 rndNumber, int nBytesRequested);

/***********************************************************************************************************************

V100_Enc_Get_Serial_Number
Gets the serial number of the device’s secure element

V100_ERROR_CODE V100_Enc_Get_Serial_Number(const V100_DEVICE_TRANSPORT_INFO* pDev, u64& SerialNumber)

Parameters
pDev                Pointer to device handle
SerialNumber        8-byte serial number of the hardware secure element

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK              Indicates operation was successful.

Remarks
None

See also
V100_Enc_Factory_Set_Key

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Get_Serial_Number(const V100_DEVICE_TRANSPORT_INFO* pDev, u64& SerialNumber);

/***********************************************************************************************************************

V100_Enc_Set_Key
Loads the specified key onto the Device. Used for factory and remote key programming.

V100_ERROR_CODE V100_Enc_Set_Key (V100_DEVICE_TRANSPORT_INFO *pDev, u8* pCGKey, uint nCGKeySize, _V100_ENC_KEY_TYPE nKeyType,
                                  u8* pCGOutputData, u32& nCGOutputDataSize)

Parameters
pDev                 Pointer to device handle
pCGKey                 The new key encrypted with the appropriate Transport Key
nCGKeySize             The size of the cryptogram going in
nKeyType             The key type (slot) that we are setting
pCGOutputData         The output data packet containing the ANSOL for mutual authentication
nCGOutputDataSize     The size of the output data packet cryptogram


Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK                 Indicates operation was successful.

Remarks
When the Device is in the Base Configuration the only key slot available to load is the Customer Transport Key (KCTK in slot KT_EXTKEY_CTK).
When the Device is in the HYB02 Configuration all static customer key slots are available to load.

The pCGKey cryptogram is constructed as follows:

KeyBlock = [ ANBIO(16 bytes) + ANSOL(16 bytes) + Slot(2 bytes) + Version(2 bytes) + KeyMode(2 bytes) +
             KCV(4 bytes) + KeyCryptoSize(2 bytes) + KeyVal(variable size) + Padding ]

The KeyCryptoSize field indicates if the KeyVal field is a cryptogram or a raw key value.
    If it is set to zero, the KeyVal is a raw key value.
    If non-zero, the value specifies the size of the KeyVal cryptogram in bytes.

The KeyBlock is then encrypted with the appropriate Transport Key for the operation:

    Customer Provisioning
    CG = RSA [ KeyBlock ] KDevPublic

    Factory Key Load
    CG = KeyModeCTK [ KeyBlock ] KCTK

    Remote Key Load
    CG = KeyModeBTK [ KeyBlock ] KBTK

The KeyMode specified in the KeyBlock must be valid for the static key slots and cannot be changed from what is defined in the key map.

The pCGOutData contains the ANSOL encrypted with the new key
    CG = KeyModeNewKey [ ANSOL + PAD ] KNewKey

See also
V100_Enc_Get_KeyVersion

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Set_Key(const V100_DEVICE_TRANSPORT_INFO *pDev, u8* pCGKey, uint nCGKeySize, _V100_ENC_KEY_TYPE nKeyType, u8** pCGOutputData, u32& nCGOutputDataSize);

/***********************************************************************************************************************

V100_Enc_Unock_Key
Decommission the Device by removing the existing Customer Transport Key on the unit.

V100_ERROR_CODE V100_Enc_Unlock_Key(const V100_DEVICE_TRANSPORT_INFO *pDev, const u8* pCGInputData, u32 nCGInputDataSize)

Parameters
pDev                 Pointer to device handle
pCGInputData        An input data packet containing the message to Decommission the Device
nCGInputDataSize    The size of the input data packet

Returns
V100_ERROR_CODE     Refer to error code documentation in Appendix B for detailed description of possible return values.
GEN_OK                 Indicates operation was successful.

Remarks

Only allowed when the Device is in the Base Configuration.

Input Data Packet

    The pCGInputData packet is generated by requesting a random number from the Device (ANBIO) and encrypting it with the
    currently loaded KCTK. The payload format will be:

    Payload = KeyModeCTK [ ANBIO (16 bytes) ] KCTK

See also
V100_Enc_Get_Rnd_Number, V100_Enc_Set_Key

************************************************************************************************************************/
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Unlock_Key(const V100_DEVICE_TRANSPORT_INFO *pDev, const u8* pCGInputData, u32 nCGInputDataSize);



VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Init(const V100_DEVICE_TRANSPORT_INFO* const pDev);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Enum_Cams(const V100_DEVICE_TRANSPORT_INFO* const pDev, string_list_t& str_list);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Set_Param_Int(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, int32_t val);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Get_Param_Int(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, int32_t& val);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Set_Param_Str(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, const char* const val);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Get_Param_Str(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, std::string& val);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Set_Param_Lng(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, double val);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Get_Param_Lng(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, double& val);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Set_Param_Bin(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, const uint8_t* const val_ptr, uint32_t val_len);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Get_Param_Bin(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, bin_data_t& val_len);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Open_Context(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t cam_id, uint32_t algo_type, int32_t& ctx);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Close_Context(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Stop_Operation(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Close_Opeation(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Extract_Template(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, int32_t imageEncoding, const void* const img_ptr, uint32_t img_len, uint64_t flags, int32_t& val);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Match_With_Template(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, const void* const templA_ptr, uint32_t len_A, const void* const templB_ptr, uint32_t len_B, int32_t& val);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Verify_With_Captured(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, const char* const galery, const char* const id, double minScore);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Identify_With_Captured(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, const char* const gallery, double minScore);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Match_With_Captured(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, const uint8_t* const bin, uint32_t len);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Identify_With_Template(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, const char* const gal_ptr, double minScore, const uint8_t* const templ_ptr, uint32_t teml_len, int32_t& val);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Verify_With_Template(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, double minScore, const char* const gal_ptr, const char* const id_ptr, const uint8_t* const templ_ptr, uint32_t templ_len, int32_t& val);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Capture_Img(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, int32_t timeout, double minimalQuality, double minimalLivenessScore, uint64_t intermediateResultFlags, uint64_t finalResultFlags, int32_t& val);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Parse_Res_Int(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Parse_Res_Double(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Parse_Res_Data(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Parse_Res_Point(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Parse_Res_Image(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Parse_Match_Gallery(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Get_Video_Frame(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, int64_t sec, int64_t& seq, int32_t& encoding, bin_data_t& data_bin);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Get_Intermediate_Res(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t operation, uint64_t resultFlags, int32_t lastSequenceNumber, v100_hfres_t& hfRes );

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Get_Final_Res(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t operation, uint64_t resultFlags, v100_hfres_t& hfRes);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Db_Add_Record_With_Captured(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t op, bool replace, const char* const id, const char* const gallery, const char* const custom);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Db_Add_Record_With_Template(const V100_DEVICE_TRANSPORT_INFO* const pDev, bool replace, const char* const id, const char* const gallery, const char* const custom, const uint8_t* const data_ptr, uint32_t data_len);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Db_Get_Record(const V100_DEVICE_TRANSPORT_INFO* const pDev, const char* const id, const char* const gallery);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Db_List_Records(const V100_DEVICE_TRANSPORT_INFO* const pDev, const char* const gallery);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Db_Del_Record(const V100_DEVICE_TRANSPORT_INFO* const pDev, const char* id, const char* gallery);

VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_FwUpdate(const V100_DEVICE_TRANSPORT_INFO* const pDev, const char* const file_name);




// Atomic_Hid_Terminate
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Terminate(const V100_DEVICE_TRANSPORT_INFO* const pDev);

// Atomic_Hid_Db_Del_Record
VCOM_CORE_EXPORT V100_ERROR_CODE V100_Hid_Db_Del_Record(const V100_DEVICE_TRANSPORT_INFO* const pDev, const char* const id, const char* const gallery);

/*********************** GetVCOMErrorStr ***********************/
///////////////////////////////////////////////////////////////////////////////
///  global public  GetVCOMErrorStr
///  Return the string associated with the specified V100_ERROR_CODE
///
///  @param [in, out]  pDev        V100_DEVICE_TRANSPORT_INFO *    pointer to device handle
///  @param [in]       nEC        uint                            The return code to marshal
///
///  @return const char* that is the string associated with the specified V100_ERROR_CODE
///
///  @remarks
///
///  @see
///
///  @author www.lumidigm.com @date 8/21/2012
///////////////////////////////////////////////////////////////////////////////
VCOM_CORE_EXPORT const char* GetVCOMErrorStr(uint nEC);

/*********************** GetOpErrorStr ***********************/
///////////////////////////////////////////////////////////////////////////////
///  global public  GetOpErrorStr
///  Return the string associated with the specified _V100_OP_ERROR
///
///  @param [in, out]  pDev        V100_DEVICE_TRANSPORT_INFO *    pointer to device handle
///  @param [in]       nOpEC    uint                            The op error code to marshal
///
///  @return const char* that is the string associated with the specified _V100_OP_ERROR
///
///  @remarks
///
///  @see
///
///  @author www.lumidigm.com @date 8/21/2012
///////////////////////////////////////////////////////////////////////////////
VCOM_CORE_EXPORT const char* GetOpErrorStr(uint nOpEC);

/*********************** GetSensorTypeStr ***********************/
///////////////////////////////////////////////////////////////////////////////
///  global public  GetSensorTypeStr
///  Return the string associated with the specified _V100_SENSOR_TYPE
///
///  @param [in, out]  pDev        V100_DEVICE_TRANSPORT_INFO *    pointer to device handle
///  @param [in]       nType    uint                            The sensor type enum to marshal
///
///  @return const char* that is the string associated with the specified _V100_SENSOR_TYPE
///
///  @remarks
///
///  @see
///
///  @author www.lumidigm.com @date 11/22/2019
///////////////////////////////////////////////////////////////////////////////
VCOM_CORE_EXPORT const char* GetSensorTypeStr(uint nType);


// Helper functions
V100_ERROR_CODE PollCaptureCompletion(const V100_DEVICE_TRANSPORT_INFO* pDev, StatusCallBack func);
V100_ERROR_CODE SendEncCommandNoResponse(const V100_DEVICE_TRANSPORT_INFO* pDev, int reoute_type, IEncCmd* pCmd);
V100_ERROR_CODE SendEncCommandResponse(const V100_DEVICE_TRANSPORT_INFO* pDev, int reoute_type, IEncCmd* pCommand, uchar** pKey, u32& nKeySize);
