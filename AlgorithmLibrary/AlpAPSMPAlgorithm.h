#pragma once
#include "AlpMPAlgoInterface.h"
#include "APSDataContainer.h"
#include <mutex>

typedef std::vector<CAPSDataContainer> SingleChannelRawData;
typedef std::vector<SingleChannelRawData> RawDataContainer;

class CAlpAPSMPAlgorithm : public CAlpAPSMPAlgoInterface
{
public:
	CAlpAPSMPAlgorithm() = delete;
	CAlpAPSMPAlgorithm(SensorType Sensortype, RawType Rawtype, std::string strLogDir, uint32_t nSiteNum);
	virtual ~CAlpAPSMPAlgorithm();
	virtual bool ImportRawData(uint8_t * pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer = false);
	virtual bool ImportRawData(uint8_t* pRawData, uint64_t nLens, APSSubFrameIndex nChannelIndex, uint32_t nIndexStart, uint32_t nNumber);
	virtual bool TNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, std::vector<double> &TNoiseData);
	virtual bool SNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double> &SNoiseData);
	virtual bool RowTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double> &RowTNoiseData);
	virtual bool ColTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double> &ColTNoiseData);
	virtual bool RowSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double> &RowSNoiseData);
	virtual bool ColSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double> &ColSNoiseData);
	virtual bool BadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<BadpixelData>& BadpixelRes);
	virtual bool HotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<HotpixelData>& HotpixelRes);
	virtual bool BLC(uint32_t nIndexStart, uint32_t nNumber, std::vector<double>& BaseMean);
	virtual bool BLC(uint32_t nIndexStart, uint32_t nNumber, uint32_t nBaseIndexStart, uint32_t nBaseNumber);
	virtual bool DPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<BadPixelMaskData>& BadPixelMask);
	virtual bool Shading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, ShadingData& ShadingRes);
	virtual bool DarkCurrent(std::vector<std::vector<double>>& Data, std::vector<double>& ExpTime, bool bUseMeanFunc, std::vector<double>& DarkCurrentRes);
	virtual bool DSNU(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, DSNUData& DSNURes);
	virtual bool DataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& DataMean);
	virtual bool Linearity(std::vector<std::vector<double>>& LightMean, std::vector<double>& ExpTime, std::vector<LinearityData>& LinearityRes);
	virtual bool OverallSystemGain(std::vector<std::vector<double>>& LightTNoiseData, std::vector<std::vector<double>>& LightMean, std::vector<double> DarkTNoiseBase, std::vector<double>& GainK);
	virtual bool Saturation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, SaturationData &SaturationRes);
	virtual bool Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, bool bNormalize, ImgType& ImgData);
	virtual bool Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, APSType& ImgData);
	virtual void SetMultiThreadEnable(bool bEnable = true);
	virtual void SetLogEnable(bool bEnable = true);
	virtual void SetAlgorithmThre(APSAlgorithmThre&AlgoThre);
	virtual APSAlgorithmThre GetAlgorithmThre();
	virtual ROIArea GetActiveArea();
	virtual void GetRawDataSize(uint32_t &nRow, uint32_t &nCol);
	virtual uint32_t GetDataNum();
	virtual bool SaveBin(uint8_t* pRawData, uint64_t nLens, std::string strSavePath);
	virtual std::string GetVersion();
protected:
	virtual bool WriteLog(std::string strMessage, uint32_t nAPSSubFrameIndex);
	virtual double Mean(std::vector<double>& RawData, uint32_t nLens);
	virtual double Mean(CAPSDataContainer& RawData, ROIArea* ROI = nullptr);
	virtual double Std(std::vector<double>& RawData, uint32_t nLens);
	virtual double Std(CAPSDataContainer& RawData, ROIArea* ROI = nullptr);
	virtual double RMS(std::vector<double>& RawData, uint32_t nLens);
	virtual double RMS(CAPSDataContainer& RawData, ROIArea* ROI = nullptr);
	virtual void Max(double& dMaxValue, uint32_t& nMaxLocal, std::vector<double>& RawData, uint32_t nLens);
	virtual void Max(double& dMaxValue, Local& MaxLocal, CAPSDataContainer& RawData, ROIArea* ROI = nullptr);
	virtual void Min(double& dMinValue, uint32_t& nMinLocal, std::vector<double>& RawData, uint32_t nLens);
	virtual void Min(double& dMinValue, Local& MinLocal, CAPSDataContainer& RawData, ROIArea* ROI = nullptr);
	inline bool PosInRoi(uint32_t nRows, uint32_t nCols, ROIArea & ROI);

	virtual bool LinearityFit(std::vector<double>& XData, std::vector<double>& YData, double& k, double& b);
	virtual void SubFrameTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& TNoiseData, bool &bRes);
	virtual void SubFrameSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& SNoiseData, bool& bRes);
	virtual void SubFrameRowTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& RowTNoiseData, bool& bRes);
	virtual void SubFrameColTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& ColTNoiseData, bool& bRes);
	virtual void SubFrameRowSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& RowSNoiseData, bool& bRes);
	virtual void SubFrameColSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& ColSNoiseData, bool& bRes);
	virtual void SubFrameBadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, BadpixelData& BadpixelRes, bool& bRes);
	virtual void SubFrameHotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, HotpixelData& HotpixelRes, bool& bRes);
	virtual void SubFrameBLC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& BaseMean, bool& bRes);
	virtual void SubFrameDPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, BadPixelMaskData& BadPixelMask, bool& bRes);
	virtual void SubFrameDataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& DataMean, bool& bRes);
	
	virtual void SubFrameBLCByColBase(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, std::vector<double>& BaseMean, bool& bRes);
	virtual void SubFrameColMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, std::vector<double>& DataMean, bool& bRes);

	virtual bool GetBlockMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, uint32_t nBlockSize, std::vector<CAPSDataContainer> & BlockData);
	virtual void SubFrameBlockMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, uint32_t nBlockSize, CAPSDataContainer &BlockData, bool& bRes);

private:
	ROIArea m_ActiveArea;
	uint32_t m_nTotalRow;
	uint32_t m_nTotalCol;
	uint32_t m_nChannelRow;
	uint32_t m_nChannelCol;
	bool m_bMultiThreadEnable;
	bool m_bLogEnable;
	SensorType m_SensorType;
	RawType m_RawType;
	std::string m_strLogFilePath;
	std::mutex m_LogMutex;
	RawDataContainer m_RawDataContainer;
	APSAlgorithmThre m_AlgorithmThre;
	uint32_t m_nSiteNum;
};