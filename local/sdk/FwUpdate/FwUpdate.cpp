#include <iostream>
#include <VCOMBase.h>
#include <HostKeyManager.h>
#include <string>

#define CHECK_RES(x)

V100_DEVICE_TRANSPORT_INFO  hDev = { 0 };

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

static void _fw_update( const char* const name ) {

    V100_ERROR_CODE retCode;
    retCode = V100_Hid_FwUpdate(&hDev, name);
    CHECK_RES(retCode);

}

int main ( int argc, char* argv[] ) {

    if (argc != 2) {
        std::cout << "Usage: FwUpdate.exe <fw_file_name>";
    } else {
        _init();
        _fw_update( argv[1] );
    }


    return 0;
}
