#include "Alp004ABDVSMPAlgorithm.h"

#define DVS_HEADER_004AB 0x0000FFFF
#define DVS_FOOTER_004AB 0x0101FFFF

CAlp004ABDVSMPAlgorithm::CAlp004ABDVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
	:CAlpDVSMPAlgorithm(Sensortype, strLogDir, nSiteNum, Pixelformat, code)
{
	m_nTotalRow = 380;
	m_nTotalCol = 672;
	m_ActiveArea = { 0, m_nTotalRow - 1, 0, m_nTotalCol - 1 };
}

CAlp004ABDVSMPAlgorithm::~CAlp004ABDVSMPAlgorithm()
{
}

bool CAlp004ABDVSMPAlgorithm::ImportRawData(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber)
{
	size_t pos = 0;
	if (m_RawDataContainer.size() < nIndexStart + nNumber)
	{
		m_RawDataContainer.resize(nIndexStart + nNumber);
	}

	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
	{
		m_RawDataContainer[nIndexStart + nIndex].Init(m_nTotalRow, m_nTotalCol, false, m_PixelFormat);
		uint64_t nTimeStamp = 0;
		if (!Decode(pBinData, &m_RawDataContainer[nIndexStart + nIndex], m_nTotalRow, m_nTotalCol, &pos, nLens, nTimeStamp))
		{
			std::string strErr = "ImportRawData: DVS Decoder error: Index: " + std::to_string(nIndex) + ", Pos: " + std::to_string(pos);
			WriteLog(strErr);
			m_nErrCode = EVS_DECODE_ERROR;
			return false;
		}
	}
	return true;
}

bool CAlp004ABDVSMPAlgorithm::ImportRawData_DropSubFrame(uint8_t *pBinData, uint64_t nLens, uint32_t nIndexStart,
    uint32_t nNumber, uint32_t &nDropSubFrameNum) {
    return false;
}

bool CAlp004ABDVSMPAlgorithm::Decode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, size_t* pnPos, size_t nBinLens, uint64_t& nTimeStamp)
{
	size_t nCurIndex = *pnPos;
	//uint32_t* HeaderCode;
	//while (nBinLens > nCurIndex + sizeof(uint32_t))
	//{
	//	HeaderCode = (uint32_t*)(pucBinData + nCurIndex);

	//	if (*HeaderCode != DVS_HEADER_004AB)
	//	{
	//		nCurIndex += 4;
	//	}
	//	else
	//	{
	//		break;
	//	}
	//}
	//if (nBinLens <= nCurIndex + sizeof(uint32_t))
	//{
	//	return false;
	//}

	//HeaderCode = (uint32_t*)(pucBinData + nCurIndex);
	//nCurIndex += sizeof(uint32_t);
	//nTimeStamp = *(uint32_t*)(pucBinData + nCurIndex);
	//nCurIndex += sizeof(uint32_t);

	bool bRet = true;

	for (uint32_t nRow = 0; nRow < m_nTotalRow; nRow++)
	{
		uint32_t nOneLineSize = (m_nTotalCol >> 2);
		if (nCurIndex + nOneLineSize > nBinLens)
		{
			bRet = false;
			break;
		}
		else
		{
			memcpy_s((void*)DVSData->m_RawData[nRow].data(), nOneLineSize, (void*)(pucBinData + nCurIndex), nOneLineSize);
			nCurIndex += nOneLineSize;
		}
	}

	if (bRet)
	{
		//uint32_t* Footer = (uint32_t*)(pucBinData + nCurIndex);

		//if (*Footer == DVS_FOOTER_004AB)
		//{
		//	nCurIndex += sizeof(uint32_t);
		//	*pnPos = nCurIndex;
		//	return true;
		//}
		*pnPos = nCurIndex;
		return true;
	}
	return false;
}
