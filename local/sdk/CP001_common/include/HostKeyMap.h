#include <map>
#include <string>
#include <iterator>

#include <V100_enc_types.h>

class HostKeyInfo {
    public:
        HostKeyInfo(_V100_ENC_KEY_TYPE key_type) : type(key_type), version(), mode(), size(), kcv() {
        }

    public:
        const _V100_ENC_KEY_TYPE type;
        u16   version;
        u16   mode;
        u16   size;
        u32   kcv;
};

class HostKeySlot {

    public:
        HostKeySlot (const HostKeySlot& rhs) : info(rhs.info.type) {

            p_key = new u2048();

            memset(p_key, 0, sizeof(u2048));

            if (rhs.info.size < sizeof(u2048)) {
                memcpy(p_key, rhs.p_key, rhs.info.size);
            } else {
                memcpy(p_key, rhs.p_key, sizeof(u2048));
            }
        }

        HostKeySlot(_V100_ENC_KEY_TYPE type) : info(type) {
            p_key = new u2048();
            memset(p_key, 0, sizeof(u2048));
        }

        ~HostKeySlot() {
            delete[] p_key;
        }

        HostKeyInfo        info;
        u8*                p_key;
};

typedef std::map<int, HostKeySlot*>     KeyMap_storage;
typedef KeyMap_storage::iterator        KeyMap_itr;

class HostKeyManager;

class HostKeyMap {

    friend class DeviceKeyManager;

    public:
        HostKeyMap() {
            m_KeyMap[KT_EXTKEY_CTK] = new HostKeySlot(KT_EXTKEY_CTK);
        }

        ~HostKeyMap() {
            KeyMap_itr it;
            for (it = m_KeyMap.begin(); it != m_KeyMap.end(); it++) {
                delete it->second;
            }
        }

        static const HostKeySlot* GetKey(_V100_ENC_KEY_TYPE keyType) {
            return m_KeyMap[keyType];
        }

        static void SetKey(HostKeySlot& key) {
            memcpy(&m_KeyMap[key.info.type]->info, &key.info, sizeof(HostKeyInfo));
            memcpy(m_KeyMap[key.info.type]->p_key, key.p_key, sizeof(u2048));
        }

    private:
        static KeyMap_storage m_KeyMap;
};
