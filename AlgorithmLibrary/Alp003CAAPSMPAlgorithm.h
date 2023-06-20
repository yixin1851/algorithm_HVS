#pragma once
#include "AlpAPSMPAlgorithm.h"

class CAlp003CAAPSMPAlgorithm : public CAlpAPSMPAlgorithm
{
public:
	CAlp003CAAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat);
	virtual ~CAlp003CAAPSMPAlgorithm();
	virtual bool ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer = false);
};

