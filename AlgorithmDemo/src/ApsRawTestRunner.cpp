#include "ApsRawTestRunner.h"

#include "AlpMPAlgoInterface.h"
#include "SimpleJson.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <direct.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef ALGORITHM_DEMO_SOURCE_DIR
#define ALGORITHM_DEMO_SOURCE_DIR "."
#endif

#ifndef ALGORITHM_DEMO_PROJECT_ROOT
#define ALGORITHM_DEMO_PROJECT_ROOT "."
#endif

namespace {

using Clock = std::chrono::high_resolution_clock;

const char* kConfigPath =
    ALGORITHM_DEMO_SOURCE_DIR "/config/aps_demo_last_config.ini";
const char* kDefaultRawPath =
    ALGORITHM_DEMO_PROJECT_ROOT
    "/test_rawdata/testdata_center_crop_3264x2448_5frames_u16_from_3280x2464.raw";
bool gInputEof = false;

struct ApsDemoConfig {
    std::string rawPath;
    SensorType sensor;
    APSRawType rawType;
    PixelFormatType pixelFormat;
    uint32_t rawRows;
    uint32_t rawCols;
    bool rawSizeOverride;
    uint32_t frames;
    uint32_t frameStart;
    bool headerFooter;
    bool multiThread;
    bool logEnable;
    bool activeAreaEnabled;
    ROIArea activeArea;
    bool roiEnabled;
    ROIArea roi;
    double badPixelThreshold;
    uint32_t badPixelRadius;
    double hotPixelThreshold;
    std::vector<std::string> selectedItems;
    std::string outputDirectory;
    std::string resultJsonPath;
};

struct TestItemDef {
    const char* id;
    const char* title;
};

struct TestMetric {
    std::string key;
    std::string value;
};

struct TestRunRecord {
    std::string id;
    std::string title;
    bool success;
    long long ms;
    std::string message;
    std::vector<TestMetric> metrics;
};

struct ApsRunResult {
    bool success;
    std::string error;
    long long importMs;
    std::vector<TestRunRecord> tests;
};

const TestItemDef kTestItems[] = {
    {"snoise", "SNoise"},
    {"tnoise", "TNoise"},
    {"badpixel", "BadPixel"},
    {"hotpixel", "HotPixel"},
    {"datamean", "DataMean"},
    {"dsnu", "DSNU"},
};

std::string Trim(const std::string& text) {
    const char* spaces = " \t\r\n";
    const size_t start = text.find_first_not_of(spaces);
    if (start == std::string::npos) {
        return "";
    }
    const size_t end = text.find_last_not_of(spaces);
    return text.substr(start, end - start + 1);
}

std::string ToLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::string StripQuotes(const std::string& text) {
    std::string out = Trim(text);
    if (out.size() >= 2 &&
        ((out.front() == '"' && out.back() == '"') ||
         (out.front() == '\'' && out.back() == '\''))) {
        out = out.substr(1, out.size() - 2);
    }
    return out;
}

std::vector<std::string> SplitList(const std::string& text) {
    std::vector<std::string> values;
    std::string token;
    for (size_t i = 0; i < text.size(); ++i) {
        const char ch = text[i];
        if (ch == ',' || ch == ';' || ch == ' ') {
            token = Trim(token);
            if (!token.empty()) {
                values.push_back(token);
            }
            token.clear();
        } else {
            token.push_back(ch);
        }
    }
    token = Trim(token);
    if (!token.empty()) {
        values.push_back(token);
    }
    return values;
}

bool ParseBool(const std::string& text, bool defaultValue) {
    const std::string value = ToLower(Trim(text));
    if (value == "1" || value == "true" || value == "yes" || value == "on") {
        return true;
    }
    if (value == "0" || value == "false" || value == "no" || value == "off") {
        return false;
    }
    return defaultValue;
}

uint32_t ParseUInt(const std::string& text, uint32_t defaultValue) {
    std::istringstream in(Trim(text));
    uint32_t value = 0;
    if (in >> value) {
        return value;
    }
    return defaultValue;
}

double ParseDouble(const std::string& text, double defaultValue) {
    std::istringstream in(Trim(text));
    double value = 0;
    if (in >> value) {
        return value;
    }
    return defaultValue;
}

bool FileExists(const std::string& path) {
    std::ifstream in(path.c_str(), std::ios::binary);
    return in.good();
}

std::vector<uint8_t> ReadBinaryFile(const std::string& path) {
    std::ifstream in(path.c_str(), std::ios::binary | std::ios::ate);
    if (!in) {
        throw std::runtime_error("failed to open raw file: " + path);
    }
    const std::streamoff size = in.tellg();
    if (size <= 0) {
        throw std::runtime_error("raw file is empty: " + path);
    }
    std::vector<uint8_t> data(static_cast<size_t>(size));
    in.seekg(0, std::ios::beg);
    in.read(reinterpret_cast<char*>(&data[0]), size);
    if (!in) {
        throw std::runtime_error("failed to read raw file: " + path);
    }
    return data;
}

std::string JoinItems(const std::vector<std::string>& items) {
    std::ostringstream out;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) {
            out << ",";
        }
        out << items[i];
    }
    return out.str();
}

std::string NormalizeItemId(const std::string& item) {
    std::string id = ToLower(Trim(item));
    if (id == "badpoint") {
        return "badpixel";
    }
    if (id == "mean") {
        return "datamean";
    }
    return id;
}

bool IsKnownItem(const std::string& id) {
    for (size_t i = 0; i < sizeof(kTestItems) / sizeof(kTestItems[0]); ++i) {
        if (id == kTestItems[i].id) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> NormalizeItems(const std::vector<std::string>& items) {
    std::vector<std::string> out;
    for (size_t i = 0; i < items.size(); ++i) {
        const std::string id = NormalizeItemId(items[i]);
        if (IsKnownItem(id) &&
            std::find(out.begin(), out.end(), id) == out.end()) {
            out.push_back(id);
        }
    }
    return out;
}

bool HasItem(const ApsDemoConfig& config, const std::string& id) {
    return std::find(config.selectedItems.begin(), config.selectedItems.end(), id) !=
           config.selectedItems.end();
}

void ToggleItem(ApsDemoConfig& config, const std::string& id) {
    std::vector<std::string>::iterator it =
        std::find(config.selectedItems.begin(), config.selectedItems.end(), id);
    if (it == config.selectedItems.end()) {
        config.selectedItems.push_back(id);
    } else {
        config.selectedItems.erase(it);
    }
}

const char* SensorToString(SensorType sensor) {
    switch (sensor) {
    case ALP_003AA: return "003AA";
    case ALP_003BA: return "003BA";
    case ALP_003BB: return "003BB";
    case ALP_003CA: return "003CA";
    case ALP_004AB: return "004AB";
    case ALP_014AA: return "014AA";
    case ALP_014BA: return "014BA";
    default: return "003CA";
    }
}

SensorType ParseSensor(const std::string& text, SensorType defaultValue) {
    const std::string value = ToLower(Trim(text));
    if (value == "003aa" || value == "alp_003aa") return ALP_003AA;
    if (value == "003ba" || value == "alp_003ba") return ALP_003BA;
    if (value == "003bb" || value == "alp_003bb") return ALP_003BB;
    if (value == "003ca" || value == "alp_003ca") return ALP_003CA;
    if (value == "004ab" || value == "alp_004ab") return ALP_004AB;
    if (value == "014aa" || value == "alp_014aa") return ALP_014AA;
    if (value == "014ba" || value == "alp_014ba") return ALP_014BA;
    return defaultValue;
}

const char* RawTypeToString(APSRawType rawType) {
    switch (rawType) {
    case RAW8: return "raw8";
    case RAW10: return "raw10";
    case RAW12: return "raw12";
    case UNPACK10: return "unpack10";
    case UNPACK12: return "unpack12";
    default: return "unpack10";
    }
}

APSRawType ParseRawType(const std::string& text, APSRawType defaultValue) {
    const std::string value = ToLower(Trim(text));
    if (value == "raw8") return RAW8;
    if (value == "raw10") return RAW10;
    if (value == "raw12") return RAW12;
    if (value == "unpack10" || value == "u10") return UNPACK10;
    if (value == "unpack12" || value == "u12") return UNPACK12;
    return defaultValue;
}

const char* PixelFormatToString(PixelFormatType format) {
    switch (format) {
    case BayerGBRG: return "BayerGBRG";
    case BayerBGGR: return "BayerBGGR";
    case BayerRGGB: return "BayerRGGB";
    case BayerGRBG: return "BayerGRBG";
    case QuadBayerGBRG: return "QuadBayerGBRG";
    case QuadBayerBGGR: return "QuadBayerBGGR";
    case QuadBayerRGGB: return "QuadBayerRGGB";
    case QuadBayerGRBG: return "QuadBayerGRBG";
    default: return "QuadBayerGBRG";
    }
}

PixelFormatType ParsePixelFormat(const std::string& text, PixelFormatType defaultValue) {
    const std::string value = ToLower(Trim(text));
    if (value == "bayergbrg") return BayerGBRG;
    if (value == "bayerbggr") return BayerBGGR;
    if (value == "bayerrggb") return BayerRGGB;
    if (value == "bayergrbg") return BayerGRBG;
    if (value == "quadbayergbrg") return QuadBayerGBRG;
    if (value == "quadbayerbggr") return QuadBayerBGGR;
    if (value == "quadbayerrggb") return QuadBayerRGGB;
    if (value == "quadbayergrbg") return QuadBayerGRBG;
    return defaultValue;
}

ApsDemoConfig DefaultConfig() {
    ApsDemoConfig config;
    config.rawPath = kDefaultRawPath;
    config.sensor = ALP_003CA;
    config.rawType = UNPACK10;
    config.pixelFormat = QuadBayerGBRG;
    config.rawRows = 2448;
    config.rawCols = 3264;
    config.rawSizeOverride = false;
    config.frames = 5;
    config.frameStart = 0;
    config.headerFooter = false;
    config.multiThread = false;
    config.logEnable = false;
    config.activeAreaEnabled = false;
    config.activeArea = {0, 1223, 0, 1631};
    config.roiEnabled = false;
    config.roi = {0, 1223, 0, 1631};
    config.badPixelThreshold = 0.19;
    config.badPixelRadius = 1;
    config.hotPixelThreshold = 120.0;
    config.selectedItems.push_back("snoise");
    config.selectedItems.push_back("tnoise");
    config.selectedItems.push_back("badpixel");
    config.outputDirectory =
        ALGORITHM_DEMO_PROJECT_ROOT "/output/aps_profile_run";
    config.resultJsonPath = "result.json";
    return config;
}

std::map<std::string, std::string> ReadIni(const std::string& path) {
    std::map<std::string, std::string> values;
    std::ifstream in(path.c_str());
    if (!in) {
        return values;
    }
    std::string section;
    std::string line;
    while (std::getline(in, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line.front() == '[' && line.back() == ']') {
            section = ToLower(Trim(line.substr(1, line.size() - 2)));
            continue;
        }
        const size_t equals = line.find('=');
        if (equals == std::string::npos) {
            continue;
        }
        const std::string key = ToLower(Trim(line.substr(0, equals)));
        const std::string value = Trim(line.substr(equals + 1));
        values[section + "." + key] = value;
    }
    return values;
}

std::string IniValue(const std::map<std::string, std::string>& values,
                     const std::string& key,
                     const std::string& defaultValue) {
    std::map<std::string, std::string>::const_iterator it = values.find(key);
    return it == values.end() ? defaultValue : it->second;
}

ApsDemoConfig LoadConfig() {
    ApsDemoConfig config = DefaultConfig();
    const std::map<std::string, std::string> values = ReadIni(kConfigPath);
    if (values.empty()) {
        return config;
    }

    config.rawPath = IniValue(values, "raw.path", config.rawPath);
    config.frames = ParseUInt(IniValue(values, "raw.frames", ""), config.frames);
    config.sensor = ParseSensor(IniValue(values, "aps.sensor", ""), config.sensor);
    config.rawType = ParseRawType(IniValue(values, "aps.raw_type", ""), config.rawType);
    config.pixelFormat = ParsePixelFormat(IniValue(values, "aps.pixel_format", ""), config.pixelFormat);
    config.multiThread = ParseBool(IniValue(values, "aps.multi_thread", ""), config.multiThread);
    config.logEnable = ParseBool(IniValue(values, "aps.log", ""), config.logEnable);

    const std::vector<std::string> items =
        NormalizeItems(SplitList(IniValue(values, "items.selected", "")));
    if (!items.empty()) {
        config.selectedItems = items;
    }

    config.roiEnabled = ParseBool(IniValue(values, "roi.enabled", ""), config.roiEnabled);
    config.roi.Up = ParseUInt(IniValue(values, "roi.up", ""), config.roi.Up);
    config.roi.Down = ParseUInt(IniValue(values, "roi.down", ""), config.roi.Down);
    config.roi.Left = ParseUInt(IniValue(values, "roi.left", ""), config.roi.Left);
    config.roi.Right = ParseUInt(IniValue(values, "roi.right", ""), config.roi.Right);

    config.badPixelThreshold =
        ParseDouble(IniValue(values, "threshold.badpixel_threshold", ""),
                    config.badPixelThreshold);
    config.badPixelRadius =
        ParseUInt(IniValue(values, "threshold.badpixel_radius", ""),
                  config.badPixelRadius);
    config.hotPixelThreshold =
        ParseDouble(IniValue(values, "threshold.hotpixel_threshold", ""),
                    config.hotPixelThreshold);
    return config;
}

void MakeDirectoryIfNeeded(const std::string& dir) {
    if (dir.empty()) {
        return;
    }
    size_t start = 0;
    if (dir.size() > 2 && dir[1] == ':') {
        start = 3;
    }
    for (size_t i = start; i <= dir.size(); ++i) {
        if (i == dir.size() || dir[i] == '/' || dir[i] == '\\') {
            const std::string part = dir.substr(0, i);
            if (!part.empty()) {
                _mkdir(part.c_str());
            }
        }
    }
}

void EnsureParentDirectory(const std::string& path) {
    const size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        MakeDirectoryIfNeeded(path.substr(0, pos));
    }
}

bool IsAbsolutePath(const std::string& path) {
    return path.size() > 2 && path[1] == ':';
}

std::string PathJoin(const std::string& dir, const std::string& name) {
    if (name.empty() || IsAbsolutePath(name)) {
        return name;
    }
    if (dir.empty()) {
        return name;
    }
    const char last = dir[dir.size() - 1];
    if (last == '/' || last == '\\') {
        return dir + name;
    }
    return dir + "/" + name;
}

std::string OutputPath(const ApsDemoConfig& config, const std::string& fileName) {
    return PathJoin(config.outputDirectory, fileName);
}

std::string NumberToString(double value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(6) << value;
    return out.str();
}

std::string NumberToString(uint64_t value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

void SaveConfig(const ApsDemoConfig& config) {
    EnsureParentDirectory(kConfigPath);
    std::ofstream out(kConfigPath);
    if (!out) {
        std::cerr << "Failed to write config: " << kConfigPath << "\n";
        return;
    }
    out << "[raw]\n";
    out << "path=" << config.rawPath << "\n";
    out << "frames=" << config.frames << "\n\n";

    out << "[aps]\n";
    out << "sensor=" << SensorToString(config.sensor) << "\n";
    out << "raw_type=" << RawTypeToString(config.rawType) << "\n";
    out << "pixel_format=" << PixelFormatToString(config.pixelFormat) << "\n";
    out << "multi_thread=" << (config.multiThread ? "true" : "false") << "\n";
    out << "log=" << (config.logEnable ? "true" : "false") << "\n\n";

    out << "[items]\n";
    out << "selected=" << JoinItems(config.selectedItems) << "\n\n";

    out << "[roi]\n";
    out << "enabled=" << (config.roiEnabled ? "true" : "false") << "\n";
    out << "up=" << config.roi.Up << "\n";
    out << "down=" << config.roi.Down << "\n";
    out << "left=" << config.roi.Left << "\n";
    out << "right=" << config.roi.Right << "\n\n";

    out << "[threshold]\n";
    out << "badpixel_threshold=" << config.badPixelThreshold << "\n";
    out << "badpixel_radius=" << config.badPixelRadius << "\n";
    out << "hotpixel_threshold=" << config.hotPixelThreshold << "\n";
}

const simple_json::Value* JsonGet(const simple_json::Value* value,
                                  const std::string& key) {
    return value == 0 ? 0 : value->get(key);
}

std::string JsonString(const simple_json::Value* value,
                       const std::string& defaultValue) {
    return value == 0 ? defaultValue : value->as_string(defaultValue);
}

bool JsonBool(const simple_json::Value* value, bool defaultValue) {
    return value == 0 ? defaultValue : value->as_bool(defaultValue);
}

uint32_t JsonUInt(const simple_json::Value* value, uint32_t defaultValue) {
    if (value == 0 || !value->is_number()) {
        return defaultValue;
    }
    const double number = value->as_number(defaultValue);
    return number < 0 ? defaultValue : static_cast<uint32_t>(number);
}

double JsonDouble(const simple_json::Value* value, double defaultValue) {
    return value == 0 ? defaultValue : value->as_number(defaultValue);
}

bool ReadRoiObject(const simple_json::Value* value, ROIArea& roi) {
    if (value == 0 || !value->is_object()) {
        return false;
    }
    const simple_json::Value* up = JsonGet(value, "up");
    const simple_json::Value* down = JsonGet(value, "down");
    const simple_json::Value* left = JsonGet(value, "left");
    const simple_json::Value* right = JsonGet(value, "right");
    if (up == 0) up = JsonGet(value, "row_start");
    if (down == 0) down = JsonGet(value, "row_end");
    if (left == 0) left = JsonGet(value, "col_start");
    if (right == 0) right = JsonGet(value, "col_end");
    if (up != 0 || down != 0 || left != 0 || right != 0) {
        roi.Up = JsonUInt(up, roi.Up);
        roi.Down = JsonUInt(down, roi.Down);
        roi.Left = JsonUInt(left, roi.Left);
        roi.Right = JsonUInt(right, roi.Right);
        return true;
    }

    const simple_json::Value* x = JsonGet(value, "x");
    const simple_json::Value* y = JsonGet(value, "y");
    const simple_json::Value* width = JsonGet(value, "width");
    const simple_json::Value* height = JsonGet(value, "height");
    if (x != 0 || y != 0 || width != 0 || height != 0) {
        const uint32_t leftValue = JsonUInt(x, roi.Left);
        const uint32_t upValue = JsonUInt(y, roi.Up);
        const uint32_t widthValue = JsonUInt(width, roi.Right - roi.Left + 1);
        const uint32_t heightValue = JsonUInt(height, roi.Down - roi.Up + 1);
        roi.Left = leftValue;
        roi.Up = upValue;
        roi.Right = leftValue + widthValue - 1;
        roi.Down = upValue + heightValue - 1;
        return true;
    }
    return false;
}

ApsDemoConfig LoadProfileConfig(const std::string& profilePath) {
    ApsDemoConfig config = DefaultConfig();
    const simple_json::Value root = simple_json::ParseFile(profilePath);
    if (!root.is_object()) {
        throw std::runtime_error("profile root must be a JSON object.");
    }

    const std::string mode = ToLower(JsonString(root.get("mode"), "aps"));
    if (mode != "aps") {
        throw std::runtime_error("only APS profile execution is implemented now.");
    }

    config.sensor = ParseSensor(JsonString(root.get("sensor"), ""), config.sensor);
    config.logEnable = JsonBool(root.get("log_enabled"), config.logEnable);
    const simple_json::Value* multiThreadValue = root.get("multi_thread");
    const uint32_t threadCount = JsonUInt(root.get("thread_count"), 0);
    if (multiThreadValue != 0) {
        config.multiThread = JsonBool(multiThreadValue, config.multiThread);
    } else if (threadCount > 1) {
        config.multiThread = true;
    }

    const simple_json::Value* raw = root.get("raw");
    if (raw != 0) {
        config.rawPath = JsonString(raw->get("path"), config.rawPath);
        config.rawType = ParseRawType(JsonString(raw->get("raw_type"), ""),
                                      config.rawType);
        config.pixelFormat =
            ParsePixelFormat(JsonString(raw->get("pixel_format"), ""),
                             config.pixelFormat);
        config.frames = JsonUInt(raw->get("frames"), config.frames);
        config.frameStart = JsonUInt(raw->get("frame_start"), config.frameStart);
        config.headerFooter = JsonBool(raw->get("header_footer"), config.headerFooter);

        const uint32_t width = JsonUInt(raw->get("width"), 0);
        const uint32_t height = JsonUInt(raw->get("height"), 0);
        if (width > 0 && height > 0) {
            config.rawRows = height;
            config.rawCols = width;
            config.rawSizeOverride = true;
        }
    }

    ROIArea activeArea = config.activeArea;
    if (ReadRoiObject(root.get("active_area"), activeArea)) {
        config.activeArea = activeArea;
        config.activeAreaEnabled = true;
    }

    ROIArea roi = config.roi;
    if (ReadRoiObject(root.get("roi"), roi)) {
        config.roi = roi;
        config.roiEnabled = true;
    }

    const simple_json::Value* output = root.get("output");
    if (output != 0) {
        config.outputDirectory =
            JsonString(output->get("directory"), config.outputDirectory);
        config.resultJsonPath =
            JsonString(output->get("result_json"), config.resultJsonPath);
    }

    std::vector<std::string> selected;
    const simple_json::Value* tests = root.get("tests");
    if (tests != 0 && tests->is_array()) {
        const simple_json::Value::ArrayType& values = tests->as_array();
        for (size_t i = 0; i < values.size(); ++i) {
            const simple_json::Value& item = values[i];
            if (!item.is_object()) {
                continue;
            }
            if (!JsonBool(item.get("enabled"), true)) {
                continue;
            }
            const std::string id = NormalizeItemId(JsonString(item.get("name"), ""));
            if (IsKnownItem(id) &&
                std::find(selected.begin(), selected.end(), id) == selected.end()) {
                selected.push_back(id);
            }

            if (id == "badpixel") {
                const simple_json::Value* thresholds = item.get("thresholds");
                config.badPixelThreshold =
                    JsonDouble(JsonGet(thresholds, "pixel"), config.badPixelThreshold);
                config.badPixelRadius =
                    JsonUInt(JsonGet(thresholds, "radius"), config.badPixelRadius);
            } else if (id == "hotpixel") {
                const simple_json::Value* thresholds = item.get("thresholds");
                config.hotPixelThreshold =
                    JsonDouble(JsonGet(thresholds, "pixel"), config.hotPixelThreshold);
            }

            ROIArea roi = config.roi;
            if (ReadRoiObject(item.get("roi"), roi)) {
                config.roi = roi;
                config.roiEnabled = true;
            }
        }
    }
    if (!selected.empty()) {
        config.selectedItems = selected;
    }
    return config;
}

uint64_t FrameBytes(uint32_t row, uint32_t col, APSRawType rawType) {
    const uint64_t pixels = static_cast<uint64_t>(row) * col;
    switch (rawType) {
    case RAW8: return pixels;
    case RAW10: return pixels / 4 * 5;
    case RAW12: return pixels / 2 * 3;
    case UNPACK10:
    case UNPACK12:
    default: return pixels * 2;
    }
}

const char* ChannelName(size_t index) {
    switch (index) {
    case Gb: return "Gb";
    case B: return "B";
    case R: return "R";
    case Gr: return "Gr";
    default: return "Frame";
    }
}

template <typename Func>
long long MeasureMs(Func func) {
    const Clock::time_point begin = Clock::now();
    func();
    const Clock::time_point end = Clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
}

void ApplyThresholds(CAlpAPSMPAlgoInterface* aps, const ApsDemoConfig& config) {
    APSAlgorithmThre thre = aps->GetAlgorithmThre();
    thre.dBadPixelThre = config.badPixelThreshold;
    thre.nBadPixelRadius = config.badPixelRadius;
    thre.dHotPixelThre = config.hotPixelThreshold;
    aps->SetAlgorithmThre(thre);
}

void PrintSNoise(const APSSNoiseType& data) {
    std::cout << "  SNoiseFrame=" << data.SNoiseFrame << "\n";
    for (size_t i = 0; i < data.SubFrameSNoiseData.size(); ++i) {
        const APSSubFrameSNoiseType& ch = data.SubFrameSNoiseData[i];
        std::cout << "  " << ChannelName(i)
                  << ": SNoise=" << ch.SNoise
                  << ", Row=" << ch.RowSNoise
                  << ", Col=" << ch.ColSNoise << "\n";
    }
}

void PrintTNoise(const APSTNoiseType& data) {
    std::cout << "  TNoiseFrame=" << data.TNoiseFrame << "\n";
    for (size_t i = 0; i < data.SubFrameTNoiseData.size(); ++i) {
        const APSSubFrameTNoiseType& ch = data.SubFrameTNoiseData[i];
        std::cout << "  " << ChannelName(i)
                  << ": Temp=" << ch.TempNoise
                  << ", Row=" << ch.RowTemp
                  << ", Col=" << ch.ColTemp
                  << ", Pixel=" << ch.PixelTemp << "\n";
    }
}

void PrintBadPixelLike(const char* name, const APSBadpixelType& data) {
    std::cout << "  " << name
              << ": total=" << data.BadPixelNum
              << ", singlet=" << data.SingletNum
              << ", couplet=" << data.CoupletNum
              << ", cluster=" << data.ClusterNum
              << ", ladder=" << data.LadderNum
              << ", maxCluster=" << data.MaxClusterSize
              << ", slidingWindowMax=" << data.nSlidingWindowMaxBadPixelNum << "\n";
    for (size_t i = 0; i < data.SubFrameBadpixelData.size(); ++i) {
        const APSSubFrameBadpixelType& ch = data.SubFrameBadpixelData[i];
        std::cout << "  " << ChannelName(i)
                  << ": total=" << ch.BadPixelNum
                  << ", singlet=" << ch.SingletNum
                  << ", couplet=" << ch.CoupletNum
                  << ", cluster=" << ch.ClusterNum
                  << ", row=" << ch.DefectRowNum
                  << ", col=" << ch.DefectColNum << "\n";
    }
}

void PrintDataMean(const APSDataMeanType& data) {
    std::cout << "  DataMeanFrame=" << data.DataMeanFrame << "\n";
    for (size_t i = 0; i < data.SubFrameDataMean.size(); ++i) {
        std::cout << "  " << ChannelName(i)
                  << ": Mean=" << data.SubFrameDataMean[i] << "\n";
    }
}

void PrintDSNU(const APSDSNUType& data) {
    std::cout << "  DSNU: RangeR=" << data.RangeR
              << ", RangeG=" << data.RangeG
              << ", RangeB=" << data.RangeB << "\n";
    std::cout << "  SignalMax=" << data.SignalMax
              << ", DeltaSignalMax=" << data.DeltaSignalMax
              << ", DeltaSignalCentreMax=" << data.DeltaSignalCentreMax
              << ", DeltaSignalEdgeMax=" << data.DeltaSignalEdgeMax
              << ", DeltaSignalCornerMax=" << data.DeltaSignalCornerMax << "\n";
}

void AddMetric(TestRunRecord& record, const std::string& key, double value) {
    TestMetric metric;
    metric.key = key;
    metric.value = NumberToString(value);
    record.metrics.push_back(metric);
}

void AddMetric(TestRunRecord& record, const std::string& key, uint64_t value) {
    TestMetric metric;
    metric.key = key;
    metric.value = NumberToString(value);
    record.metrics.push_back(metric);
}

void FillSNoiseMetrics(TestRunRecord& record, const APSSNoiseType& data) {
    AddMetric(record, "SNoiseFrame", static_cast<uint64_t>(data.SNoiseFrame));
    for (size_t i = 0; i < data.SubFrameSNoiseData.size(); ++i) {
        const std::string prefix = std::string(ChannelName(i)) + ".";
        const APSSubFrameSNoiseType& ch = data.SubFrameSNoiseData[i];
        AddMetric(record, prefix + "SNoise", ch.SNoise);
        AddMetric(record, prefix + "RowSNoise", ch.RowSNoise);
        AddMetric(record, prefix + "ColSNoise", ch.ColSNoise);
    }
}

void FillTNoiseMetrics(TestRunRecord& record, const APSTNoiseType& data) {
    AddMetric(record, "TNoiseFrame", static_cast<uint64_t>(data.TNoiseFrame));
    for (size_t i = 0; i < data.SubFrameTNoiseData.size(); ++i) {
        const std::string prefix = std::string(ChannelName(i)) + ".";
        const APSSubFrameTNoiseType& ch = data.SubFrameTNoiseData[i];
        AddMetric(record, prefix + "TempNoise", ch.TempNoise);
        AddMetric(record, prefix + "RowTemp", ch.RowTemp);
        AddMetric(record, prefix + "ColTemp", ch.ColTemp);
        AddMetric(record, prefix + "PixelTemp", ch.PixelTemp);
    }
}

void FillBadPixelMetrics(TestRunRecord& record, const APSBadpixelType& data) {
    AddMetric(record, "BadPixelNum", static_cast<uint64_t>(data.BadPixelNum));
    AddMetric(record, "SingletNum", static_cast<uint64_t>(data.SingletNum));
    AddMetric(record, "CoupletNum", static_cast<uint64_t>(data.CoupletNum));
    AddMetric(record, "ClusterNum", static_cast<uint64_t>(data.ClusterNum));
    AddMetric(record, "LadderNum", static_cast<uint64_t>(data.LadderNum));
    AddMetric(record, "MaxClusterSize", static_cast<uint64_t>(data.MaxClusterSize));
    AddMetric(record, "SlidingWindowMaxBadPixelNum",
              static_cast<uint64_t>(data.nSlidingWindowMaxBadPixelNum));
    for (size_t i = 0; i < data.SubFrameBadpixelData.size(); ++i) {
        const std::string prefix = std::string(ChannelName(i)) + ".";
        const APSSubFrameBadpixelType& ch = data.SubFrameBadpixelData[i];
        AddMetric(record, prefix + "BadPixelNum", static_cast<uint64_t>(ch.BadPixelNum));
        AddMetric(record, prefix + "SingletNum", static_cast<uint64_t>(ch.SingletNum));
        AddMetric(record, prefix + "CoupletNum", static_cast<uint64_t>(ch.CoupletNum));
        AddMetric(record, prefix + "ClusterNum", static_cast<uint64_t>(ch.ClusterNum));
        AddMetric(record, prefix + "DefectRowNum", static_cast<uint64_t>(ch.DefectRowNum));
        AddMetric(record, prefix + "DefectColNum", static_cast<uint64_t>(ch.DefectColNum));
    }
}

void FillDataMeanMetrics(TestRunRecord& record, const APSDataMeanType& data) {
    AddMetric(record, "DataMeanFrame", static_cast<uint64_t>(data.DataMeanFrame));
    for (size_t i = 0; i < data.SubFrameDataMean.size(); ++i) {
        AddMetric(record, std::string(ChannelName(i)) + ".Mean", data.SubFrameDataMean[i]);
    }
}

void FillDSNUMetrics(TestRunRecord& record, const APSDSNUType& data) {
    AddMetric(record, "RangeR", data.RangeR);
    AddMetric(record, "RangeG", data.RangeG);
    AddMetric(record, "RangeB", data.RangeB);
    AddMetric(record, "SignalMax", data.SignalMax);
    AddMetric(record, "DeltaSignalMax", data.DeltaSignalMax);
    AddMetric(record, "DeltaSignalCentreMax", data.DeltaSignalCentreMax);
    AddMetric(record, "DeltaSignalEdgeMax", data.DeltaSignalEdgeMax);
    AddMetric(record, "DeltaSignalCornerMax", data.DeltaSignalCornerMax);
}

bool ValidateRoi(const ROIArea& roi, uint32_t row, uint32_t col) {
    return roi.Up <= roi.Down && roi.Left <= roi.Right &&
           roi.Down < row && roi.Right < col;
}

void WriteResultJson(const ApsDemoConfig& config, const ApsRunResult& result) {
    const std::string path = OutputPath(config, config.resultJsonPath);
    EnsureParentDirectory(path);
    std::ofstream out(path.c_str(), std::ios::out | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("failed to write result json: " + path);
    }

    out << "{\n";
    out << "  \"status\": \"" << (result.success ? "ok" : "failed") << "\",\n";
    out << "  \"error\": \"" << simple_json::EscapeString(result.error) << "\",\n";
    out << "  \"raw\": {\n";
    out << "    \"path\": \"" << simple_json::EscapeString(config.rawPath) << "\",\n";
    out << "    \"sensor\": \"ALP_" << SensorToString(config.sensor) << "\",\n";
    out << "    \"raw_type\": \"" << RawTypeToString(config.rawType) << "\",\n";
    out << "    \"pixel_format\": \"" << PixelFormatToString(config.pixelFormat) << "\",\n";
    out << "    \"frames\": " << config.frames << "\n";
    out << "  },\n";
    out << "  \"import_ms\": " << result.importMs << ",\n";
    out << "  \"tests\": [\n";
    for (size_t i = 0; i < result.tests.size(); ++i) {
        const TestRunRecord& test = result.tests[i];
        out << "    {\n";
        out << "      \"name\": \"" << simple_json::EscapeString(test.id) << "\",\n";
        out << "      \"title\": \"" << simple_json::EscapeString(test.title) << "\",\n";
        out << "      \"status\": \"" << (test.success ? "ok" : "failed") << "\",\n";
        out << "      \"ms\": " << test.ms << ",\n";
        out << "      \"message\": \"" << simple_json::EscapeString(test.message) << "\",\n";
        out << "      \"metrics\": {\n";
        for (size_t j = 0; j < test.metrics.size(); ++j) {
            out << "        \"" << simple_json::EscapeString(test.metrics[j].key)
                << "\": \"" << simple_json::EscapeString(test.metrics[j].value) << "\"";
            if (j + 1 < test.metrics.size()) {
                out << ",";
            }
            out << "\n";
        }
        out << "      }\n";
        out << "    }";
        if (i + 1 < result.tests.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
}

TestRunRecord MakeRecord(const std::string& id) {
    TestRunRecord record;
    record.id = id;
    record.title = id;
    for (size_t i = 0; i < sizeof(kTestItems) / sizeof(kTestItems[0]); ++i) {
        if (id == kTestItems[i].id) {
            record.title = kTestItems[i].title;
            break;
        }
    }
    record.success = false;
    record.ms = 0;
    return record;
}

void SetRunFailure(ApsRunResult& result, const std::string& message) {
    result.success = false;
    result.error = message;
    std::cout << message << "\n";
}

bool RunSelectedTests(ApsDemoConfig& config, ApsRunResult* runResult) {
    ApsRunResult localResult;
    localResult.success = true;
    localResult.importMs = 0;
    ApsRunResult& result = runResult == 0 ? localResult : *runResult;
    result.success = true;
    result.error.clear();
    result.importMs = 0;
    result.tests.clear();

    if (config.selectedItems.empty()) {
        SetRunFailure(result, "No test item selected.");
        return false;
    }
    if (!FileExists(config.rawPath)) {
        SetRunFailure(result, "Raw file does not exist: " + config.rawPath);
        return false;
    }

    std::unique_ptr<CAlpAPSMPAlgoInterface> aps(
        CreateAPSAlgoInterface(config.sensor, config.rawType, "", config.pixelFormat, 0));
    if (!aps) {
        SetRunFailure(result, "CreateAPSAlgoInterface failed.");
        return false;
    }

    aps->SetLogEnable(config.logEnable);
    aps->SetMultiThreadEnable(config.multiThread);
    if (config.rawSizeOverride) {
        aps->SetRawDataSize(config.rawRows, config.rawCols);
    }
    if (config.activeAreaEnabled) {
        aps->SetActiveArea(config.activeArea);
    }
    ApplyThresholds(aps.get(), config);

    uint32_t row = 0;
    uint32_t col = 0;
    aps->GetRawDataSize(row, col);
    const uint64_t oneFrameBytes = FrameBytes(row, col, config.rawType);
    const uint64_t expectedBytes = oneFrameBytes * config.frames;
    std::vector<uint8_t> raw = ReadBinaryFile(config.rawPath);
    if (raw.size() != expectedBytes) {
        std::ostringstream message;
        message << "Raw size mismatch. raw=" << raw.size()
                << " bytes, expected=" << expectedBytes
                << " bytes (" << row << "x" << col
                << ", frames=" << config.frames
                << ", raw_type=" << RawTypeToString(config.rawType) << ")";
        SetRunFailure(result, message.str());
        return false;
    }

    ROIArea* roi = nullptr;
    ROIArea roiValue = config.roi;
    if (config.roiEnabled) {
        if (!ValidateRoi(roiValue, row / 2, col / 2)) {
            std::ostringstream message;
            message << "ROI is invalid for sub-frame size "
                    << row / 2 << "x" << col / 2 << ".";
            SetRunFailure(result, message.str());
            return false;
        }
        roi = &roiValue;
    }

    std::cout << "\nImport raw...\n";
    const long long importMs = MeasureMs([&]() {
        if (!aps->ImportRawData(&raw[0], raw.size(), config.frameStart,
                                config.frames, config.headerFooter)) {
            throw std::runtime_error("ImportRawData failed.");
        }
    });
    result.importMs = importMs;
    std::cout << "ImportRawData: " << importMs << " ms\n";

    std::cout << "\nRun selected items:\n";
    for (size_t i = 0; i < config.selectedItems.size(); ++i) {
        const std::string& item = config.selectedItems[i];
        TestRunRecord record = MakeRecord(item);
        try {
            if (item == "snoise") {
                APSSNoiseType result;
                record.ms = MeasureMs([&]() {
                    if (!aps->SNoise(config.frameStart, config.frames, roi, result)) {
                        throw std::runtime_error("SNoise failed.");
                    }
                });
                record.success = true;
                FillSNoiseMetrics(record, result);
                std::cout << "[SNoise] " << record.ms << " ms\n";
                PrintSNoise(result);
            } else if (item == "tnoise") {
                if (config.frames < 2) {
                    record.message = "frames must be >= 2.";
                    std::cout << "[TNoise] skipped: " << record.message << "\n";
                    result.tests.push_back(record);
                    continue;
                }
                APSTNoiseType result;
                record.ms = MeasureMs([&]() {
                    if (!aps->TNoise(config.frameStart, config.frames, roi, result)) {
                        throw std::runtime_error("TNoise failed.");
                    }
                });
                record.success = true;
                FillTNoiseMetrics(record, result);
                std::cout << "[TNoise] " << record.ms << " ms\n";
                PrintTNoise(result);
            } else if (item == "badpixel") {
                APSBadpixelType result;
                record.ms = MeasureMs([&]() {
                    if (!aps->BadPixel(config.frameStart, config.frames, roi, result)) {
                        throw std::runtime_error("BadPixel failed.");
                    }
                });
                record.success = true;
                FillBadPixelMetrics(record, result);
                std::cout << "[BadPixel] " << record.ms << " ms\n";
                PrintBadPixelLike("BadPixel", result);
            } else if (item == "hotpixel") {
                APSBadpixelType result;
                record.ms = MeasureMs([&]() {
                    if (!aps->HotPixel(config.frameStart, config.frames, roi, result)) {
                        throw std::runtime_error("HotPixel failed.");
                    }
                });
                record.success = true;
                FillBadPixelMetrics(record, result);
                std::cout << "[HotPixel] " << record.ms << " ms\n";
                PrintBadPixelLike("HotPixel", result);
            } else if (item == "datamean") {
                APSDataMeanType result;
                record.ms = MeasureMs([&]() {
                    if (!aps->DataMean(config.frameStart, config.frames, roi, result)) {
                        throw std::runtime_error("DataMean failed.");
                    }
                });
                record.success = true;
                FillDataMeanMetrics(record, result);
                std::cout << "[DataMean] " << record.ms << " ms\n";
                PrintDataMean(result);
            } else if (item == "dsnu") {
                APSDSNUType result;
                record.ms = MeasureMs([&]() {
                    if (!aps->DSNU(config.frameStart, config.frames, roi, result)) {
                        throw std::runtime_error("DSNU failed.");
                    }
                });
                record.success = true;
                FillDSNUMetrics(record, result);
                std::cout << "[DSNU] " << record.ms << " ms\n";
                PrintDSNU(result);
            }
        } catch (const std::exception& e) {
            record.success = false;
            record.message = e.what();
            result.success = false;
            if (result.error.empty()) {
                result.error = record.message;
            }
            std::cout << "[" << item << "] " << e.what() << "\n";
        }
        result.tests.push_back(record);
        std::cout << "\n";
    }
    return result.success;
}

void PrintStatus(const ApsDemoConfig& config) {
    std::cout << "\n=== APS Raw Test Demo ===\n";
    std::cout << "Config: " << kConfigPath << "\n";
    std::cout << "Raw: " << config.rawPath << "\n";
    std::cout << "Sensor: ALP_" << SensorToString(config.sensor)
              << ", RawType: " << RawTypeToString(config.rawType)
              << ", PixelFormat: " << PixelFormatToString(config.pixelFormat) << "\n";
    std::cout << "Frames: " << config.frames
              << ", Thread: " << (config.multiThread ? "multi" : "single")
              << ", Log: " << (config.logEnable ? "on" : "off") << "\n";
    std::cout << "ROI: ";
    if (config.roiEnabled) {
        std::cout << "enabled [up=" << config.roi.Up
                  << ", down=" << config.roi.Down
                  << ", left=" << config.roi.Left
                  << ", right=" << config.roi.Right << "]\n";
    } else {
        std::cout << "disabled\n";
    }
    std::cout << "Threshold: badpixel=" << config.badPixelThreshold
              << ", radius=" << config.badPixelRadius
              << ", hotpixel=" << config.hotPixelThreshold << "\n";
    std::cout << "Items: " << JoinItems(config.selectedItems) << "\n";
}

void PrintMainMenu() {
    std::cout << "\nMenu:\n";
    std::cout << "  1  Set raw path\n";
    std::cout << "  2  Set frames\n";
    std::cout << "  3  Toggle thread mode\n";
    std::cout << "  4  Select test items\n";
    std::cout << "  5  Configure ROI\n";
    std::cout << "  6  Configure thresholds\n";
    std::cout << "  7  Run selected tests\n";
    std::cout << "  8  Save config\n";
    std::cout << "  9  Restore default config\n";
    std::cout << "  0  Exit\n";
    std::cout << "Input: ";
}

std::string ReadLine() {
    std::string line;
    if (!std::getline(std::cin, line)) {
        gInputEof = true;
        return "";
    }
    line.erase(std::remove(line.begin(), line.end(), '\0'), line.end());
    line.erase(std::remove(line.begin(), line.end(), static_cast<char>(0xFF)), line.end());
    line.erase(std::remove(line.begin(), line.end(), static_cast<char>(0xFE)), line.end());
    return Trim(line);
}

void PromptRawPath(ApsDemoConfig& config) {
    std::cout << "Current raw: " << config.rawPath << "\n";
    std::cout << "New raw path (empty to keep): ";
    const std::string text = StripQuotes(ReadLine());
    if (gInputEof) {
        return;
    }
    if (!text.empty()) {
        config.rawPath = text;
    }
}

void PromptFrames(ApsDemoConfig& config) {
    std::cout << "Current frames: " << config.frames << "\n";
    std::cout << "New frames: ";
    const std::string text = ReadLine();
    if (gInputEof) {
        return;
    }
    if (!text.empty()) {
        config.frames = ParseUInt(text, config.frames);
    }
}

void SelectItems(ApsDemoConfig& config) {
    while (true) {
        std::cout << "\nSelect test items:\n";
        for (size_t i = 0; i < sizeof(kTestItems) / sizeof(kTestItems[0]); ++i) {
            const bool selected = HasItem(config, kTestItems[i].id);
            std::cout << "  " << (i + 1) << "  ["
                      << (selected ? "x" : " ") << "] "
                      << kTestItems[i].title << "\n";
        }
        std::cout << "Commands: number/list toggles, a=all, n=none, b=back\n";
        std::cout << "Input: ";
        const std::string input = ToLower(ReadLine());
        if (gInputEof) {
            config.selectedItems = NormalizeItems(config.selectedItems);
            return;
        }
        if (input == "b" || input == "back" || input.empty()) {
            config.selectedItems = NormalizeItems(config.selectedItems);
            return;
        }
        if (input == "a" || input == "all") {
            config.selectedItems.clear();
            for (size_t i = 0; i < sizeof(kTestItems) / sizeof(kTestItems[0]); ++i) {
                config.selectedItems.push_back(kTestItems[i].id);
            }
            continue;
        }
        if (input == "n" || input == "none") {
            config.selectedItems.clear();
            continue;
        }
        const std::vector<std::string> parts = SplitList(input);
        for (size_t i = 0; i < parts.size(); ++i) {
            const uint32_t index = ParseUInt(parts[i], 0);
            if (index >= 1 && index <= sizeof(kTestItems) / sizeof(kTestItems[0])) {
                ToggleItem(config, kTestItems[index - 1].id);
            } else {
                const std::string id = NormalizeItemId(parts[i]);
                if (IsKnownItem(id)) {
                    ToggleItem(config, id);
                }
            }
        }
    }
}

void ConfigureRoi(ApsDemoConfig& config) {
    std::cout << "ROI enabled now: " << (config.roiEnabled ? "true" : "false") << "\n";
    std::cout << "Enable ROI? (y/n, empty to toggle): ";
    const std::string enabled = ToLower(ReadLine());
    if (gInputEof) {
        return;
    }
    if (enabled.empty()) {
        config.roiEnabled = !config.roiEnabled;
    } else if (enabled == "y" || enabled == "yes") {
        config.roiEnabled = true;
    } else if (enabled == "n" || enabled == "no") {
        config.roiEnabled = false;
    }
    if (!config.roiEnabled) {
        return;
    }
    std::cout << "Current ROI: "
              << config.roi.Up << "," << config.roi.Down << ","
              << config.roi.Left << "," << config.roi.Right << "\n";
    std::cout << "Input up,down,left,right (empty to keep): ";
    const std::vector<std::string> values = SplitList(ReadLine());
    if (gInputEof) {
        return;
    }
    if (values.size() == 4) {
        config.roi.Up = ParseUInt(values[0], config.roi.Up);
        config.roi.Down = ParseUInt(values[1], config.roi.Down);
        config.roi.Left = ParseUInt(values[2], config.roi.Left);
        config.roi.Right = ParseUInt(values[3], config.roi.Right);
    }
}

void ConfigureThresholds(ApsDemoConfig& config) {
    std::cout << "BadPixel threshold now: " << config.badPixelThreshold << "\n";
    std::cout << "New BadPixel threshold (empty to keep): ";
    std::string text = ReadLine();
    if (gInputEof) {
        return;
    }
    if (!text.empty()) {
        config.badPixelThreshold = ParseDouble(text, config.badPixelThreshold);
    }

    std::cout << "BadPixel radius now: " << config.badPixelRadius << "\n";
    std::cout << "New BadPixel radius (empty to keep): ";
    text = ReadLine();
    if (gInputEof) {
        return;
    }
    if (!text.empty()) {
        config.badPixelRadius = ParseUInt(text, config.badPixelRadius);
    }

    std::cout << "HotPixel threshold now: " << config.hotPixelThreshold << "\n";
    std::cout << "New HotPixel threshold (empty to keep): ";
    text = ReadLine();
    if (gInputEof) {
        return;
    }
    if (!text.empty()) {
        config.hotPixelThreshold = ParseDouble(text, config.hotPixelThreshold);
    }
}

} // namespace

int RunApsRawTestProfile(const std::string& profilePath) {
    std::cout << std::fixed << std::setprecision(6) << std::unitbuf;

    ApsDemoConfig config = DefaultConfig();
    ApsRunResult result;
    result.success = false;
    result.importMs = 0;

    try {
        config = LoadProfileConfig(profilePath);
        std::cout << "Profile: " << profilePath << "\n";
        const bool ok = RunSelectedTests(config, &result);

        WriteResultJson(config, result);
        std::cout << "Result JSON: " << OutputPath(config, config.resultJsonPath) << "\n";
        return ok ? 0 : 2;
    } catch (const std::exception& e) {
        result.success = false;
        result.error = e.what();
        std::cout << "Profile run failed: " << e.what() << "\n";
        try {
            WriteResultJson(config, result);
        } catch (...) {
        }
        return 1;
    }
}

int RunApsRawTestDemo() {
    ApsDemoConfig config = LoadConfig();
    std::cout << std::fixed << std::setprecision(6);

    while (true) {
        PrintStatus(config);
        PrintMainMenu();
        const std::string input = ToLower(ReadLine());
        if (gInputEof) {
            SaveConfig(config);
            return 0;
        }

        if (input == "0" || input == "q" || input == "quit" || input == "exit") {
            SaveConfig(config);
            std::cout << "Config saved. Bye.\n";
            return 0;
        } else if (input == "1") {
            PromptRawPath(config);
            SaveConfig(config);
        } else if (input == "2") {
            PromptFrames(config);
            SaveConfig(config);
        } else if (input == "3") {
            config.multiThread = !config.multiThread;
            SaveConfig(config);
        } else if (input == "4") {
            SelectItems(config);
            SaveConfig(config);
        } else if (input == "5") {
            ConfigureRoi(config);
            SaveConfig(config);
        } else if (input == "6") {
            ConfigureThresholds(config);
            SaveConfig(config);
        } else if (input == "7" || input == "r" || input == "run") {
            try {
                RunSelectedTests(config, 0);
            } catch (const std::exception& e) {
                std::cout << "Run failed: " << e.what() << "\n";
            }
            SaveConfig(config);
        } else if (input == "8" || input == "s" || input == "save") {
            SaveConfig(config);
            std::cout << "Config saved: " << kConfigPath << "\n";
        } else if (input == "9") {
            config = DefaultConfig();
            SaveConfig(config);
            std::cout << "Default config restored.\n";
        } else if (input.empty()) {
            continue;
        } else {
            std::cout << "Unknown command.\n";
        }
    }
}
