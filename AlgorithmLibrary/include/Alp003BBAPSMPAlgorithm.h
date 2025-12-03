#pragma once
#include "Alp003BAAPSMPAlgorithm.h"

class CAlp003BBAPSMPAlgorithm : public CAlp003BAAPSMPAlgorithm
{
public:
	CAlp003BBAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code);
	virtual ~CAlp003BBAPSMPAlgorithm();
};