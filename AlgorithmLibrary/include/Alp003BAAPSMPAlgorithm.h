#pragma once
#include "AlpAPSMPAlgorithm.h"

class CAlp003BAAPSMPAlgorithm : public CAlpAPSMPAlgorithm
{
public:
	CAlp003BAAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code);
	virtual ~CAlp003BAAPSMPAlgorithm();
	virtual bool ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer = false);
	virtual bool BadPixelLocalToOtpType(std::vector<Local>& BadPixelLocal, std::vector<uint8_t>& OtpData);
};