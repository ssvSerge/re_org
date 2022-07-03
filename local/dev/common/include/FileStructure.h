#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <array>
#include <cstdlib>
#include <cstring>
#include <chrono>
#ifdef __linux__
#include "ISecureElement.h"
#endif
#include "json.hpp"
#include "CommonCypherTypes.h"

enum class FileOperationTypes
{
    TEMP,
    SYSTEM_IMAGE,
    CONFIGURATION,
    SPOOF_MODEL,
    MANIFEST,
    RESTORE_SECURITY,
    DEBUG_SINGLE_FILE,
};

// Warning: File align is 2 bytes!
class FileSegment {
public:
    struct FixedLengthMemberStruct {
        uint32_t HeaderSize = 0;
        uint64_t timestamp = 0;
        uint64_t FileSize = 0;
        FileOperationTypes FileOperation = FileOperationTypes::TEMP;
    } FixedMembers;
    uint64_t FileNameLength() const { return file_name_.length(); }
    const char* FileName() const { return file_name_.c_str(); }
    const std::string FileNameStr() const { return file_name_; }
    std::vector<unsigned char> FileBuffer{};        // Cleared with Serialize(), Filled with Deserialize()

    const std::vector<unsigned char> Serialize();
    bool Deserialize(const unsigned char* buffer, uint64_t& offset);

    void SetFileName(std::string file_name) { file_name_ = file_name; }
    void SetFileOperation(FileOperationTypes op_type) { FixedMembers.FileOperation = op_type; }

private:
    std::string file_name_;
};

class EncFileSegment
{
    public:
    #pragma pack(push)
    #pragma pack(1)
    struct FixedLengthMemberStruct {
        uint64_t timestamp = 0;
        uint32_t HeaderSize = 0;
        uint64_t FileSize = 0;
        FileOperationTypes FileOperation = FileOperationTypes::TEMP;
    } FixedMembers;
    #pragma pack(pop)
    uint64_t FileNameLength() const { return file_name_.length(); }
    const char* FileName() const { return file_name_.c_str(); }
    const std::string FileNameStr() const { return file_name_; }
    std::vector<unsigned char> FileBuffer{};        // Cleared with Serialize(), Filled with Deserialize()

    const std::vector<unsigned char> Serialize();
    bool Deserialize(const unsigned char* buffer, uint64_t& offset);

    FileOperationTypes GetFileOperation() { return FixedMembers.FileOperation;}
    void SetFileName(std::string file_name) { file_name_ = file_name; }
    void SetFileOperation(FileOperationTypes op_type) { FixedMembers.FileOperation = op_type; }

private:
    std::string file_name_;
};

// Warning: File align is 2 bytes!
class UpdatePackage
{
public:
    struct FixedLengthHeader {
        uint32_t HeaderSize = 0;
        uint64_t CryptogramSize = 0;
        uint16_t DevFlag = 0;
        uint16_t HeaderVer = 0;
        uint32_t PackagerVer = 0;
        //uint64_t ReservedField = 0;
    }FixedPackageHeaders;
    std::vector<unsigned char> UpdatePackageBuffer; // Filled with RunEncryption(), cleared with RunDecrypt()
    std::vector<FileSegment> FileSegments; // Filled with RunDecrypt(), cleared with RunEncryption()
    std::vector<unsigned char> Signature;       // Filled with RunEncryption(), validated with RunDecryption()
    bool IsSignatureCorrect = false;
    void SetEncryptionMethod() {};
    //CryptoServiceProvider EncryptionMethod;
    bool PackUpdatePayload();
    // No-copy (hopefully) during decryption.
    bool UnpackUpdatePayload();
private:
};

class EncUpdatePackage
{
    public:
    std::vector<unsigned char> UpdatePackageBuffer; // Filled with RunEncryption(), cleared with RunDecrypt()
    std::vector<EncFileSegment> FileSegments; // Filled with RunDecrypt(), cleared with RunEncryption()
    std::vector<unsigned char> Signature;       // Filled with RunEncryption(), validated with RunDecryption()
    bool IsSignatureCorrect = false;
    bool PackUpdatePayload();
    bool UnpackUpdatePayload();
    void FreeEncryptedFileBuffer()
    {
        UpdatePackageBuffer.swap(UpdatePackageBuffer);
    }
    EncUpdatePackage() = default;
    EncUpdatePackage(const EncUpdatePackage& old_instance) = default;
    private:
    #ifdef __linux__
    ISecureElement *client_instance_;
    #endif
};

class ManifestInfo
{
    public:
        void Serialize(nlohmann::json &json_object)
        {
            json_object = existance_map_;
        }
        void Deserialize(const nlohmann::json &json_object)
        {
            existance_map_ = json_object.get<std::unordered_map<std::string, int>>();
        }
        void AddFile(const std::string &filename)
        {
            existance_map_.insert(std::pair<std::string, int>(filename, 0b01));
        }
        void SetFileExistance(const std::string& filename)
        {
            existance_map_[filename] |= 0b10;
        }
        bool GetFileExistance(const std::string& filename)
        {
            return existance_map_[filename] & 0b10;
        }
        bool IsEmptyManifest()
        {
            return existance_map_.size() == 0;
        }
        size_t GetManifestItemCount()
        {
            return existance_map_.size();
        }
        std::unordered_map<std::string, int> &GetExistanceMap()
        {
            return existance_map_;
        }
        void ClearManifest()
        {
            existance_map_.swap(existance_map_);
        }
    private:
        std::unordered_map<std::string, int> existance_map_;

};

#if 0
void demo()
{
    // Psuedo code for packaging.
    UpdatePackage package;
    package.EncryptionMethod = ...
        foreach(file in filelist)
    {
        FileSegment seg;
        call seg.SetFileName(whatever filename you want to call it);
        call seg.SetFileOperation(firmware ? key ? etc...);
        set seg.FileSize to file.size();
        Read all content to seg.FileBuffer;
        package.FileSegments.push_back(seg);
    }
    package.PackUpdatePayload();

    // Psuedo code for unpacking
    std::vector<unsigned char> buffer;->from TransactionBroker interface, blob.
        UpdatePackage package;
    package.UpdatePackageBuffer = buffer;
    package.UnpackUpdatePayload();
    foreach(file in packge.FileSegments)
    {
        write file to filesystem with file.FileName
    }
}
#endif
