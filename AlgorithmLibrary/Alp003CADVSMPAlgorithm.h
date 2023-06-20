#pragma once

#include "AlpDVSMPAlgorithm.h"

typedef struct
{
	uint64_t Header_vec : 24;
	uint64_t Timestamp : 40;
}Alp003CAFormatHeader;

typedef struct
{
	uint64_t Roi_row_start : 9;
	uint64_t Roi_row_stop : 9;
	uint64_t Roi_col_start : 9;
	uint64_t Roi_col_stop : 9;
	uint64_t : 4;
	uint64_t Dynamic : 4;
	uint64_t Subframe : 4;
	uint64_t Colflip : 1;
	uint64_t Rowflip : 1;
	uint64_t : 1;
	uint64_t Framemode : 1;
	uint64_t Pixmode : 3;
	uint64_t Highfps : 1;
	uint64_t  : 8;
}Alp003CAFormatStatic;

typedef struct
{
	uint64_t Footer_vec : 32;
	uint64_t Subframesize : 16;
	uint64_t Max_row_index : 9;
	uint64_t  : 6;
	uint64_t Dropflag: 1;
}Alp003CAFormatFooter;

typedef struct
{
	uint8_t Pix0 : 2;
	uint8_t Pix1 : 2;
	uint8_t Pix2 : 2;
	uint8_t Pix3 : 2;
}Alp003CAFormatEventGroup;

typedef struct
{
	uint8_t Voidbytelen : 6;
	uint8_t flag : 2;
}Alp003CAFormatVoidByte;

class CAlp003CADVSMPAlgorithm : public CAlpDVSMPAlgorithm
{
public:
	CAlp003CADVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat);
	virtual ~CAlp003CADVSMPAlgorithm();
	virtual bool ImportRawData(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber);

protected:
	bool Decode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex, uint64_t& nTimeStamp);
	bool FrameModeDecode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRowStart, uint32_t nRowStop, uint32_t nColStart, uint32_t nColStop, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex);
	bool EventModeDecode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRowStart, uint32_t nRowStop, uint32_t nColStart, uint32_t nColStop, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex);
	void SetData(CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, uint8_t nSubFrameIndex, uint8_t nEventFlag);
};