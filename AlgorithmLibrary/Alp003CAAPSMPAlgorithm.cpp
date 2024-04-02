#include "Alp003CAAPSMPAlgorithm.h"

CAlp003CAAPSMPAlgorithm::CAlp003CAAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
	: CAlpAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, nSiteNum, Pixelformat, code)
{
	m_ActiveArea = { 0, 1223, 0, 1631 };
	m_nChannelRow = 1224;
	m_nChannelCol = 1632;
	m_nTotalRow = 2448;
	m_nTotalCol = 3264;
	m_AlgorithmThre.nDSNURowBlockNum = 30;
	m_AlgorithmThre.nDSNUColBlockNum = 40;
	m_AlgorithmThre.nDSNURowBlockSize = 40;
	m_AlgorithmThre.nDSNUColBlockSize = 40;
}

CAlp003CAAPSMPAlgorithm::~CAlp003CAAPSMPAlgorithm()
{
}

bool CAlp003CAAPSMPAlgorithm::ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer)
{
	uint32_t nOneFrameSize = 0;
	uint32_t nHeaderSize = 0;
	uint32_t nFooterSize = 0;

	uint8_t Header[8] = { 0 };
	uint8_t Footer[8] = { 0 };

	uint8_t Header_003CA[] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xe5 };
	uint8_t Footer_003CA[] = { 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xe5 };

	memcpy_s(Header, sizeof(Header), Header_003CA, sizeof(Header_003CA));
	memcpy_s(Footer, sizeof(Footer), Footer_003CA, sizeof(Footer_003CA));
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

bool CAlp003CAAPSMPAlgorithm::BadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& BadpixelRes)
{
	bool bRet = CAlpAPSMPAlgorithm::BadPixel(nIndexStart, nNumber, ROI, BadpixelRes);
	if (bRet)
	{
		std::vector<std::vector<uint32_t>> BadPixelMask(m_nTotalRow + 8);
		for (uint32_t i = 0; i < m_nTotalRow + 8; i++)
		{
			BadPixelMask[i].resize(m_nTotalCol + 8, 0);
		}

		for (uint32_t n = 0; n < BadpixelRes.BadPixelMask.BadPixelNum; n++)
		{
			Local temp = BadpixelRes.BadPixelMask.LocalData[n];
			BadPixelMask[temp.x + 4][temp.y + 4] = BadpixelRes.BadPixelMask.Flag[n];
		}

		for (uint32_t row = 0; row < 4; row++)
		{
			for (uint32_t col = 0; col < m_nTotalCol + 8; col++)
			{
				BadPixelMask[row][col] = BadPixelMask[row + 4][col];
				BadPixelMask[row + m_nTotalRow + 4][col] = BadPixelMask[row + m_nTotalRow][col];
			}
		}

		for (uint32_t col = 0; col < 4; col++)
		{
			for (uint32_t row = 0; row < m_nTotalRow + 8; row++)
			{
				BadPixelMask[row][col] = BadPixelMask[row][col + 4];
				BadPixelMask[row][col + m_nTotalCol + 4] = BadPixelMask[row][col + m_nTotalCol];
			}
		}

		int x0[] = { -3, 0, 1, -3, 1, 3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -3, -4, 1,  4, -4, -3, 4, 1 };
		int y0[] = { -3, -3, -3, 0, 0, 1, 1, 1, -3, -3, 0, 0, 1, 1, -2, -1, -2,-1, -2,-1, -4, -3, -4, -3, 1, 4, 1, 4 };
		int x1[] = { -1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3, -1, 3 };
		int y1[] = { -3, -3, -3,  0,  0,  1,  1,  1, -3, -3,  0,  0,  1,  1, -2, -1, -2, -1, -2, -1, -4,  4 };
		int x2[] = { -3, 0, 1, -3, 1, -3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -4, 4 };
		int y2[] = { -1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2, -1, 3 };
		int x3[] = { -1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3 };
		int y3[] = { -1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2 };

		for (uint32_t n = 0; n < BadpixelRes.BadPixelMask.BadPixelNum; n++)
		{
			Local temp = BadpixelRes.BadPixelMask.LocalData[n];

			int* x = nullptr;
			int* y = nullptr;
			int nCheckLen = 0;
			if (temp.x % 2 == 0 && temp.y % 2 == 0)
			{
				x = x0;
				y = y0;
				nCheckLen = sizeof(x0) / sizeof(x0[0]);
			}
			else if (temp.x % 2 == 0 && temp.y % 2 == 1)
			{
				x = x1;
				y = y1;
				nCheckLen = sizeof(x1) / sizeof(x1[0]);
			}
			else if (temp.x % 2 == 1 && temp.y % 2 == 0)
			{
				x = x2;
				y = y2;
				nCheckLen = sizeof(x2) / sizeof(x2[0]);
			}
			else
			{
				x = x3;
				y = y3;
				nCheckLen = sizeof(x3) / sizeof(x3[0]);
			}
			bool bFindBadPixel = false;
			for (int i = 0; i < nCheckLen; i++)
			{
				if (BadPixelMask[temp.x + 4 + y[i]][temp.y + 4 + x[i]] > 0)
				{
					bFindBadPixel = true;
					break;
				}
			}

			if (!bFindBadPixel)
			{
				BadpixelRes.BadPixelMask.Flag[n] = APX003CA_ON_CHIP_CALIBRATION_FLAG;
			}
		}
	}

	return bRet;
}

bool CAlp003CAAPSMPAlgorithm::HotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& HotpixelRes)
{
	bool bRet = CAlpAPSMPAlgorithm::HotPixel(nIndexStart, nNumber, ROI, HotpixelRes);
	if (bRet)
	{
		std::vector<std::vector<uint32_t>> BadPixelMask(m_nTotalRow + 8);
		for (uint32_t i = 0; i < m_nTotalRow + 8; i++)
		{
			BadPixelMask[i].resize(m_nTotalCol + 8, 0);
		}

		for (uint32_t n = 0; n < HotpixelRes.BadPixelMask.BadPixelNum; n++)
		{
			Local temp = HotpixelRes.BadPixelMask.LocalData[n];
			BadPixelMask[temp.x + 4][temp.y + 4] = HotpixelRes.BadPixelMask.Flag[n];
		}

		for (uint32_t row = 0; row < 4; row++)
		{
			for (uint32_t col = 0; col < m_nTotalCol + 8; col++)
			{
				BadPixelMask[row][col] = BadPixelMask[row + 4][col];
				BadPixelMask[row + m_nTotalRow + 4][col] = BadPixelMask[row + m_nTotalRow][col];
			}
		}

		for (uint32_t col = 0; col < 4; col++)
		{
			for (uint32_t row = 0; row < m_nTotalRow + 8; row++)
			{
				BadPixelMask[row][col] = BadPixelMask[row][col + 4];
				BadPixelMask[row][col + m_nTotalCol + 4] = BadPixelMask[row][col + m_nTotalCol];
			}
		}

		int x0[] = { -3, 0, 1, -3, 1, 3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -3, -4, 1,  4, -4, -3, 4, 1 };
		int y0[] = { -3, -3, -3, 0, 0, 1, 1, 1, -3, -3, 0, 0, 1, 1, -2, -1, -2,-1, -2,-1, -4, -3, -4, -3, 1, 4, 1, 4 };
		int x1[] = { -1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3, -1, 3 };
		int y1[] = { -3, -3, -3,  0,  0,  1,  1,  1, -3, -3,  0,  0,  1,  1, -2, -1, -2, -1, -2, -1, -4,  4 };
		int x2[] = { -3, 0, 1, -3, 1, -3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -4, 4 };
		int y2[] = { -1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2, -1, 3 };
		int x3[] = { -1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3 };
		int y3[] = { -1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2 };

		for (uint32_t n = 0; n < HotpixelRes.BadPixelMask.BadPixelNum; n++)
		{
			Local temp = HotpixelRes.BadPixelMask.LocalData[n];

			int* x = nullptr;
			int* y = nullptr;
			int nCheckLen = 0;
			if (temp.x % 2 == 0 && temp.y % 2 == 0)
			{
				x = x0;
				y = y0;
				nCheckLen = sizeof(x0) / sizeof(x0[0]);
			}
			else if (temp.x % 2 == 0 && temp.y % 2 == 1)
			{
				x = x1;
				y = y1;
				nCheckLen = sizeof(x1) / sizeof(x1[0]);
			}
			else if (temp.x % 2 == 1 && temp.y % 2 == 0)
			{
				x = x2;
				y = y2;
				nCheckLen = sizeof(x2) / sizeof(x2[0]);
			}
			else
			{
				x = x3;
				y = y3;
				nCheckLen = sizeof(x3) / sizeof(x3[0]);
			}
			bool bFindBadPixel = false;
			for (int i = 0; i < nCheckLen; i++)
			{
				if (BadPixelMask[temp.x + 4 + y[i]][temp.y + 4 + x[i]] > 0)
				{
					bFindBadPixel = true;
					break;
				}
			}

			if (!bFindBadPixel)
			{
				HotpixelRes.BadPixelMask.Flag[n] = APX003CA_ON_CHIP_CALIBRATION_FLAG;
			}
		}
	}

	return bRet;
}

bool CAlp003CAAPSMPAlgorithm::DPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<Local>& BadPixelLocal)
{
	if (0 == nNumber || nIndexStart >= m_RawDataContainer[0].size() || (nIndexStart + nNumber) > m_RawDataContainer[0].size())
	{
		std::string strErr = "DPC: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	int x0[] = { -3, 0, 1, -3, 1, 3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -3, -4, 1,  4, -4, -3, 4, 1 };
	int y0[] = { -3, -3, -3, 0, 0, 1, 1, 1, -3, -3, 0, 0, 1, 1, -2, -1, -2,-1, -2,-1, -4, -3, -4, -3, 1, 4, 1, 4 };
	int x1[] = { -1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3, -1, 3 };
	int y1[] = { -3, -3, -3,  0,  0,  1,  1,  1, -3, -3,  0,  0,  1,  1, -2, -1, -2, -1, -2, -1, -4,  4 };
	int x2[] = { -3, 0, 1, -3, 1, -3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -4, 4 };
	int y2[] = { -1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2, -1, 3 };
	int x3[] = { -1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3 };
	int y3[] = { -1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2 };

	for (uint32_t nIndex = nIndexStart; nIndex < nIndexStart + nNumber; nIndex++)
	{
		for (uint32_t n = 0; n < BadPixelLocal.size(); n++)
		{
			auto totalLocal = BadPixelLocal[n];
			int nRow = totalLocal.x;
			int nCol = totalLocal.y;

			Local PixelLocal[20];
			int tempRow = 0;
			int tempCol = 0;
			for (int i = 0; i < 20; i++)
			{
				if (nRow % 2 == 0 && nCol % 2 == 0)
				{
					tempRow = nRow + y0[i];
					tempCol = nCol + x0[i];
				}
				else if (nRow % 2 == 0 && nCol % 2 == 1)
				{
					tempRow = nRow + y1[i];
					tempCol = nCol + x1[i];
				}
				else if (nRow % 2 == 1 && nCol % 2 == 0)
				{
					tempRow = nRow + y2[i];
					tempCol = nCol + x2[i];
				}
				else
				{
					tempRow = nRow + y3[i];
					tempCol = nCol + x3[i];
				}
				if (tempRow < 0)
				{
					tempRow += 4;
				}
				else if (tempRow >= m_nTotalRow)
				{
					tempRow -= 4;
				}
				if (tempCol < 0)
				{
					tempCol += 4;
				}
				else if (tempCol >= m_nTotalCol)
				{
					tempCol -= 4;
				}
				PixelLocal[i].x = tempRow;
				PixelLocal[i].y = tempCol;
			}
			double p0, p1, p2, p3, p5, p6, p7, p8, g0, g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11;
			GetDataFromSubFrame(nIndex, PixelLocal[0].x, PixelLocal[0].y, p0);
			GetDataFromSubFrame(nIndex, PixelLocal[1].x, PixelLocal[1].y, p1);
			GetDataFromSubFrame(nIndex, PixelLocal[2].x, PixelLocal[2].y, p2);
			GetDataFromSubFrame(nIndex, PixelLocal[3].x, PixelLocal[3].y, p3);
			GetDataFromSubFrame(nIndex, PixelLocal[4].x, PixelLocal[4].y, p5);
			GetDataFromSubFrame(nIndex, PixelLocal[5].x, PixelLocal[5].y, p6);
			GetDataFromSubFrame(nIndex, PixelLocal[6].x, PixelLocal[6].y, p7);
			GetDataFromSubFrame(nIndex, PixelLocal[7].x, PixelLocal[7].y, p8);
			GetDataFromSubFrame(nIndex, PixelLocal[8].x, PixelLocal[8].y, g0);
			GetDataFromSubFrame(nIndex, PixelLocal[9].x, PixelLocal[9].y, g1);
			GetDataFromSubFrame(nIndex, PixelLocal[10].x, PixelLocal[10].y, g2);
			GetDataFromSubFrame(nIndex, PixelLocal[11].x, PixelLocal[11].y, g3);
			GetDataFromSubFrame(nIndex, PixelLocal[12].x, PixelLocal[12].y, g4);
			GetDataFromSubFrame(nIndex, PixelLocal[13].x, PixelLocal[13].y, g5);
			GetDataFromSubFrame(nIndex, PixelLocal[14].x, PixelLocal[14].y, g6);
			GetDataFromSubFrame(nIndex, PixelLocal[15].x, PixelLocal[15].y, g7);
			GetDataFromSubFrame(nIndex, PixelLocal[16].x, PixelLocal[16].y, g8);
			GetDataFromSubFrame(nIndex, PixelLocal[17].x, PixelLocal[17].y, g9);
			GetDataFromSubFrame(nIndex, PixelLocal[18].x, PixelLocal[18].y, g10);
			GetDataFromSubFrame(nIndex, PixelLocal[19].x, PixelLocal[19].y, g11);

			double dh4 = abs(g2 + g3 - (g0 + g1) / 2 - (g4 + g5) / 2);

			double dv4 = abs(g8 + g9 - (g6 + g7) / 2 - (g10 + g11) / 2);

			double dhori = (abs(2 * p1 - p0 - p2) + abs(p3 - p5) * 2 + abs(2 * p7 - p6 - p8) + dh4) / 4;

			double dvert = (abs(2 * p3 - p0 - p6) + abs(p1 - p7) * 2 + abs(2 * p5 - p2 - p8) + dv4) / 4;

			double hori_value = 0, vert_value = 0;

			if (nRow % 2 == 0 && nCol % 2 == 0)
			{
				hori_value = 0.75 * p5 + 0.25 * p3;

				vert_value = 0.75 * p7 + 0.25 * p1;
			}
			else if (nRow % 2 == 0 && nCol % 2 == 1)
			{
				hori_value = 0.75 * p3 + 0.25 * p5;

				vert_value = 0.75 * p7 + 0.25 * p1;

			}
			else if (nRow % 2 == 1 && nCol % 2 == 0)
			{
				hori_value = 0.75 * p5 + 0.25 * p3;

				vert_value = 0.75 * p1 + 0.25 * p7;
			}
			else
			{
				hori_value = 0.75 * p3 + 0.25 * p5;

				vert_value = 0.75 * p1 + 0.25 * p7;
			}

			if ((dvert + dhori) == 0)
			{
				std::string strErr = "DPC: dvert + dhori == 0 ";
				WriteLog(strErr, SubFrameIndex::All);
				return false;
			}

			double p4 = (hori_value * dvert + vert_value * dhori) / (dvert + dhori);

			SetDataToSubFrame(nIndex, nRow, nCol, int(p4));
		}
	}
	return true;
}

