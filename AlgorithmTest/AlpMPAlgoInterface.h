#pragma once

#ifdef API_EXPORTS
#define ALP_ALGO_DLL_API _declspec(dllexport)
#else
#define ALP_ALGO_DLL_API _declspec(dllimport)
#endif

#include <cstdint>
#include <vector>
#include <string>

#define APS_HOT_PIXEL_SINGLET_FLAG 0x01
#define APS_HOT_PIXEL_COUPLET_FLAG 0x02
#define APS_HOT_PIXEL_CLUSTER_FLAG 0x04
#define APS_HOT_PIXEL_LADDER_FLAG 0x08

#define APS_BAD_PIXEL_SINGLET_FLAG 0x11
#define APS_BAD_PIXEL_COUPLET_FLAG 0x12
#define APS_BAD_PIXEL_CLUSTER_FLAG 0x14
#define APS_BAD_PIXEL_LADDER_FLAG 0x18

#define APX003CA_ON_CHIP_CALIBRATION_FLAG 100

#define DVS_DEAD_PIXEL_FLAG 0x01
#define DVS_HOT_PIXEL_FLAG 0x04

#ifdef API_C_TYPE_INTERFACE
#include "AlpMPAlgoCTypeInterface.h"
#else
// =======[ Public Type ]=====
typedef enum
{
	Gb,
	B,
	R,
	Gr,
	All,
}SubFrameIndex;

typedef enum
{
	ALP_003AA,
	ALP_003BA,
	ALP_003BB,
	ALP_003CA,
	ALP_004AB,
	ALP_014AA,
	ALP_014BA,
}SensorType;

typedef enum
{
	BayerGBRG,
	BayerBGGR,
	BayerRGGB,
	BayerGRBG,
	QuadBayerGBRG,
	QuadBayerBGGR,
	QuadBayerRGGB,
	QuadBayerGRBG,
}PixelFormatType;

typedef enum
{
	OffEventsOnly = 1,
	OnEventsOnly,
	On_OffEvents,
}DVSLightTrigerType;

typedef struct
{
	uint32_t Up;
	uint32_t Down;
	uint32_t Left;
	uint32_t Right;
}ROIArea;

typedef enum
{
	RAW8,
	RAW10,
	RAW12,
	UNPACK10,
	UNPACK12,
}APSRawType;

typedef enum
{
	APS_Code_16_Subframe = 1,
	APS_Code_HVS = 2,
}APSCodeType;

typedef enum
{
	DVS_Code_1_4_Bining = 1,
	DVS_Code_HVS = 2,
	DVS_Code_1_2_Subsample = 4,
	DVS_Code_1_4_Subsample = 8,
	DVS_Code_1_8_Subsample = 16,
}DVSCodeType;

typedef std::vector<std::vector<uint8_t>> ImgType;
typedef std::vector<std::vector<double>> APSType;



typedef struct
{
	uint32_t x;
	uint32_t y;
}Local;

bool operator==(const Local& lh, const Local& rh);

typedef struct
{
	uint32_t BadPixelNum;
	std::vector<Local> LocalData;
	std::vector<uint8_t> Flag;
	std::vector<float> DiffData;
}BadPixelMaskType;

typedef struct
{
	uint32_t BadPixelNum;
	uint32_t SingletNum;
	uint32_t CoupletNum;
	uint32_t ClusterNum;
	uint32_t DefectRowNum;
	uint32_t DefectColNum;
	uint32_t MaxClusterSize;
	BadPixelMaskType BadPixelMask;
}APSSubFrameBadpixelType;

typedef struct
{
	uint32_t BadPixelNum;
	uint32_t SingletNum;
	uint32_t CoupletNum;
	uint32_t LadderNum;
	uint32_t ClusterNum;
	uint32_t MaxClusterSize;
	BadPixelMaskType BadPixelMask;
	std::vector<APSSubFrameBadpixelType> SubFrameBadpixelData;
}APSBadpixelType;

typedef struct
{
	double TempNoise;
	double RowTemp;
	double ColTemp;
	double TempRNRatio;
	double TempCNRatio;
	double PixelTemp;
}APSSubFrameTNoiseType;

typedef struct
{
	double TNoiseFrame;
	std::vector<APSSubFrameTNoiseType> SubFrameTNoiseData;
}APSTNoiseType;

typedef struct
{
	double SNoise;
	double RowSNoise;
	double ColSNoise;
}APSSubFrameSNoiseType;

typedef struct
{
	double SNoiseFrame;
	std::vector<APSSubFrameSNoiseType> SubFrameSNoiseData;
}APSSNoiseType;

typedef struct
{
	std::vector<std::vector<double>> YShadingData;
	double YShadingLT;
	double YShadingLB;
	double YShadingRT;
	double YShadingRB;
}APSYShadingType;

typedef struct
{
	uint32_t CenterRow;
	uint32_t CenterCol;
}APSOpticalCenterType;

typedef struct
{
	double PedestalMax[SubFrameIndex::All];
	double PedestalMin[SubFrameIndex::All];
}APSPedestalVariationType;

typedef double APSReadNoiseType;

typedef struct
{
	std::vector<std::vector<double>> ColorShadingRGData;
	std::vector<std::vector<double>> ColorShadingBGData;
	double ColorShadingRGLT;
	double ColorShadingRGLB;
	double ColorShadingRGRT;
	double ColorShadingRGRB;
	double ColorShadingBGLT;
	double ColorShadingBGLB;
	double ColorShadingBGRT;
	double ColorShadingBGRB;
}APSColorShadingType;

typedef struct
{
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
}APSDSNUType;

typedef struct
{
	double b;
	double k;
	double LeMax;
	double LeMin;
}APSSubFrameLinearityType;

typedef struct
{
	std::vector<APSSubFrameLinearityType> SubFrameLinearityData;
}APSLinearityType;

typedef struct
{
	std::vector<double> SubFrameGainK;
}APSOverallSystemGainType;

typedef struct
{
	double SaturationMean;
	double SaturationTNoise;
	double SaturationSNR;
}APSSaturationType;

typedef struct
{
	double DR_dB;
	double ReadNoise;
	double ReadNoise_e;
	double FWC;
	double FWC_e;
	double ConversionGain;
	std::vector<double> ReadNoiseData;
	std::vector<double> TNoiseData;
	std::vector<double> DataMean;
}APSOETCType;

typedef struct
{
	double MaxSSNR;
	std::vector<double> SNoiseData;
	std::vector<double> DataMean;
	std::vector<double> SSNR;
}APSSSNRType;

typedef struct
{
	double DataMeanFrame;
	std::vector<double> SubFrameDataMean;
}APSDataMeanType;

typedef struct
{
	std::vector<double> SubFrameKValue;
}APSDarkCurrentType;

typedef struct
{
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
}APSAlgorithmThre;

typedef struct
{
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
}DVSAlgorithmThre;

typedef struct
{
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
}DVSStationaryNoiseType;

typedef struct
{
	uint32_t nDataNumber;
	std::vector<uint32_t> AllEventsNum[SubFrameIndex::All + 1];
	std::vector<uint32_t> OnEventsNum[SubFrameIndex::All + 1];
	std::vector<uint32_t> OffEventsNum[SubFrameIndex::All + 1];
	std::vector<uint32_t> NoEventsNum[SubFrameIndex::All + 1];
}DVSEventsNumberCountType;

typedef struct
{
	double OnEventsRatio[SubFrameIndex::All + 1];

	double R_Gb_OnEventsRatio;
	double B_Gb_OnEventsRatio;
	double Gr_Gb_OnEventsRatio;

	double OffEventsRatio[SubFrameIndex::All + 1];

	double R_Gb_OffEventsRatio;
	double B_Gb_OffEventsRatio;
	double Gr_Gb_OffEventsRatio;
}DVSImageContrastSensitivityType;

typedef struct
{
	double dAccompaniedPeakOnEventsRatio[SubFrameIndex::All + 1];
	double dDelayedPeakOnEventsRatio[SubFrameIndex::All + 1];
	double dAccompaniedPeakOffEventsRatio[SubFrameIndex::All + 1];
	double dDelayedPeakOffEventsRatio[SubFrameIndex::All + 1];
}DVSAccompaniedPeakAndDelayedPeakType;

typedef struct
{
	uint32_t nOnEventsPeakNumber;
	std::vector<uint32_t> OnEventsPeakPos;

	uint32_t nOffEventsPeakNumber;
	std::vector<uint32_t> OffEventsPeakPos;
}DVSPeakInfo;



typedef struct
{
	std::vector<std::vector<double>> UniformityBlockData;
	double UniformityRatio;
}DVSStationaryUniformityType;

typedef struct
{
	double dOnEventsUniformityRatio[SubFrameIndex::All + 1];
	std::vector<std::vector<double>> OnEventsUniformityBlockData[SubFrameIndex::All + 1];
	double dOffEventsUniformityRatio[SubFrameIndex::All + 1];
	std::vector<std::vector<double>> OffEventsUniformityBlockData[SubFrameIndex::All + 1];
}DVSSpatialResponseUniformityType;

typedef struct
{
	uint32_t nOffEventsDeadPixelNum;
	uint32_t nOffEventsDeadLineNum;
	uint32_t nOffEventsClusterNum;
	BadPixelMaskType OffEventsBadPixelMask;
    uint32_t nOffEventsMaxClusterSize;

	uint32_t nOnEventsDeadPixelNum;
	uint32_t nOnEventsDeadLineNum;
	uint32_t nOnEventsClusterNum;
	BadPixelMaskType OnEventsBadPixelMask;
    uint32_t nOnEventsMaxClusterSize;
}DVSBadpixelType;

typedef struct
{
	uint32_t HotPixelNum;
	uint32_t HotLineNum;
	uint32_t SingletNum;
	uint32_t CoupletNum;
	uint32_t TripletNum;
	uint32_t FourConnectedNum;
	uint32_t ClusterNum;
    uint32_t MaxClusterSize;
	BadPixelMaskType HotPixelMask;
}DVSHotpixelType;

typedef enum
{
	TEST_NO_ERROR = 0,
	ALGO_HANDLE_ERROR = 0x80000001,
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
}DvsErrCode;
// =================================
#endif
class ALP_ALGO_DLL_API CAlpAPSMPAlgoInterface
{
public:
	static CAlpAPSMPAlgoInterface* CreateAPSAlgoInterface(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, PixelFormatType Pixelformat = PixelFormatType::QuadBayerGBRG, int code = 0);
	virtual ~CAlpAPSMPAlgoInterface();
	virtual bool ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer = false) = 0;
	virtual bool TNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSTNoiseType& TNoiseRes) = 0;
	virtual bool SNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSNoiseType& SNoiseRes) = 0;
	virtual bool BadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& BadpixelRes) = 0;
	virtual bool HotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& HotpixelRes) = 0;
	virtual bool BLC(uint32_t nIndexStart, uint32_t nNumber) = 0;
	virtual bool BLC(uint32_t nIndexStart, uint32_t nNumber, APSDataMeanType& BaseMean) = 0;
	virtual bool BLC(uint32_t nIndexStart, uint32_t nNumber, uint32_t nBaseIndexStart, uint32_t nBaseNumber) = 0;
	virtual bool DPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<Local>& BadPixelLocal) = 0;
	virtual bool BadPixelLocalToOtpType(std::vector<Local> BadPixelLocal, std::vector<uint8_t>& OtpData) = 0;
	virtual bool YShading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSYShadingType& YShadingRes) = 0;
	virtual bool ColorShading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSColorShadingType& ColorShadingRes) = 0;
	virtual bool OpticalCenter(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSOpticalCenterType& OpticalCenterRes) = 0;
	virtual bool PedestalVariation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSPedestalVariationType& PedestalVariationRes) = 0;
	virtual bool ReadNoise(uint32_t nIndex1, uint32_t nIndex2, ROIArea* ROI, APSReadNoiseType& ReadNoiseRes) = 0;
	virtual bool DarkCurrent(std::vector<APSDataMeanType>& DataMean, std::vector<double>& ExpTime, APSDarkCurrentType& DarkCurrentRes) = 0;
	virtual bool DarkCurrent(std::vector<APSTNoiseType>& TNoiseData, std::vector<double>& ExpTime, APSDarkCurrentType& DarkCurrentRes) = 0;
	virtual bool DSNU(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSDSNUType& DSNURes) = 0;
	virtual bool DataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSDataMeanType& DataMean) = 0;
	virtual bool Linearity(std::vector<APSDataMeanType>& LightMean, std::vector<double>& ExpTime, APSLinearityType& LinearityRes) = 0;
	virtual bool OverallSystemGain(std::vector<APSTNoiseType>& LightTNoiseData, std::vector<APSDataMeanType>& LightMean, APSTNoiseType DarkTNoiseBase, APSOverallSystemGainType &GainRes) = 0;
	virtual bool Saturation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSaturationType& SaturationRes) = 0;
	virtual bool OETC(uint32_t nIndexStart, uint32_t nNumber, uint32_t nNumberInOneStep, ROIArea* ROI, SubFrameIndex nChannelIndex, APSOETCType& OETCRes) = 0;
	virtual bool Linearity(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSSNRType& SSNRRes) = 0;
	virtual bool Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, bool bNormalize, ImgType& ImgData) = 0;
	virtual bool Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSType& ImgData) = 0;
	virtual bool Show(uint32_t nIndex, uint16_t * RawData) = 0;
	virtual void SetMultiThreadEnable(bool bEnable = true) = 0;
	virtual void SetLogEnable(bool bEnable = true) = 0;
	virtual void SetAlgorithmThre(APSAlgorithmThre& AlgoThre) = 0;
	virtual APSAlgorithmThre GetAlgorithmThre() = 0;
	virtual uint32_t GetDataNum() = 0;
	virtual bool SaveBin(uint8_t* pRawData, uint64_t nLens, std::string strSavePath) = 0;
	virtual ROIArea GetActiveArea() = 0;
	virtual void GetRawDataSize(uint32_t& nRow, uint32_t& nCol) = 0;
	virtual void SetActiveArea(ROIArea ActiveArea) = 0;
	virtual void SetRawDataSize(uint32_t nRow, uint32_t nCol) = 0;
	virtual std::string GetVersion() = 0;
	virtual int GetCode() = 0;
private:
	static uint32_t m_nSiteNumber;
};


class ALP_ALGO_DLL_API CAlpDVSMPAlgoInterface
{
public:
	static CAlpDVSMPAlgoInterface * CreateDVSAlgoInterface(SensorType Sensortype, std::string strLogDir, PixelFormatType Pixelformat = PixelFormatType::BayerGBRG, int code = 0);
	virtual ~CAlpDVSMPAlgoInterface();
	virtual bool ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber) = 0;
	virtual bool ImportRawData_DropSubFrame(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, uint32_t &nDropSubFrameNum) = 0;
	virtual bool EventsNumberCount(uint32_t nIndexStart, uint32_t nNumber, DVSEventsNumberCountType& EventsNumberCountRes) = 0;
	virtual bool StationaryNoise(uint32_t nIndexStart, uint32_t nNumber, DVSStationaryNoiseType& StationaryNoiseRes) = 0;
	virtual bool StationaryUniformity(uint32_t nIndexStart, uint32_t nNumber, DVSStationaryUniformityType& UniformityRes) = 0;
	virtual bool HotPixel(uint32_t nIndexStart, uint32_t nNumber, DVSHotpixelType& HotpixelRes) = 0;
	virtual bool FindPeak(uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, DVSPeakInfo& Peak, DVSLightTrigerType Light) = 0;
	virtual bool ImageContrastSensitivity(uint32_t nIndexStart, uint32_t nNumber, DVSPeakInfo* Peak, uint32_t nPeakNum, DVSLightTrigerType Light, DVSImageContrastSensitivityType& ImageContrastSensitivityRes) = 0;
	virtual bool AccompaniedPeakAndDelayedPeak(uint32_t nIndexStart, uint32_t nNumber, DVSPeakInfo* Peak, uint32_t nPeakNum, DVSLightTrigerType Light, DVSAccompaniedPeakAndDelayedPeakType& AccompaniedPeakAndDelayedPeakRes) = 0;
	virtual bool SpatialResponseUniformity(uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, DVSPeakInfo* Peak, uint32_t nPeakNum, DVSLightTrigerType Light, DVSSpatialResponseUniformityType& SpatialResponseUniformityRes) = 0;
	virtual bool BadPixel(uint32_t nIndexStart, uint32_t nNumber, DVSPeakInfo* Peak, uint32_t nPeakNum, DVSLightTrigerType Light, DVSBadpixelType& BadpixelRes) = 0;
	virtual bool Show(uint32_t nIndex, uint8_t NoEventFlag, uint8_t OnEventFlag, uint8_t OffEventFlag, ImgType& ImgData) = 0;
	virtual void SetMultiThreadEnable(bool bEnable = true) = 0;
	virtual void SetLogEnable(bool bEnable = true) = 0;
	virtual void SetAlgorithmThre(DVSAlgorithmThre& AlgoThre) = 0;
	virtual DVSAlgorithmThre GetAlgorithmThre() = 0;
	virtual uint32_t GetDataNum() = 0;
	virtual bool SaveBin(uint8_t* pRawData, uint64_t nLens, std::string strSavePath) = 0;
	virtual ROIArea GetActiveArea() = 0;
	virtual void GetRawDataSize(uint32_t& nRow, uint32_t& nCol) = 0;
	virtual void SetActiveArea(ROIArea ActiveArea) = 0;
	virtual void SetRawDataSize(uint32_t nRow, uint32_t nCol) = 0;
	virtual std::string GetVersion() = 0;
	virtual int GetCode() = 0;
	virtual uint32_t GetErrCode() = 0;

    /**
     * @brief Input Target_Evevt([%]), output the corresponding light intensity transition points.
     * @param eventRatioPercent, input Target_EventRatio[%]. eg: eventRatioPercent = 50, representing 50% Events Ratio.
     * @param vecEvent, On/Off Events vector.
     * @param vecLightIntensity, On/Off Light Intensity jump points vector.
     * @param targetLightIntensity output value, if return value < 0.
     * @return true, calculate done.
     * @return false, calculate error.
     */
    virtual bool CalcLightIntensity(double eventRatioPercent, std::vector<double> vecEvent, std::vector<std::pair<double, double>> vecLightIntensity, double& targetLightIntensity) = 0;
private:
	static uint32_t m_nSiteNumber;
};

ALP_ALGO_DLL_API CAlpAPSMPAlgoInterface* CreateAPSAlgoInterface(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, PixelFormatType Pixelformat = PixelFormatType::QuadBayerGBRG, int code = 0);
ALP_ALGO_DLL_API CAlpDVSMPAlgoInterface* CreateDVSAlgoInterface(SensorType Sensortype, std::string strLogDir, PixelFormatType Pixelformat = PixelFormatType::BayerGBRG, int code = 0);
