#pragma once
#include "AlpDVSMPAlgorithm.h"
#include "DVS03BADecoder.h"

class CAlp003BADVSMPAlgorithm : public CAlpDVSMPAlgorithm
{
public:
	CAlp003BADVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code);
	virtual ~CAlp003BADVSMPAlgorithm();
	virtual bool ImportRawData(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber);
    virtual bool ImportRawData_DropSubFrame(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, uint32_t &nDropSubFrameNum);


protected:
	CDVS03BADecoder m_03BADVSDecoder;
};