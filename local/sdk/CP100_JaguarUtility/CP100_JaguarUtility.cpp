#include <iostream>
#include <VCOMBase.h>
#include <HostKeyManager.h>
#include <string>

#define CHECK_RES(x)

V100_DEVICE_TRANSPORT_INFO  hDev = { 0 };
uint8_t BIN_DATA[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
u8    TEMPLATE_A[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
u8    TEMPLATE_B[] = { 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11 };

static V100_ERROR_CODE _init(void) {

    V100_ERROR_CODE rc;

    _V100_INTERFACE_CONFIGURATION_TYPE devConfig;
    u32     nSN1;
    int     nDevices = 0;

    rc = V100_Get_Num_USB_Devices(&nDevices);
    CHECK_RES(rc);

    rc = V100_Open(&hDev);
    CHECK_RES(rc);

    rc = V100_Get_Config(&hDev, &devConfig);
    CHECK_RES(rc);

    rc = V100_Get_Serial(&hDev, &nSN1);
    CHECK_RES(rc);

    return GEN_OK;
}

static V100_ERROR_CODE _close(void) {
    V100_Close(&hDev);
    return GEN_OK;
}

static void _get_cams_list(void) {

    V100_ERROR_CODE retCode;
    string_list_t   strList;

    retCode = V100_Hid_Init(&hDev);
    CHECK_RES(retCode);

    retCode = V100_Hid_Enum_Cams(&hDev, strList);
    CHECK_RES(retCode);

    retCode = V100_Hid_Terminate(&hDev);
    CHECK_RES(retCode);
}

static void _get_set_get_parameters(void) {

    V100_ERROR_CODE retCode;
    int32_t         val_int32;
    std::string     val_str;
    bin_data_t      bin_data;
    bin_data_t      val_bin;
    double          val_dbl;

    retCode = V100_Hid_Init(&hDev);
    CHECK_RES(retCode);

    retCode = V100_Hid_Set_Param_Int(&hDev, 12345678, 23456789, 33445566);
    CHECK_RES(retCode);

    retCode = V100_Hid_Set_Param_Str(&hDev, 5555, 6666, "val_b_long_long_long_long_str");
    CHECK_RES(retCode);

    retCode = V100_Hid_Set_Param_Lng(&hDev, 77777777, 88888888, 0.12345);
    CHECK_RES(retCode);

    retCode = V100_Hid_Set_Param_Bin(&hDev, 88888888, 99999999, BIN_DATA, sizeof(BIN_DATA));
    CHECK_RES(retCode);

    retCode = V100_Hid_Get_Param_Int(&hDev, 12345678, 23456789, val_int32);
    CHECK_RES(retCode);

    retCode = V100_Hid_Get_Param_Str(&hDev, 66666666, 77777777, val_str);
    CHECK_RES(retCode);

    retCode = V100_Hid_Get_Param_Lng(&hDev, 77777777, 88888888, val_dbl);
    CHECK_RES(retCode);

    retCode = V100_Hid_Get_Param_Bin(&hDev, 99999999, 11111111, val_bin);
    CHECK_RES(retCode);

    retCode = V100_Hid_Terminate(&hDev);
    CHECK_RES(retCode);
}

static void _manage_context(void) {

    V100_ERROR_CODE retCode;
    int32_t         val_int32;

    retCode = V100_Hid_Init(&hDev);
    CHECK_RES(retCode);

    retCode = V100_Hid_Open_Context(&hDev, 13579, 24680, val_int32);
    CHECK_RES(retCode);

    retCode = V100_Hid_Stop_Operation(&hDev, 11223344);
    CHECK_RES(retCode);

    retCode = V100_Hid_Close_Context(&hDev, 12345678);
    CHECK_RES(retCode);

    retCode = V100_Hid_Terminate(&hDev);
    CHECK_RES(retCode);
}

static void _test_identify_verify_match() {

    V100_ERROR_CODE retCode;
    int32_t         val_int32;
    bin_data_t      bin_data;

    retCode = V100_Hid_Init(&hDev);
    CHECK_RES(retCode);

    retCode = V100_Hid_Verify_With_Captured(&hDev, 1234, "gal_as_string", "id_as_string", 0.25);
    CHECK_RES(retCode);

    retCode = V100_Hid_Verify_With_Template(&hDev, 2345, 0.12, "gal_as_string", "id_as_string", TEMPLATE_A, sizeof(TEMPLATE_A), val_int32);
    CHECK_RES(retCode);

    retCode = V100_Hid_Identify_With_Captured(&hDev, 3456, "gal_as_string", 0.5);
    CHECK_RES(retCode);

    retCode = V100_Hid_Identify_With_Template(&hDev, 4567, "gal_as_string", 0.2, TEMPLATE_A, sizeof(TEMPLATE_A), val_int32);
    CHECK_RES(retCode);

    retCode = V100_Hid_Match_With_Captured(&hDev, 5678, BIN_DATA, sizeof(BIN_DATA));
    CHECK_RES(retCode);

    retCode = V100_Hid_Match_With_Template(&hDev, 6789, TEMPLATE_A, sizeof(TEMPLATE_A), TEMPLATE_B, sizeof(TEMPLATE_B), val_int32);
    CHECK_RES(retCode);

    retCode = V100_Hid_Terminate(&hDev);
    CHECK_RES(retCode);
}

static void _test_db_commands() {

    V100_ERROR_CODE retCode;

    retCode = V100_Hid_Init(&hDev);
    CHECK_RES(retCode);

    retCode = V100_Hid_Db_Add_Record_With_Captured(&hDev, 0x1234, true, "id_as_str", "gal_as_str", "custom_as_str");
    CHECK_RES(retCode);

    retCode = V100_Hid_Db_Add_Record_With_Template(&hDev, true, "id_as_str", "gal_as_str", "custom_as_str", BIN_DATA, sizeof(BIN_DATA));
    CHECK_RES(retCode);

    retCode = V100_Hid_Db_Get_Record(&hDev, "id_as_str", "gal_as_str");
    CHECK_RES(retCode);

    retCode = V100_Hid_Db_List_Records(&hDev, "gal_as_str");
    CHECK_RES(retCode);

    retCode = V100_Hid_Terminate(&hDev);
    CHECK_RES(retCode);
}

static void _test_results() {

    V100_ERROR_CODE retCode;
    v100_hfres_t    hf_data;
    bin_data_t      data_bin;
    int32_t         val_int32;
    int64_t         val_int64;

    retCode = V100_Hid_Init(&hDev);
    CHECK_RES(retCode);

    retCode = V100_Hid_Get_Intermediate_Res(&hDev, 1234, 2345, 34567, hf_data);
    CHECK_RES(retCode);

    retCode = V100_Hid_Get_Final_Res(&hDev, 8765, 7654, hf_data);
    CHECK_RES(retCode);

    retCode = V100_Hid_Extract_Template(&hDev, 3210, 2109, BIN_DATA, sizeof(BIN_DATA), 1098, val_int32);
    CHECK_RES(retCode);

    retCode = V100_Hid_Capture_Img(&hDev, 234, 345, 0.33, 0.99, 456, 567, val_int32);
    CHECK_RES(retCode);

    retCode = V100_Hid_Get_Video_Frame(&hDev, 45678, 11223344, val_int64, val_int32, data_bin);
    CHECK_RES(retCode);

    retCode = V100_Hid_Terminate(&hDev);
    CHECK_RES(retCode);
}

int main() {

    _init();
    // _get_cams_list();
    // _get_set_get_parameters();
    // _manage_context();
    // _test_identify_verify_match();
    _test_results();
    _close();

    return 0;
}
