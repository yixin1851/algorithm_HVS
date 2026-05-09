#pragma once
#include "AlpAPSMPAlgorithm.h"

class CAlp014AAAPSMPAlgorithm : public CAlpAPSMPAlgorithm
{
public:
	CAlp014AAAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code);
	virtual ~CAlp014AAAPSMPAlgorithm();
	virtual bool ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer = false);
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
	virtual bool Saturation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSaturationType& SaturationRes);
	virtual bool OETC(uint32_t nIndexStart, uint32_t nNumber, uint32_t nNumberInOneStep, ROIArea* ROI, SubFrameIndex nChannelIndex, APSOETCType& OETCRes);
	virtual bool Linearity(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSSNRType& SSNRRes);
    virtual bool SpatialFrequencyResponse(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, uint32_t BlackLevelValue, APSColorShadingType& ShadingRes);
    virtual bool RelativeIllumination(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, uint32_t BlackLevelValue, APSRIType& RIRes);
    virtual bool RelativeUniformity(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, uint32_t BlackLevelValue, APSRUType &RURes);
    virtual bool Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, bool bNormalize, ImgType& ImgData);
	virtual bool Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSType& ImgData);
	virtual void SetRawDataSize(uint32_t nRow, uint32_t nCol);
protected:
	virtual bool GetBlockMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, uint32_t nRowBlockNum, uint32_t nColBlockNum, std::vector<CAPSDataContainer>& BlockData, uint32_t nSubRowBlockSize = 0, uint32_t nSubColBlockSize = 0);
	void SetDataToFrame(uint32_t nIndex, uint32_t nRowStart, uint32_t nRows, uint16_t* RawData);
private:
	uint32_t m_nMaxSubFramesNum;
};