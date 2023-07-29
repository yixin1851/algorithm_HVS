#pragma once
#include "AlpDVSMPAlgorithm.h"

class CAlp003AADVSMPAlgorithm : public CAlpDVSMPAlgorithm
{
public:
	CAlp003AADVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code);
	virtual ~CAlp003AADVSMPAlgorithm();
	virtual bool ImportRawData(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber);

protected:
	bool Decoder(uint8_t* pucBinData, uint8_t* pucRawData, size_t nRow, size_t nCol, size_t* pnPos, size_t nBinLens);
};