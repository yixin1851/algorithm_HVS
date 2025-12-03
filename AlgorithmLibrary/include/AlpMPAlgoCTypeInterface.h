#pragma once

#ifdef API_EXPORTS
#define ALP_ALGO_DLL_API_C _declspec(dllexport)
#else
#define ALP_ALGO_DLL_API_C _declspec(dllimport)
#endif

#include <stdint.h>
// #include <stdbool.h>

#define MAX_DATA_NUMBER 200

typedef void * HANDLE;

#ifndef API_C_TYPE_INTERFACE
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
// =================================


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
}CStationaryNoiseData;

typedef struct
{
	uint32_t nDataNumber;
	uint32_t AllEventsNum[SubFrameIndex::All + 1][MAX_DATA_NUMBER];
	uint32_t OnEventsNum[SubFrameIndex::All + 1][MAX_DATA_NUMBER];
	uint32_t OffEventsNum[SubFrameIndex::All + 1][MAX_DATA_NUMBER];
	uint32_t NoEventsNum[SubFrameIndex::All + 1][MAX_DATA_NUMBER];
}CEventsNumberCountData;

typedef struct
{
	double UniformityRatio;
}CStationaryUniformityData;

typedef struct
{
	uint32_t HotPixelNum;
	uint32_t HotLineNum;
	uint32_t SingletNum;
	uint32_t CoupletNum;
	uint32_t TripletNum;
	uint32_t FourConnectedNum;
	uint32_t ClusterNum;
}CHotpixelData;

typedef struct
{
	uint32_t nOnEventsPeakNumber;
	uint32_t OnEventsPeakPos[MAX_DATA_NUMBER];

	uint32_t nOffEventsPeakNumber;
	uint32_t OffEventsPeakPos[MAX_DATA_NUMBER];
}CPeakInfo;

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
}CImageContrastSensitivityData;

typedef struct
{
	double dAccompaniedPeakOnEventsRatio[SubFrameIndex::All + 1];
	double dDelayedPeakOnEventsRatio[SubFrameIndex::All + 1];
	double dAccompaniedPeakOffEventsRatio[SubFrameIndex::All + 1];
	double dDelayedPeakOffEventsRatio[SubFrameIndex::All + 1];
}CAccompaniedPeakAndDelayedPeakData;

typedef struct
{
	double dOnEventsUniformityRatio[SubFrameIndex::All + 1];
	double dOffEventsUniformityRatio[SubFrameIndex::All + 1];
}CSpatialResponseUniformityData;

typedef struct
{
	uint32_t nOffEventsDeadPixelNum;
	uint32_t nOffEventsClusterNum;

	uint32_t nOnEventsDeadPixelNum;
	uint32_t nOnEventsClusterNum;
}CDVSBadpixelData;

typedef struct
{
	double   dHotPixelThre;
	double   dHotLineThre;
	double   dDeadPixelThre;
	uint32_t nPeakCycle;
	uint32_t nStationaryUniformityRowBlockNum;
	uint32_t nStationaryUniformityColBlockNum;
	uint32_t nSpatialResponseUniformityRowBlockNum;
	uint32_t nSpatialResponseUniformityColBlockNum;
}CDVSAlgorithmThre;

#ifdef __cplusplus
extern "C"
{
#endif
	ALP_ALGO_DLL_API_C HANDLE __stdcall InitHandleDVS(SensorType Sensortype, PixelFormatType Pixelformat = PixelFormatType::BayerGBRG, int code = 0);

	ALP_ALGO_DLL_API_C void __stdcall DeleteHandleDVS(HANDLE h);

	ALP_ALGO_DLL_API_C uint32_t __stdcall ImportRawDataDVS(HANDLE h, uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber);

	ALP_ALGO_DLL_API_C uint32_t __stdcall EventsNumberCountDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, CEventsNumberCountData* EventsNumberCountRes);

	ALP_ALGO_DLL_API_C uint32_t __stdcall StationaryNoiseDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, CStationaryNoiseData* StationaryNoiseRes);

	ALP_ALGO_DLL_API_C uint32_t __stdcall StationaryUniformityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, CStationaryUniformityData* UniformityRes);

	ALP_ALGO_DLL_API_C uint32_t __stdcall HotPixelDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, CHotpixelData* HotpixelRes);

	ALP_ALGO_DLL_API_C uint32_t __stdcall FindPeakDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, CPeakInfo* Peak, DVSLightTrigerType Light);

	ALP_ALGO_DLL_API_C uint32_t __stdcall ImageContrastSensitivityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, DVSLightTrigerType Light, CImageContrastSensitivityData* ImageContrastSensitivityRes);

	ALP_ALGO_DLL_API_C uint32_t __stdcall AccompaniedPeakAndDelayedPeakDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, DVSLightTrigerType Light, CAccompaniedPeakAndDelayedPeakData* AccompaniedPeakAndDelayedPeakRes);

	ALP_ALGO_DLL_API_C uint32_t __stdcall SpatialResponseUniformityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, uint32_t nPeakNum, DVSLightTrigerType Light, CSpatialResponseUniformityData* SpatialResponseUniformityRes);

	ALP_ALGO_DLL_API_C uint32_t __stdcall BadPixelDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, DVSLightTrigerType Light, CDVSBadpixelData* BadpixelRes);

	ALP_ALGO_DLL_API_C uint32_t __stdcall ShowDVS(HANDLE h, uint32_t nIndex, uint8_t NoEventFlag, uint8_t OnEventFlag, uint8_t OffEventFlag, uint8_t * ImgData);

	ALP_ALGO_DLL_API_C uint32_t __stdcall SetMultiThreadEnableDVS(HANDLE h, bool bEnable);

	ALP_ALGO_DLL_API_C uint32_t __stdcall SetAlgorithmThreDVS(HANDLE h, CDVSAlgorithmThre* AlgoThre);

	ALP_ALGO_DLL_API_C uint32_t __stdcall GetAlgorithmThreDVS(HANDLE h, CDVSAlgorithmThre* AlgoThre);

	ALP_ALGO_DLL_API_C uint32_t __stdcall GetDataNumDVS(HANDLE h, uint32_t * nDataNum);

	ALP_ALGO_DLL_API_C uint32_t __stdcall GetRawDataSizeDVS(HANDLE h, uint32_t* nRow, uint32_t* nCol);

	ALP_ALGO_DLL_API_C uint32_t __stdcall AlpGetVersionDVS(HANDLE h, char * ver, uint32_t nLen);
#ifdef __cplusplus
}
#endif
#endif
