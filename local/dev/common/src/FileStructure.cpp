#include "FileStructure.h"
#include "ICypher.h"

// From Stackoverflow: https://stackoverflow.com/questions/53365538/how-to-determine-whether-to-use-filesystem-or-experimental-filesystem
#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

// Check for feature test macro for <filesystem>
#   if defined(__cpp_lib_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0

// Check for feature test macro for <experimental/filesystem>
#   elif defined(__cpp_lib_experimental_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// We can't check if headers exist...
// Let's assume experimental to be safe
#   elif !defined(__has_include)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// Check if the header "<filesystem>" exists
#   elif __has_include(<filesystem>)

// If we're compiling on Visual Studio and are not compiling with C++17, we need to use experimental
#       ifdef _MSC_VER

// Check and include header that defines "_HAS_CXX17"
#           if __has_include(<yvals_core.h>)
#               include <yvals_core.h>

// Check for enabled C++17 support
#               if defined(_HAS_CXX17) && _HAS_CXX17
// We're using C++17, so let's use the normal version
#                   define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#               endif
#           endif

// If the marco isn't defined yet, that means any of the other VS specific checks failed, so we need to use experimental
#           ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#               define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#           endif

// Not on Visual Studio. Let's use the normal version
#       else // #ifdef _MSC_VER
#           define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#       endif

// Check if the header "<filesystem>" exists
#   elif __has_include(<experimental/filesystem>)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// Fail if neither header is available with a nice error message
#   else
#       error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#   endif

// We priously determined that we need the exprimental version
#   if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
// Include it
#       include <experimental/filesystem>

// We need the alias from std::experimental::filesystem to std::filesystem
namespace std {
    namespace filesystem = experimental::filesystem;
}

// We have a decent compiler and can use the normal version
#   else
// Include it
#       include <filesystem>
#   endif

#endif // #ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

#ifdef __linux__
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#else
#endif

void PrintOffsetAndData(const int64_t& offset, const int64_t& size, const std::string& desc)
{
    fprintf(stdout, "offset = %ld, size = %ld, %s\n", offset, size, desc.c_str());
}

const std::vector<unsigned char> FileSegment::Serialize() {
    if (FileBuffer.size() <= 0)
    {
        throw "invalid file size!";
    }
    FixedMembers.HeaderSize = sizeof(FixedLengthMemberStruct);
    FixedMembers.FileSize = FileBuffer.size();
    FixedMembers.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    std::vector<unsigned char> buffer(
        FixedMembers.HeaderSize + sizeof(uint64_t) + FileNameLength() + FixedMembers.FileSize);
    unsigned int offset = 0;
    auto file_name_len = file_name_.length();

    // Section: [Header Size] [Timestamp] [FileSize] [FileOperation]
    PrintOffsetAndData(offset, sizeof(FixedLengthMemberStruct), "header " + file_name_);
    memcpy(buffer.data() + offset, &FixedMembers,
        sizeof(FixedLengthMemberStruct));
    offset += sizeof(FixedLengthMemberStruct);
    // Section: [FileName Size]
    PrintOffsetAndData(offset, sizeof(uint64_t), "uint64_t file_name_len" );
    memcpy(buffer.data() + offset, &file_name_len, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    // Section: [FileName]
    PrintOffsetAndData(offset, FileNameLength(), "file_name");
    memcpy(buffer.data() + offset, FileName(), FileNameLength());
    offset += FileNameLength();
    // Section: [File Blob]
    PrintOffsetAndData(offset, FileBuffer.size(), "blob");
    memcpy(buffer.data() + offset, FileBuffer.data(), FileBuffer.size());
    offset += FileBuffer.size();
    PrintOffsetAndData(offset, 0, "blob_finish");
    // Clear FileBuffer
    //FileBuffer.clear();
    return buffer;
}

bool FileSegment::Deserialize(const unsigned char* buffer, uint64_t& offset)
{
    uint64_t filename_len = 0;
    // Section: [Header Size]
    uint32_t header_size = 0;
    memcpy(&header_size, buffer+offset, sizeof(uint32_t));
    if (header_size == 0)
    {
        return false;
    }

    // Section: [Header Size] [Timestamp] [FileSize] [FileOperation]
    PrintOffsetAndData(offset, header_size, "header " + file_name_);
    memcpy(&FixedMembers, buffer + offset, header_size);
    offset += header_size;
    // Section: [FileName Size]
    memcpy(&filename_len, buffer + offset, sizeof(uint64_t));
    PrintOffsetAndData(offset, sizeof(uint64_t), "uint64_t file_name_len");
    offset += sizeof(uint64_t);
    // Section: [FileName]
    PrintOffsetAndData(offset, filename_len, "file_name");
    file_name_ = std::string((const char*)buffer + offset, filename_len);
    offset += filename_len;
    // Section: [FileBlob]
    PrintOffsetAndData(offset, FixedMembers.FileSize, "blob");
    FileBuffer.resize(FixedMembers.FileSize);
    memcpy(FileBuffer.data(), buffer + offset, FixedMembers.FileSize);
    offset += FixedMembers.FileSize;
    PrintOffsetAndData(offset, 0, "blob_finish");
    return true;
}

const std::vector<unsigned char> EncFileSegment::Serialize() {
    if (FileBuffer.size() <= 0)
    {
        throw "invalid file size!";
    }
    FixedMembers.HeaderSize = sizeof(FixedLengthMemberStruct);
    FixedMembers.FileSize = FileBuffer.size();
    FixedMembers.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    std::vector<unsigned char> buffer(
        FixedMembers.HeaderSize + sizeof(uint64_t) + FileNameLength() + FixedMembers.FileSize);
    unsigned int offset = 0;
    auto file_name_len = file_name_.length();

    // Section: [Header Size] [Timestamp] [FileSize] [FileOperation]
    PrintOffsetAndData(offset, sizeof(FixedLengthMemberStruct), "header " + file_name_);
    memcpy(buffer.data() + offset, &FixedMembers,
        sizeof(FixedLengthMemberStruct));
    offset += sizeof(FixedLengthMemberStruct);
    // Section: [FileName Size]
    PrintOffsetAndData(offset, sizeof(uint64_t), "uint64_t file_name_len" );
    memcpy(buffer.data() + offset, &file_name_len, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    // Section: [FileName]
    PrintOffsetAndData(offset, FileNameLength(), "file_name");
    memcpy(buffer.data() + offset, FileName(), FileNameLength());
    offset += FileNameLength();
    // Section: [File Blob]
    PrintOffsetAndData(offset, FileBuffer.size(), "blob");
    memcpy(buffer.data() + offset, FileBuffer.data(), FileBuffer.size());
    offset += FileBuffer.size();
    PrintOffsetAndData(offset, 0, "blob_finish");
    // Clear FileBuffer
    //FileBuffer.clear();
    return buffer;
}

bool EncFileSegment::Deserialize(const unsigned char* buffer, uint64_t& offset)
{
    uint64_t filename_len = 0;
    // Section: [Header Size]
    uint32_t header_size = 0;
    // Skip first 64 bytes (timestamp)
    memcpy(&header_size, buffer+offset + sizeof(uint64_t), sizeof(uint32_t));
    if (header_size == 0)
    {
        return false;
    }

    // Section: [Header Size] [Timestamp] [FileSize] [FileOperation]
    PrintOffsetAndData(offset, header_size, "header " + file_name_);
    memcpy(&FixedMembers, buffer + offset, header_size);
    offset += header_size;
    // Section: [FileName Size]
    memcpy(&filename_len, buffer + offset, sizeof(uint64_t));
    PrintOffsetAndData(offset, sizeof(uint64_t), "uint64_t file_name_len");
    offset += sizeof(uint64_t);
    // Section: [FileName]
    PrintOffsetAndData(offset, filename_len, "file_name");
    file_name_ = std::string((const char*)buffer + offset, filename_len);
    offset += filename_len;
    // Section: [FileBlob]
    PrintOffsetAndData(offset, FixedMembers.FileSize, "blob");
    FileBuffer.resize(FixedMembers.FileSize);
    memcpy(FileBuffer.data(), buffer + offset, FixedMembers.FileSize);
    offset += FixedMembers.FileSize;
    PrintOffsetAndData(offset, 0, "blob_finish");
    return true;
}

bool UpdatePackage::PackUpdatePayload()
{
    FixedPackageHeaders.HeaderSize = sizeof(FixedLengthHeader);
    // Reserve some space at the beginning for header
    UpdatePackageBuffer.resize(sizeof(FixedLengthHeader));

    for (auto it : FileSegments)
    {
        auto serialized_item = it.Serialize();
        UpdatePackageBuffer.insert(UpdatePackageBuffer.end(), std::begin(serialized_item), std::end(serialized_item));
    }
    // Run Encrypt and get signature on UpdatePackageBuffer
    // EncryptionMethod.EncryptAndGetSignature(UpdatePackageBuffer, Signature)
    //FileSegments.clear();
    FixedPackageHeaders.CryptogramSize = UpdatePackageBuffer.size() - sizeof(FixedLengthHeader);
    // Fill the Encrypted buffer reserved space.
    memcpy(UpdatePackageBuffer.data(), &FixedPackageHeaders, sizeof(FixedLengthHeader));

    Signature.insert(Signature.end(), { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0xff, 0xaa, 0x10, 0x20, 0xab });
    // Insert the Signature to package end
    UpdatePackageBuffer.insert(UpdatePackageBuffer.end(), Signature.begin(), Signature.end());
    return true;
}

bool UpdatePackage::UnpackUpdatePayload()
{
    memcpy(&FixedPackageHeaders, UpdatePackageBuffer.data(), sizeof(uint32_t));
    auto header_size = FixedPackageHeaders.HeaderSize;
    if (header_size == 0)
    {
        return false;
    }

    // Fill Fixed Header
    memcpy(&FixedPackageHeaders, UpdatePackageBuffer.data(), header_size);

    uint64_t processed_size = header_size;
    // Run Decrypt on UpdatePackageBuffer
    // EncryptionMethod.DecryptAndGetSignature(UpdatePackageBuffer,Signature)

    // Now UpdatePackageBuffer should be filled with unencrypted data
    while (processed_size < FixedPackageHeaders.CryptogramSize + header_size)
    {
        FileSegment file_seg;
        auto parse_success = file_seg.Deserialize(UpdatePackageBuffer.data(), processed_size);
        if (!parse_success)
        {
            return false;
        }
        FileSegments.push_back(std::move(file_seg));
    }
    // Now the processed_size is at the index of Signature section, and also the size of processed data.
    uint64_t SignatureSize = UpdatePackageBuffer.size() - processed_size;
    Signature.resize((uint64_t)SignatureSize);
    memcpy(Signature.data(), UpdatePackageBuffer.data() + processed_size, SignatureSize);

    // Calculate the signature based on unencrypted data.
    // calc_sig = CalcSig(UpdatePackageBuffer.data(), sizeof(FixedLengthHeader), processed_size);

    std::vector<unsigned char> calc_sig{ 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0xff, 0xaa, 0x10, 0x20, 0xab }; // For now this is a dummy!

    if (memcmp(calc_sig.data(), Signature.data(), calc_sig.size()) != 0)
    {
        IsSignatureCorrect = false;
    }
    else {
        IsSignatureCorrect = true;
    }

    UpdatePackageBuffer.clear();
    UpdatePackageBuffer.shrink_to_fit();

    return true;
}


bool EncUpdatePackage::PackUpdatePayload()
{
    for(auto it:FileSegments)
    {
        auto serialized_item = it.Serialize();
        UpdatePackageBuffer.insert(UpdatePackageBuffer.end(), std::begin(serialized_item), std::end(serialized_item));
    }

    auto clear_data_size = UpdatePackageBuffer.size();
    auto enc_data_size = clear_data_size;
    uint8_t *enc_data = nullptr;
    const int32_t block_size = 128/8; // For AES: block size is 128-bit, 16 bytes.
    if (clear_data_size % block_size != 0)
    {
        enc_data_size = clear_data_size + (block_size - (enc_data_size % block_size));
    }
    enc_data = new uint8_t[enc_data_size] {};
    uint8_t fw_aes_iv[16] {};
    uint8_t fw_key[32] {};
    memcpy(fw_aes_iv, kFwKeyIv, 16); // IV only use first 16 bytes.
    memcpy(fw_key, kFwKey, 32);
    uint8_t *hmac_data = new uint8_t[64] {}; // HMAC for SHA512 is 64-bytes, 512-bits.
    int32_t hmac_size = 64;
    UpdatePackageBuffer.resize(enc_data_size);
    uint8_t* padded_buffer_data = new uint8_t[enc_data_size]{};
    memcpy(padded_buffer_data, UpdatePackageBuffer.data(), UpdatePackageBuffer.size());

    // Packger side don't have HBSEClient.
    auto success = ICypher::GetInstance()->GetAES()->EncryptData(padded_buffer_data, enc_data, enc_data_size, enc_data_size, kFwKey, kFwKeyLen, fw_aes_iv, AES_CBC);
    if (!success)
    {
        delete[] enc_data;
        delete[] hmac_data;
        enc_data = nullptr;
        hmac_data = nullptr;
        return false;
    }
    success = ICypher::GetInstance()->GetHMAC()->HMAC(enc_data, enc_data_size, fw_key, 32, hmac_data, &hmac_size, SHA512_MODE);
    if (!success)
    {
        delete[] enc_data;
        delete[] hmac_data;
        enc_data = nullptr;
        hmac_data = nullptr;
        return false;
    }
    Signature.resize(hmac_size);

    //memset(padded_buffer_data, 0, enc_data_size);
    // Validate
    uint8_t* new_dec_data = new uint8_t[enc_data_size]{};
    memcpy(fw_aes_iv, kFwKeyIv, 16); // IV only use first 16 bytes.

    ICypher::GetInstance()->GetAES()->DecryptData(enc_data, new_dec_data, enc_data_size, enc_data_size, kFwKey, kFwKeyLen, fw_aes_iv, AES_CBC);

    int cmp_result = memcmp(new_dec_data, padded_buffer_data, enc_data_size);
    fprintf(stdout, "memcmp result = %d", cmp_result);

    std::vector<unsigned char> enc_vect;
    enc_vect.resize(enc_data_size);
    memcpy(enc_vect.data(), enc_data, enc_data_size);
    UpdatePackageBuffer.swap(enc_vect);
    memcpy(Signature.data(), hmac_data, hmac_size);
    UpdatePackageBuffer.insert(UpdatePackageBuffer.end(), Signature.begin(), Signature.end());
    return true;
}

bool EncUpdatePackage::UnpackUpdatePayload()
{
#ifdef __linux__
    int64_t enc_buffer_len = UpdatePackageBuffer.size() - 64;
    int64_t full_buffer_len = UpdatePackageBuffer.size();
    if (enc_buffer_len <= 0)
    {
        fprintf(stdout, "Error: UnpackUpdatePayload failed, update package size incorrect!");
        return false;
    }
    if (client_instance_ == nullptr)
    {
        client_instance_ = ISecureElement::GetInstance();
    }

    uint8_t fw_aes_iv[16] {};
    memcpy(fw_aes_iv, kFwKeyIv, 16); // IV only use first 16 bytes.
    AsyncExecResult async_exec_result { AsyncExecStates::In_Progress, CryptExecStatus::In_Progress };

    int fd = open(kUpdateShmFileName, O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        fprintf(stdout, "\nError opening/creating mmaped file!\n");
        client_instance_->Disconnect();
        client_instance_ = nullptr;
        return false;
    }
    if (ftruncate(fd, full_buffer_len) == -1 )
    {
        fprintf(stdout, "\nError setting file length!\n");
        client_instance_->Disconnect();
        client_instance_ = nullptr;
        return false;
    }
    char *mapped = (char*)mmap(nullptr, full_buffer_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == (void*)-1)
    {
        fprintf(stdout, "\nError: mmap failed!");
        client_instance_->Disconnect();
        client_instance_ = nullptr;
        return false;
    }
    memcpy(mapped, UpdatePackageBuffer.data(), full_buffer_len);
    UpdatePackageBuffer.swap(UpdatePackageBuffer); // Clear buffer!
    UpdatePackageBuffer.shrink_to_fit();

    close(fd);

    fd = shm_open(kUpdateStateShmName, O_CREAT | O_RDWR, 0666 );
    ftruncate(fd, sizeof(AsyncExecResult) );
    char* state_map  = (char*) mmap(nullptr, sizeof(AsyncExecResult), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    memset(state_map, 0, sizeof(AsyncExecResult));

    auto exec_success = client_instance_->Execute_Verify_And_Decrypt(reinterpret_cast<const uint8_t*>(kUpdateShmFileName), sizeof(kUpdateShmFileName), fw_aes_iv , 16);

    if (exec_success != CryptExecStatus::Successful)
    {
        fprintf(stdout, "Error: Execute_Verify_And_Decrypt failed, data is not valid!");
        munmap(state_map, sizeof(AsyncExecResult));
        munmap(mapped, full_buffer_len);
        unlink(kUpdateShmFileName);
        shm_unlink(kUpdateStateShmName);
        client_instance_->Disconnect();
        client_instance_ = nullptr;
        return false;
    }
    // Detect if decrypt is finished. This may take long time so check every 0.2 seconds.
    while(true)
    {
        memcpy((char*)&async_exec_result, state_map, sizeof(AsyncExecResult));
        if (async_exec_result.async_result != AsyncExecStates::In_Progress)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    if (async_exec_result.async_result != AsyncExecStates::Exec_OK || async_exec_result.exec_result != CryptExecStatus::Successful)
    {
        if (async_exec_result.exec_result == CryptExecStatus::Validation_Error)
        {
            fprintf(stdout, "\nError: Decrypt/Verify failed, data is not valid!");
            IsSignatureCorrect = false;
        }
        if (async_exec_result.exec_result == CryptExecStatus::Hash_Error)
        {
            fprintf(stdout, "\nHMAC calculation failed to execute.");
        }
        if (async_exec_result.exec_result == CryptExecStatus::Decrypt_Error)
        {
            fprintf(stdout, "\nData decryption failed.");
        }

        munmap(state_map, sizeof(AsyncExecResult));
        munmap(mapped, full_buffer_len);
        unlink(kUpdateShmFileName);
        shm_unlink(kUpdateStateShmName);
        client_instance_->Disconnect();
        client_instance_ = nullptr;
        return false;
    }
    IsSignatureCorrect = true;
    // AES decrypt doesn't change the size. Discard last 64 bytes (HMAC 512)
    UpdatePackageBuffer.resize(enc_buffer_len);
    memcpy(UpdatePackageBuffer.data(), mapped, enc_buffer_len);
    munmap(mapped, full_buffer_len);
    munmap(state_map, sizeof(AsyncExecResult));

    uint64_t processed_size = 0;

    // Now UpdatePackageBuffer should be filled with unencrypted data
    while (processed_size < UpdatePackageBuffer.size())
    {
        EncFileSegment file_seg;
        auto parse_success = file_seg.Deserialize(UpdatePackageBuffer.data(), processed_size);
        if (!parse_success)
        {
            UpdatePackageBuffer.clear();
            UpdatePackageBuffer.shrink_to_fit();
            unlink(kUpdateShmFileName);
            shm_unlink(kUpdateStateShmName);
            client_instance_->Disconnect();
            client_instance_ = nullptr;
            return false;
        }
        FileSegments.push_back(std::move(file_seg));
        if ((UpdatePackageBuffer.size() - processed_size) < 16)
        {
            processed_size += (UpdatePackageBuffer.size()-processed_size); // Handling small file padding!
        }
    }

    UpdatePackageBuffer.clear();
    UpdatePackageBuffer.shrink_to_fit();
    unlink(kUpdateShmFileName);
    shm_unlink(kUpdateStateShmName);
    client_instance_->Disconnect();
    client_instance_ = nullptr;
    return true;

#else
    uint32_t bufsize = UpdatePackageBuffer.size() - 64;
    uint8_t* cgbuffer = new uint8_t[bufsize]{};
    memcpy(cgbuffer, UpdatePackageBuffer.data(), UpdatePackageBuffer.size() - 64);
    uint8_t* clrbuffer = new uint8_t[bufsize]{};
    uint8_t fw_aes_iv[16]{};
    uint8_t fw_key[32]{};
    memcpy(fw_aes_iv, kFwKeyIv, 16); // IV only use first 16 bytes.
    memcpy(fw_key, kFwKey, 32);
    auto success = ICypher::GetInstance()->GetAES()->DecryptData(cgbuffer, clrbuffer, bufsize, bufsize, kFwKey, kFwKeyLen, fw_aes_iv, AES_CBC);
    EncFileSegment seg;
    uint64_t offset = 0;
    seg.Deserialize(clrbuffer, offset);


#endif
    return true;
}