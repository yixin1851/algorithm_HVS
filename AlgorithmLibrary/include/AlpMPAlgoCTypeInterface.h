#pragma once

#ifdef API_EXPORTS
#define ALP_ALGO_DLL_API_C _declspec(dllexport)
#else
#define ALP_ALGO_DLL_API_C _declspec(dllimport)
#endif

#include <stdint.h>
// #include <stdbool.h>
#include <vector>

#define MAX_DATA_NUMBER 200

typedef void *HANDLE;

// C接口需要设置该宏, 或在工程编译期间进行预编译
// #define API_C_TYPE_INTERFACE

#ifndef API_C_TYPE_INTERFACE
#else
// =======[ Public Type ]=====
typedef enum {
    Gb,
    B,
    R,
    Gr,
    All,
} SubFrameIndex;

typedef enum {
    ALP_003AA,
    ALP_003BA,
    ALP_003BB,
    ALP_003CA,
    ALP_004AB,
    ALP_014AA,
    ALP_014BA,
} SensorType;

typedef enum {
    BayerGBRG,
    BayerBGGR,
    BayerRGGB,
    BayerGRBG,
    QuadBayerGBRG,
    QuadBayerBGGR,
    QuadBayerRGGB,
    QuadBayerGRBG,
} PixelFormatType;

typedef enum {
    OffEventsOnly = 1,
    OnEventsOnly,
    On_OffEvents,
} DVSLightTrigerType;

typedef struct {
    uint32_t Up;
    uint32_t Down;
    uint32_t Left;
    uint32_t Right;
} ROIArea;

typedef enum {
    RAW8,
    RAW10,
    RAW12,
    UNPACK10,
    UNPACK12,
} APSRawType;

typedef enum {
    APS_Code_16_Subframe = 1,
    APS_Code_HVS = 2,
} APSCodeType;

typedef enum {
    DVS_Code_1_4_Bining = 1,
    DVS_Code_HVS = 2,
    DVS_Code_1_2_Subsample = 4,
    DVS_Code_1_4_Subsample = 8,
    DVS_Code_1_8_Subsample = 16,
} DVSCodeType;

typedef std::vector<std::vector<uint8_t> > ImgType;
typedef std::vector<std::vector<double> > APSType;


typedef struct {
    uint32_t x;
    uint32_t y;
} Local;

bool operator==(const Local &lh, const Local &rh);

typedef struct {
    uint32_t BadPixelNum;
    std::vector<Local> LocalData;
    std::vector<uint8_t> Flag;
    std::vector<float> DiffData;
} BadPixelMaskType;

typedef struct {
    uint32_t BadPixelNum;
    // std::vector<Local> LocalData;
    Local *LocalData;
    size_t LocalDataSize;
    // std::vector<uint8_t> Flag;
    uint8_t *Flag;
    size_t FlagSize;
    // std::vector<float> DiffData;
    uint32_t *DiffData;
    size_t DiffDataSize;
} BadPixelMaskTypeC;

typedef struct {
    uint32_t BadPixelNum;
    uint32_t SingletNum;
    uint32_t CoupletNum;
    uint32_t ClusterNum;
    uint32_t DefectRowNum;
    uint32_t DefectColNum;
    uint32_t MaxClusterSize;
    BadPixelMaskType BadPixelMask;
} APSSubFrameBadpixelType;

typedef struct {
    uint32_t BadPixelNum;
    uint32_t SingletNum;
    uint32_t CoupletNum;
    uint32_t LadderNum;
    uint32_t ClusterNum;
    uint32_t MaxClusterSize;
    BadPixelMaskType BadPixelMask;
    std::vector<APSSubFrameBadpixelType> SubFrameBadpixelData;
} APSBadpixelType;

typedef struct {
    uint32_t BadPixelNum;
    uint32_t SingletNum;
    uint32_t CoupletNum;
    uint32_t ClusterNum;
    uint32_t DefectRowNum;
    uint32_t DefectColNum;
    uint32_t MaxClusterSize;
    BadPixelMaskTypeC BadPixelMask;
} APSSubFrameBadpixelTypeC;

typedef struct {
    uint32_t BadPixelNum;
    uint32_t SingletNum;
    uint32_t CoupletNum;
    uint32_t LadderNum;
    uint32_t ClusterNum;
    uint32_t MaxClusterSize;
    BadPixelMaskTypeC BadPixelMask;
    size_t SubFrameBadpixelDataSize;
    APSSubFrameBadpixelTypeC *SubFrameBadpixelData;
} APSBadpixelTypeC;

typedef struct {
    double TempNoise;
    double RowTemp;
    double ColTemp;
    double TempRNRatio;
    double TempCNRatio;
    double PixelTemp;
} APSSubFrameTNoiseType;

typedef struct {
    double SNoise;
    double RowSNoise;
    double ColSNoise;
} APSSubFrameSNoiseType;

typedef struct {
    std::vector<std::vector<double> > YShadingData;
    double YShadingLT;
    double YShadingLB;
    double YShadingRT;
    double YShadingRB;
} APSYShadingType;

typedef struct {
    uint32_t CenterRow;
    uint32_t CenterCol;
} APSOpticalCenterType;

typedef struct {
    double PedestalMax[SubFrameIndex::All];
    double PedestalMin[SubFrameIndex::All];
} APSPedestalVariationType;
#ifdef __cplusplus
typedef struct {
    double TNoiseFrame;
    std::vector<APSSubFrameTNoiseType> SubFrameTNoiseData;
} APSTNoiseType;

typedef struct {
    double SNoiseFrame;
    std::vector<APSSubFrameSNoiseType> SubFrameSNoiseData;
} APSSNoiseType;
#endif

typedef double APSReadNoiseType;

typedef struct {
    std::vector<std::vector<double> > ColorShadingRGData;
    std::vector<std::vector<double> > ColorShadingBGData;
    double ColorShadingRGLT;
    double ColorShadingRGLB;
    double ColorShadingRGRT;
    double ColorShadingRGRB;
    double ColorShadingBGLT;
    double ColorShadingBGLB;
    double ColorShadingBGRT;
    double ColorShadingBGRB;
} APSColorShadingType;

typedef struct {
    double RangeR;
    double RangeG;
    double RangeB;
    double SignalMax;
    double DeltaSignalMax;
    double DeltaSignalCentreMax;
    double DeltaSignalEdgeMax;
    double DeltaSignalCornerMax;
    double RMax;
    double RMin;
    double GMax;
    double GMin;
    double BMax;
    double BMin;
} APSDSNUType;

typedef struct {
    double b;
    double k;
    double LeMax;
    double LeMin;
} APSSubFrameLinearityType;

typedef struct {
    std::vector<APSSubFrameLinearityType> SubFrameLinearityData;
} APSLinearityType;

typedef struct {
    std::vector<double> SubFrameGainK;
} APSOverallSystemGainType;

typedef struct {
    double SaturationMean;
    double SaturationTNoise;
    double SaturationSNR;
} APSSaturationType;

typedef struct {
    double DR_dB;
    double ReadNoise;
    double ReadNoise_e;
    double FWC;
    double FWC_e;
    double ConversionGain;
    std::vector<double> ReadNoiseData;
    std::vector<double> TNoiseData;
    std::vector<double> DataMean;
} APSOETCType;

typedef struct {
    double MaxSSNR;
    std::vector<double> SNoiseData;
    std::vector<double> DataMean;
    std::vector<double> SSNR;
} APSSSNRType;

typedef struct {
    double DataMeanFrame;
    std::vector<double> SubFrameDataMean;
} APSDataMeanType;

typedef struct {
    std::vector<double> SubFrameKValue;
} APSDarkCurrentType;

typedef struct {
    double dHotPixelThre;
    double dHotLineThre;
    double dBadPixelThre;
    double dBadLineThre;
    uint32_t nBadPixelRadius;
    uint32_t nBadLineRadius;
    uint32_t nDSNURowBlockNum;
    uint32_t nDSNUColBlockNum;
    uint32_t nDSNURowBlockSize;
    uint32_t nDSNUColBlockSize;
    uint32_t nYShadingRowBlockNum;
    uint32_t nYShadingColBlockNum;
    uint32_t nColorShadingRowBlockNum;
    uint32_t nColorShadingColBlockNum;
    uint32_t nPedestalVariationRowBlockNum;
    uint32_t nPedestalVariationColBlockNum;
    uint32_t nPedestalVariationRowBlockSize;
    uint32_t nPedestalVariationColBlockSize;
    uint32_t nBadPixelMaxLen;
    uint32_t nBadPixelLocalRowOffset;
    uint32_t nBadPixelLocalColOffset;
    uint32_t nLinearityRadius;
    uint32_t nOETCRadius;
} APSAlgorithmThre;

typedef struct {
    double dHotPixelThre;
    double dHotLineThre;
    double dDeadPixelThre;
    double dDeadLineThre;
    uint32_t nHotPixelClusterSizeThre;
    uint32_t nDeadPixelClusterSizeThre;
    uint32_t nPeakCycle;
    uint32_t nStationaryUniformityRowBlockNum;
    uint32_t nStationaryUniformityColBlockNum;
    uint32_t nSpatialResponseUniformityRowBlockNum;
    uint32_t nSpatialResponseUniformityColBlockNum;
    double dFlashRatioThre;
    uint32_t m_nHotPixelSlidingWindowWidth;
    uint32_t m_nHotPixelSlidingWindowHeight;
    uint32_t m_nBadPixelSlidingWindowWidth;
    uint32_t m_nBadPixelSlidingWindowHeight;
} DVSAlgorithmThre;

typedef struct {
    double dStationaryNoiseMeanOn;
    double dStationaryNoiseMeanOff;
    double dStationaryNoiseMeanAll;
    double dStationaryNoiseStdOn;
    double dStationaryNoiseStdOff;
    double dStationaryNoiseStdAll;
    double dStationaryRowTNoise;
    double dStationaryColTNoise;
    uint32_t nFlashFrameNumber;
    double dMaxStationaryNoise;
} DVSStationaryNoiseType;

typedef struct {
    uint32_t nDataNumber;
    std::vector<uint32_t> AllEventsNum[SubFrameIndex::All + 1];
    std::vector<uint32_t> OnEventsNum[SubFrameIndex::All + 1];
    std::vector<uint32_t> OffEventsNum[SubFrameIndex::All + 1];
    std::vector<uint32_t> NoEventsNum[SubFrameIndex::All + 1];
} DVSEventsNumberCountType;

typedef struct {
    double OnEventsRatio[SubFrameIndex::All + 1];

    double R_Gb_OnEventsRatio;
    double B_Gb_OnEventsRatio;
    double Gr_Gb_OnEventsRatio;

    double OffEventsRatio[SubFrameIndex::All + 1];

    double R_Gb_OffEventsRatio;
    double B_Gb_OffEventsRatio;
    double Gr_Gb_OffEventsRatio;
} DVSImageContrastSensitivityType;

typedef struct {
    double dAccompaniedPeakOnEventsRatio[SubFrameIndex::All + 1];
    double dDelayedPeakOnEventsRatio[SubFrameIndex::All + 1];
    double dAccompaniedPeakOffEventsRatio[SubFrameIndex::All + 1];
    double dDelayedPeakOffEventsRatio[SubFrameIndex::All + 1];
} DVSAccompaniedPeakAndDelayedPeakType;

typedef struct {
    uint32_t nOnEventsPeakNumber;
    std::vector<uint32_t> OnEventsPeakPos;

    uint32_t nOffEventsPeakNumber;
    std::vector<uint32_t> OffEventsPeakPos;
} DVSPeakInfo;

typedef struct {
    uint32_t nOnEventsPeakNumber;
    size_t OnEventsPeakPosSize;
    uint32_t *OnEventsPeakPos;

    uint32_t nOffEventsPeakNumber;
    size_t OffEventsPeakPosSize;
    uint32_t *OffEventsPeakPos;
} DVSPeakInfoTypeC;

typedef struct {
    std::vector<std::vector<double> > UniformityBlockData;
    double UniformityRatio;
} DVSStationaryUniformityType;

typedef struct {
    double dOnEventsUniformityRatio[SubFrameIndex::All + 1];
    std::vector<std::vector<double> > OnEventsUniformityBlockData[SubFrameIndex::All + 1];
    double dOffEventsUniformityRatio[SubFrameIndex::All + 1];
    std::vector<std::vector<double> > OffEventsUniformityBlockData[SubFrameIndex::All + 1];
} DVSSpatialResponseUniformityType;

typedef struct {
    uint32_t nOffEventsDeadPixelNum;
    uint32_t nOffEventsDeadLineNum;
    uint32_t nOffEventsClusterNum;
    BadPixelMaskType OffEventsBadPixelMask;
    uint32_t nOffEventsMaxClusterSize;
    uint32_t nOffEventsSlidingWindowMaxDeadPixelNum;

    uint32_t nOnEventsDeadPixelNum;
    uint32_t nOnEventsDeadLineNum;
    uint32_t nOnEventsClusterNum;
    BadPixelMaskType OnEventsBadPixelMask;
    uint32_t nOnEventsMaxClusterSize;
    uint32_t nOnEventsSlidingWindowMaxDeadPixelNum;
} DVSBadpixelType;

typedef struct {
    uint32_t nOffEventsDeadPixelNum;
    uint32_t nOffEventsDeadLineNum;
    uint32_t nOffEventsClusterNum;
    BadPixelMaskTypeC OffEventsBadPixelMask;
    uint32_t nOffEventsMaxClusterSize;
    uint32_t nOffEventsSlidingWindowMaxDeadPixelNum;

    uint32_t nOnEventsDeadPixelNum;
    uint32_t nOnEventsDeadLineNum;
    uint32_t nOnEventsClusterNum;
    BadPixelMaskTypeC OnEventsBadPixelMask;
    uint32_t nOnEventsMaxClusterSize;
    uint32_t nOnEventsSlidingWindowMaxDeadPixelNum;
} DVSBadpixelTypeC;

typedef struct {
    uint32_t HotPixelNum;
    uint32_t HotLineNum;
    uint32_t SingletNum;
    uint32_t CoupletNum;
    uint32_t TripletNum;
    uint32_t FourConnectedNum;
    uint32_t ClusterNum;
    BadPixelMaskType HotPixelMask;
    uint32_t MaxClusterSize;
    uint32_t SlidingWindowMaxHotPixelNum;
} DVSHotpixelType;

typedef struct {
    uint32_t HotPixelNum;
    uint32_t HotLineNum;
    uint32_t SingletNum;
    uint32_t CoupletNum;
    uint32_t TripletNum;
    uint32_t FourConnectedNum;
    uint32_t ClusterNum;
    BadPixelMaskTypeC HotPixelMask;
    uint32_t MaxClusterSize;
    uint32_t SlidingWindowMaxHotPixelNum;
} DVSHotpixelTypeC;

typedef enum {
    TEST_NO_ERROR = 0,
    ALGO_HANDLE_ERROR = 0x80000001,
    UNKNOWN_ERROR = 0x90000001,
    EVS_DECODE_ERROR = 0x80000002,
    DATA_INDEX_ERROR = 0x80000003,
    FIND_PEAK_NUM_SET_ERROR = 0x80000004,
    FIND_PEAK_ERROR = 0x80000005,
    PEAK_NUM_ERROR = 0x80000006,
    DATA_LENS_ERROR = 0x80000007,
    DATA_ROI_SET_ERROR = 0x80000008,
    SAVE_DATA_ERROR = 0x80000009,
    BEYOND_MAX_RES_NUM = 0x8000000A,
    EVENTS_EQU_ZERO = 0x8000000B,
    INVALID_PARAMETER_ERROR = 0x8000000C,
    MEMORY_ALLOCATION_ERROR = 0x8000000D,
    FUNCTION_ERROR = 0x8000000E,
    EVS_SUB_SAMPLE_ERROR = 0x8000000F,
} ErrCode;

// =================================


typedef struct {
    double dStationaryNoiseMeanOn;
    double dStationaryNoiseMeanOff;
    double dStationaryNoiseMeanAll;
    double dStationaryNoiseStdOn;
    double dStationaryNoiseStdOff;
    double dStationaryNoiseStdAll;
    double dStationaryRowSNoise;
    double dStationaryColSNoise;
} CStationaryNoiseData;

typedef struct {
    uint32_t nDataNumber;
    uint32_t AllEventsNum[SubFrameIndex::All + 1][MAX_DATA_NUMBER];
    uint32_t OnEventsNum[SubFrameIndex::All + 1][MAX_DATA_NUMBER];
    uint32_t OffEventsNum[SubFrameIndex::All + 1][MAX_DATA_NUMBER];
    uint32_t NoEventsNum[SubFrameIndex::All + 1][MAX_DATA_NUMBER];
} CEventsNumberCountData;

typedef struct {
    double UniformityRatio;
} CStationaryUniformityData;

typedef struct {
    uint32_t HotPixelNum;
    uint32_t HotLineNum;
    uint32_t SingletNum;
    uint32_t CoupletNum;
    uint32_t TripletNum;
    uint32_t FourConnectedNum;
    uint32_t ClusterNum;
} CHotpixelData;

typedef struct {
    uint32_t nOnEventsPeakNumber;
    uint32_t OnEventsPeakPos[MAX_DATA_NUMBER];

    uint32_t nOffEventsPeakNumber;
    uint32_t OffEventsPeakPos[MAX_DATA_NUMBER];
} CPeakInfo;

typedef struct {
    double OnEventsRatio[SubFrameIndex::All + 1];

    double R_Gb_OnEventsRatio;
    double B_Gb_OnEventsRatio;
    double Gr_Gb_OnEventsRatio;

    double OffEventsRatio[SubFrameIndex::All + 1];

    double R_Gb_OffEventsRatio;
    double B_Gb_OffEventsRatio;
    double Gr_Gb_OffEventsRatio;
} CImageContrastSensitivityData;

typedef struct {
    double dAccompaniedPeakOnEventsRatio[SubFrameIndex::All + 1];
    double dDelayedPeakOnEventsRatio[SubFrameIndex::All + 1];
    double dAccompaniedPeakOffEventsRatio[SubFrameIndex::All + 1];
    double dDelayedPeakOffEventsRatio[SubFrameIndex::All + 1];
} CAccompaniedPeakAndDelayedPeakData;

typedef struct {
    double dOnEventsUniformityRatio[SubFrameIndex::All + 1];
    double dOffEventsUniformityRatio[SubFrameIndex::All + 1];
} CSpatialResponseUniformityData;

typedef struct {
    uint32_t nOffEventsDeadPixelNum;
    uint32_t nOffEventsClusterNum;

    uint32_t nOnEventsDeadPixelNum;
    uint32_t nOnEventsClusterNum;
} CDVSBadpixelData;

typedef struct {
    double dHotPixelThre;
    double dHotLineThre;
    double dDeadPixelThre;
    uint32_t nPeakCycle;
    uint32_t nStationaryUniformityRowBlockNum;
    uint32_t nStationaryUniformityColBlockNum;
    uint32_t nSpatialResponseUniformityRowBlockNum;
    uint32_t nSpatialResponseUniformityColBlockNum;
} CDVSAlgorithmThre;

// C接口的结构体定义
typedef struct {
    double TNoiseFrame;
    APSSubFrameTNoiseType *SubFrameTNoiseData;
    size_t SubFrameTNoiseDataCount;
} APSTNoiseTypeC;

#ifdef __cplusplus
extern "C" {
#endif
// | ============ DVS ================================================================================================ |
ALP_ALGO_DLL_API_C HANDLE __stdcall InitHandleDVS(SensorType Sensortype, char *strLogDir, PixelFormatType Pixelformat,
                                                  int code);

ALP_ALGO_DLL_API_C void __stdcall DeleteHandleDVS(HANDLE h);

ALP_ALGO_DLL_API_C uint32_t __stdcall ImportRawDataDVS(HANDLE h, uint8_t *pRawData, uint64_t nLens,
                                                       uint32_t nIndexStart, uint32_t nNumber);

ALP_ALGO_DLL_API_C uint32_t __stdcall EventsNumberCountDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                                           DVSEventsNumberCountType *EventsNumberCountRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall StationaryNoiseDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                                         DVSStationaryNoiseType *StationaryNoiseRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall StationaryUniformityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                                              DVSStationaryUniformityType *UniformityRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall FindPeakDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum,
                                                  DVSPeakInfo *Peak, DVSLightTrigerType Light);

ALP_ALGO_DLL_API_C uint32_t __stdcall ImageContrastSensitivityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                                                  DVSPeakInfo *Peak,
                                                                  uint32_t nPeakNum, DVSLightTrigerType Light,
                                                                  DVSImageContrastSensitivityType *
                                                                  ImageContrastSensitivityRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall AccompaniedPeakAndDelayedPeakDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                                                       DVSPeakInfo *Peak, uint32_t nPeakNum,
                                                                       DVSLightTrigerType Light,
                                                                       DVSAccompaniedPeakAndDelayedPeakType *
                                                                       AccompaniedPeakAndDelayedPeakRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall SpatialResponseUniformityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                                                   ROIArea *ROI, DVSPeakInfo *Peak, uint32_t nPeakNum,
                                                                   DVSLightTrigerType Light,
                                                                   DVSSpatialResponseUniformityType *
                                                                   SpatialResponseUniformityRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall BadPixelTypeCDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                                       DVSPeakInfo *Peak,
                                                       uint32_t nPeakNum, DVSLightTrigerType Light,
                                                       DVSBadpixelTypeC *BadpixelRes);

ALP_ALGO_DLL_API_C void __stdcall BadPixelTypeCDVS_Free(DVSBadpixelTypeC *BadpixelRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall HotPixelTypeCDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                                       DVSHotpixelTypeC *HotpixelRes);

ALP_ALGO_DLL_API_C void __stdcall HotPixelTypeCDVS_Free(DVSHotpixelTypeC *HotpixelRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall ShowDVS(HANDLE h, uint32_t nIndex, uint8_t NoEventFlag, uint8_t OnEventFlag,
                                              uint8_t OffEventFlag, ImgType *ImgData);

ALP_ALGO_DLL_API_C uint32_t __stdcall SetMultiThreadEnableDVS(HANDLE h, bool bEnable);

ALP_ALGO_DLL_API_C uint32_t __stdcall SetLogEnableDVS(HANDLE h, bool bEnable);

ALP_ALGO_DLL_API_C uint32_t __stdcall SetAlgorithmThreDVS(HANDLE h, DVSAlgorithmThre *AlgoThre);

ALP_ALGO_DLL_API_C uint32_t __stdcall GetAlgorithmThreDVS(HANDLE h, DVSAlgorithmThre *AlgoThre);

ALP_ALGO_DLL_API_C uint32_t __stdcall GetDataNumDVS(HANDLE h, uint32_t *nDataNum);

ALP_ALGO_DLL_API_C uint32_t __stdcall GetActiveAreaDVS(ROIArea *ROI);

ALP_ALGO_DLL_API_C uint32_t __stdcall SetActiveAreaDVS(ROIArea ROI);

ALP_ALGO_DLL_API_C uint32_t __stdcall GetRawDataSizeDVS(HANDLE h, uint32_t *nRow, uint32_t *nCol);

ALP_ALGO_DLL_API_C uint32_t __stdcall AlpGetVersionDVS(HANDLE h, char *ver, uint32_t nLen);

/**
* @brief Input Target_Evevt([%]), output the corresponding light intensity transition points.
* @param h, handle.
* @param eventRatioPercent, input Target_EventRatio[%]. eg: eventRatioPercent = 50, representing 50% Events Ratio.
* @param vecEvent, On/Off Events vector.
* @param vecLightIntensity, On/Off Light Intensity jump points vector.
* @param targetLightIntensity output value, if return value < 0.
* @return true, calculate done.
* @return false, calculate error.
*/
ALP_ALGO_DLL_API_C uint32_t __stdcall CalcLightIntensityDVS(HANDLE h, double eventRatioPercent,
                                                            std::vector<double> vecEvent,
                                                            std::vector<std::pair<double, double> > vecLightIntensity,
                                                            double &targetLightIntensity);

// | ================================================================================================================= |

// | ============ APS ================================================================================================ |
ALP_ALGO_DLL_API_C HANDLE __stdcall InitHandleAPS(SensorType Sensortype, APSRawType APSRawtype, char *strLogDir,
                                                  PixelFormatType Pixelformat, int code);

ALP_ALGO_DLL_API_C void __stdcall DeleteHandleAPS(HANDLE h);

ALP_ALGO_DLL_API_C uint32_t __stdcall ImportRawDataAPS(HANDLE h, uint8_t *pRawData, uint64_t nLens,
                                                       uint32_t nIndexStart, uint32_t nNumber);

/**
 * @brief CType接口, 获取TNoise数据
 * @note 调用成功后，必须调用 TNoiseAPS_Free() 释放内存
 * @warning 忘记调用 TNoiseAPS_Free() 会导致内存泄漏
 * @example
 *  void example() {
 *      HANDLE h = ...; // handle
 *      APSTNoiseTypeC result = {0};
 *      uint32_t ret = TNoiseAPS(h, 0, 10, &result);
 *      if (ret == TEST_NO_ERROR) {
 *          printf("TNoiseFrame: %f\n", result.TNoiseFrame);
 *          printf("SubFrame count: %zu\n", (size_t)result.SubFrameTNoiseDataCount);
 *          // process data
 *          for (size_t i = 0; i < result.SubFrameTNoiseDataCount; i++) {
 *              // process result.SubFrameTNoiseData[i]
 *          }
 *          // release APSTNoiseTypeC
 *          TNoiseAPS_Free(&result);
 *      } else {
 *          printf("Error code: %u\n", ret);
 *      }
 *  }
 */
ALP_ALGO_DLL_API_C uint32_t __stdcall TNoiseAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                APSTNoiseTypeC *apsTNoiseRes);

ALP_ALGO_DLL_API_C void __stdcall TNoiseAPS_Free(APSTNoiseTypeC *apsTNoiseRes);

// std::vector
ALP_ALGO_DLL_API_C uint32_t __stdcall SNoiseAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                APSSNoiseType *APSSNoiseRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall BadPixelTypeCAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                       APSBadpixelTypeC *BadpixelRes);

ALP_ALGO_DLL_API_C void __stdcall BadPixelTypeCAPS_Free(APSBadpixelTypeC *BadpixelRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall HotPixelTypeCAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                       APSBadpixelTypeC *HotpixelRes);

ALP_ALGO_DLL_API_C void __stdcall HotPixelTypeCAPS_Free(APSBadpixelTypeC *HotpixelRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall BLCAPS_1(HANDLE h, uint32_t nIndexStart, uint32_t nNumber);

// std::vector APSDataMeanType
ALP_ALGO_DLL_API_C uint32_t __stdcall BLCAPS_2(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                               APSDataMeanType *APSDataMeanRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall BLCAPS_3(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                               uint32_t nBaseIndexStart, uint32_t nBaseNumber);

ALP_ALGO_DLL_API_C uint32_t __stdcall YShadingAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                  APSYShadingType *YShadingRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall ColorShadingAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                      APSColorShadingType *ColorShadingRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall OpticalCenterAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                       APSOpticalCenterType *OpticalCenterRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall PedestalVariationAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                                           ROIArea *ROI,
                                                           APSPedestalVariationType *PedestalVariationRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall ReadNoiseAPS(HANDLE h, uint32_t nIndex1, uint32_t nIndex2, ROIArea *ROI,
                                                   APSReadNoiseType *ReadNoiseRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall DarkCurrentDataMeanAPS(HANDLE h, const APSDataMeanType *dataMean,
                                                             size_t dataMeanCount, const double *expTime,
                                                             size_t expTimeCount, APSDarkCurrentType *darkCurrentRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall DarkCurrentTNoiseAPS(HANDLE h, const APSTNoiseType *pTNoiseData,
                                                           size_t iTNoiseDataCount, const double *expTime,
                                                           size_t expTimeCount, APSDarkCurrentType *darkCurrentRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall DSNUAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                              APSDSNUType &DSNURes);

ALP_ALGO_DLL_API_C uint32_t __stdcall DataMeanAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                  APSDataMeanType *DataMean);

// C std::vector不兼容
ALP_ALGO_DLL_API_C uint32_t __stdcall LinearityAPS(HANDLE h, std::vector<APSDataMeanType> *LightMean,
                                                   std::vector<double> *ExpTime, APSLinearityType *LinearityRes);

// C std::vector不兼容
ALP_ALGO_DLL_API_C uint32_t __stdcall OverallSystemGainAPS(HANDLE h, std::vector<APSTNoiseType> *LightTNoiseData,
                                                           std::vector<APSDataMeanType> *LightMean,
                                                           APSTNoiseType DarkTNoiseBase,
                                                           APSOverallSystemGainType *GainRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall SaturationAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                    SubFrameIndex nChannelIndex, APSSaturationType *SaturationRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall OETCAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                              uint32_t nNumberInOneStep, ROIArea *ROI,
                                              SubFrameIndex nChannelIndex, APSOETCType *OETCRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall LinearitySNRAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                      SubFrameIndex nChannelIndex, APSSSNRType *SSNRRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall ShowAPS_1(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                SubFrameIndex nChannelIndex, bool bNormalize, ImgType *ImgData);

ALP_ALGO_DLL_API_C uint32_t __stdcall ShowAPS_2(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                SubFrameIndex nChannelIndex, APSType *ImgData);

ALP_ALGO_DLL_API_C uint32_t __stdcall ShowAPS_3(HANDLE h, uint32_t nIndex, uint16_t *RawData);

ALP_ALGO_DLL_API_C uint32_t __stdcall SetMultiThreadEnableAPS(HANDLE h, bool bEnable = true);

ALP_ALGO_DLL_API_C uint32_t __stdcall SetLogEnableAPS(HANDLE h, bool bEnable = true);

ALP_ALGO_DLL_API_C uint32_t __stdcall SetAlgorithmThreAPS(HANDLE h, APSAlgorithmThre *AlgoThre);

ALP_ALGO_DLL_API_C uint32_t __stdcall GetAlgorithmThreAPS(HANDLE h, APSAlgorithmThre *AlgoThre);

ALP_ALGO_DLL_API_C uint32_t __stdcall GetDataNumAPS(HANDLE h, uint32_t *DataNum);

ALP_ALGO_DLL_API_C uint32_t __stdcall SaveBinAPS(HANDLE h, uint8_t *pRawData, uint64_t nLens, const char *strSavePath);

ALP_ALGO_DLL_API_C uint32_t __stdcall GetActiveAreaAPS(HANDLE h, ROIArea *ROIAreaRes);

ALP_ALGO_DLL_API_C uint32_t __stdcall GetRawDataSizeAPS(HANDLE h, uint32_t *nRow, uint32_t *nCol);

ALP_ALGO_DLL_API_C uint32_t __stdcall SetActiveAreaAPS(HANDLE h, ROIArea ActiveArea);

ALP_ALGO_DLL_API_C uint32_t __stdcall SetRawDataSizeAPS(HANDLE h, uint32_t nRow, uint32_t nCol);

ALP_ALGO_DLL_API_C uint32_t __stdcall AlpGetVersionAPS(HANDLE h, char *ver, uint32_t nLen);

ALP_ALGO_DLL_API_C uint32_t __stdcall GetCodeAPS(HANDLE h, int *Code);
#ifdef __cplusplus
}
#endif
#endif
