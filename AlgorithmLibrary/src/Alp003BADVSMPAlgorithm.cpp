#include "Alp003BADVSMPAlgorithm.h"

CAlp003BADVSMPAlgorithm::CAlp003BADVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
	:CAlpDVSMPAlgorithm(Sensortype, strLogDir, nSiteNum, Pixelformat, code)
{
	m_nTotalRow = (1224 - 56) / 2;
	m_nTotalCol = 1632;
	m_ActiveArea = { 0, m_nTotalRow - 1, 0, m_nTotalCol - 1 };

	m_03BADVSDecoder.SetHalfMode(true);
	m_03BADVSDecoder.SetCheckSimpleFooter(true);
}

CAlp003BADVSMPAlgorithm::~CAlp003BADVSMPAlgorithm()
{
}

bool CAlp003BADVSMPAlgorithm::ImportRawData(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber)
{
	size_t pos = 0;
	if (m_RawDataContainer.size() < nIndexStart + nNumber)
	{
		m_RawDataContainer.resize(nIndexStart + nNumber);
	}

	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
	{
		uint8_t nNeedSubFrameIndex = 0;
		m_RawDataContainer[nIndexStart + nIndex].Init(m_nTotalRow, m_nTotalCol, true, m_PixelFormat);
		uint8_t nSubFrameIndex = 0;
		uint64_t nTimeStamp = 0;
		while (nNeedSubFrameIndex != 4)
		{
			if (m_03BADVSDecoder.DVS_Decode(pBinData, &m_RawDataContainer[nIndexStart + nIndex], m_nTotalRow, m_nTotalCol, &pos, nLens, nSubFrameIndex, nTimeStamp) && nSubFrameIndex == nNeedSubFrameIndex)
			{
				++nNeedSubFrameIndex;
			}
			else
			{
				std::string strErr = "ImportRawData: DVS Decoder error: Index: " + std::to_string(nIndex) + ", Pos: " + std::to_string(pos);
				WriteLog(strErr);
				m_nErrCode = EVS_DECODE_ERROR;
				return false;
			}
		}
	}
	return true;
}

bool CAlp003BADVSMPAlgorithm::ImportRawData_DropSubFrame(uint8_t *pBinData, uint64_t nLens, uint32_t nIndexStart,
    uint32_t nNumber, uint32_t nMode, size_t &nDropSubFrameNum) {
    return false;
}
