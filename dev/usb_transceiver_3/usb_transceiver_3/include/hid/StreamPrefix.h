#ifndef __hid_stream_streamprefix_h__
#define __hid_stream_streamprefix_h__

#include <stdint.h>

#include <hid/HidTypes.h>

#include <chrono>
#include <ctime>
#include <cstring>

namespace hid {

namespace stream {

    using time_source_t = std::chrono::steady_clock;
    using time_point_t  = std::chrono::time_point<time_source_t>;
    using duration_ns_t = std::chrono::nanoseconds;
    using duration_us_t = std::chrono::microseconds;
    using duration_ms_t = std::chrono::milliseconds;

    enum class cmd_t : uint32_t {
        STREAM_CMD_NONE             =   0, ///< Placeholder for 0 state. 
        STREAM_CMD_REQUEST          = 100, ///< Request from host to device to perform command. Payload expected.
        STREAM_CMD_RESPONSE         = 101, ///< Response from device to host. Payload expected.
        STREAM_CMD_PING_REQUEST     = 102, ///< Request from host to device to check connection state. Payload not expected.
        STREAM_CMD_PING_RESPONSE    = 103, ///< Response from device to host. 
        STREAM_CMD_ERROR            = 104  ///< Device not able to handle command. Param (see param_t) declares the reason. 
    };

    class params_t {
        public:
            cmd_t       cmd;
            uint32_t    param;
            uint32_t    len;
    };

    class Prefix {

        public:

            static bool SetParams ( const params_t& params, hid::types::storage_t& storage ) {

                bool ret_val = false;

                try {

                    storage.resize ( PrefixSize() );

                    uint32_t* ptr = reinterpret_cast<uint32_t*> (storage.data ());

                    ptr [ OFFSET_MAGIC ] = PREFIX_MAGIC;
                    ptr [ OFFSET_CMD   ] = static_cast<uint32_t> (params.cmd);
                    ptr [ OFFSET_CODE  ] = params.param;
                    ptr [ OFFSET_EXP   ] = 0; // TIMEOUT_MS. Shall be configured separately.
                    ptr [ OFFSET_LEN   ] = params.len;

                    CrcSet( storage );

                    ret_val = true;

                } catch( ... ) {
                    // LOG Exception. No Memory could be thrown.
                }

                return ret_val;
            }

            static bool SetTimeout ( std::chrono::milliseconds timeout_ms, hid::types::storage_t& storage )  {
                bool ret_val = false;
                if ( storage.size () == PrefixSize() ) {
                    TsSet  ( timeout_ms.count(), storage );
                    CrcSet ( storage );
                    ret_val = true;
                }
                return ret_val;
            }

            static bool SetTimeout ( std::chrono::seconds timeout_sec, hid::types::storage_t& storage ) {
                auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeout_sec);
                return SetTimeout ( timeout_ms, storage );
            }

            static bool GetParams ( const hid::types::storage_t& storage, params_t& params )  {

                bool ret_val = false;

                params.cmd   = cmd_t::STREAM_CMD_NONE;
                params.param = 0;
                params.len   = 0;

                if ( Valid ( storage ) ) {
                    const uint32_t* const ptr = reinterpret_cast<const uint32_t*> ( storage.data() );
                    params.cmd    = static_cast<cmd_t> ( ptr[OFFSET_CMD] );
                    params.param  = ptr [ OFFSET_CODE ];
                    params.len    = ptr [ OFFSET_LEN ];
                    ret_val       = true;
                }

                return ret_val;
            }

            static bool ExpirationTimeValid ( const hid::types::storage_t& storage, bool& is_valid )   {

                bool ret_val = false;

                is_valid = false;

                if ( storage.size () == PrefixSize() ) {
                    uint32_t exp_time_ms = TsGet( storage );
                    if ( exp_time_ms != 0 ) {
                        is_valid = true;
                    }
                    ret_val = true;
                }

                return ret_val;
            }

            static bool ExpirationTimeGet ( const hid::types::storage_t& storage, duration_ms_t& expiration_ms )  {

                bool ret_val = false;

                bool expiration_defined = false;
                bool expiration_valid   = false;

                expiration_ms = duration_ms_t(0);

                expiration_valid = ExpirationTimeValid ( storage, expiration_defined );
                if ( expiration_valid && expiration_defined ) {
                    const uint32_t* const ptr = reinterpret_cast<const uint32_t*> (storage.data ());
                    uint32_t duration_ms = ptr [OFFSET_EXP];
                    expiration_ms = duration_ms_t( duration_ms );
                    ret_val = true;
                }

                return ret_val;

            }

            static bool Valid ( const hid::types::storage_t& storage ) {
                
                bool ret_val = false;

                if ( storage.size() == PrefixSize() ) {

                    const uint32_t* const ptr = reinterpret_cast<const uint32_t*> (storage.data ());

                    uint32_t crc_cals  =  CrcCalc ( storage );
                    uint32_t crc_recv  =  ptr [ OFFSET_CRC ];

                    ret_val = (crc_cals == crc_recv);
                }

                return ret_val;                
            }

            static constexpr size_t PrefixSize () {
                // +  0 uint32_t     magic
                // +  4 uint32_t     command
                // +  8 uint32_t     code
                // + 12 uint64_t     timestamp
                // + 16 uint32_t     payload_len
                // + 20 uint32_t     crc
                // + 24
                return 24;
            }

        private:

            static uint32_t TsGet ( const hid::types::storage_t& storage ) {
                const uint32_t* ptr = reinterpret_cast<const uint32_t*> (storage.data ());
                return ptr[ OFFSET_EXP ];
            }

            static void TsSet ( size_t ms_cnt, hid::types::storage_t& storage ) {
                uint32_t* ptr = reinterpret_cast<uint32_t*> (storage.data ());
                ptr [ OFFSET_EXP ] = static_cast<uint32_t> (ms_cnt);
            }

            static void CrcSet ( hid::types::storage_t& storage ) {

                uint32_t* ptr = reinterpret_cast<uint32_t*> (storage.data ());

                uint32_t  crc = 0xFFFFFFFF;

                crc -= ptr [ OFFSET_MAGIC ];
                crc -= ptr [ OFFSET_CMD   ];
                crc -= ptr [ OFFSET_CODE  ];
                crc -= ptr [ OFFSET_EXP   ];
                crc -= ptr [ OFFSET_LEN   ];

                ptr [OFFSET_CRC] = crc;
            }

            static uint32_t CrcCalc ( const hid::types::storage_t& storage ) {

                const uint32_t* const ptr = reinterpret_cast<const uint32_t*> (storage.data ());

                uint32_t crc = 0xFFFFFFFF;

                crc -= ptr [OFFSET_MAGIC];
                crc -= ptr [OFFSET_CMD];
                crc -= ptr [OFFSET_CODE];
                crc -= ptr [OFFSET_EXP];
                crc -= ptr [OFFSET_LEN];

                return crc;
            }

        private:
            static constexpr int OFFSET_MAGIC = 0;
            static constexpr int OFFSET_CMD   = 1;
            static constexpr int OFFSET_CODE  = 2;
            static constexpr int OFFSET_EXP   = 3;
            static constexpr int OFFSET_LEN   = 4;
            static constexpr int OFFSET_CRC   = 5;
            static constexpr int PREFIX_MAGIC = 0x37464564;
    };

}

}

#endif

