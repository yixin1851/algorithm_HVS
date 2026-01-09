#pragma once
#include "Alp003BADVSMPAlgorithm.h"
#include "DVS03BADecoder.h"

class CAlp003BBDVSMPAlgorithm : public CAlp003BADVSMPAlgorithm
{
public:
	CAlp003BBDVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code);
	virtual ~CAlp003BBDVSMPAlgorithm();
    virtual bool ImportRawData_DropSubFrame(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, uint32_t nMode, size_t &nDropSubFrameNum);

};