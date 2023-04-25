#pragma once

#ifdef API_EXPORTS
#define ALP_ALGO_DLL_API _declspec(dllexport)
#else
#define ALP_ALGO_DLL_API _declspec(dllimport)
#endif

#include <cstdint>
#include <vector>
#include <string>

#define APS_BAD_PIXEL_FLAG 1
#define APS_DEAD_PIXEL_FLAG 2
#define APS_HOT_PIXEL_FLAG 3

#define DVS_DEAD_PIXEL_FLAG 1
#define DVS_ERROR_PIXEL_FLAG 2
#define DVS_HOT_PIXEL_FLAG 3

typedef std::vector<std::vector<uint8_t>> ImgType;
typedef std::vector<std::vector<double>> APSType;

typedef struct
{
	uint32_t Up;
	uint32_t Down;
	uint32_t Left;
	uint32_t Right;
}ROIArea;

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
}BadPixelMaskData;

typedef struct
{
	uint32_t BadPixelNum;
	uint32_t DeadPixelNum;
	uint32_t DeadLineNum;
	uint32_t SingletNum;
	uint32_t CoupletNum;
	uint32_t ClusterNum;
	BadPixelMaskData BadPixelMask;
}BadpixelData;

typedef struct
{
	uint32_t HotPixelNum;
	uint32_t HotLineNum;
	BadPixelMaskData HotPixelMask;
}HotpixelData;

typedef struct
{
	uint32_t CenterRow;
	uint32_t CenterCol;
	std::vector<double> LumaShadingLT;
	std::vector<double> LumaShadingLB;
	std::vector<double> LumaShadingRT;
	std::vector<double> LumaShadingRB;
	double R_Gb_Ratio;
	double B_Gb_Ratio;
	double Gr_Gb_Ratio;
}ShadingData;

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
}DSNUData;

typedef struct
{
	double b;
	double k;
	double LeMax;
	double LeMin;
}LinearityData;

typedef struct
{
	double SaturationMean;
	double SaturationTNoise;
	double SaturationSNR;
}SaturationData;

typedef enum
{
	ALP_003AA,
	ALP_003BA,
	ALP_003CA,
	ALP_004AA,
}SensorType;

typedef enum
{
	RAW8,
	RAW10,
	RAW12,
}RawType;

typedef enum
{
	Gb1,
	Gb2,
	B1,
	B2,
	R1,
	R2,
	Gr1,
	Gr2,
	SubFrameNum,
}APSSubFrameIndex;

typedef enum
{
	Gb,
	B,
	R,
	Gr,
	All,
}DVSSubFrameIndex;

typedef struct
{
	double dHotPixelThre;
	double dHotLineThre;
	double dBadPixelThre;
	double dDeadPixelThre;
	double dDeadLineThre;
	uint32_t nBadPixelRadius;
	uint32_t nOpticalFindRadius;
	uint32_t nShadingTestRadius;
	uint32_t nDSNUBlockSize;
}APSAlgorithmThre;


typedef struct
{
	double dHotPixelThre;
	double dHotLineThre;
	double dDeadPixelThre;
	double dErrorPixelThre;
	uint32_t nFindPeakNum;
	double dFindPeakThre;
	uint32_t nStationaryUniformityRowBlockNum;
	uint32_t nStationaryUniformityColBlockNum;
	uint32_t nSpatialResponseUniformityRowBlockNum;
	uint32_t nSpatialResponseUniformityColBlockNum;
}DVSAlgorithmThre;

typedef struct
{
	double dStationaryNoiseMeanOn;
	double dStationaryNoiseMeanOff;
	double dStationaryNoiseMeanAll;
	double dStationaryNoiseStdOn;
	double dStationaryNoiseStdOff;
	double dStationaryNoiseStdAll;
	double dStationaryRowSNoise;
	double dStationaryColSNoise;
}StationaryNoiseData;

typedef struct
{
	uint32_t nDataNumber;
	std::vector<uint32_t> AllEventsNum[DVSSubFrameIndex::All + 1];
	std::vector<uint32_t> OnEventsNum[DVSSubFrameIndex::All + 1];
	std::vector<uint32_t> OffEventsNum[DVSSubFrameIndex::All + 1];
	std::vector<uint32_t> NoEventsNum[DVSSubFrameIndex::All + 1];
}EventsNumberCountData;

typedef struct
{
	double OnEventsRatio[DVSSubFrameIndex::All + 1];

	double R_Gb_OnEventsRatio;
	double B_Gb_OnEventsRatio;
	double Gr_Gb_OnEventsRatio;

	double OffEventsRatio[DVSSubFrameIndex::All + 1];

	double R_Gb_OffEventsRatio;
	double B_Gb_OffEventsRatio;
	double Gr_Gb_OffEventsRatio;
}ImageContrastSensitivityData;

typedef struct
{
	double dAccompaniedPeakOnEventsRatio[DVSSubFrameIndex::All + 1];
	double dDelayedPeakOnEventsRatio[DVSSubFrameIndex::All + 1];
	double dAccompaniedPeakOffEventsRatio[DVSSubFrameIndex::All + 1];
	double dDelayedPeakOffEventsRatio[DVSSubFrameIndex::All + 1];
}AccompaniedPeakAndDelayedPeakData;

typedef struct
{
	uint32_t nOnEventsPeakNumber;
	std::vector<uint32_t> OnEventsPeakPos;

	uint32_t nOffEventsPeakNumber;
	std::vector<uint32_t> OffEventsPeakPos;
}PeakInfo;

typedef enum
{
	OffEventsOnly = 1,
	OnEventsOnly,
	On_OffEvents,
}LightTrigerType;

typedef struct
{
	std::vector<std::vector<double>> UniformityBlockData;
	double UniformityRatio;
}StationaryUniformityData;

typedef struct
{
	double dOnEventsUniformityRatio[DVSSubFrameIndex::All + 1];
	std::vector<std::vector<double>> OnEventsUniformityBlockData[DVSSubFrameIndex::All + 1];
	double dOffEventsUniformityRatio[DVSSubFrameIndex::All + 1];
	std::vector<std::vector<double>> OffEventsUniformityBlockData[DVSSubFrameIndex::All + 1];
}SpatialResponseUniformityData;

typedef struct
{
	uint32_t nOffEventsDeadPixelNum;
	uint32_t nOffEventsErrorPixelNum;
	uint32_t nOffEventsClusterNum;
	BadPixelMaskData OffEventsBadPixelMask;

	uint32_t nOnEventsDeadPixelNum;
	uint32_t nOnEventsErrorPixelNum;
	uint32_t nOnEventsClusterNum;
	BadPixelMaskData OnEventsBadPixelMask;
}DVSBadpixelData;

class ALP_ALGO_DLL_API CAlpAPSMPAlgoInterface
{
public:
	static CAlpAPSMPAlgoInterface* CreateAPSAlgoInterface(SensorType Sensortype, RawType Rawtype, std::string strLogDir);
	virtual ~CAlpAPSMPAlgoInterface();
	virtual bool ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer = false) = 0;
	virtual bool ImportRawData(uint8_t* pRawData, uint64_t nLens, APSSubFrameIndex nChannelIndex, uint32_t nIndexStart, uint32_t nNumber) = 0;
	virtual bool TNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& TNoiseData) = 0;
	virtual bool SNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& SNoiseData) = 0;
	virtual bool RowTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& RowTNoiseData) = 0;
	virtual bool ColTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& ColTNoiseData) = 0;
	virtual bool RowSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& RowSNoiseData) = 0;
	virtual bool ColSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& ColSNoiseData) = 0;
	virtual bool BadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<BadpixelData>& BadpixelRes) = 0;
	virtual bool HotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<HotpixelData>& HotpixelRes) = 0;
	virtual bool BLC(uint32_t nIndexStart, uint32_t nNumber, std::vector<double>& BaseMean) = 0;
	virtual bool BLC(uint32_t nIndexStart, uint32_t nNumber, uint32_t nBaseIndexStart, uint32_t nBaseNumber) = 0;
	virtual bool DPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<BadPixelMaskData>& BadPixelMask) = 0;
	virtual bool Shading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, ShadingData& ShadingRes) = 0;
	virtual bool DarkCurrent(std::vector<std::vector<double>>& Data, std::vector<double>& ExpTime, bool bUseMeanFunc, std::vector<double>& DarkCurrentRes) = 0;
	virtual bool DSNU(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, DSNUData& DSNURes) = 0;
	virtual bool DataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& DataMean) = 0;
	virtual bool Linearity(std::vector<std::vector<double>>& LightMean, std::vector<double>& ExpTime, std::vector<LinearityData>& LinearityRes) = 0;
	virtual bool OverallSystemGain(std::vector<std::vector<double>>& LightTNoiseData, std::vector<std::vector<double>>& LightMean, std::vector<double> DarkTNoiseBase, std::vector<double>& GainK) = 0;
	virtual bool Saturation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, SaturationData& SaturationRes) = 0;
	virtual bool Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, bool bNormalize, ImgType& ImgData) = 0;
	virtual bool Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, APSType& ImgData) = 0;
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
private:
	static uint32_t m_nSiteNumber;
};


class ALP_ALGO_DLL_API CAlpDVSMPAlgoInterface
{
public:
	static CAlpDVSMPAlgoInterface * CreateDVSAlgoInterface(SensorType Sensortype, std::string strLogDir);
	virtual ~CAlpDVSMPAlgoInterface();
	virtual bool ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber) = 0;
	virtual bool EventsNumberCount(uint32_t nIndexStart, uint32_t nNumber, EventsNumberCountData& EventsNumberCountRes) = 0;
	virtual bool StationaryNoise(uint32_t nIndexStart, uint32_t nNumber, StationaryNoiseData& StationaryNoiseRes) = 0;
	virtual bool StationaryUniformity(uint32_t nIndexStart, uint32_t nNumber, StationaryUniformityData& UniformityRes) = 0;
	virtual bool HotPixel(uint32_t nIndexStart, uint32_t nNumber, HotpixelData& HotpixelRes) = 0;
	virtual bool FindPeak(uint32_t nIndexStart, uint32_t nNumber, PeakInfo& Peak, LightTrigerType Light) = 0;
	virtual bool ImageContrastSensitivity(uint32_t nIndexStart, uint32_t nNumber, PeakInfo* Peak, uint32_t nPeakNum, LightTrigerType Light, ImageContrastSensitivityData& ImageContrastSensitivityRes) = 0;
	virtual bool AccompaniedPeakAndDelayedPeak(uint32_t nIndexStart, uint32_t nNumber, PeakInfo* Peak, uint32_t nPeakNum, LightTrigerType Light, AccompaniedPeakAndDelayedPeakData& AccompaniedPeakAndDelayedPeakRes) = 0;
	virtual bool SpatialResponseUniformity(uint32_t nIndexStart, uint32_t nNumber, PeakInfo* Peak, uint32_t nPeakNum, LightTrigerType Light, SpatialResponseUniformityData& SpatialResponseUniformityRes) = 0;
	virtual bool BadPixel(uint32_t nIndexStart, uint32_t nNumber, PeakInfo* Peak, uint32_t nPeakNum, LightTrigerType Light, DVSBadpixelData& BadpixelRes) = 0;
	virtual bool Show(uint32_t nIndex, uint8_t NoEventFlag, uint8_t OnEventFlag, uint8_t OffEventFlag, ImgType& ImgData) = 0;
	virtual void SetMultiThreadEnable(bool bEnable = true) = 0;
	virtual void SetLogEnable(bool bEnable = true) = 0;
	virtual void SetAlgorithmThre(DVSAlgorithmThre& AlgoThre) = 0;
	virtual DVSAlgorithmThre GetAlgorithmThre() = 0;
	virtual uint32_t GetDataNum() = 0;
	virtual bool SaveBin(uint8_t* pRawData, uint64_t nLens, std::string strSavePath) = 0;
	virtual void GetRawDataSize(uint32_t& nRow, uint32_t& nCol) = 0;
	virtual std::string GetVersion() = 0;
private:
	static uint32_t m_nSiteNumber;
};

ALP_ALGO_DLL_API CAlpAPSMPAlgoInterface* CreateAPSAlgoInterface(SensorType Sensortype, RawType Rawtype, std::string strLogDir);
ALP_ALGO_DLL_API CAlpDVSMPAlgoInterface* CreateDVSAlgoInterface(SensorType Sensortype, std::string strLogDir);
