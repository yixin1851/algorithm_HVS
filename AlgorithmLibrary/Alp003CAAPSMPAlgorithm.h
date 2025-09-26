#pragma once
#include "AlpAPSMPAlgorithm.h"

class CAlp003CAAPSMPAlgorithm : public CAlpAPSMPAlgorithm
{
public:
	CAlp003CAAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code);
	virtual ~CAlp003CAAPSMPAlgorithm();
	virtual bool ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer = false);
	virtual bool BadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& BadpixelRes);
	virtual bool HotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& HotpixelRes);
	virtual bool DPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<Local>& BadPixelLocal);
protected:
	virtual void SubFrameBadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameBadpixelType& BadpixelRes, bool& bRes);
	virtual void SubFrameHotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameBadpixelType& HotpixelRes, bool& bRes);
	virtual void DPC_APS_Only(uint32_t nIndex, ROIArea* ROI, std::vector<Local>& BadPixelLocal, bool& bRes);
	virtual void DPC_HVS(uint32_t nIndex, ROIArea* ROI, std::vector<Local>& BadPixelLocal, bool& bRes);
private:
	bool m_bHVS_DPC;
};

