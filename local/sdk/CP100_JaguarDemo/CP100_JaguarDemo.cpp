#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#else // !_WIN32
#include <unistd.h>
#endif  // !_WIN32
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <queue>
#include <set>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <HFApi.h>
#include <HFTemplatePrivate.h>

#ifdef _WIN32
#pragma warning(disable : 4996)
#endif  // _WIN32

#define VERIFY(cond) \
    if (!(cond))     \
        throw HFERROR_GENERAL;

#define VERIFY_OK(code)     \
    if (code != HFERROR_OK) \
        throw code;

char GetChar()
{
    char x;
    std::cin >> x;
    return x;
}

void StoreImage(HFImage image, const std::string& fileName)
{
    uint8_t* data;
    uint64_t size;
    VERIFY_OK(HFExportImage(image, HFIMAGE_ENCODING_PNG, &data, &size))
        std::ofstream out(fileName.c_str(), std::ios::out | std::ios::binary);
    out.write(reinterpret_cast<const char*>(data), size);
    HFFree(data);
}

struct Settings
{
    bool _help;
    bool _verbose;
    bool _guiEnabled;
    bool _replayVideo;
    bool _recordVideo;
    bool _dumpVideo;
    bool _plainStream;
    bool _matchColoredBox;
    bool _liveness;
    bool _saveimgs;
    uint32_t _livenessAlgo;
    std::string _recordingName;
    std::string _dumpName;
    std::string _replayName;
    std::string _cameraSettings;
    std::string _rscameraSettings;
    std::string _ambacameraSettings;
    static const std::string DefaultRecFileName;
    static const std::string DefaultDmpDirName;

    Settings(int argc, char** argv)
        : _help(false), _verbose(false), _guiEnabled(false), _replayVideo(false), _recordVideo(false), _dumpVideo(false),
        _plainStream(false), _matchColoredBox(false), _liveness(false), _saveimgs(false), _livenessAlgo(0), _recordingName(DefaultRecFileName),
        _dumpName(DefaultDmpDirName), _replayName(DefaultRecFileName), _cameraSettings("0:800:600"),
        _rscameraSettings(), _ambacameraSettings()
    {
        for (int i = 1; i < argc; i++)
        {
            std::string param(argv[i]);
            if ("-h" == param)
            {
                _help = true;
            }
            else if ("-v" == param)
            {
                _verbose = true;
            }
            else if ("-gui" == param)
            {
                _guiEnabled = true;
            }
            else if ("-liveness" == param)
            {
                _livenessAlgo = HF_ALGORITHM_PARAVISION;
                _liveness = true;
            }
            else if ("-liveness=" == param.substr(0, 10))
            {
                _liveness = true;
                if (param.substr(10) == "paravision") {
                    _livenessAlgo = HF_ALGORITHM_PARAVISION;
                }
                else if (param.substr(10) == "pad") {
                    _livenessAlgo = HF_ALGORITHM_HID_PAD;
                }
                else {
                    Print("Unknown liveness algo: " + param.substr(10), true);
                    throw std::exception();
                }
            }
            else if ("-mode=" == param.substr(0, 6))
            {
                _plainStream = param.substr(6) == "plain";
                _matchColoredBox = param.substr(6) == "match";
            }
            else if ("-cam=" == param.substr(0, 5))
            {
                _cameraSettings = param.substr(5);
            }
            else if ("-rscam=" == param.substr(0, 7))
            {
                _rscameraSettings = param.substr(7);
            }
            else if ("-ambacam=" == param.substr(0, 9))
            {
                _ambacameraSettings = param.substr(9);
            }
            else if ("-rec" == param.substr(0, 4))
            {
                if (param.length() > 4 && param[4] != '=')
                    throw std::exception();
                _recordVideo = true;
                _recordingName = param.length() > 4 ? param.substr(5) : DefaultRecFileName;
            }
            else if ("-dmp" == param.substr(0, 4))
            {
                if (param.length() > 4 && param[4] != '=')
                    throw std::exception();
                _dumpVideo = true;
                _dumpName = param.length() > 4 ? param.substr(5) : DefaultDmpDirName;
            }
            else if ("-play" == param.substr(0, 5))
            {
                if (param.length() > 5 && param[5] != '=')
                    throw std::exception();
                _replayVideo = true;
                _replayName = param.length() > 5 ? param.substr(6) : DefaultRecFileName;
            }
            else if ("-saveimgs" == param)
            {
                _saveimgs = true;
            }
            else
            {
                Print("Unknown option: " + param, true);
                throw HFERROR_GENERAL;
            }
        }
        Print("Settings:");
        Print(_verbose ? "- verbose: enabled" : "- verbose: disabled");
        Print(_liveness ? "- liveness detection: enabled" : "- liveness detection: disabled");
        Print(_guiEnabled ? "- gui output: enabled" : "- gui output: disabled");
        Print(_plainStream ? "- additional stream information: none" : "- additional stream information: full");
        Print(_replayVideo ? "- replay from: " + _recordingName : "- replay: disabled");
        Print(_recordVideo ? "- recording to: " + _replayName : "- recording: disabled");
        Print(_dumpVideo ? "- exporting data to: " + _dumpName : "- exporting data: disabled");
        Print("- camera settings: " + _cameraSettings);
        Print("- rscamera settings: " + _rscameraSettings);
        Print("- ambacamera settings: " + _rscameraSettings);
    }

    void Print(const std::string& msg, bool force = false)
    {
        if (_verbose || force)
            std::cout << msg << std::endl;
    }

    void PrintUsage()
    {
        std::cout << "HidFaceSample [OPTION]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "-h" << std::endl;
        std::cout << "    Show this help" << std::endl;
        std::cout << "-v" << std::endl;
        std::cout << "    Verbose output" << std::endl;
        std::cout << "-cam=SETTINGS" << std::endl;
        std::cout << "    Select camera and its settings" << std::endl;
        std::cout << "    - USB: ID:WIDTH:HEIGHT" << std::endl;
        std::cout << "    - IP: URL" << std::endl;
        std::cout << "-rscam=SETTINGS" << std::endl;
        std::cout << "    Select camera and its settings" << std::endl;
        std::cout << "    - INTEL: ID:WIDTH:HEIGHT:DEPTH_WIDTH:DEPTH_HEIGHT" << std::endl;
        std::cout << "-ambacam=SETTINGS" << std::endl;
        std::cout << "    Select camera and its settings" << std::endl;
        std::cout << "    - AMBARELLA: ID:WIDTH:HEIGHT" << std::endl; //:TODO: AMBA: width&height are not set by the amba camera, TBD whether it will be possible to set them by it
        std::cout << "-liveness" << std::endl;
        std::cout << "    Enable liveness detection with the default algorithm (paravision)" << std::endl;
        std::cout << "-liveness=ALGO" << std::endl;
        std::cout << "    Enable liveness detection with the selected algorithm; supported options: paravision, pad" << std::endl;
        std::cout << "-gui" << std::endl;
        std::cout << "    Show the camera stream in a window" << std::endl;
        std::cout << "    additional information can be disabled by GUI_MODE=plain" << std::endl;
        std::cout << "-mode=STREAM_MODE" << std::endl;
        std::cout << "    Specify how much information will be shown, in the GUI and recording" << std::endl;
        std::cout << "    - plain : no additional information, only original data" << std::endl;
        std::cout << "    - enhanced : default, bounding box, timestamp, frame count, quality and other info over the image" << std::endl;
        std::cout << "    - match : same as enhanced but the bounding box is colored based on match and not quality" << std::endl;
        std::cout << "-rec[=FILE_NAME]" << std::endl;
        std::cout << "    Record the camera stream as shown in GUI to a file, default file name is " << DefaultRecFileName << std::endl;
        std::cout << "-dmp[=DIR_NAME]" << std::endl;
        std::cout << "    Export the camera output to images, default directory is " << DefaultDmpDirName << std::endl;
        std::cout << "-play[=FILE_NAME]" << std::endl;
        std::cout << "    Replay a video from a file instead of camera input, default file name is " << DefaultRecFileName << std::endl;
        std::cout << "-saveimgs" << std::endl;
        std::cout << "    Saves images in the frame set from the last identification" << std::endl;
    }
};

const std::string Settings::DefaultRecFileName = "stream.avi";
const std::string Settings::DefaultDmpDirName = "camera_export";

class Demo
{
    Settings _settings;
    HFContext _camera;
    std::thread _saveThread;
    std::atomic_bool _saveContinue;
    std::queue<cv::Mat> _saveQueue;
    std::mutex _saveLock;
    size_t _frameCounter;
    size_t _bioCounter;
    std::set<int64_t> _usedIds;
    HFBiometricResult _lastResult;
    void* _templ;
    size_t _templSize;

    void InstrumentFrame(cv::Mat& frame)
    {
        cv::Scalar green(0, 230, 0, 255);
        cv::Scalar red(0, 0, 255, 255);
        time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        cv::putText(frame, std::ctime(&now), cv::Point(10, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, green, 1, cv::LINE_AA);
        cv::putText(frame, std::to_string(++_frameCounter), cv::Point(frame.size().width - 60, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, green, 1, cv::LINE_AA);
        cv::putText(frame, std::to_string(_bioCounter), cv::Point(frame.size().width / 2 - 10, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, green, 1, cv::LINE_AA);
        cv::putText(frame, "ID: " + std::string(_lastResult.id == nullptr ? "" : _lastResult.id), cv::Point(10, frame.size().height - 10), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, _lastResult.success ? green : red, 1, cv::LINE_AA);
        cv::putText(frame, "Conf: " + std::to_string(_lastResult.confidence), cv::Point(frame.size().width / 2 - 100, frame.size().height - 10), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, _lastResult.facePresent ? green : red, 1, cv::LINE_AA);
        cv::putText(frame, "Quality: " + std::to_string(_lastResult.quality), cv::Point(frame.size().width - 150, frame.size().height - 10), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, _lastResult.facePresent ? green : red, 1, cv::LINE_AA);
        if (_lastResult.facePresent)
        {
            cv::Scalar boxColor(0, static_cast<int>(_lastResult.quality * 255), static_cast<int>((1 - _lastResult.quality) * 255), 255);
            if (_settings._matchColoredBox)
                boxColor = _lastResult.success ? green : red;
            cv::rectangle(frame, cv::Rect(_lastResult.boundBox.x, _lastResult.boundBox.y, _lastResult.boundBox.width, _lastResult.boundBox.height),
                boxColor, 1);
        }
    }

    static void StreamCallback(void* callbackContext, int32_t errorCode, uint32_t seq, HFFrameSet frameSet)
    {
        void* mat = NULL;

        if (HFFrameSetGetMat(frameSet, HFIMAGE_TYPE_BGR, &mat) == HFERROR_OK) {
            cv::Mat& frame = *reinterpret_cast<cv::Mat*>(mat);
            Demo* demo = reinterpret_cast<Demo*>(callbackContext);
            if (!demo->_settings._plainStream)
                demo->InstrumentFrame(frame);
            if (demo->_settings._guiEnabled)
            {
                cv::imshow("Display window", frame);
                cv::waitKey(5);
            }
            if (demo->_saveContinue.load())
            {
                std::lock_guard<std::mutex> lock(demo->_saveLock);
                demo->_saveQueue.emplace(frame);
            }
        }
    }

    static bool IdentifyCallback(void* callbackContext, int32_t errorCode, HFBiometricResult* result)
    {
        Demo* demo = reinterpret_cast<Demo*>(callbackContext);
        demo->_lastResult = *result;
        demo->_bioCounter++;
        if (result->success)
        {
            HFImage img;
            demo->_settings.Print("User identified as " + std::string(result->id) + " spoof probability is " + std::to_string(result->spoofProbability), true);
            if (demo->_settings._saveimgs)
            {
                VERIFY_OK(HFFrameSetGetImage(result->frameSet, HFIMAGE_TYPE_BGR, &img));
                StoreImage(img, "identify.png");
                HFFree(img);
#ifdef _PLATFORM_CV2X
                if (HFFrameSetGetImage(result->frameSet, HFIMAGE_TYPE_INFRARED, &img) == HFERROR_OK)
                {
                    StoreImage(img, "identify_infrared.png");
                    HFFree(img);
                }
                if (HFFrameSetGetImage(result->frameSet, HFIMAGE_TYPE_PATTERN, &img) == HFERROR_OK)
                {
                    StoreImage(img, "identify_pattern.png");
                    HFFree(img);
                }
#else
                if (HFFrameSetGetImage(result->frameSet, HFIMAGE_TYPE_DEPTH, &img) == HFERROR_OK)
                {
                    StoreImage(img, "identify_depth.png");
                    HFFree(img);
                }
#endif
            }
        }
        return true;
    }

    static bool CaptureCallback(void* callbackContext, int32_t errorCode, HFBiometricResult* result) {
        Demo* demo = reinterpret_cast<Demo*>(callbackContext);
        if (result->success)
        {
            if (demo->_templ != nullptr)
            {
                free(demo->_templ);
                demo->_templ = nullptr;
            }
            demo->_templ = malloc(result->templSize);
            if (demo->_templ != nullptr)
            {
                memcpy(demo->_templ, result->templ, result->templSize);
                demo->_templSize = result->templSize;
            }
        }
        else
        {
            demo->_settings.Print("Image quality insufficient " + std::to_string(result->quality), true);
        }
        return !result->success;
    }

    bool ReadDatabase(const char* filename, HFDatabaseRecord** database, int* databaseSize)
    {
        FILE* fh = fopen(filename, "rb");
        if (fh != NULL)
        {
            fseek(fh, 0L, SEEK_END);
            int size = ftell(fh);
            uint8_t* dataAlloc = (uint8_t*)malloc(size);
            uint8_t* data = dataAlloc;
            if (data == nullptr)
            {
                throw std::exception();
            }
            rewind(fh);
            fread(data, size, 1, fh);
            fclose(fh);

            int32_t count = *((int32_t*)data);
            data += sizeof(int32_t);
            HFDatabaseRecord* db = (HFDatabaseRecord*)malloc(sizeof(HFDatabaseRecord) * count);
            if (db == nullptr)
            {
                throw std::exception();
            }
            for (int i = 0; i < count; i++)
            {
                char* id = (char*)malloc(20 * sizeof(char));
                if (id == nullptr)
                {
                    throw std::exception();
                }
                strcpy(id, std::to_string(*((int32_t*)data)).c_str());
                db[i].recordId = id;
                db[i].customData = nullptr;
                data += sizeof(int32_t);
                db[i].templSize = *((int32_t*)data);
                data += sizeof(int32_t);
                db[i].templ = malloc(db[i].templSize);
                if (db[i].templ == nullptr)
                {
                    throw std::exception();
                }
                memcpy(db[i].templ, data, db[i].templSize);
                data += db[i].templSize;
            }

            *database = db;
            *databaseSize = count;
            free(dataAlloc);
            return true;
        }
        else
        {
            return false;
        }

    }

    void WriteDatabase(const char* filename, HFDatabaseRecord* database, int databaseSize)
    {
        FILE* fh = fopen(filename, "wb");
        if (fh != NULL)
        {
            int32_t size = (2 * databaseSize + 1) * sizeof(int32_t);
            for (int i = 0; i < databaseSize; i++)
            {
                size += database[i].templSize;
            }
            uint8_t* data = (uint8_t*)malloc(size);
            if (data == nullptr)
            {
                throw std::exception();
            }
            void* start = data;

            *((int32_t*)data) = databaseSize;
            data += sizeof(int32_t);
            for (int i = 0; i < databaseSize; i++)
            {
                *((int32_t*)data) = std::stoi(database[i].recordId);
                data += sizeof(int32_t);
                *((int32_t*)data) = database[i].templSize;
                data += sizeof(int32_t);
                memcpy(data, database[i].templ, database[i].templSize);
                data += database[i].templSize;
            }

            fwrite(start, size, 1, fh); // write 10 bytes from our buffer
            fclose(fh);
            free(start);
        }
    }

public:
    Demo(const Settings& settings)
        : _settings(settings), _camera(nullptr), _saveContinue(false), _frameCounter(0), _bioCounter(0), _lastResult{ 0 }, _templ{ nullptr }
    {
        _settings.Print("Initializing ...");
        VERIFY_OK(HFInit(""))
            HFVersion version;
        VERIFY_OK(HFGetVersion(&version))
            std::cout << "HFApi version: " << version.major << version.minor << version.patch << version.build << std::endl;

        if (_settings._guiEnabled)
        {
            _settings.Print("GUI enabled, streaming camera output...");
#ifndef _WIN32
            cv::namedWindow("Display window", cv::WINDOW_NORMAL);
#endif  // _WIN32
        }
    }
    void LoadDatabase(const std::string& dbName, HFLoadDatabaseMode mode)
    {
#ifdef _WIN32
        if (_access(dbName.c_str(), 0) != -1)
#else // !_WIN32
        if (access(dbName.c_str(), F_OK) != -1)
#endif  // !_WIN32
        {
            _settings.Print("Loading database " + dbName, true);
            HFDatabaseRecord* database;
            int databaseSize;
            if (ReadDatabase(dbName.c_str(), &database, &databaseSize))
            {
                std::cout << "The database size: " << databaseSize << std::endl;
                uint32_t* conflictRecords = nullptr;
                size_t duplicateRecSize;
                VERIFY_OK(HFLoadDatabase(_camera, database, databaseSize, false, 0.90, mode, &conflictRecords, &duplicateRecSize))
                    for (size_t i = 0; i < databaseSize; i++)
                    {
                        _usedIds.insert(std::stoi(database[i].recordId));
                        free(database[i].templ);
                        free(const_cast<char*>(database[i].recordId));
                    }
                free(database);
                if (conflictRecords != nullptr)
                {
                    std::cout << "LoadDatabase found " << duplicateRecSize << " conflicting records." << std::endl;
                    free(conflictRecords);
                }

            }
        }
    }
    void StoreDatabase(const std::string& dbName)
    {
        _settings.Print("Exporting database " + dbName, true);
        HFDatabaseRecord* databaseRecords;
        size_t recordCount;
        VERIFY_OK(HFSaveDatabase(&databaseRecords, &recordCount));
        WriteDatabase(dbName.c_str(), databaseRecords, recordCount);
        HFFree(databaseRecords);
    }
    void OpenCamera(const std::string& camera, const std::string& recognitionEngine, const std::string& livenessEngine)
    {
        VERIFY(_camera == nullptr)
            _settings.Print("Opening camera ...");
        const char* livenessSettings = (_settings._liveness) ? livenessEngine.c_str() : NULL;
        VERIFY_OK(HFOpen((_settings._replayVideo ? "playbackLoop=true;playbackFile=" + _settings._replayName : camera + ";fourcc=MJPG").c_str(), recognitionEngine.c_str(), livenessSettings, &_camera))
            VERIFY(_camera != nullptr)
            HFStatus status;
        VERIFY_OK(HFGetStatus(_camera, &status))
            VERIFY(status == HFSTATUS_READY)
            _settings.Print("Camera ready");

        HFContextCapabilities* contextCapabilities = nullptr;
        HFGetContextCapabilities(_camera, &contextCapabilities);
        std::cout << "Number of camera capabilities: " << contextCapabilities->numberofCameraCapabilites << std::endl;
        std::cout << "Number of biometric capabilities: " << contextCapabilities->numberofBiometricCapabilities << std::endl;
        HFFree(contextCapabilities);

        if (_settings._recordVideo)
        {
            _saveContinue.store(true);
            _saveThread = std::thread([this]() {
                int32_t width, height, fps;
                VERIFY_OK(HFGetParam(_camera, HF_PARAMETER_CAMERA_WIDTH, &width))
                    VERIFY_OK(HFGetParam(_camera, HF_PARAMETER_CAMERA_HEIGHT, &height))
                    VERIFY_OK(HFGetParam(_camera, HF_PARAMETER_CAMERA_FPS, &fps))
                    while (fps < 1)
                    {
                        VERIFY_OK(HFGetParam(_camera, HF_PARAMETER_CAMERA_FPS, &fps))
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    }
                _settings.Print("Recording video with resolution " + std::to_string(width) + "x" + std::to_string(height) + " FPS " + std::to_string(fps));
                cv::VideoWriter video(_settings._recordingName, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(width, height));
                while (_saveContinue.load() || _saveQueue.size() > 0)
                {
                    if (_saveQueue.size() > 0)
                    {
                        _saveLock.lock();
                        cv::Mat frame(_saveQueue.front());
                        _saveQueue.pop();
                        _saveLock.unlock();
                        video.write(frame);
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }
                video.release();
                });
        }
        if (_settings._guiEnabled || _settings._recordVideo)
        {
            VERIFY_OK(HFSetVideoCallback(_camera, StreamCallback, this));
        }
    }
    void CloseCamera()
    {
        VERIFY(_camera != nullptr)
            VERIFY_OK(HFClose(_camera))
            _camera = nullptr;
    }
    void CaptureEnrollTemplate(HFContext context, int64_t timeout)
    {
        HFStatus status = HFSTATUS_BUSY;

        VERIFY_OK(HFCaptureTemplate(
            context, timeout, HF_TEMPLATE_PURPOSE_ENROLL, CaptureCallback, this));
        while (HFGetStatus(context, &status) == HFERROR_OK && status == HFSTATUS_BUSY)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    /**
     * @brief Tests HFMatch by capturing two templates 5 seconds apart.
     */
    void MatchTemplates()
    {
        HFStatus status = HFSTATUS_BUSY;

        printf("Capturing the first template...\n");
        VERIFY_OK(HFCaptureTemplate(
            _camera, -1, HF_TEMPLATE_PURPOSE_IDENTIFY, CaptureCallback, this));
        while (HFGetStatus(_camera, &status) == HFERROR_OK && status == HFSTATUS_BUSY)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        HFTemplate* templA = reinterpret_cast<HFTemplate*>(malloc(_templSize));
        memcpy(templA, _templ, _templSize);
        size_t templASize = _templSize;

        std::this_thread::sleep_for(std::chrono::seconds(5));

        printf("Capturing the second template...\n");
        VERIFY_OK(HFCaptureTemplate(
            _camera, -1, HF_TEMPLATE_PURPOSE_IDENTIFY, CaptureCallback, this));
        while (HFGetStatus(_camera, &status) == HFERROR_OK && status == HFSTATUS_BUSY)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        HFTemplate* templB = reinterpret_cast<HFTemplate*>(malloc(_templSize));
        memcpy(templB, _templ, _templSize);
        size_t templBSize = _templSize;

        double confidence = 0;
        VERIFY_OK(HFMatch(_camera, templA, templASize, templB, templBSize, &confidence));
        printf("Confidence in matching two templates: %f ...\n", confidence);

        free(templA);
        free(templB);

    }
    void EnrollUser()
    {
        int64_t id = -1;
        for (int64_t i = 0; i < 10000; i++)
        {
            if (_usedIds.find(i) == _usedIds.end())
            {
                id = i;
                break;
            }
        }
        VERIFY(id != -1)
            CaptureEnrollTemplate(_camera, -1);
        _settings.Print("Enrolling user " + std::to_string(id));
        HFDatabaseRecord rec = { const_cast<char*>(std::to_string(id).c_str()), nullptr, _templSize, _templ };
        try
        {
            VERIFY_OK(HFAddRecord(_camera, &rec, true, false, 0.90));
            _usedIds.insert(id);
        }
        catch (int& exceptionCode)
        {
            if (exceptionCode == HFERROR_ALREADY_PRESENT)
            {
                printf("\nDuplicate template found. User was not added.\n");
            }
            else
            {
                throw;
            }
        }

        HFStatus status;
        VERIFY_OK(HFGetStatus(_camera, &status))
            while (status == HFSTATUS_BUSY)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                VERIFY_OK(HFGetStatus(_camera, &status))
            }
        _lastResult = { 0 };
    }

    void EnrollUser(const std::string& fileName)
    {
        FILE* fh = fopen(fileName.c_str(), "rb");
        if (fh != NULL)
        {
            fseek(fh, 0L, SEEK_END);
            int size = ftell(fh);
            uint8_t* data = (uint8_t*)malloc(size);
            if (data == nullptr)
            {
                throw std::exception();
            }
            rewind(fh);
            fread(data, size, 1, fh);
            fclose(fh);

            int64_t id = -1;
            for (int64_t i = 0; i < 10000; i++)
            {
                if (_usedIds.find(i) == _usedIds.end())
                {
                    id = i;
                    break;
                }
            }
            VERIFY(id != -1)

                HFImage img;

            _settings.Print("Enrolling user " + std::to_string(id));
            VERIFY_OK(HFImportImage(data, size, HFIMAGE_ENCODING_JPEG, &img));
            HFBiometricResult* biometricResult = nullptr;

            VERIFY_OK(HFCaptureTemplateFromImage(_camera, HF_TEMPLATE_PURPOSE_ENROLL, img, &biometricResult));

            if (biometricResult->templ != nullptr)
            {
                HFDatabaseRecord rec = { const_cast<char*>(std::to_string(id).c_str()), nullptr, biometricResult->templSize, biometricResult->templ };
                try
                {
                    VERIFY_OK(HFAddRecord(_camera, &rec, true, false, 0.90));
                    _usedIds.insert(id);
                }
                catch (int& exceptionCode)
                {
                    if (exceptionCode == HFERROR_ALREADY_PRESENT)
                    {
                        printf("\nDuplicate template found. User was not added.\n");
                    }
                    else
                    {
                        throw;
                    }
                }
            }
            else
            {
                _settings.Print("Error: " + fileName + " - image has insufficient quality!");
            }
            HFFree(biometricResult);
            HFFree(img);
            free(data);

            _lastResult = { 0 };
        }
    }

    void IdentifyUsers()
    {
        printf("Identifying the enrolled users (n to stop) ...\n");
        _lastResult = { 0 };
        VERIFY_OK(HFIdentify(_camera, HF_INFINITE_TIMEOUT, IdentifyCallback, this));
    }
    void Stop()
    {
        VERIFY_OK(HFStop(_camera));
    }
    ~Demo()
    {
        HFStop(_camera);
        if (_settings._recordVideo)
            _settings.Print("Recording remaining video frames ...", true);
        _saveContinue.store(false);
        if (_saveThread.joinable())
            _saveThread.join();
        _settings.Print("Terminating ...");
        HFTerminate();
        if (_settings._guiEnabled)
        {
            cv::destroyAllWindows();
        }
        if (_templ != nullptr)
        {
            free(_templ);
            _templ = nullptr;
        }
    }
};

int main(int argc, char** argv)
{
    try {
        Settings settings(argc, argv);
        if (settings._help)
        {
            settings.PrintUsage();
        }
        else
        {
            int32_t camnum, width, height, depthWidth, depthHeight;
            char a, b, c, d;
            std::string camConf;
            std::string recognitionConf;
            std::string livenessConf = "antispoofOnlyCenteredFaces=true";
            if (settings._rscameraSettings.length() > 0)
            {
                std::stringstream rs(settings._rscameraSettings);
                rs >> camnum >> a >> width >> b >> height >> c >> depthWidth >> d >> depthHeight;
                if (a != ':' || b != ':' || c != ':' || d != ':')
                    throw std::invalid_argument("RS camera config format error");
                camConf = "rsCamNum=" + std::to_string(camnum) +
                    ";width=" + std::to_string(width) +
                    ";height=" + std::to_string(height) +
                    ";widthDepth=" + std::to_string(depthWidth) +
                    ";heightDepth=" + std::to_string(depthHeight);
            }
            else if (settings._ambacameraSettings.length() > 0)
            {
                std::stringstream ss(settings._ambacameraSettings);
                ss >> camnum >> a >> width >> b >> height;
                if (a != ':' || b != ':')
                    throw std::invalid_argument("Amba camera config format error");
                camConf = "ambaCamNum=" + std::to_string(camnum) +
                    ";width=" + std::to_string(width) +
                    ";height=" + std::to_string(height);
            }
            else
            {
                std::stringstream ss(settings._cameraSettings);
                ss >> camnum >> a >> width >> b >> height;
                if (a != ':' || b != ':')
                    camConf = "url=" + settings._cameraSettings;
                else
                    camConf = "camNum=" + std::to_string(camnum) +
                    ";width=" + std::to_string(width) +
                    ";height=" + std::to_string(height);
            }
            if (settings._dumpVideo)
                camConf += ";recordDir=" + settings._dumpName;
            camConf += ";idleTimeout=-1";
            settings.Print("Camera config string - " + camConf);
            Demo demo(settings);


#ifdef _WIN32
            recognitionConf += ";modelsPath=tarka";
#endif  // _WIN32

            livenessConf = "algorithm = " + std::to_string(settings._livenessAlgo) + ";" + livenessConf;
            demo.OpenCamera(camConf, recognitionConf, livenessConf);

            printf("Do you want to test LoadDatabase? (y/n)\n");
            if (GetChar() == 'y')
                demo.LoadDatabase("db.bin", HFLOADDATABASE_REPLACE);

            printf("Do you want to test LoadDatabase the second time in the APPEND mode? (y/n)\n");
            if (GetChar() == 'y')
                demo.LoadDatabase("db.bin", HFLOADDATABASE_APPEND);

            printf("Do you want to test MatchTemplates? (y/n)\n");
            if (GetChar() == 'y')
                demo.MatchTemplates();

            printf("You now have an option to add users from face.jpg upto 10 times.\n");
            printf("Provide a new face.jpg before pressing y to continue.\n");
            for (int i = 0; i < 10; i++)
            {
                printf("Continue loading a user from face.jpg? Press n to discontinue. \n");
                if (GetChar() == 'n')
                    break;
                demo.EnrollUser("face.jpg");
            }

            for (int i = 0; i < 10000; i++)
            {
                printf("Add another user? (y/n)\n");
                if (GetChar() == 'n')
                    break;
                demo.EnrollUser();
            }
            demo.IdentifyUsers();
            while (GetChar() != 'n')
                ;
            demo.Stop();
            demo.StoreDatabase("db.bin");
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Demo failed - " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Demo failed" << std::endl;
    }
}
