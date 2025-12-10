#pragma once
#include "AlpMPAlgoInterface.h"
#include "APSDataContainer.h"
#include <mutex>

bool operator< (const Local& lh, const Local& rh);

typedef std::vector<CAPSDataContainer> SingleChannelRawData;
typedef std::vector<SingleChannelRawData> RawDataContainer;

class CAlpAPSMPAlgorithm : public CAlpAPSMPAlgoInterface
{
public:
	CAlpAPSMPAlgorithm() = delete;
	CAlpAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code);
	virtual ~CAlpAPSMPAlgorithm();
	virtual bool ImportRawData(uint8_t * pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer = false) = 0;
	virtual bool TNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSTNoiseType& TNoiseRes);
	virtual bool SNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSNoiseType& SNoiseRes);
	virtual bool BadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& BadpixelRes);
	virtual bool HotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& HotpixelRes);
	virtual bool BLC(uint32_t nIndexStart, uint32_t nNumber);
	virtual bool BLC(uint32_t nIndexStart, uint32_t nNumber, APSDataMeanType& BaseMean);
	virtual bool BLC(uint32_t nIndexStart, uint32_t nNumber, uint32_t nBaseIndexStart, uint32_t nBaseNumber);
	virtual bool DPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<Local>& BadPixelLocal);
	virtual bool BadPixelLocalToOtpType(std::vector<Local> BadPixelLocal, std::vector<uint8_t>& OtpData);
	virtual bool YShading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSYShadingType& ShadingRes);
	virtual bool ColorShading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSColorShadingType& ShadingRes);
	virtual bool OpticalCenter(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSOpticalCenterType& OpticalCenterRes);
	virtual bool PedestalVariation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSPedestalVariationType& PedestalVariationRes);
	virtual bool ReadNoise(uint32_t nIndex1, uint32_t nIndex2, ROIArea* ROI, APSReadNoiseType& ReadNoiseRes);
	virtual bool DarkCurrent(std::vector<APSDataMeanType>& DataMean, std::vector<double>& ExpTime, APSDarkCurrentType& DarkCurrentRes);
	virtual bool DarkCurrent(std::vector<APSTNoiseType>& Data, std::vector<double>& ExpTime, APSDarkCurrentType& DarkCurrentRes);
	virtual bool DSNU(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSDSNUType& DSNURes);
	virtual bool DataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSDataMeanType& DataMean);
	virtual bool Linearity(std::vector<APSDataMeanType>& LightMean, std::vector<double>& ExpTime, APSLinearityType& LinearityRes);
	virtual bool OverallSystemGain(std::vector<APSTNoiseType>& LightTNoiseData, std::vector<APSDataMeanType>& LightMean, APSTNoiseType DarkTNoiseBase, APSOverallSystemGainType& GainRes);
	virtual bool Saturation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSaturationType &SaturationRes);
	virtual bool OETC(uint32_t nIndexStart, uint32_t nNumber, uint32_t nNumberInOneStep, ROIArea* ROI, SubFrameIndex nChannelIndex, APSOETCType& OETCRes);
	virtual bool Linearity(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSSNRType& SSNRRes);
	virtual bool Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, bool bNormalize, ImgType& ImgData);
	virtual bool Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSType& ImgData);
	virtual bool Show(uint32_t nIndex, uint16_t* RawData);
	virtual void SetMultiThreadEnable(bool bEnable = true);
	virtual void SetLogEnable(bool bEnable = true);
	virtual void SetAlgorithmThre(APSAlgorithmThre&AlgoThre);
	virtual APSAlgorithmThre GetAlgorithmThre();
	virtual ROIArea GetActiveArea();
	virtual void GetRawDataSize(uint32_t &nRow, uint32_t &nCol);
	virtual void SetActiveArea(ROIArea ActiveArea);
	virtual void SetRawDataSize(uint32_t nRow, uint32_t nCol);
	virtual uint32_t GetDataNum();
	virtual bool SaveBin(uint8_t* pRawData, uint64_t nLens, std::string strSavePath);
	virtual std::string GetVersion();
	virtual int GetCode();
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
	virtual void SubFrameTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameTNoiseType& TNoise, bool &bRes, RawDataContainer &DataContainer);
	virtual void SubFrameSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameSNoiseType& SNoiseData, bool& bRes, RawDataContainer &DataContainer);
	virtual void SubFrameBadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameBadpixelType& BadpixelRes, bool& bRes);
	virtual void SubFrameHotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameBadpixelType& HotpixelRes, bool& bRes);
	virtual void SubFrameBLC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, double& BaseMean, bool& bRes);
	virtual void SubFrameDPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, std::vector<Local>& BadPixelList, bool& bRes);
	virtual void SubFrameDataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, double& DataMean, bool& bRes, RawDataContainer& DataContainer);
	
	virtual void SubFrameBLCByColBase(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, std::vector<double>& BaseMean, bool& bRes);
	virtual void SubFrameColMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, std::vector<double>& DataMean, bool& bRes);

	virtual bool GetBlockMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, uint32_t nRowBlockNum, uint32_t nColBlockNum, std::vector<CAPSDataContainer> & BlockData, uint32_t nSubRowBlockSize = 0, uint32_t nSubColBlockSize = 0);
	virtual void SubFrameBlockMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, uint32_t nRowBlockNum, uint32_t nColBlockNum, uint32_t nSubRowBlockSize, uint32_t nSubColBlockSize, CAPSDataContainer &BlockData, bool& bRes);

	void SetDataToSubFrame(uint32_t nIndex, uint32_t nRows, uint32_t nCols, double dValue);
	void SubFrameLocalToTotalLocal(Local SubLocal, SubFrameIndex nChannelIndex, Local& TotalLocal);
	void GetDataFromSubFrame(uint32_t nIndex, uint32_t nRows, uint32_t nCols, double &dValue);
	void TotalLocalToSubFrameLocal(Local TotalLocal, SubFrameIndex &nChannelIndex, Local &SubLocal);
	void SetDataToFrame(uint32_t nIndex, uint32_t nRowStart, uint32_t nRows, uint16_t * RawData);

	void SubFrameReadNoise(uint32_t nIndex1, uint32_t nIndex2, ROIArea* ROI, SubFrameIndex nChannelIndex, APSReadNoiseType& ReadNoiseRes, bool& bRes, RawDataContainer& DataContainer);

	void ImportDataTo16SubFrame(uint32_t nIndexStart, uint32_t nNumber);
protected:
	ROIArea m_ActiveArea;
	uint32_t m_nTotalRow;
	uint32_t m_nTotalCol;
	uint32_t m_nChannelRow;
	uint32_t m_nChannelCol;
	bool m_bMultiThreadEnable;
	bool m_bLogEnable;
	SensorType m_SensorType;
	APSRawType m_RawType;
	std::string m_strLogFilePath;
	std::mutex m_LogMutex;
	RawDataContainer m_RawDataContainer;
	RawDataContainer m_16SubRawDataContainer;
	APSAlgorithmThre m_AlgorithmThre;
	uint32_t m_nSiteNum;
	double m_dPedestal;
	PixelFormatType m_PixelFormat;
	bool m_bUse16SubFrame;
	int m_nCode;
};