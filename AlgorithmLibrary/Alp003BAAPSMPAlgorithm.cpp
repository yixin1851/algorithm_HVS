#include "Alp003BAAPSMPAlgorithm.h"

CAlp003BAAPSMPAlgorithm::CAlp003BAAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat)
	: CAlpAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, nSiteNum, Pixelformat)
{
	m_ActiveArea = { 0, 1169, 0, 1631 };
	m_nChannelRow = 1170;
	m_nChannelCol = 1632;
	m_nTotalRow = 2340;
	m_nTotalCol = 3264;
	m_AlgorithmThre.nDSNURowBlockNum = 58;
	m_AlgorithmThre.nDSNUColBlockNum = 80;
}

CAlp003BAAPSMPAlgorithm::~CAlp003BAAPSMPAlgorithm()
{
}

bool CAlp003BAAPSMPAlgorithm::ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer)
{
	uint32_t nOneFrameSize = 0;
	uint32_t nHeaderSize = 0;
	uint32_t nFooterSize = 0;

	uint8_t Header[8] = { 0 };
	uint8_t Footer[8] = { 0 };

	uint8_t Header_003BA[] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xe5 };
	uint8_t Footer_003BA[] = { 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xe5 };

	memcpy_s(Header, sizeof(Header), Header_003BA, sizeof(Header_003BA));
	memcpy_s(Footer, sizeof(Footer), Footer_003BA, sizeof(Footer_003BA));
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
	else if(m_RawType == RAW12)
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
	return true;
}

bool CAlp003BAAPSMPAlgorithm::BadPixelLocalToOtpType(std::vector<Local>& BadPixelLocal, std::vector<uint8_t>& OtpData)
{
	return false;
}

