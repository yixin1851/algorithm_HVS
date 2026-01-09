#pragma once
#include "AlpDVSMPAlgorithm.h"

typedef struct
{
	uint32_t Header_vec;
	uint32_t Timestamp_L;
	uint32_t Timestamp_H;
	uint32_t Reserve;
}Alp014AAFormatHeader;

typedef struct
{
	uint64_t Roi_row_start : 12;
	uint64_t Roi_row_stop : 12;
	uint64_t Roi_col_start : 12;
	uint64_t Roi_col_stop : 12;
	uint64_t subsample : 1;
	uint64_t binning_mode : 1;
	uint64_t high_fps : 1;
	uint64_t rd_pal : 1;
	uint64_t Subframe : 5;
	uint64_t Framemode : 1;
	uint64_t yflip : 1;
	uint64_t xflip : 1;
	uint64_t pix_mode : 4;
}Alp014AAFormatStatic;

typedef struct
{
	uint64_t Subframesize : 16;
	uint64_t Max_row_index : 9;
	uint64_t : 6;
	uint64_t Dropflag : 1;
	uint64_t Footer_vec : 32;
}Alp014AAFormatFooter;

typedef struct
{
	uint8_t Pix0 : 2;
	uint8_t Pix1 : 2;
	uint8_t Pix2 : 2;
	uint8_t Pix3 : 2;
}Alp014AAFormatEventGroup;

typedef struct
{
	uint8_t Voidbytelen : 6;
	uint8_t flag : 2;
}Alp014AAFormatVoidByte;

class CAlp014AADVSMPAlgorithm : public CAlpDVSMPAlgorithm
{
public:
	CAlp014AADVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code);
	virtual ~CAlp014AADVSMPAlgorithm();
	virtual bool ImportRawData(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber);
    virtual bool ImportRawData_DropSubFrame(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, uint32_t nMode, size_t &nDropSubFrameNum);


protected:
	bool Decode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex, uint64_t& nTimeStamp);
	bool FrameModeDecode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRowStart, uint32_t nRowStop, uint32_t nColStart, uint32_t nColStop, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex);
	bool EventModeDecode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRowStart, uint32_t nRowStop, uint32_t nColStart, uint32_t nColStop, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex);
	void SetData(CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, uint8_t nSubFrameIndex, uint8_t nEventFlag);
private:
	bool m_bHVS;
};