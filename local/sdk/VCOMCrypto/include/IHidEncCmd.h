/***************************************************************************************/
// �Copyright 2020 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.
//
// For a list of applicable patents and patents pending, visit www.hidglobal.com/patents/
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
/***************************************************************************************/

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <stdint.h>

#include "V100_shared_types.h"
#include "IEncCmd.h"
#include "IMemMgr.h"

#include <VcomBaseTypes.h>

#define ENVELOPE_INFO_SIZE            14

typedef unsigned char               uchar;
typedef unsigned int                uint;

enum class hid_types_t {
    hid_type_err        =  0,
    hid_uint64_t        =  1,
    hid_int64_t         =  2,
    hid_uint32_t        =  3,
    hid_int32_t         =  4,
    hid_uint16_t        =  5,
    hid_int16_t         =  6,
    hid_uint8_t         =  7,
    hid_int8_t          =  8,
    hid_double_t        =  9,
    hid_char_ptr_t      = 10,
    hid_byte_ptr_t      = 11,
    hid_matches_ptr_t   = 12,
    hid_array_t         = 13,
};

typedef std::string                 array_str_t;
typedef std::vector<uint8_t>        array_bin_t;

class JagMachRec {
    public:
        array_str_t         recordId;
        array_str_t         galleryID;
        array_str_t         customData;
        double              matchScore;
};

class JagMatches {
    public:
        array_str_t     id;
        double          confidence;
};

typedef std::vector<JagMatches>   array_match_t;
typedef std::vector<JagMachRec>   array_match_rec_t;

enum class hid_store_option_t {
    hid_store_option_len_fixed    = 1,
    hid_store_option_len_variadic = 2,
};

class JagDbRec {
    public:
        array_str_t         recordId;
        array_str_t         customData;
        array_bin_t         templ;
};;

class IHidEncCmd : public IEncCmd {

    public:
        IHidEncCmd() {
            m_data_ptr_         = nullptr;
            m_challenge_len_    = 0;
            m_challenge_offset_ = 0;
        }

        virtual ~IHidEncCmd() {}

    public:
        virtual bool PackChallenge    ( uchar** pPacket, uint& nSize );
        virtual bool UnpackChallenge  ( const uchar* pPacket, uint nSize );
        virtual bool PackResponse     ( uchar** pPacket, uint& nSize );
        virtual bool UnpackResponse   ( const uchar* pPacket, uint nSize );
        virtual void dump(std::stringstream& msg) = 0;
        virtual void Exec() = 0;

    public:
        template <class T>
        static void _logF(const char* const prefix, const T val) {
            std::cout << "    ";
            std::cout << prefix;
            std::cout << ": ";
            std::cout << val;
            std::cout << std::endl;
        }

        static void _logV(const char* const prefix, const array_bin_t& data) {
            std::cout << "    ";
            std::cout << prefix;
            std::cout << ": ";
            for (size_t i = 0; i < data.size(); i++) {
                std::cout << (int)data[i] << " ";
            }
            std::cout << std::endl;
        }

        static void _logV(const char* const prefix, const array_str_t& data) {
            std::cout << "    ";
            std::cout << prefix;
            std::cout << ": ";
            std::cout << data;
            std::cout << std::endl;
        }

    protected:
        template <class T>
        bool StoreFix   ( const bool prev_cmd_success, const T val );
        bool StoreVar   ( const bool prev_cmd_success, const array_str_t&   src );
        bool StoreVar   ( const bool prev_cmd_success, const array_bin_t&   src );
        bool StoreVar   ( const bool prev_cmd_success, const array_match_t& src );
        bool StoreArray ( const bool prev_cmd_success, const uint32_t       val );

        template <class T>
        bool LoadFix    ( const bool prev_cmd_success, T& dst );
        bool LoadVar    ( const bool prev_cmd_success, array_str_t&   dst );
        bool LoadVar    ( const bool prev_cmd_success, array_bin_t&   dst );
        bool LoadVar    ( const bool prev_cmd_success, array_match_t& dst );
        bool LoadArray  ( const bool prev_cmd_success, uint32_t&      val );

    protected:
        virtual void do_cleanup()         = 0;
        virtual bool PackChallengeHid()   = 0;
        virtual bool UnpackChallengeHid() = 0;
        virtual bool PackResponseHid()    = 0;
        virtual bool UnpackResponseHid()  = 0;

    protected:
        bool do_stream_out ( hid_types_t type, const void* const data, size_t len, hid_store_option_t len_option );
        bool do_stream_in  ( const hid_types_t type_req, const size_t dst_len, void* dst );
        std::string to_str ( const array_bin_t& arr);
        std::string to_str ( const array_str_t& arr);
        bool load_hf_res   ();
        bool store_hf_res  ();
        void process_hf_res( const void * const hfRes );

    private:
        template <class T>
        hid_types_t do_get_type_fix(const T val);

        template <class T>
        void do_store_val  ( T val );

        template <class T>
        void do_load_val   ( T& val );

        void do_prep_header();
        void do_fix_header();

    protected:
        std::vector<uint8_t>  m_storage_;
        const uint8_t*        m_data_ptr_;
        size_t                m_challenge_len_;
        size_t                m_challenge_offset_;
        JagDbRec              m_db_rec;
        uint16_t              m_pack_sohv;
        uint32_t              m_pack_cmd;
        uint16_t              m_pack_arg;
        uint32_t              m_pack_len;

    protected:
        v100_hfres_t          m_hf_res;
};
