#include "AlpAPSMPAlgorithm.h"
#include <thread>
#include <algorithm>
#include <stack>
#include <map>
#include <fstream>
#include "Changelist.h"

CAlpAPSMPAlgorithm::CAlpAPSMPAlgorithm(SensorType Sensortype, RawType Rawtype, std::string strLogDir, uint32_t nSiteNum)
{
	m_nSiteNum = nSiteNum;
	m_bMultiThreadEnable = false;
	m_bLogEnable = false;
	m_SensorType = Sensortype;
	m_RawType = Rawtype;
	m_AlgorithmThre.dBadPixelThre = 0.19;
	m_AlgorithmThre.dDeadLineThre = 0.5;
	m_AlgorithmThre.dDeadPixelThre = 0.8;
	m_AlgorithmThre.dHotLineThre = 0.5;
	m_AlgorithmThre.dHotPixelThre = 120;
	m_AlgorithmThre.nBadPixelRadius = 2;
	m_AlgorithmThre.nOpticalFindRadius = 100;
	m_AlgorithmThre.nShadingTestRadius = 10;
	m_AlgorithmThre.nDSNUBlockSize = 10;
	m_RawDataContainer.resize(APSSubFrameIndex::SubFrameNum);

	if (Sensortype == SensorType::ALP_003AA)
	{
		m_ActiveArea = { 5, 620, 4, 799 };
		m_nChannelRow = 622;
		m_nChannelCol = 828;
		m_nTotalRow = 2488;
		m_nTotalCol = 1656;
	}
	else if (Sensortype == SensorType::ALP_003BA)
	{
#if 0
		m_ActiveArea = { 27, 611, 0, 815 };
		m_nChannelRow = 612;
		m_nChannelCol = 816;
		m_nTotalRow = 2448;
		m_nTotalCol = 1632;
#else
		m_ActiveArea = { 0, 584, 0, 815 };
		m_nChannelRow = 585;
		m_nChannelCol = 816;
		m_nTotalRow = 2340;
		m_nTotalCol = 1632;
#endif
	}

	if (strLogDir != "")
	{
		std::time_t t = std::time(nullptr);
		std::tm now;
		localtime_s(&now, &t);
		char str_time[100] = { 0 };
		strftime(str_time, sizeof(str_time), "%Y_%m_%d_%H_%M_%S", &now);
		m_strLogFilePath = strLogDir + "\\" + str_time + "_Site" + std::to_string(m_nSiteNum) + "_APS_Test.log";
	}
	else
	{
		m_strLogFilePath = "";
	}
}

CAlpAPSMPAlgorithm::~CAlpAPSMPAlgorithm()
{
}

bool CAlpAPSMPAlgorithm::ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer)
{
	uint32_t nOneFrameSize = 0;
	uint32_t nHeaderSize = 0;
	uint32_t nFooterSize = 0;

	uint8_t Header[8] = { 0 };
	uint8_t Footer[8] = { 0 };

	uint8_t Header_003AA_8Bit[] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf4 };
	uint8_t Footer_003AA_8Bit[] = { 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf4 };

	uint8_t Header_003AA_10Bit[] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf7 };
	uint8_t Footer_003AA_10Bit[] = { 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf7 };

	uint8_t Header_003BA[] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xe5 };
	uint8_t Footer_003BA[] = { 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xe5 };


	if (m_SensorType == ALP_003BA)
	{
		memcpy_s(Header, sizeof(Header), Header_003BA, sizeof(Header_003BA));
		memcpy_s(Footer, sizeof(Footer), Footer_003BA, sizeof(Footer_003BA));
		nHeaderSize = 64;
		nFooterSize = 8;
	}
	else if (m_SensorType == ALP_003AA)
	{
		if (m_RawType == RAW8)
		{
			memcpy_s(Header, sizeof(Header), Header_003AA_8Bit, sizeof(Header_003AA_8Bit));
			memcpy_s(Footer, sizeof(Footer), Footer_003AA_8Bit, sizeof(Footer_003AA_8Bit));
		}
		else if (m_RawType == RAW10)
		{
			memcpy_s(Header, sizeof(Header), Header_003AA_10Bit, sizeof(Header_003AA_10Bit));
			memcpy_s(Footer, sizeof(Footer), Footer_003AA_10Bit, sizeof(Footer_003AA_10Bit));
		}
		nHeaderSize = 32;
		nFooterSize = 8;
	}

	if (m_RawType == RAW8)
	{
		nOneFrameSize = m_nTotalRow * m_nTotalCol;
	}
	else if (m_RawType == RAW10)
	{
		nOneFrameSize = m_nTotalRow * m_nTotalCol / 4 * 5;
	}
	else
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
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return false;
	}
	for (uint32_t nChannelIndex = 0; nChannelIndex < APSSubFrameIndex::SubFrameNum; nChannelIndex++)
	{
		if (m_RawDataContainer[nChannelIndex].size() < nIndexStart + nNumber)
		{
			m_RawDataContainer[nChannelIndex].resize(nIndexStart + nNumber);
		}
	}

	uint32_t nIndex = 0;

	for (uint32_t i = 0; i < nNumber; i++)
	{
		for (uint32_t nChannelIndex = 0; nChannelIndex < APSSubFrameIndex::SubFrameNum; nChannelIndex++)
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
				WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
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
				if (m_SensorType == ALP_003BA)
				{
					tempData[0] = ((uint16_t)(pRawData[nIndex])) << 2;
					tempData[1] = ((uint16_t)(pRawData[nIndex + 1])) << 2;
					tempData[2] = ((uint16_t)(pRawData[nIndex + 2])) << 2;
					tempData[3] = ((uint16_t)(pRawData[nIndex + 3])) << 2;

					tempData[0] += (((uint16_t)(pRawData[nIndex + 4])) & 3);
					tempData[1] += ((((uint16_t)(pRawData[nIndex + 4])) >> 2) & 3);
					tempData[2] += ((((uint16_t)(pRawData[nIndex + 4])) >> 4) & 3);
					tempData[3] += ((((uint16_t)(pRawData[nIndex + 4])) >> 6) & 3);
				}
				else if(m_SensorType == ALP_003AA)
				{
					tempData[0] = ((uint16_t)(pRawData[nIndex])) + (((uint16_t)(pRawData[nIndex + 1]) & 0x3) << 8);
					tempData[1] = (((uint16_t)(pRawData[nIndex + 1])) >> 2) + (((uint16_t)(pRawData[nIndex + 2]) & 0x0F) << 6);
					tempData[2] = (((uint16_t)(pRawData[nIndex + 2])) >> 4) + (((uint16_t)(pRawData[nIndex + 3]) & 0x3F) << 4);
					tempData[3] = (((uint16_t)(pRawData[nIndex + 3])) >> 6) + (((uint16_t)(pRawData[nIndex + 4])) << 2);
				}

				nIndex += 5;
			}
			else
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
			switch (nRows % 4)
			{
			case 0:
				m_RawDataContainer[Gb1][nIndexStart + i].m_RawData[nRows / 4][nCols / 2] = tempData[0];
				m_RawDataContainer[B1][nIndexStart + i].m_RawData[nRows / 4][nCols / 2] = tempData[1];
				m_RawDataContainer[Gb1][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[2];
				m_RawDataContainer[B1][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[3];
				break;
			case 1:
				if (m_SensorType == ALP_003BA)
				{
					m_RawDataContainer[B2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2] = tempData[0];
					m_RawDataContainer[Gb2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[1];
					m_RawDataContainer[B2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[2];
					if (nCols != m_nTotalCol - 4)
					{
						m_RawDataContainer[Gb2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 2] = tempData[3];
					}
					else
					{
						//m_RawDataContainer[Gb2][nIndexStart + i].m_RawData[nRows / 4][0] = tempData[3];
						m_RawDataContainer[Gb2][nIndexStart + i].m_RawData[nRows / 4][0] = m_RawDataContainer[Gb1][nIndexStart + i].m_RawData[nRows / 4][0];
					}
				}
				else
				{
					m_RawDataContainer[Gb2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2] = tempData[0];
					m_RawDataContainer[B2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2] = tempData[1];
					m_RawDataContainer[Gb2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[2];
					m_RawDataContainer[B2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[3];
				}
				break;
			case 2:
				m_RawDataContainer[R1][nIndexStart + i].m_RawData[nRows / 4][nCols / 2] = tempData[0];
				m_RawDataContainer[Gr1][nIndexStart + i].m_RawData[nRows / 4][nCols / 2] = tempData[1];
				m_RawDataContainer[R1][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[2];
				m_RawDataContainer[Gr1][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[3];
				break;
			case 3:
				if (m_SensorType == ALP_003BA)
				{
					m_RawDataContainer[Gr2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2] = tempData[0];
					m_RawDataContainer[R2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[1];
					m_RawDataContainer[Gr2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[2];
					if (nCols != m_nTotalCol - 4)
					{
						m_RawDataContainer[R2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 2] = tempData[3];
					}
					else
					{
						//m_RawDataContainer[R2][nIndexStart + i].m_RawData[nRows / 4][0] = tempData[3];
						m_RawDataContainer[R2][nIndexStart + i].m_RawData[nRows / 4][0] = m_RawDataContainer[R1][nIndexStart + i].m_RawData[nRows / 4][0];
					}
				}
				else
				{
					m_RawDataContainer[R2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2] = tempData[0];
					m_RawDataContainer[Gr2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2] = tempData[1];
					m_RawDataContainer[R2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[2];
					m_RawDataContainer[Gr2][nIndexStart + i].m_RawData[nRows / 4][nCols / 2 + 1] = tempData[3];
				}
				break;
			}
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
				WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
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

bool CAlpAPSMPAlgorithm::ImportRawData(uint8_t* pRawData, uint64_t nLens, APSSubFrameIndex nChannelIndex, uint32_t nIndexStart, uint32_t nNumber)
{
	uint32_t nRawDataByte = 1;
	if (m_RawType == RAW8)
		nRawDataByte = 1;
	else
		nRawDataByte = 2;
	if ((nLens / nRawDataByte / m_nChannelRow / m_nChannelCol) < nNumber)
	{
		std::string strErr = "ImportRawData: RawData buffer Lens less than frames number: Lens: " + std::to_string(nLens) + ", Number: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		return false;
	}
	if (m_RawDataContainer[nChannelIndex].size() < nIndexStart + nNumber)
	{
		m_RawDataContainer[nChannelIndex].resize(nIndexStart + nNumber);
	}
	uint64_t nIndex = 0;
	for (uint32_t i = 0; i < nNumber; i++)
	{
		CAPSDataContainer& CurContainer = m_RawDataContainer[nChannelIndex][nIndexStart + i];
		if (CurContainer.m_nRow != m_nChannelRow || CurContainer.m_nCol != m_nChannelCol)
		{
			CurContainer.Init(m_nChannelRow, m_nChannelCol);
		}
		for (uint32_t rows = 0; rows < m_nChannelRow; rows++)
		{
			for (uint32_t cols = 0; cols < m_nChannelCol; cols++)
			{
				if (m_RawType == RAW8)
					CurContainer.m_RawData[rows][cols] = pRawData[nIndex];
				else
					CurContainer.m_RawData[rows][cols] = (pRawData[nIndex + 1] << 8) + pRawData[nIndex];
				nIndex += nRawDataByte;
			}
		}
	}
	return true;
}

bool CAlpAPSMPAlgorithm::TNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& TNoiseData)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];
	TNoiseData.resize(APSSubFrameIndex::SubFrameNum);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameTNoise, this, nIndexStart, nNumber, ROI, APSSubFrameIndex(i), std::ref(TNoiseData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameTNoise(nIndexStart, nNumber, ROI, APSSubFrameIndex(i), TNoiseData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::SNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& SNoiseData)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];
	SNoiseData.resize(APSSubFrameIndex::SubFrameNum);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameSNoise, this, nIndexStart, nNumber, ROI, APSSubFrameIndex(i), std::ref(SNoiseData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameSNoise(nIndexStart, nNumber, ROI, APSSubFrameIndex(i), SNoiseData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::RowTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& RowTNoiseData)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];
	RowTNoiseData.resize(APSSubFrameIndex::SubFrameNum);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameRowTNoise, this, nIndexStart, nNumber, ROI, APSSubFrameIndex(i), std::ref(RowTNoiseData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameRowTNoise(nIndexStart, nNumber, ROI, APSSubFrameIndex(i), RowTNoiseData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::ColTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& ColTNoiseData)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];
	ColTNoiseData.resize(APSSubFrameIndex::SubFrameNum);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameColTNoise, this, nIndexStart, nNumber, ROI, APSSubFrameIndex(i), std::ref(ColTNoiseData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameColTNoise(nIndexStart, nNumber, ROI, APSSubFrameIndex(i), ColTNoiseData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::RowSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& RowSNoiseData)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];
	RowSNoiseData.resize(APSSubFrameIndex::SubFrameNum);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameRowSNoise, this, nIndexStart, nNumber, ROI, APSSubFrameIndex(i), std::ref(RowSNoiseData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameRowSNoise(nIndexStart, nNumber, ROI, APSSubFrameIndex(i), RowSNoiseData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::ColSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& ColSNoiseData)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];
	ColSNoiseData.resize(APSSubFrameIndex::SubFrameNum);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameColSNoise, this, nIndexStart, nNumber, ROI, APSSubFrameIndex(i), std::ref(ColSNoiseData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameColSNoise(nIndexStart, nNumber, ROI, APSSubFrameIndex(i), ColSNoiseData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::BadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<BadpixelData>& BadpixelRes)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];
	BadpixelRes.resize(APSSubFrameIndex::SubFrameNum);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameBadPixel, this, nIndexStart, nNumber, ROI, APSSubFrameIndex(i), std::ref(BadpixelRes[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameBadPixel(nIndexStart, nNumber, ROI, APSSubFrameIndex(i), BadpixelRes[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::HotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<HotpixelData>& HotpixelRes)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];
	HotpixelRes.resize(APSSubFrameIndex::SubFrameNum);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameHotPixel, this, nIndexStart, nNumber, ROI, APSSubFrameIndex(i), std::ref(HotpixelRes[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameHotPixel(nIndexStart, nNumber, ROI, APSSubFrameIndex(i), HotpixelRes[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::BLC(uint32_t nIndexStart, uint32_t nNumber, std::vector<double>& BaseMean)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];

	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameBLC, this, nIndexStart, nNumber, nullptr, APSSubFrameIndex(i), std::ref(BaseMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameBLC(nIndexStart, nNumber, nullptr, APSSubFrameIndex(i), BaseMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::BLC(uint32_t nIndexStart, uint32_t nNumber, uint32_t nBaseIndexStart, uint32_t nBaseNumber)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];
	std::vector<double> ColMean[APSSubFrameIndex::SubFrameNum];

	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameColMean, this, nBaseIndexStart, nBaseNumber, nullptr, APSSubFrameIndex(i), std::ref(ColMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameColMean(nBaseIndexStart, nBaseNumber, nullptr, APSSubFrameIndex(i), ColMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}

	if (!bRet)
	{
		return false;
	}

	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameBLCByColBase, this, nIndexStart, nNumber, nullptr, APSSubFrameIndex(i), std::ref(ColMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameBLCByColBase(nIndexStart, nNumber, nullptr, APSSubFrameIndex(i), ColMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::DPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<BadPixelMaskData>& BadPixelMask)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];

	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameDPC, this, nIndexStart, nNumber, ROI, APSSubFrameIndex(i), std::ref(BadPixelMask[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameDPC(nIndexStart, nNumber, ROI, APSSubFrameIndex(i), BadPixelMask[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::Shading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, ShadingData& ShadingRes)
{
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	for (uint32_t nChannelIndex = 0; nChannelIndex < APSSubFrameIndex::SubFrameNum; nChannelIndex++)
	{
		if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
		{
			std::string strErr = "Shading: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
			WriteLog(strErr, nChannelIndex);
			return false;
		}
	}

	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "Shading: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return false;
	}

	uint32_t AACenterRow = (RealRoi.Up + RealRoi.Down) / 2;
	uint32_t AACenterCol = (RealRoi.Left + RealRoi.Right) / 2;
	ROIArea OpticalFindArea = { AACenterRow - m_AlgorithmThre.nOpticalFindRadius + 1, AACenterRow + m_AlgorithmThre.nOpticalFindRadius, \
								AACenterCol - m_AlgorithmThre.nOpticalFindRadius + 1, AACenterCol + m_AlgorithmThre.nOpticalFindRadius };

	if (OpticalFindArea.Down >= m_nChannelRow || OpticalFindArea.Right >= m_nChannelCol || OpticalFindArea.Down < OpticalFindArea.Up || OpticalFindArea.Right < OpticalFindArea.Left)
	{
		std::string strErr = "Shading: Optical Find ROI error: ROI: " + std::to_string(OpticalFindArea.Up) + ", " + std::to_string(OpticalFindArea.Down) + ", " + std::to_string(OpticalFindArea.Left) + ", " + std::to_string(OpticalFindArea.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return false;
	}

	std::vector<double> RowMean(2 * m_AlgorithmThre.nOpticalFindRadius, 0);
	std::vector<double> ColMean(2 * m_AlgorithmThre.nOpticalFindRadius, 0);

	for (uint32_t nRows = OpticalFindArea.Up; nRows <= OpticalFindArea.Down; nRows++)
	{
		for (uint32_t nCols = OpticalFindArea.Left; nCols <= OpticalFindArea.Right; nCols++)
		{
			double dMeanData = 0;
			for (uint32_t nFrameIndex = 0; nFrameIndex < nNumber; nFrameIndex++)
			{
				for (uint32_t nChannelIndex = 0; nChannelIndex < APSSubFrameIndex::SubFrameNum; nChannelIndex++)
				{
					dMeanData += m_RawDataContainer[nChannelIndex][nIndexStart + nFrameIndex].m_RawData[nRows][nCols];
				}
			}
			dMeanData /= (nNumber * APSSubFrameIndex::SubFrameNum);
			RowMean[nRows - OpticalFindArea.Up] += dMeanData;
			ColMean[nCols - OpticalFindArea.Left] += dMeanData;
		}
	}

	double dMaxValue = 0;
	Max(dMaxValue, ShadingRes.CenterRow, RowMean, RowMean.size());
	ShadingRes.CenterRow += OpticalFindArea.Up;
	Max(dMaxValue, ShadingRes.CenterCol, ColMean, ColMean.size());
	ShadingRes.CenterCol += OpticalFindArea.Left;

	ROIArea RoiOC = { ShadingRes.CenterRow - m_AlgorithmThre.nShadingTestRadius + 1, ShadingRes.CenterRow + m_AlgorithmThre.nShadingTestRadius, \
					ShadingRes.CenterCol - m_AlgorithmThre.nShadingTestRadius + 1, ShadingRes.CenterCol + m_AlgorithmThre.nShadingTestRadius };

	ROIArea RoiLT = { RealRoi.Up, RealRoi.Up + 2 * m_AlgorithmThre.nShadingTestRadius - 1, RealRoi.Left, RealRoi.Left + 2 * m_AlgorithmThre.nShadingTestRadius - 1 };
	ROIArea RoiLB = { RealRoi.Down - 2 * m_AlgorithmThre.nShadingTestRadius + 1, RealRoi.Down, RealRoi.Left, RealRoi.Left + 2 * m_AlgorithmThre.nShadingTestRadius - 1 };
	ROIArea RoiRT = { RealRoi.Up, RealRoi.Up + 2 * m_AlgorithmThre.nShadingTestRadius - 1, RealRoi.Right - 2 * m_AlgorithmThre.nShadingTestRadius + 1, RealRoi.Right };
	ROIArea RoiRB = { RealRoi.Down - 2 * m_AlgorithmThre.nShadingTestRadius + 1, RealRoi.Down, RealRoi.Right - 2 * m_AlgorithmThre.nShadingTestRadius + 1, RealRoi.Right };

	std::vector<double>CenterMean(APSSubFrameIndex::SubFrameNum);
	std::vector<double>LTMean(APSSubFrameIndex::SubFrameNum);
	std::vector<double>LBMean(APSSubFrameIndex::SubFrameNum);
	std::vector<double>RTMean(APSSubFrameIndex::SubFrameNum);
	std::vector<double>RBMean(APSSubFrameIndex::SubFrameNum);

	ShadingRes.LumaShadingLT.resize(APSSubFrameIndex::SubFrameNum);
	ShadingRes.LumaShadingLB.resize(APSSubFrameIndex::SubFrameNum);
	ShadingRes.LumaShadingRT.resize(APSSubFrameIndex::SubFrameNum);
	ShadingRes.LumaShadingRB.resize(APSSubFrameIndex::SubFrameNum);

	for (uint32_t nChannelIndex = 0; nChannelIndex < APSSubFrameIndex::SubFrameNum; nChannelIndex++)
	{
		bool bRes = false;
		SubFrameDataMean(nIndexStart, nNumber, &RoiOC, (APSSubFrameIndex)nChannelIndex, CenterMean[nChannelIndex], bRes);
		if (!bRes)
		{
			return false;
		}
		SubFrameDataMean(nIndexStart, nNumber, &RoiLT, (APSSubFrameIndex)nChannelIndex, LTMean[nChannelIndex], bRes);
		if (!bRes)
		{
			return false;
		}
		SubFrameDataMean(nIndexStart, nNumber, &RoiLB, (APSSubFrameIndex)nChannelIndex, LBMean[nChannelIndex], bRes);
		if (!bRes)
		{
			return false;
		}
		SubFrameDataMean(nIndexStart, nNumber, &RoiRT, (APSSubFrameIndex)nChannelIndex, RTMean[nChannelIndex], bRes);
		if (!bRes)
		{
			return false;
		}
		SubFrameDataMean(nIndexStart, nNumber, &RoiRB, (APSSubFrameIndex)nChannelIndex, RBMean[nChannelIndex], bRes);
		if (!bRes)
		{
			return false;
		}
		ShadingRes.LumaShadingLT[nChannelIndex] = LTMean[nChannelIndex] / CenterMean[nChannelIndex] * 100;
		ShadingRes.LumaShadingRT[nChannelIndex] = RTMean[nChannelIndex] / CenterMean[nChannelIndex] * 100;
		ShadingRes.LumaShadingLB[nChannelIndex] = LBMean[nChannelIndex] / CenterMean[nChannelIndex] * 100;
		ShadingRes.LumaShadingRB[nChannelIndex] = RBMean[nChannelIndex] / CenterMean[nChannelIndex] * 100;
	}
	ShadingRes.R_Gb_Ratio = (CenterMean[APSSubFrameIndex::R1] + CenterMean[APSSubFrameIndex::R2]) / (CenterMean[APSSubFrameIndex::Gb1] + CenterMean[APSSubFrameIndex::Gb2]) * 100;
	ShadingRes.B_Gb_Ratio = (CenterMean[APSSubFrameIndex::B1] + CenterMean[APSSubFrameIndex::B2]) / (CenterMean[APSSubFrameIndex::Gb1] + CenterMean[APSSubFrameIndex::Gb2]) * 100;
	ShadingRes.Gr_Gb_Ratio = (CenterMean[APSSubFrameIndex::Gr1] + CenterMean[APSSubFrameIndex::Gr2]) / (CenterMean[APSSubFrameIndex::Gb1] + CenterMean[APSSubFrameIndex::Gb2]) * 100;
	return true;
}

bool CAlpAPSMPAlgorithm::DarkCurrent(std::vector<std::vector<double>>& Data, std::vector<double>& ExpTime, bool bUseMeanFunc, std::vector<double>& DarkCurrentRes)
{
	if (Data.size() != ExpTime.size())
	{
		std::string strErr = "DarkCurrent: Size Error: Data Size: " + std::to_string(Data.size()) + ", ExpTime Size: " + std::to_string(ExpTime.size());
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return false;
	}
	DarkCurrentRes.resize(APSSubFrameIndex::SubFrameNum);
	for (uint32_t nChannelIndex = 0; nChannelIndex < APSSubFrameIndex::SubFrameNum; nChannelIndex++)
	{
		uint32_t nDataNum = Data.size();
		std::vector<double> XData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			XData[nIndex] = ExpTime[nIndex];
		}
		std::vector<double> YData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			if (bUseMeanFunc)
			{
				YData[nIndex] = Data[nIndex][nChannelIndex];
			}
			else
			{
				YData[nIndex] = Data[nIndex][nChannelIndex] * Data[nIndex][nChannelIndex];
			}
		}
		double k = 0.0, b = 0.0;
		if (LinearityFit(XData, YData, k, b))
		{
			DarkCurrentRes[nChannelIndex] = k * 1000;
		}
		else
		{
			std::string strErr = "DarkCurrent: LinearityFit Error";
			WriteLog(strErr, nChannelIndex);
			return false;
		}
	}

	return true;
}

bool CAlpAPSMPAlgorithm::DSNU(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, DSNUData& DSNURes)
{
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	for (uint32_t nChannelIndex = 0; nChannelIndex < APSSubFrameIndex::SubFrameNum; nChannelIndex++)
	{
		if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
		{
			std::string strErr = "DSNU: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
			WriteLog(strErr, nChannelIndex);
			return false;
		}
	}

	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "DSNU: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return false;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;
	uint32_t nRowBlockNum = nRow / m_AlgorithmThre.nDSNUBlockSize;
	uint32_t nColBlockNum = nCol / m_AlgorithmThre.nDSNUBlockSize;
	std::vector<CAPSDataContainer>BlockData(APSSubFrameIndex::SubFrameNum);

	if (!GetBlockMean(nIndexStart, nNumber, &RealRoi, m_AlgorithmThre.nDSNUBlockSize, BlockData))
	{
		std::string strErr = "DSNU: GetBlockMean error";
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return false;
	}

	CAPSDataContainer R;
	R.Init(nRowBlockNum, nColBlockNum);
	CAPSDataContainer G;
	G.Init(nRowBlockNum, nColBlockNum);
	CAPSDataContainer B;
	B.Init(nRowBlockNum, nColBlockNum);

	double dPedestal = 0;

	for (uint32_t nRowBlocks = 0; nRowBlocks < nRowBlockNum; nRowBlocks++)
	{
		for (uint32_t nColBlocks = 0; nColBlocks < nColBlockNum; nColBlocks++)
		{
			R.m_RawData[nRowBlocks][nColBlocks] = (BlockData[R1].m_RawData[nRowBlocks][nColBlocks] + BlockData[R2].m_RawData[nRowBlocks][nColBlocks]) / 2;
			G.m_RawData[nRowBlocks][nColBlocks] = (BlockData[Gr1].m_RawData[nRowBlocks][nColBlocks] + BlockData[Gr2].m_RawData[nRowBlocks][nColBlocks] + BlockData[Gb1].m_RawData[nRowBlocks][nColBlocks] + BlockData[Gb2].m_RawData[nRowBlocks][nColBlocks]) / 4;
			B.m_RawData[nRowBlocks][nColBlocks] = (BlockData[B1].m_RawData[nRowBlocks][nColBlocks] + BlockData[B2].m_RawData[nRowBlocks][nColBlocks]) / 2;
			dPedestal += (R.m_RawData[nRowBlocks][nColBlocks] + G.m_RawData[nRowBlocks][nColBlocks] + B.m_RawData[nRowBlocks][nColBlocks]) / 3;
		}
	}
	dPedestal /= nRowBlockNum * nColBlockNum;

	double MaxR = R.m_RawData[0][0], MinR = R.m_RawData[0][0], MaxG = G.m_RawData[0][0], MinG = G.m_RawData[0][0], MaxB = B.m_RawData[0][0], MinB = B.m_RawData[0][0], MaxSignal = 0;

	ROIArea ROICorner[4] = { { 0, 1, 0, 1 }, { 0, 1, nColBlockNum - 2, nColBlockNum - 1 }, { nRowBlockNum - 2, nRowBlockNum - 1, 0, 1 }, { nRowBlockNum - 2, nRowBlockNum - 1, nColBlockNum - 2, nColBlockNum - 1 } };
	ROIArea ROIEdge[4] = { { 0, 1, 2, nColBlockNum - 3 }, { nRowBlockNum - 2, nRowBlockNum - 1, 2, nColBlockNum - 3 }, {2, nRowBlockNum - 3, 0, 1 }, {2, nRowBlockNum - 3, nColBlockNum - 2, nColBlockNum - 1 }, };
	ROIArea ROICenter = { 2, nRowBlockNum - 3, 2, nColBlockNum - 3 };

	double dMinCornerR = R.m_RawData[0][0], dMinCornerG = G.m_RawData[0][0], dMinCornerB = B.m_RawData[0][0];
	double dMinEdgeR = R.m_RawData[0][2], dMinEdgeG = G.m_RawData[0][2], dMinEdgeB = B.m_RawData[0][2];
	double dMinCenterR = R.m_RawData[2][2], dMinCenterG = G.m_RawData[2][2], dMinCenterB = B.m_RawData[2][2];

	for (uint32_t nRowBlockIndex = 0; nRowBlockIndex < nRowBlockNum; nRowBlockIndex++)
	{
		for (uint32_t nColBlockIndex = 0; nColBlockIndex < nColBlockNum; nColBlockIndex++)
		{
			if (R.m_RawData[nRowBlockIndex][nColBlockIndex] > MaxR)
				MaxR = R.m_RawData[nRowBlockIndex][nColBlockIndex];
			if (R.m_RawData[nRowBlockIndex][nColBlockIndex] < MinR)
				MinR = R.m_RawData[nRowBlockIndex][nColBlockIndex];
			if (G.m_RawData[nRowBlockIndex][nColBlockIndex] > MaxG)
				MaxG = G.m_RawData[nRowBlockIndex][nColBlockIndex];
			if (G.m_RawData[nRowBlockIndex][nColBlockIndex] < MinG)
				MinG = G.m_RawData[nRowBlockIndex][nColBlockIndex];
			if (B.m_RawData[nRowBlockIndex][nColBlockIndex] > MaxB)
				MaxB = B.m_RawData[nRowBlockIndex][nColBlockIndex];
			if (B.m_RawData[nRowBlockIndex][nColBlockIndex] < MinB)
				MinB = B.m_RawData[nRowBlockIndex][nColBlockIndex];
			double dSignal = sqrt(pow(R.m_RawData[nRowBlockIndex][nColBlockIndex] - dPedestal, 2) + pow(G.m_RawData[nRowBlockIndex][nColBlockIndex] - dPedestal, 2) + pow(B.m_RawData[nRowBlockIndex][nColBlockIndex] - dPedestal, 2));
			if (dSignal > MaxSignal)
				MaxSignal = dSignal;

			if (PosInRoi(nRowBlockIndex, nColBlockIndex, ROICenter))
			{
				if (R.m_RawData[nRowBlockIndex][nColBlockIndex] < dMinCenterR)
					dMinCenterR = R.m_RawData[nRowBlockIndex][nColBlockIndex];
				if (G.m_RawData[nRowBlockIndex][nColBlockIndex] < dMinCenterG)
					dMinCenterG = G.m_RawData[nRowBlockIndex][nColBlockIndex];
				if (B.m_RawData[nRowBlockIndex][nColBlockIndex] < dMinCenterB)
					dMinCenterB = B.m_RawData[nRowBlockIndex][nColBlockIndex];
			}
			else if (PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[0]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[1]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[2]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[3]))
			{
				if (R.m_RawData[nRowBlockIndex][nColBlockIndex] < dMinCornerR)
					dMinCornerR = R.m_RawData[nRowBlockIndex][nColBlockIndex];
				if (G.m_RawData[nRowBlockIndex][nColBlockIndex] < dMinCornerG)
					dMinCornerG = G.m_RawData[nRowBlockIndex][nColBlockIndex];
				if (B.m_RawData[nRowBlockIndex][nColBlockIndex] < dMinCornerB)
					dMinCornerB = B.m_RawData[nRowBlockIndex][nColBlockIndex];
			}
			else
			{
				if (R.m_RawData[nRowBlockIndex][nColBlockIndex] < dMinEdgeR)
					dMinEdgeR = R.m_RawData[nRowBlockIndex][nColBlockIndex];
				if (G.m_RawData[nRowBlockIndex][nColBlockIndex] < dMinEdgeG)
					dMinEdgeG = G.m_RawData[nRowBlockIndex][nColBlockIndex];
				if (B.m_RawData[nRowBlockIndex][nColBlockIndex] < dMinEdgeB)
					dMinEdgeB = B.m_RawData[nRowBlockIndex][nColBlockIndex];
			}
		}
	}

	DSNURes.RangeR = MaxR - MinR;
	DSNURes.RangeG = MaxG - MinG;
	DSNURes.RangeB = MaxB - MinB;
	DSNURes.SignalMax = MaxSignal;

	double dDeltaSignal = 0;
	DSNURes.DeltaSignalCentreMax = 0;
	DSNURes.DeltaSignalCornerMax = 0;
	DSNURes.DeltaSignalEdgeMax = 0;
	DSNURes.DeltaSignalMax = 0;

	for (uint32_t nRowBlockIndex = 0; nRowBlockIndex < nRowBlockNum; nRowBlockIndex++)
	{
		for (uint32_t nColBlockIndex = 0; nColBlockIndex < nColBlockNum; nColBlockIndex++)
		{
			if (PosInRoi(nRowBlockIndex, nColBlockIndex, ROICenter))
			{
				dDeltaSignal = sqrt(pow(R.m_RawData[nRowBlockIndex][nColBlockIndex] - dMinCenterR, 2) + pow(G.m_RawData[nRowBlockIndex][nColBlockIndex] - dMinCenterG, 2) + pow(B.m_RawData[nRowBlockIndex][nColBlockIndex] - dMinCenterB, 2));
				if (dDeltaSignal > DSNURes.DeltaSignalCentreMax)
				{
					DSNURes.DeltaSignalCentreMax = dDeltaSignal;
				}
			}
			else if (PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[0]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[1]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[2]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[3]))
			{
				dDeltaSignal = sqrt(pow(R.m_RawData[nRowBlockIndex][nColBlockIndex] - dMinCornerR, 2) + pow(G.m_RawData[nRowBlockIndex][nColBlockIndex] - dMinCornerG, 2) + pow(B.m_RawData[nRowBlockIndex][nColBlockIndex] - dMinCornerB, 2));
				if (dDeltaSignal > DSNURes.DeltaSignalCornerMax)
				{
					DSNURes.DeltaSignalCornerMax = dDeltaSignal;
				}
			}
			else
			{
				dDeltaSignal = sqrt(pow(R.m_RawData[nRowBlockIndex][nColBlockIndex] - dMinEdgeR, 2) + pow(G.m_RawData[nRowBlockIndex][nColBlockIndex] - dMinEdgeG, 2) + pow(B.m_RawData[nRowBlockIndex][nColBlockIndex] - dMinEdgeB, 2));
				if (dDeltaSignal > DSNURes.DeltaSignalEdgeMax)
				{
					DSNURes.DeltaSignalEdgeMax = dDeltaSignal;
				}
			}
			if (dDeltaSignal > DSNURes.DeltaSignalMax)
			{
				DSNURes.DeltaSignalMax = dDeltaSignal;
			}
		}
	}
	return true;
}

bool CAlpAPSMPAlgorithm::DataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<double>& DataMean)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];
	DataMean.resize(APSSubFrameIndex::SubFrameNum);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameDataMean, this, nIndexStart, nNumber, ROI, APSSubFrameIndex(i), std::ref(DataMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameDataMean(nIndexStart, nNumber, ROI, APSSubFrameIndex(i), DataMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::Linearity(std::vector<std::vector<double>>& LightMean, std::vector<double>& ExpTime, std::vector<LinearityData>& LinearityRes)
{
	if (LightMean.size() != ExpTime.size())
	{
		std::string strErr = "Linearity: Size Error: LightMean Size: " + std::to_string(LightMean.size()) + ", ExpTime Size: " + std::to_string(ExpTime.size());
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return false;
	}
	LinearityRes.resize(APSSubFrameIndex::SubFrameNum);
	for (uint32_t nChannelIndex = 0; nChannelIndex < APSSubFrameIndex::SubFrameNum; nChannelIndex++)
	{
		uint32_t nDataNum = LightMean.size();
		std::vector<double> XData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			XData[nIndex] = ExpTime[nIndex];
		}
		std::vector<double> YData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			YData[nIndex] = LightMean[nIndex][nChannelIndex];
		}
		if (LinearityFit(XData, YData, LinearityRes[nChannelIndex].k, LinearityRes[nChannelIndex].b))
		{
			LinearityRes[nChannelIndex].LeMax = -100000, LinearityRes[nChannelIndex].LeMin = 10000;
			for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
			{
				double FitY = ExpTime[nIndex] * LinearityRes[nChannelIndex].k + LinearityRes[nChannelIndex].b;
				double LE = 100 * (YData[nIndex] - FitY) / FitY;
				if (LE > LinearityRes[nChannelIndex].LeMax)
					LinearityRes[nChannelIndex].LeMax = LE;
				if (LE < LinearityRes[nChannelIndex].LeMin)
					LinearityRes[nChannelIndex].LeMin = LE;
			}
		}
		else
		{
			std::string strErr = "Linearity: LinearityFit Error";
			WriteLog(strErr, nChannelIndex);
			return false;
		}
	}

	return true;
}

bool CAlpAPSMPAlgorithm::OverallSystemGain(std::vector<std::vector<double>>& LightTNoiseData, std::vector<std::vector<double>>& LightMean, std::vector<double> DarkTNoiseBase, std::vector<double>& GainK)
{
	if (LightTNoiseData.size() != LightMean.size())
	{
		std::string strErr = "OverallSystemGain: Size Error: LightTNoiseData Size: " + std::to_string(LightTNoiseData.size()) + ", LightMean Size: " + std::to_string(LightMean.size());
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return false;
	}
	GainK.resize(APSSubFrameIndex::SubFrameNum);
	for (uint32_t nChannelIndex = 0; nChannelIndex < APSSubFrameIndex::SubFrameNum; nChannelIndex++)
	{
		uint32_t nDataNum = LightMean.size();
		std::vector<double> XData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			XData[nIndex] = LightMean[nIndex][nChannelIndex];
		}
		std::vector<double> YData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			YData[nIndex] = LightTNoiseData[nIndex][nChannelIndex] * LightTNoiseData[nIndex][nChannelIndex] - DarkTNoiseBase[nChannelIndex] * DarkTNoiseBase[nChannelIndex];
		}
		double k = 0.0, b = 0.0;
		if (LinearityFit(XData, YData, k, b))
		{
			GainK[nChannelIndex] = k;
		}
		else
		{
			std::string strErr = "OverallSystemGain: LinearityFit Error";
			WriteLog(strErr, nChannelIndex);
			return false;
		}
	}

	return true;
}

bool CAlpAPSMPAlgorithm::Saturation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, SaturationData& SaturationRes)
{
	bool bRes = false;
	SubFrameDataMean(nIndexStart, nNumber, ROI, nChannelIndex, SaturationRes.SaturationMean, bRes);
	if (!bRes)
	{
		return false;
	}
	SubFrameTNoise(nIndexStart, nNumber, ROI, nChannelIndex, SaturationRes.SaturationTNoise, bRes);
	if (!bRes)
	{
		return false;
	}
	if (0 == SaturationRes.SaturationTNoise)
	{
		std::string strErr = "Saturation: TNoise abnormal";
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return false;
	}

	SaturationRes.SaturationSNR = SaturationRes.SaturationMean / SaturationRes.SaturationTNoise;

	return true;
}

bool CAlpAPSMPAlgorithm::Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, bool bNormalize, ImgType& ImgData)
{
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "Show: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		return false;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "Show: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return false;
	}
	ImgData.resize(m_nChannelRow);
	for (uint32_t nIndex = 0; nIndex < m_nChannelRow; nIndex++)
	{
		ImgData[nIndex].resize(m_nChannelCol);
	}
	if (!bNormalize)
	{
		for (uint32_t nRows = 0; nRows < m_nChannelRow; nRows++)
		{
			for (uint32_t nCols = 0; nCols < m_nChannelCol; nCols++)
			{
				if (PosInRoi(nRows, nCols, RealRoi))
				{
					double temp = 0;
					for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
					{
						temp += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols];
					}
					temp /= nNumber;
					if (temp < 0)
					{
						ImgData[nRows][nCols] = 0;
					}
					else
					{
						if (m_RawType == RawType::RAW8)
						{
							ImgData[nRows][nCols] = floor(temp);
						}
						else if (m_RawType == RawType::RAW10)
						{
							ImgData[nRows][nCols] = floor(temp / 4);
						}
						else
						{
							ImgData[nRows][nCols] = floor(temp / 16);
						}
					}
				}
				else
				{
					ImgData[nRows][nCols] = 0;
				}
			}
		}
	}
	else
	{
		double dMaxValue = 0, dMinValue = 0;
		Local temp;
		CAPSDataContainer DataContainer;
		DataContainer.Init(m_nChannelRow, m_nChannelCol, true);
		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			DataContainer += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex];
		}
		DataContainer /= nNumber;
		Max(dMaxValue, temp, DataContainer, &RealRoi);
		Min(dMinValue, temp, DataContainer, &RealRoi);

		for (uint32_t nRows = 0; nRows < m_nChannelRow; nRows++)
		{
			for (uint32_t nCols = 0; nCols < m_nChannelCol; nCols++)
			{
				if (PosInRoi(nRows, nCols, RealRoi))
				{
					double NewValue = (DataContainer.m_RawData[nRows][nCols] - dMinValue) / (dMaxValue - dMinValue) * 255;
					ImgData[nRows][nCols] = round(NewValue);
				}
				else
				{
					ImgData[nRows][nCols] = 0;
				}
			}
		}
	}
	return true;
}

bool CAlpAPSMPAlgorithm::Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, APSType& ImgData)
{
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "Show: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		return false;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "Show: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return false;
	}
	ImgData.resize(m_nChannelRow);
	for (uint32_t nIndex = 0; nIndex < m_nChannelRow; nIndex++)
	{
		ImgData[nIndex].resize(m_nChannelCol);
	}
	for (uint32_t nRows = 0; nRows < m_nChannelRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < m_nChannelCol; nCols++)
		{
			if (PosInRoi(nRows, nCols, RealRoi))
			{
				double temp = 0;
				for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
				{
					temp += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols];
				}
				ImgData[nRows][nCols] = temp / nNumber;
			}
			else
			{
				ImgData[nRows][nCols] = 0;
			}
		}
	}

	return true;
}

void CAlpAPSMPAlgorithm::SetMultiThreadEnable(bool bEnable)
{
	m_bMultiThreadEnable = bEnable;
}

void CAlpAPSMPAlgorithm::SetLogEnable(bool bEnable)
{
	m_bLogEnable = bEnable;
}

void CAlpAPSMPAlgorithm::SetAlgorithmThre(APSAlgorithmThre& AlgoThre)
{
	m_AlgorithmThre = AlgoThre;
}

APSAlgorithmThre CAlpAPSMPAlgorithm::GetAlgorithmThre()
{
	return m_AlgorithmThre;
}

ROIArea CAlpAPSMPAlgorithm::GetActiveArea()
{
	return m_ActiveArea;
}

void CAlpAPSMPAlgorithm::GetRawDataSize(uint32_t& nRow, uint32_t& nCol)
{
	nRow = m_nChannelRow;
	nCol = m_nChannelCol;
}

void CAlpAPSMPAlgorithm::SetActiveArea(ROIArea ActiveArea)
{
	m_ActiveArea = ActiveArea;
}

void CAlpAPSMPAlgorithm::SetRawDataSize(uint32_t nRow, uint32_t nCol)
{
	m_nTotalRow = nRow;
	m_nTotalCol = nCol;
	m_nChannelRow = m_nTotalRow / 4;
	m_nChannelCol = m_nTotalCol / 2;
}

uint32_t CAlpAPSMPAlgorithm::GetDataNum()
{
	if (m_RawDataContainer.size() > 0 && m_RawDataContainer[0].size() > 0)
	{
		return m_RawDataContainer[0].size();
	}
	else
	{
		return 0;
	}
}

bool CAlpAPSMPAlgorithm::SaveBin(uint8_t* pRawData, uint64_t nLens, std::string strSavePath)
{
	bool bRet = true;
	std::ofstream outfile;
	outfile.open(strSavePath, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!outfile.fail())
	{
		outfile.write((const char*)pRawData, nLens);
		bRet = !outfile.fail();
		outfile.close();
	}
	else
	{
		bRet = false;
	}
	return bRet;
}

std::string CAlpAPSMPAlgorithm::GetVersion()
{
	return APS_MP_ALGORITHM_VERSION;
}

bool CAlpAPSMPAlgorithm::WriteLog(std::string strMessage, uint32_t nAPSSubFrameIndex)
{
	if (!m_bLogEnable || m_strLogFilePath == "")
	{
		return false;
	}
	m_LogMutex.lock();
	bool bRet = true;
	std::time_t t = std::time(nullptr);
	std::tm now;
	localtime_s(&now, &t);
	char str_time[100] = { 0 };
	strftime(str_time, sizeof(str_time), "[%Y-%m-%d %H:%M:%S] ", &now);
	std::string strLine = "";
	strLine += str_time;
	strLine = strLine + "SubFrame " + std::to_string(nAPSSubFrameIndex) + ": ";
	strLine = strLine + strMessage + "\n";
	std::ofstream outfile;
	outfile.open(m_strLogFilePath, std::ios::app);
	if (!outfile.fail())
	{
		outfile << strLine;
		outfile.close();
		bRet = true;
	}
	else
	{
		bRet = false;
	}
	m_LogMutex.unlock();
	return bRet;
}

double CAlpAPSMPAlgorithm::Mean(std::vector<double>& RawData, uint32_t nLens)
{
	if (nLens < 1 || nLens > RawData.size())
	{
		std::string strErr = "Mean: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return 0.0;
	}
	double dMean = 0;
	for (uint32_t nIndex = 0; nIndex < nLens; nIndex++)
	{
		dMean += RawData[nIndex];
	}
	return dMean / nLens;
}

double CAlpAPSMPAlgorithm::Mean(CAPSDataContainer& RawData, ROIArea* ROI)
{
	ROIArea RealRoi;
	if (ROI == nullptr)
	{
		RealRoi = { 0, RawData.m_nRow - 1, 0, RawData.m_nCol - 1 };
	}
	else
	{
		RealRoi = *ROI;
	}
	uint32_t nSize = (RealRoi.Down - RealRoi.Up + 1) * (RealRoi.Right - RealRoi.Left + 1);
	if (nSize < 1 || RealRoi.Down >= RawData.m_nRow || RealRoi.Right >= RawData.m_nCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "Mean: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(RawData.m_nRow) + ", Col: " + std::to_string(RawData.m_nCol);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return 0.0;
	}
	double dMean = 0;
	for (uint32_t nRows = RealRoi.Up; nRows <= RealRoi.Down; nRows++)
	{
		for (uint32_t nCols = RealRoi.Left; nCols <= RealRoi.Right; nCols++)
		{
			dMean += RawData.m_RawData[nRows][nCols];
		}
	}
	dMean /= nSize;
	return dMean;
}

double CAlpAPSMPAlgorithm::Std(std::vector<double>& RawData, uint32_t nLens)
{
	if (nLens < 2 || nLens > RawData.size())
	{
		std::string strErr = "Std: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return 0.0;
	}
	double dMean = Mean(RawData, nLens);
	double dStd = 0;
	for (uint32_t nIndex = 0; nIndex < nLens; nIndex++)
	{
		dStd += (RawData[nIndex] - dMean) * (RawData[nIndex] - dMean);
	}
	dStd = sqrt(dStd / (nLens - 1));
	return dStd;
}

double CAlpAPSMPAlgorithm::Std(CAPSDataContainer& RawData, ROIArea* ROI)
{
	ROIArea RealRoi;
	if (ROI == nullptr)
	{
		RealRoi = { 0, RawData.m_nRow - 1, 0, RawData.m_nCol - 1 };
	}
	else
	{
		RealRoi = *ROI;
	}
	uint32_t nSize = (RealRoi.Down - RealRoi.Up + 1) * (RealRoi.Right - RealRoi.Left + 1);
	if (nSize < 2 || RealRoi.Down >= RawData.m_nRow || RealRoi.Right >= RawData.m_nCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "Std: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(RawData.m_nRow) + ", Col: " + std::to_string(RawData.m_nCol);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return 0.0;
	}
	double dMean = Mean(RawData, &RealRoi);
	double dStd = 0;
	for (uint32_t nRows = RealRoi.Up; nRows <= RealRoi.Down; nRows++)
	{
		for (uint32_t nCols = RealRoi.Left; nCols <= RealRoi.Right; nCols++)
		{
			dStd += (RawData.m_RawData[nRows][nCols] - dMean) * (RawData.m_RawData[nRows][nCols] - dMean);
		}
	}
	dStd = sqrt(dStd / (nSize - 1));
	return dStd;
}

double CAlpAPSMPAlgorithm::RMS(std::vector<double>& RawData, uint32_t nLens)
{
	if (nLens < 1 || nLens > RawData.size())
	{
		std::string strErr = "RMS: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return 0.0;
	}
	double dRMS = 0;
	for (uint32_t nIndex = 0; nIndex < nLens; nIndex++)
	{
		dRMS += RawData[nIndex] * RawData[nIndex];
	}
	dRMS = sqrt(dRMS / nLens);
	return dRMS;
}

double CAlpAPSMPAlgorithm::RMS(CAPSDataContainer& RawData, ROIArea* ROI)
{
	ROIArea RealRoi;
	if (ROI == nullptr)
	{
		RealRoi = { 0, RawData.m_nRow - 1, 0, RawData.m_nCol - 1 };
	}
	else
	{
		RealRoi = *ROI;
	}
	uint32_t nSize = (RealRoi.Down - RealRoi.Up + 1) * (RealRoi.Right - RealRoi.Left + 1);
	if (nSize < 1 || RealRoi.Down >= RawData.m_nRow || RealRoi.Right >= RawData.m_nCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "RMS: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(RawData.m_nRow) + ", Col: " + std::to_string(RawData.m_nCol);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return 0.0;
	}
	double dRMS = 0;
	for (uint32_t nRows = RealRoi.Up; nRows <= RealRoi.Down; nRows++)
	{
		for (uint32_t nCols = RealRoi.Left; nCols <= RealRoi.Right; nCols++)
		{
			dRMS += RawData.m_RawData[nRows][nCols] * RawData.m_RawData[nRows][nCols];
		}
	}
	dRMS = sqrt(dRMS / nSize);
	return dRMS;
}

void CAlpAPSMPAlgorithm::Max(double& dMaxValue, uint32_t& nMaxLocal, std::vector<double>& RawData, uint32_t nLens)
{
	if (nLens < 1 || nLens > RawData.size())
	{
		std::string strErr = "Max: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return;
	}
	dMaxValue = RawData[0];
	nMaxLocal = 0;
	for (uint32_t nIndex = 0; nIndex < nLens; nIndex++)
	{
		if (RawData[nIndex] > dMaxValue)
		{
			dMaxValue = RawData[nIndex];
			nMaxLocal = nIndex;
		}
	}
}

void CAlpAPSMPAlgorithm::Max(double& dMaxValue, Local& MaxLocal, CAPSDataContainer& RawData, ROIArea* ROI)
{
	ROIArea RealRoi;
	if (ROI == nullptr)
	{
		RealRoi = { 0, RawData.m_nRow - 1, 0, RawData.m_nCol - 1 };
	}
	else
	{
		RealRoi = *ROI;
	}
	uint32_t nSize = (RealRoi.Down - RealRoi.Up + 1) * (RealRoi.Right - RealRoi.Left + 1);
	if (nSize < 1 || RealRoi.Down >= RawData.m_nRow || RealRoi.Right >= RawData.m_nCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "Max: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(RawData.m_nRow) + ", Col: " + std::to_string(RawData.m_nCol);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return;
	}
	dMaxValue = RawData.m_RawData[RealRoi.Up][RealRoi.Left];
	MaxLocal.x = RealRoi.Up;
	MaxLocal.y = RealRoi.Left;
	for (uint32_t nRows = RealRoi.Up; nRows <= RealRoi.Down; nRows++)
	{
		for (uint32_t nCols = RealRoi.Left; nCols <= RealRoi.Right; nCols++)
		{
			if (RawData.m_RawData[nRows][nCols] > dMaxValue)
			{
				dMaxValue = RawData.m_RawData[nRows][nCols];
				MaxLocal.x = nRows;
				MaxLocal.y = nCols;
			}
		}
	}
}

void CAlpAPSMPAlgorithm::Min(double& dMinValue, uint32_t& nMinLocal, std::vector<double>& RawData, uint32_t nLens)
{
	if (nLens < 1 || nLens > RawData.size())
	{
		std::string strErr = "Min: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return;
	}
	dMinValue = RawData[0];
	nMinLocal = 0;
	for (uint32_t nIndex = 0; nIndex < nLens; nIndex++)
	{
		if (RawData[nIndex] < dMinValue)
		{
			dMinValue = RawData[nIndex];
			nMinLocal = nIndex;
		}
	}
}

void CAlpAPSMPAlgorithm::Min(double& dMinValue, Local& MinLocal, CAPSDataContainer& RawData, ROIArea* ROI)
{
	ROIArea RealRoi;
	if (ROI == nullptr)
	{
		RealRoi = { 0, RawData.m_nRow - 1, 0, RawData.m_nCol - 1 };
	}
	else
	{
		RealRoi = *ROI;
	}
	uint32_t nSize = (RealRoi.Down - RealRoi.Up + 1) * (RealRoi.Right - RealRoi.Left + 1);
	if (nSize < 1 || RealRoi.Down >= RawData.m_nRow || RealRoi.Right >= RawData.m_nCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "Max: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(RawData.m_nRow) + ", Col: " + std::to_string(RawData.m_nCol);
		WriteLog(strErr, APSSubFrameIndex::SubFrameNum);
		return;
	}
	dMinValue = RawData.m_RawData[RealRoi.Up][RealRoi.Left];
	MinLocal.x = RealRoi.Up;
	MinLocal.y = RealRoi.Left;
	for (uint32_t nRows = RealRoi.Up; nRows <= RealRoi.Down; nRows++)
	{
		for (uint32_t nCols = RealRoi.Left; nCols <= RealRoi.Right; nCols++)
		{
			if (RawData.m_RawData[nRows][nCols] < dMinValue)
			{
				dMinValue = RawData.m_RawData[nRows][nCols];
				MinLocal.x = nRows;
				MinLocal.y = nCols;
			}
		}
	}
}

inline bool CAlpAPSMPAlgorithm::PosInRoi(uint32_t nRows, uint32_t nCols, ROIArea& ROI)
{
	if (nRows >= ROI.Up && nRows <= ROI.Down && nCols >= ROI.Left && nCols <= ROI.Right)
	{
		return true;
	}
	return false;
}

bool CAlpAPSMPAlgorithm::LinearityFit(std::vector<double>& XData, std::vector<double>& YData, double& k, double& b)
{
	if (0 == XData.size() || XData.size() != YData.size())
	{
		return false;
	}
	double dSumX = 0, dSumXY = 0, dSumY = 0, dSumX2 = 0;
	uint32_t nSize = XData.size();
	for (uint32_t nIndex = 0; nIndex < nSize; nIndex++)
	{
		dSumX += XData[nIndex];
		dSumY += YData[nIndex];
		dSumXY += XData[nIndex] * YData[nIndex];
		dSumX2 += XData[nIndex] * XData[nIndex];
	}

	if (0 == nSize * dSumX2 - dSumX * dSumX)
	{
		return false;
	}

	k = (nSize * dSumXY - dSumX * dSumY) / (nSize * dSumX2 - dSumX * dSumX);
	b = dSumY / nSize - dSumX / nSize * k;
	return true;
}

void CAlpAPSMPAlgorithm::SubFrameTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& TNoiseData, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (nNumber < 2 || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameTNoise: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameTNoise: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	CAPSDataContainer PixelTNoiseArray;
	PixelTNoiseArray.Init(nRow, nCol);
	std::vector<double> onePixelInMultiFrames(nNumber);

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++)
		{
			for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
			{
				onePixelInMultiFrames[nIndex] = m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			}
			PixelTNoiseArray.m_RawData[nRows][nCols] = Std(onePixelInMultiFrames, nNumber);
		}
	}
	TNoiseData = RMS(PixelTNoiseArray);
	return;
}

void CAlpAPSMPAlgorithm::SubFrameSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& SNoiseData, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameSNoise: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameSNoise: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;
	CAPSDataContainer PixelSNoiseArray;
	PixelSNoiseArray.Init(nRow, nCol, true);

	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
	{
		for (uint32_t nRows = 0; nRows < nRow; nRows++)
		{
			for (uint32_t nCols = 0; nCols < nCol; nCols++)
			{
				PixelSNoiseArray.m_RawData[nRows][nCols] += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			}
		}
	}
	PixelSNoiseArray /= nNumber;
	SNoiseData = Std(PixelSNoiseArray);
	return;
}

void CAlpAPSMPAlgorithm::SubFrameRowTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& RowTNoiseData, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (nNumber < 2 || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameRowTNoise: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameRowTNoise: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	std::vector<double> RowNoise(nRow);

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		std::vector<double> RowMean(nNumber, 0);

		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			for (uint32_t nCols = 0; nCols < nCol; nCols++)
			{
				RowMean[nIndex] += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			}
			RowMean[nIndex] /= nCol;
		}
		RowNoise[nRows] = Std(RowMean, nNumber);
	}
	RowTNoiseData = RMS(RowNoise, nRow);
	return;
}

void CAlpAPSMPAlgorithm::SubFrameColTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& ColTNoiseData, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (nNumber < 2 || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameColTNoise: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameColTNoise: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	std::vector<double> ColNoise(nCol);

	for (uint32_t nCols = 0; nCols < nCol; nCols++)
	{
		std::vector<double> ColMean(nNumber, 0);

		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			for (uint32_t nRows = 0; nRows < nRow; nRows++)
			{
				ColMean[nIndex] += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			}
			ColMean[nIndex] /= nRow;
		}
		ColNoise[nCols] = Std(ColMean, nNumber);
	}
	ColTNoiseData = RMS(ColNoise, nCol);
	return;
}

void CAlpAPSMPAlgorithm::SubFrameRowSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& RowSNoiseData, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameRowSNoise: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameRowSNoise: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	std::vector<double> RowMean(nRow);

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			for (uint32_t nCols = 0; nCols < nCol; nCols++)
			{
				RowMean[nRows] += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			}
		}
		RowMean[nRows] /= (nCol * nNumber);
	}
	RowSNoiseData = Std(RowMean, nRow);
	return;
}

void CAlpAPSMPAlgorithm::SubFrameColSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& ColSNoiseData, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameColSNoise: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameColSNoise: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	std::vector<double> ColMean(nCol);

	for (uint32_t nCols = 0; nCols < nCol; nCols++)
	{
		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			for (uint32_t nRows = 0; nRows < nRow; nRows++)
			{
				ColMean[nCols] += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			}
		}
		ColMean[nCols] /= (nRow * nNumber);
	}
	ColSNoiseData = Std(ColMean, nCol);
	return;
}

void CAlpAPSMPAlgorithm::SubFrameBadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, BadpixelData& BadpixelRes, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameBadPixel: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameBadPixel: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	CAPSDataContainer PixelMeanArray;
	PixelMeanArray.Init(nRow, nCol, true);
	std::vector<std::vector<uint32_t>> BadPixelMask(nRow);
	for (uint32_t i = 0; i < nRow; i++)
	{
		BadPixelMask[i].resize(nCol);
	}
	std::vector<double> SortData((2 * m_AlgorithmThre.nBadPixelRadius + 1) * (2 * m_AlgorithmThre.nBadPixelRadius + 1));

	BadpixelRes.BadPixelNum = 0;
	BadpixelRes.DeadPixelNum = 0;
	BadpixelRes.BadPixelMask.LocalData.clear();
	BadpixelRes.BadPixelMask.Flag.clear();
	BadpixelRes.BadPixelMask.BadPixelNum = 0;
	std::vector<uint32_t> RowDeadLine(nRow, 0);
	std::vector<uint32_t> ColDeadLine(nCol, 0);

	for (uint32_t nRows = 0; nRows < nRow + m_AlgorithmThre.nBadPixelRadius; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol + m_AlgorithmThre.nBadPixelRadius; nCols++)
		{
			if (nRows < nRow && nCols < nCol)
			{
				for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
				{
					PixelMeanArray.m_RawData[nRows][nCols] += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
				}
				PixelMeanArray.m_RawData[nRows][nCols] /= nNumber;
			}
			if ((nRows - m_AlgorithmThre.nBadPixelRadius < nRow) && (nCols - m_AlgorithmThre.nBadPixelRadius < nCol))
			{
				uint32_t uSize = 0;
				double dSurroundPixle = 0;
				for (uint32_t i = 0; i < 2 * m_AlgorithmThre.nBadPixelRadius + 1; i++)
				{
					for (uint32_t j = 0; j < 2 * m_AlgorithmThre.nBadPixelRadius + 1; j++)
					{
						if ((nRows - i < nRow) && (nCols - j < nCol) && (i != m_AlgorithmThre.nBadPixelRadius || j != m_AlgorithmThre.nBadPixelRadius))
						{
							dSurroundPixle += PixelMeanArray.m_RawData[nRows - i][nCols - j];
							//SortData[uSize] = PixelMeanArray.m_RawData[nRows - i][nCols - j];
							uSize++;
						}
					}
				}
				if (uSize)
				{
					dSurroundPixle /= uSize;
				}
				else
				{
					dSurroundPixle = PixelMeanArray.m_RawData[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius];
				}
				//std::sort(SortData.begin(), SortData.begin() + uSize);
				//double dSurroundPixle = SortData[uSize / 2];
				double dCurrentPixel = PixelMeanArray.m_RawData[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius];

				if (abs(dCurrentPixel - dSurroundPixle) / dSurroundPixle > m_AlgorithmThre.dBadPixelThre)
				{
					BadpixelRes.BadPixelMask.BadPixelNum++;
					BadpixelRes.BadPixelMask.LocalData.push_back({ nRows - m_AlgorithmThre.nBadPixelRadius + RealRoi.Up, nCols - m_AlgorithmThre.nBadPixelRadius + RealRoi.Left });

					BadpixelRes.BadPixelNum++;
					if (abs(dCurrentPixel - dSurroundPixle) / dSurroundPixle > m_AlgorithmThre.dDeadPixelThre)
					{
						BadpixelRes.DeadPixelNum++;
						++RowDeadLine[nRows - m_AlgorithmThre.nBadPixelRadius];
						++ColDeadLine[nCols - m_AlgorithmThre.nBadPixelRadius];
						BadPixelMask[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius] = APS_DEAD_PIXEL_FLAG;
						BadpixelRes.BadPixelMask.Flag.push_back(APS_DEAD_PIXEL_FLAG);
					}
					else
					{
						BadPixelMask[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius] = APS_BAD_PIXEL_FLAG;
						BadpixelRes.BadPixelMask.Flag.push_back(APS_BAD_PIXEL_FLAG);
					}
				}
				else
				{
					BadPixelMask[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius] = 0;
				}
			}
		}
	}
	BadpixelRes.DeadLineNum = 0;
	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		if (RowDeadLine[nRows] > nCol * m_AlgorithmThre.dDeadLineThre)
		{
			++BadpixelRes.DeadLineNum;
		}
	}
	for (uint32_t nCols = 0; nCols < nCol; nCols++)
	{
		if (ColDeadLine[nCols] > nRow * m_AlgorithmThre.dDeadLineThre)
		{
			++BadpixelRes.DeadLineNum;
		}
	}
	uint32_t ConnectedAreaFlag = 0xFFFFFFFF;
	BadpixelRes.SingletNum = 0;
	BadpixelRes.CoupletNum = 0;
	BadpixelRes.ClusterNum = 0;

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++)
		{
			if (BadPixelMask[nRows][nCols] != 0 && BadPixelMask[nRows][nCols] < ConnectedAreaFlag)
			{
				uint32_t AreaSize = 0;
				std::stack<Local> Search;
				Search.push({ nRows, nCols });
				while (!Search.empty())
				{
					Local temp = Search.top();
					Search.pop();
					BadPixelMask[temp.x][temp.y] = ConnectedAreaFlag;
					AreaSize++;
					for (uint32_t nTempRows = temp.x - 1; nTempRows <= temp.x + 1; nTempRows++)
					{
						if (nTempRows < nRow)
						{
							for (uint32_t nTempCols = temp.y - 1; nTempCols <= temp.y + 1; nTempCols++)
							{
								if (nTempCols < nCol && BadPixelMask[nTempRows][nTempCols] != 0 && BadPixelMask[nTempRows][nTempCols] < ConnectedAreaFlag)
								{
									Search.push({ nTempRows , nTempCols });
								}
							}
						}
					}
				}
				if (AreaSize == 1)
				{
					BadpixelRes.SingletNum++;
				}
				else if (AreaSize == 2)
				{
					BadpixelRes.CoupletNum++;
				}
				else
				{
					BadpixelRes.ClusterNum++;
				}
				ConnectedAreaFlag--;
			}
		}
	}
	return;
}

void CAlpAPSMPAlgorithm::SubFrameHotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, HotpixelData& HotpixelRes, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameHotpixel: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameHotPixel: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	CAPSDataContainer PixelMeanArray;
	PixelMeanArray.Init(nRow, nCol, true);
	std::vector<double> SortData((2 * m_AlgorithmThre.nBadPixelRadius + 1) * (2 * m_AlgorithmThre.nBadPixelRadius + 1));

	HotpixelRes.HotPixelNum = 0;
	HotpixelRes.HotPixelMask.LocalData.clear();
	HotpixelRes.HotPixelMask.Flag.clear();
	HotpixelRes.HotPixelMask.BadPixelNum = 0;

	std::vector<uint32_t> RowHotLine(nRow, 0);
	std::vector<uint32_t> ColHotLine(nCol, 0);

	for (uint32_t nRows = 0; nRows < nRow + m_AlgorithmThre.nBadPixelRadius; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol + m_AlgorithmThre.nBadPixelRadius; nCols++)
		{
			if (nRows < nRow && nCols < nCol)
			{
				for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
				{
					PixelMeanArray.m_RawData[nRows][nCols] += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
				}
				PixelMeanArray.m_RawData[nRows][nCols] /= nNumber;
			}
			if ((nRows - m_AlgorithmThre.nBadPixelRadius < nRow) && (nCols - m_AlgorithmThre.nBadPixelRadius < nCol))
			{
				uint32_t uSize = 0;
				double dSurroundPixle = 0;
				for (uint32_t i = 0; i < 2 * m_AlgorithmThre.nBadPixelRadius + 1; i++)
				{
					for (uint32_t j = 0; j < 2 * m_AlgorithmThre.nBadPixelRadius + 1; j++)
					{
						if ((nRows - i < nRow) && (nCols - j < nCol) && (i != m_AlgorithmThre.nBadPixelRadius || j != m_AlgorithmThre.nBadPixelRadius))
						{
							dSurroundPixle += PixelMeanArray.m_RawData[nRows - i][nCols - j];
							//SortData[uSize] = PixelMeanArray.m_RawData[nRows - i][nCols - j];
							uSize++;
						}
					}
				}
				if (uSize)
				{
					dSurroundPixle /= uSize;
				}
				else
				{
					dSurroundPixle = PixelMeanArray.m_RawData[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius];
				}
				//std::sort(SortData.begin(), SortData.begin() + uSize);
				//dSurroundPixle = SortData[uSize / 2];
				double dCurrentPixel = PixelMeanArray.m_RawData[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius];

				if (abs(dCurrentPixel - dSurroundPixle) > m_AlgorithmThre.dHotPixelThre)
				{
					HotpixelRes.HotPixelMask.BadPixelNum++;
					HotpixelRes.HotPixelMask.LocalData.push_back({ nRows - m_AlgorithmThre.nBadPixelRadius + RealRoi.Up, nCols - m_AlgorithmThre.nBadPixelRadius + RealRoi.Left });
					HotpixelRes.HotPixelMask.Flag.push_back(APS_HOT_PIXEL_FLAG);
					HotpixelRes.HotPixelNum++;

					++RowHotLine[nRows - m_AlgorithmThre.nBadPixelRadius];
					++ColHotLine[nCols - m_AlgorithmThre.nBadPixelRadius];
				}
			}
		}
	}
	HotpixelRes.HotLineNum = 0;
	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		if (RowHotLine[nRows] > nCol * m_AlgorithmThre.dHotLineThre)
		{
			++HotpixelRes.HotLineNum;
		}
	}
	for (uint32_t nCols = 0; nCols < nCol; nCols++)
	{
		if (ColHotLine[nCols] > nRow * m_AlgorithmThre.dHotLineThre)
		{
			++HotpixelRes.HotLineNum;
		}
	}
	return;
}

void CAlpAPSMPAlgorithm::SubFrameBLC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& BaseMean, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameDPC: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameDPC: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}

	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
	{
		for (uint32_t nRows = RealRoi.Up; nRows <= RealRoi.Down; nRows++)
		{
			for (uint32_t nCols = RealRoi.Left; nCols <= RealRoi.Right; nCols++)
			{
				m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols] -= BaseMean;
			}
		}
	}
	return;
}

void CAlpAPSMPAlgorithm::SubFrameDPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, BadPixelMaskData& BadPixelMask, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameDPC: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameDPC: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}

	for (uint32_t nBadPixelIndex = 0; nBadPixelIndex < BadPixelMask.BadPixelNum; nBadPixelIndex++)
	{
		uint32_t nBadpixelRows = BadPixelMask.LocalData[nBadPixelIndex].x;
		uint32_t nBadpixelCols = BadPixelMask.LocalData[nBadPixelIndex].y;

		for (uint32_t nBadPixelIndex = 0; nBadPixelIndex < BadPixelMask.BadPixelNum; nBadPixelIndex++)
		{
			for (uint32_t nFrameIndex = 0; nFrameIndex < nNumber; nFrameIndex++)
			{
				CAPSDataContainer& CurRawData = m_RawDataContainer[nChannelIndex][nIndexStart + nFrameIndex];
				uint32_t nSize = 0;
				double dMeanData = 0;
				for (uint32_t nCurRows = nBadpixelRows - 1; nCurRows <= nBadpixelRows + 1; nCurRows++)
				{
					if (nCurRows >= RealRoi.Up && nCurRows <= RealRoi.Down)
					{
						for (uint32_t nCurCols = nBadpixelCols - 1; nCurCols <= nBadpixelCols + 1; nCurCols++)
						{
							if (nCurCols >= RealRoi.Left && nCurCols <= RealRoi.Right && BadPixelMask.LocalData.end() == std::find(BadPixelMask.LocalData.begin(), BadPixelMask.LocalData.end(), Local{ nCurRows, nCurCols }))
							{
								dMeanData += CurRawData.m_RawData[nCurRows][nCurCols];
								nSize++;
							}
						}
					}
				}
				if (nSize > 0)
				{
					CurRawData.m_RawData[nBadpixelRows][nBadpixelCols] = dMeanData / nSize;
				}
			}
		}
	}
	return;
}

void CAlpAPSMPAlgorithm::SubFrameDataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, double& DataMean, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameDataMean: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameDataMean: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}

	DataMean = 0;
	for (uint32_t nRows = RealRoi.Up; nRows <= RealRoi.Down; nRows++)
	{
		for (uint32_t nCols = RealRoi.Left; nCols <= RealRoi.Right; nCols++)
		{
			for (uint32_t nFrameIndex = 0; nFrameIndex < nNumber; nFrameIndex++)
			{
				DataMean += m_RawDataContainer[nChannelIndex][nIndexStart + nFrameIndex].m_RawData[nRows][nCols];
			}
		}
	}
	DataMean /= ((RealRoi.Down - RealRoi.Up + 1) * (RealRoi.Right - RealRoi.Left + 1) * nNumber);

	return;
}

void CAlpAPSMPAlgorithm::SubFrameBLCByColBase(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, std::vector<double>& BaseMean, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameBLCByColBase: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameBLCByColBase: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	uint32_t nAACol = RealRoi.Right - RealRoi.Left + 1;

	if (nAACol != BaseMean.size())
	{
		std::string strErr = "SubFrameBLCByColBase: Base size error: ROI Col size: " + std::to_string(nAACol) + ", Base size: " + std::to_string(BaseMean.size());
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
	{
		for (uint32_t nRows = RealRoi.Up; nRows <= RealRoi.Down; nRows++)
		{
			for (uint32_t nCols = RealRoi.Left; nCols <= RealRoi.Right; nCols++)
			{
				m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols] -= BaseMean[nCols - RealRoi.Left];
			}
		}
	}
	return;
}

void CAlpAPSMPAlgorithm::SubFrameColMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, std::vector<double>& DataMean, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameBLCByColBase: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameBLCByColBase: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	uint32_t nAACol = RealRoi.Right - RealRoi.Left + 1;
	uint32_t nAARow = RealRoi.Down - RealRoi.Up + 1;
	DataMean.resize(nAACol);
	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
	{
		for (uint32_t nCols = RealRoi.Left; nCols <= RealRoi.Right; nCols++)
		{
			DataMean[nCols - RealRoi.Left] = 0;
			for (uint32_t nRows = RealRoi.Up; nRows <= RealRoi.Down; nRows++)
			{
				DataMean[nCols - RealRoi.Left] += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols];
			}
			DataMean[nCols - RealRoi.Left] /= nAARow;
		}
	}
	return;
}

bool CAlpAPSMPAlgorithm::GetBlockMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, uint32_t nBlockSize, std::vector<CAPSDataContainer>& BlockData)
{
	bool bRet = true;
	bool bSubRes[APSSubFrameIndex::SubFrameNum];

	if (m_bMultiThreadEnable)
	{
		std::thread* t[APSSubFrameIndex::SubFrameNum];

		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameBlockMean, this, nIndexStart, nNumber, ROI, APSSubFrameIndex(i), nBlockSize, std::ref(BlockData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			SubFrameBlockMean(nIndexStart, nNumber, ROI, APSSubFrameIndex(i), nBlockSize, BlockData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

void CAlpAPSMPAlgorithm::SubFrameBlockMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSubFrameIndex nChannelIndex, uint32_t nBlockSize, CAPSDataContainer& BlockData, bool& bRes)
{
	bRes = true;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameBlockMean: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}

	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameBlockMean: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;
	uint32_t nRowBlockNum = nRow / nBlockSize;
	uint32_t nColBlockNum = nCol / nBlockSize;
	uint32_t nRowMod = nRow % nRowBlockNum;
	uint32_t nColMod = nCol % nColBlockNum;

	BlockData.Init(nRowBlockNum, nColBlockNum);

	uint32_t nRowBlockSize = 0, nColBlockSize = 0, nRowIndex = RealRoi.Up, nColIndex = RealRoi.Left;
	double dPedestal = 0;
	for (uint32_t nRowBlockIndex = 0; nRowBlockIndex < nRowBlockNum; nRowBlockIndex++)
	{
		if (0 == nRowBlockIndex || nRowBlockNum - 1 == nRowBlockIndex)
		{
			nRowBlockSize = m_AlgorithmThre.nDSNUBlockSize + nRowMod / 2;
		}
		else
		{
			nRowBlockSize = m_AlgorithmThre.nDSNUBlockSize;
		}
		nColIndex = RealRoi.Left;
		for (uint32_t nColBlockIndex = 0; nColBlockIndex < nColBlockNum; nColBlockIndex++)
		{
			if (0 == nColBlockIndex || nColBlockNum - 1 == nColBlockIndex)
			{
				nColBlockSize = m_AlgorithmThre.nDSNUBlockSize + nColMod / 2;
			}
			else
			{
				nColBlockSize = m_AlgorithmThre.nDSNUBlockSize;
			}
			ROIArea temp = { nRowIndex, nRowIndex + nRowBlockSize - 1, nColIndex, nColIndex + nColBlockSize - 1 };

			double dValue = 0;
			BlockData.m_RawData[nRowBlockIndex][nColBlockIndex] = 0;
			for (uint32_t nRows = temp.Up; nRows <= temp.Down; nRows++)
			{
				for (uint32_t nCols = temp.Left; nCols <= temp.Right; nCols++)
				{
					for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
					{
						BlockData.m_RawData[nRowBlockIndex][nColBlockIndex] += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols];
					}
				}
			}
			BlockData.m_RawData[nRowBlockIndex][nColBlockIndex] /= (nNumber * nRowBlockSize * nColBlockSize);
			nColIndex += nColBlockSize;
		}
		nRowIndex += nRowBlockSize;
	}
}
