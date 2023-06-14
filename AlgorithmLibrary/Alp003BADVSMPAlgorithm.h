#pragma once
#include "AlpDVSMPAlgorithm.h"
#include "DVS03BADecoder.h"

class CAlp003BADVSMPAlgorithm : public CAlpDVSMPAlgorithm
{
public:
	CAlp003BADVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum);
	virtual ~CAlp003BADVSMPAlgorithm();
	virtual bool ImportRawData(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber);

protected:
	CDVS03BADecoder m_03BADVSDecoder;
};