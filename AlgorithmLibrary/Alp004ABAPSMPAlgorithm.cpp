#include "Alp004ABAPSMPAlgorithm.h"

CAlp004ABAPSMPAlgorithm::CAlp004ABAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
	: CAlpAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, nSiteNum, Pixelformat, code)
{
	m_ActiveArea = { 0, 769, 0, 1343 };
	m_nChannelRow = 770;
	m_nChannelCol = 1344;
	m_nTotalRow = 1540;
	m_nTotalCol = 2688;
	m_AlgorithmThre.nDSNURowBlockNum = 18;
	m_AlgorithmThre.nDSNUColBlockNum = 32;
	m_AlgorithmThre.nDSNURowBlockSize = 40;
	m_AlgorithmThre.nDSNUColBlockSize = 40;
	m_AlgorithmThre.nPedestalVariationRowBlockNum = 8;
	m_AlgorithmThre.nPedestalVariationColBlockNum = 8;
	m_AlgorithmThre.nPedestalVariationRowBlockSize = 90;
	m_AlgorithmThre.nPedestalVariationColBlockSize = 160;
}

CAlp004ABAPSMPAlgorithm::~CAlp004ABAPSMPAlgorithm()
{
}

bool CAlp004ABAPSMPAlgorithm::ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer)
{
	uint32_t nOneFrameSize = 0;
	uint32_t nHeaderSize = 0;
	uint32_t nFooterSize = 0;

	uint8_t Header[8] = { 0 };
	uint8_t Footer[8] = { 0 };

	uint8_t Header_004AB[] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xe5 };
	uint8_t Footer_004AB[] = { 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xe5 };

	memcpy_s(Header, sizeof(Header), Header_004AB, sizeof(Header_004AB));
	memcpy_s(Footer, sizeof(Footer), Footer_004AB, sizeof(Footer_004AB));
	nHeaderSize = 64;
	nFooterSize = 8;

	if (m_RawType == RAW8)
	{
		nOneFrameSize = m_nTotalRow * m_nTotalCol;
	}
	else if (m_RawType == RAW10)
	{
		nOneFrameSize = m_nTotalRow * m_nTotalCol / 4 * 5;
	}
	else if (m_RawType == UNPACK10 || m_RawType == UNPACK12)
	{
		nOneFrameSize = m_nTotalRow * m_nTotalCol * 2;
	}
	else if (m_RawType == RAW12)
	{
		nOneFrameSize = m_nTotalRow * m_nTotalCol / 2 * 3;
	}
	if (bHeader_Footer)
	{
		nOneFrameSize += nHeaderSize + nFooterSize;
	}

	if ((nLens / nOneFrameSize) < nNumber)
	{
		std::string strErr = "ImportRawData: RawData buffer Lens less than frames number: Lens: " + std::to_string(nLens) + ", Number: " + std::to_string(nNumber);
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
	{
		if (m_RawDataContainer[nChannelIndex].size() < nIndexStart + nNumber)
		{
			m_RawDataContainer[nChannelIndex].resize(nIndexStart + nNumber);
		}
	}

	uint32_t nIndex = 0;

	for (uint32_t i = 0; i < nNumber; i++)
	{
		for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
		{
			CAPSDataContainer& CurContainer = m_RawDataContainer[nChannelIndex][nIndexStart + i];
			if (CurContainer.m_nRow != m_nChannelRow || CurContainer.m_nCol != m_nChannelCol)
			{
				CurContainer.Init(m_nChannelRow, m_nChannelCol);
			}
		}

		if (bHeader_Footer)
		{
			while (nIndex < nLens - sizeof(Header) && 0 != memcmp(pRawData + nIndex, Header, sizeof(Header)))
			{
				nIndex += 8;
			}

			if (nIndex >= nLens - sizeof(Header) || 0 != memcmp(pRawData + nIndex, Header, sizeof(Header)))
			{
				std::string strErr = "ImportRawData: Header Fail: Index " + std::to_string(i);
				WriteLog(strErr, SubFrameIndex::All);
				return false;
			}
			else
			{
				nIndex += nHeaderSize;
			}
		}

		uint32_t nRows = 0, nCols = 0;
		for (; nIndex < nLens;)
		{
			uint16_t tempData[4] = { 0 };
			if (m_RawType == RAW8)
			{
				tempData[0] = pRawData[nIndex];
				tempData[1] = pRawData[nIndex + 1];
				tempData[2] = pRawData[nIndex + 2];
				tempData[3] = pRawData[nIndex + 3];

				nIndex += 4;
			}
			else if (m_RawType == RAW10)
			{
				tempData[0] = ((uint16_t)(pRawData[nIndex])) << 2;
				tempData[1] = ((uint16_t)(pRawData[nIndex + 1])) << 2;
				tempData[2] = ((uint16_t)(pRawData[nIndex + 2])) << 2;
				tempData[3] = ((uint16_t)(pRawData[nIndex + 3])) << 2;

				tempData[0] += (((uint16_t)(pRawData[nIndex + 4])) & 3);
				tempData[1] += ((((uint16_t)(pRawData[nIndex + 4])) >> 2) & 3);
				tempData[2] += ((((uint16_t)(pRawData[nIndex + 4])) >> 4) & 3);
				tempData[3] += ((((uint16_t)(pRawData[nIndex + 4])) >> 6) & 3);

				nIndex += 5;
			}
			else if (m_RawType == UNPACK10 || m_RawType == UNPACK12)
			{
				tempData[0] = ((uint16_t)(pRawData[nIndex])) + ((uint16_t)(pRawData[nIndex + 1]) << 8);
				tempData[1] = ((uint16_t)(pRawData[nIndex + 2])) + ((uint16_t)(pRawData[nIndex + 3]) << 8);
				tempData[2] = ((uint16_t)(pRawData[nIndex + 4])) + ((uint16_t)(pRawData[nIndex + 5]) << 8);
				tempData[3] = ((uint16_t)(pRawData[nIndex + 6])) + ((uint16_t)(pRawData[nIndex + 7]) << 8);

				nIndex += 8;
			}
			else if (m_RawType == RAW12)
			{
				tempData[0] = ((uint16_t)(pRawData[nIndex])) << 4;
				tempData[1] = ((uint16_t)(pRawData[nIndex + 1])) << 4;
				tempData[0] += (((uint16_t)(pRawData[nIndex + 2])) & 15);
				tempData[1] += ((((uint16_t)(pRawData[nIndex + 2])) >> 4) & 15);

				tempData[2] = ((uint16_t)(pRawData[nIndex + 3])) << 4;
				tempData[3] = ((uint16_t)(pRawData[nIndex + 4])) << 4;
				tempData[2] += (((uint16_t)(pRawData[nIndex + 5])) & 15);
				tempData[3] += ((((uint16_t)(pRawData[nIndex + 5])) >> 4) & 15);

				nIndex += 6;
			}
			SetDataToSubFrame(nIndexStart + i, nRows, nCols, tempData[0]);
			SetDataToSubFrame(nIndexStart + i, nRows, nCols + 1, tempData[1]);
			SetDataToSubFrame(nIndexStart + i, nRows, nCols + 2, tempData[2]);
			SetDataToSubFrame(nIndexStart + i, nRows, nCols + 3, tempData[3]);
			nCols += 4;
			if (nCols == m_nTotalCol)
			{
				nRows += 1;
				nCols = 0;
			}
			if (nRows == m_nTotalRow)
			{
				break;
			}
		}

		if (bHeader_Footer)
		{
			if (0 != memcmp(pRawData + nIndex, Footer, sizeof(Footer)))
			{
				std::string strErr = "ImportRawData: Footer Fail: Index " + std::to_string(i);
				WriteLog(strErr, SubFrameIndex::All);
				return false;
			}
			else
			{
				nIndex += nFooterSize;
			}
		}
	}
	if (m_bUse16SubFrame)
	{
		ImportDataTo16SubFrame(nIndexStart, nNumber);
	}
	return true;
}

bool CAlp004ABAPSMPAlgorithm::BadPixelLocalToOtpType(std::vector<Local>& BadPixelLocal, std::vector<uint8_t>& OtpData)
{
	return false;
}

