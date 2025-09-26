#pragma once
#include "Alp014AAAPSMPAlgorithm.h"

class CAlp014BAAPSMPAlgorithm : public CAlp014AAAPSMPAlgorithm
{
public:
	CAlp014BAAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code);
	virtual ~CAlp014BAAPSMPAlgorithm();
};
