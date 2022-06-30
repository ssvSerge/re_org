#ifndef __HFTEMPLATE_PRIVATE_H__
#define __HFTEMPLATE_PRIVATE_H__

#include <jaguar/HFApi.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Codes of different algorithms
     */
    typedef enum _TAG_HFAlgorithm
    {
        HF_ALGORITHM_PARAVISION = 0,
        HF_ALGORITHM_PARAVISION_TARKA = 0x00000000,
        HF_ALGORITHM_PARAVISION_KORA = 0x00010000,
        HF_ALGORITHM_PARAVISION_ZILL = 0x00020000,
        HF_ALGORITHM_PARAVISION_GEN5_FAST = 0x00030000,

        HF_ALGORITHM_HID = 0x01000000,
        HF_ALGORITHM_HID_PAD = 0x01000000,
        HF_ALGORITHM_EMPTY = 0xFFFFFFFF
    } HFAlgorithm;

    /**
     * @brief Template struct containing both metadata and the algorithm-dependent data.
     */
    typedef struct _TAG_HFTemplate
    {
        /**
         * @brief 'HFTp'
         */
        uint32_t magic;
        /**
         * @brief Structure version.
         */
        uint32_t formatVersion;
        /**
         * @brief Algorithm identifier.
         *
         * Byte 3 (MSB): Algorithm vendor code. Paravision = 0
         * Byte 2: Algorithm variant. Paravision Tarka = 0, Paravision Kora = 1, Paravision Zill = 2
         * Byte 0-1: Algorithm version. Exact meaning is algo dependent – we do not want to keep the exact algorithm version, but we want to distinguish the mutually incompatible versions or version clusters. For Paravision we start with 0.
         * Example: Paravision Tarka = 0x00000000, Paravison Kora = 0x00010000, Paravison Zill = 0x00020000.
         */
        uint32_t templateDataType;
        /**
         * @brief Template data length (in bytes).
         */
        uint32_t templateDataLen;
        /**
         * @brief Template quality [0,1].
         *
         */
        float quality;
        /**
         * @brief Reserved.
         */
        uint32_t reserved[5];
        /**
         * @brief Variable-length byte array.
         *
         *  In the case of Paravision algo the real template data is "float embeddings[]" casted to byte array.
         */
        uint8_t templateData[1];
    } HFTemplate;


    /**
     * @brief Calculate the size of the entire template structure including the templateData.
     *
     * @param templ [in] target template
     * @param size [out] size of the entire template
     * @return int32_t Hid Face error code (see ...)
     */
    int32_t HFTemplateSize(const HFTemplate *templ, uint32_t *size);

    /**
     * @brief Calculate the future size of the entire template structure including the templateData for allocation.
     *
     * @param dataLen [in] data size of the new template
     * @param size [out] size of the entire template
     * @return int32_t Hid Face error code (see ...)
     */
    int32_t HFNewTemplateSize(uint32_t dataLen, uint32_t *size);

    /**
     * @brief Verify that the template is valid.
     * 
     * Returns \c HFERROR_GENERAL for invalid template, \c HFERROR_OK for valid one.
     *
     * @param templ [in] verified template
     * @return int32_t Hid Face error code (see ...)
     */
    int32_t HFTemplateVerify(const HFTemplate *templ);

#ifdef __cplusplus
}
#endif

#endif // __HFTEMPLATE_PRIVATE_H__