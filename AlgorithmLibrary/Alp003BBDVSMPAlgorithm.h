#pragma once
#include "Alp003BADVSMPAlgorithm.h"
#include "DVS03BADecoder.h"

class CAlp003BBDVSMPAlgorithm : public CAlp003BADVSMPAlgorithm
{
public:
	CAlp003BBDVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat);
	virtual ~CAlp003BBDVSMPAlgorithm();
};