#pragma once
#include "AlpMPAlgoInterface.h"
#include "DVSDataContainer.h"
#include "APSDataContainer.h"
#include "DVS03BADecoder.h"
#include <mutex>

typedef std::vector<CDVSDataContainer> DVSRawDataContainer;

class CAlpDVSMPAlgorithm : public CAlpDVSMPAlgoInterface
{
public:
	CAlpDVSMPAlgorithm() = delete;
	CAlpDVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum);
	virtual ~CAlpDVSMPAlgorithm();
	virtual bool ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber);
	virtual bool EventsNumberCount(uint32_t nIndexStart, uint32_t nNumber, EventsNumberCountData& EventsNumberCountRes);
	virtual bool StationaryNoise(uint32_t nIndexStart, uint32_t nNumber, StationaryNoiseData& StationaryNoiseRes);
	virtual bool StationaryUniformity(uint32_t nIndexStart, uint32_t nNumber, StationaryUniformityData& UniformityRes);
	virtual bool HotPixel(uint32_t nIndexStart, uint32_t nNumber, HotpixelData& HotpixelRes);
	virtual bool FindPeak(uint32_t nIndexStart, uint32_t nNumber, PeakInfo& Peak, LightTrigerType Light);
	virtual bool ImageContrastSensitivity(uint32_t nIndexStart, uint32_t nNumber, PeakInfo* Peak, uint32_t nPeakNum, LightTrigerType Light, ImageContrastSensitivityData& ImageContrastSensitivityRes);
	virtual bool AccompaniedPeakAndDelayedPeak(uint32_t nIndexStart, uint32_t nNumber, PeakInfo* Peak, uint32_t nPeakNum, LightTrigerType Light, AccompaniedPeakAndDelayedPeakData& AccompaniedPeakAndDelayedPeakRes);
	virtual bool SpatialResponseUniformity(uint32_t nIndexStart, uint32_t nNumber, PeakInfo* Peak, uint32_t nPeakNum, LightTrigerType Light, SpatialResponseUniformityData& SpatialResponseUniformityRes);
	virtual bool BadPixel(uint32_t nIndexStart, uint32_t nNumber, PeakInfo* Peak, uint32_t nPeakNum, LightTrigerType Light, DVSBadpixelData& BadpixelRes);
	virtual bool Show(uint32_t nIndex, uint8_t NoEventFlag, uint8_t OnEventFlag, uint8_t OffEventFlag, ImgType& ImgData);
	virtual void SetMultiThreadEnable(bool bEnable = true);
	virtual void SetLogEnable(bool bEnable = true);
	virtual void SetAlgorithmThre(DVSAlgorithmThre& AlgoThre);
	virtual DVSAlgorithmThre GetAlgorithmThre();
	virtual uint32_t GetDataNum();
	virtual bool SaveBin(uint8_t* pRawData, uint64_t nLens, std::string strSavePath);
	virtual ROIArea GetActiveArea();
	virtual void GetRawDataSize(uint32_t& nRow, uint32_t& nCol);
	virtual void SetActiveArea(ROIArea ActiveArea);
	virtual void SetRawDataSize(uint32_t nRow, uint32_t nCol);
	virtual std::string GetVersion();
	virtual uint32_t GetErrCode();
protected:
	virtual void ThreadEventsNumberCount(uint32_t nIndexStart, uint32_t nNumberStart, uint32_t nNumberEnd, EventsNumberCountData& EventsNumberCountRes);
	virtual double Mean(std::vector<double>& RawData, uint32_t nLens);
	virtual double Mean(std::vector<uint32_t>& RawData, uint32_t nLens);
	virtual double Mean(CAPSDataContainer& RawData, ROIArea* ROI = nullptr);
	virtual double Std(std::vector<double>& RawData, uint32_t nLens);
	virtual double Std(std::vector<uint32_t>& RawData, uint32_t nLens);
	virtual double Std(CAPSDataContainer& RawData, ROIArea* ROI);
	virtual void Max(double& dMaxValue, uint32_t& nMaxLocal, std::vector<double>& RawData, uint32_t nLens);
	virtual void Max(double& dMaxValue, uint32_t& nMaxLocal, std::vector<uint32_t>& RawData, uint32_t nLens);
	virtual void Max(double& dMaxValue, Local& MaxLocal, CAPSDataContainer& RawData, ROIArea* ROI = nullptr);
	virtual void Min(double& dMinValue, uint32_t& nMinLocal, std::vector<double>& RawData, uint32_t nLens);
	virtual void Min(double& dMinValue, Local& MinLocal, CAPSDataContainer& RawData, ROIArea* ROI = nullptr);
	virtual bool WriteLog(std::string strMessage);
private:
	ROIArea m_ActiveArea;
	uint32_t m_nTotalRow;
	uint32_t m_nTotalCol;
	bool m_bMultiThreadEnable;
	bool m_bLogEnable;
	SensorType m_SensorType;
	std::string m_strLogFilePath;
	std::mutex m_LogMutex;
	DVSRawDataContainer m_RawDataContainer;
	DVSAlgorithmThre m_AlgorithmThre;
	uint32_t m_nSiteNum;
	CDVS03BADecoder m_03BADVSDecoder;
	uint32_t m_nErrCode;
};