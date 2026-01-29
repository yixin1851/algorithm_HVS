#pragma once
#include "AlpDVSMPAlgorithm.h"

class CAlp004ABDVSMPAlgorithm : public CAlpDVSMPAlgorithm
{
public:
	CAlp004ABDVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code);
	virtual ~CAlp004ABDVSMPAlgorithm();
	virtual bool ImportRawData(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber);
    virtual bool ImportRawData_DropSubFrame(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, uint32_t &nDropSubFrameNum);


protected:
	bool Decode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, size_t* pnPos, size_t nBinLens, uint64_t& nTimeStamp);
};