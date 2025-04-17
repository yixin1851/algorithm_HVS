#include "Alp014AAAPSMPAlgorithm.h"

CAlp014AAAPSMPAlgorithm::CAlp014AAAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
	: CAlpAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, nSiteNum, Pixelformat, code)
{
	if ((code & APSCodeType::APS_Code_HVS) == APSCodeType::APS_Code_HVS)
	{
		m_ActiveArea = { 0, 479, 0, 1279 };
		m_nChannelRow = 480;
		m_nChannelCol = 1280;
		m_nTotalRow = 480;
		m_nTotalCol = 1280;

		m_AlgorithmThre.nDSNURowBlockNum = 12;
		m_AlgorithmThre.nDSNUColBlockNum = 32;
		m_AlgorithmThre.nDSNURowBlockSize = 40;
		m_AlgorithmThre.nDSNUColBlockSize = 40;
		m_AlgorithmThre.nPedestalVariationRowBlockNum = 8;
		m_AlgorithmThre.nPedestalVariationColBlockNum = 8;
		m_AlgorithmThre.nPedestalVariationRowBlockSize = 60;
		m_AlgorithmThre.nPedestalVariationColBlockSize = 160;
	}
	else
	{
		m_ActiveArea = { 0, 959, 0, 1279 };
		m_nChannelRow = 960;
		m_nChannelCol = 1280;
		m_nTotalRow = 960;
		m_nTotalCol = 1280;

		m_AlgorithmThre.nDSNURowBlockNum = 24;
		m_AlgorithmThre.nDSNUColBlockNum = 32;
		m_AlgorithmThre.nDSNURowBlockSize = 40;
		m_AlgorithmThre.nDSNUColBlockSize = 40;
		m_AlgorithmThre.nPedestalVariationRowBlockNum = 8;
		m_AlgorithmThre.nPedestalVariationColBlockNum = 8;
		m_AlgorithmThre.nPedestalVariationRowBlockSize = 120;
		m_AlgorithmThre.nPedestalVariationColBlockSize = 160;
	}

	m_AlgorithmThre.nOETCRadius = 128;
	m_AlgorithmThre.nLinearityRadius = 32;

	m_nMaxSubFramesNum = 1;

	m_RawDataContainer.resize(m_nMaxSubFramesNum);
}

CAlp014AAAPSMPAlgorithm::~CAlp014AAAPSMPAlgorithm()
{
}

bool CAlp014AAAPSMPAlgorithm::ImportRawData(uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, bool bHeader_Footer)
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
		WriteLog(strErr, m_nMaxSubFramesNum);
		return false;
	}
	for (uint32_t nChannelIndex = 0; nChannelIndex < m_nMaxSubFramesNum; nChannelIndex++)
	{
		if (m_RawDataContainer[nChannelIndex].size() < nIndexStart + nNumber)
		{
			m_RawDataContainer[nChannelIndex].resize(nIndexStart + nNumber);
		}
	}

	uint32_t nIndex = 0;

	for (uint32_t i = 0; i < nNumber; i++)
	{
		for (uint32_t nChannelIndex = 0; nChannelIndex < m_nMaxSubFramesNum; nChannelIndex++)
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
				WriteLog(strErr, m_nMaxSubFramesNum);
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
			m_RawDataContainer[0][nIndexStart + i].m_RawData[nRows][nCols] = tempData[0];
			m_RawDataContainer[0][nIndexStart + i].m_RawData[nRows][nCols + 1] = tempData[1];
			m_RawDataContainer[0][nIndexStart + i].m_RawData[nRows][nCols + 2] = tempData[2];
			m_RawDataContainer[0][nIndexStart + i].m_RawData[nRows][nCols + 3] = tempData[3];
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
				WriteLog(strErr, m_nMaxSubFramesNum);
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

bool CAlp014AAAPSMPAlgorithm::TNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSTNoiseType& TNoiseRes)
{
	bool bRet = true;
	uint32_t nChannelNum = m_nMaxSubFramesNum;
	RawDataContainer& DataContainer = m_RawDataContainer;
	bool* bSubRes = new bool[nChannelNum];
	TNoiseRes.SubFrameTNoiseData.resize(nChannelNum);
	TNoiseRes.TNoiseFrame = 0;
	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}

	if (m_bMultiThreadEnable)
	{
		std::vector<std::thread*> t(nChannelNum);

		for (uint32_t i = 0; i < nChannelNum; i++)
		{
			t[i] = new std::thread(&CAlp014AAAPSMPAlgorithm::SubFrameTNoise, this, nIndexStart, nNumber, &RealRoi, SubFrameIndex(i), std::ref(TNoiseRes.SubFrameTNoiseData[i]), std::ref(bSubRes[i]), std::ref(DataContainer));
		}
		for (uint32_t i = 0; i < nChannelNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < nChannelNum; i++)
		{
			SubFrameTNoise(nIndexStart, nNumber, &RealRoi, SubFrameIndex(i), TNoiseRes.SubFrameTNoiseData[i], bSubRes[i], DataContainer);
		}
	}
	for (uint32_t i = 0; i < nChannelNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	delete[] bSubRes;

	if (bRet)
	{
		TNoiseRes.TNoiseFrame = TNoiseRes.SubFrameTNoiseData[0].TempNoise;
	}
	return bRet;
}

bool CAlp014AAAPSMPAlgorithm::SNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSNoiseType& SNoiseData)
{
	bool bRet = true;
	uint32_t nChannelNum = m_nMaxSubFramesNum;
	RawDataContainer& DataContainer = m_RawDataContainer;
	bool* bSubRes = new bool[nChannelNum];
	SNoiseData.SNoiseFrame = 0;
	SNoiseData.SubFrameSNoiseData.resize(nChannelNum);

	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}

	if (m_bMultiThreadEnable)
	{
		std::vector<std::thread*> t(nChannelNum);

		for (uint32_t i = 0; i < nChannelNum; i++)
		{
			t[i] = new std::thread(&CAlp014AAAPSMPAlgorithm::SubFrameSNoise, this, nIndexStart, nNumber, &RealRoi, SubFrameIndex(i), std::ref(SNoiseData.SubFrameSNoiseData[i]), std::ref(bSubRes[i]), std::ref(DataContainer));
		}
		for (uint32_t i = 0; i < nChannelNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < nChannelNum; i++)
		{
			SubFrameSNoise(nIndexStart, nNumber, &RealRoi, SubFrameIndex(i), SNoiseData.SubFrameSNoiseData[i], bSubRes[i], DataContainer);
		}
	}
	for (uint32_t i = 0; i < nChannelNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	delete[] bSubRes;

	if (bRet)
	{
		SNoiseData.SNoiseFrame = SNoiseData.SubFrameSNoiseData[0].SNoise;
	}
	return bRet;
}

bool CAlp014AAAPSMPAlgorithm::BadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& BadpixelRes)
{
	bool bRet = true;
	bool* bSubRes = new bool[m_nMaxSubFramesNum];
	BadpixelRes.BadPixelNum = 0;
	BadpixelRes.ClusterNum = 0;
	BadpixelRes.CoupletNum = 0;
	BadpixelRes.LadderNum = 0;
	BadpixelRes.SingletNum = 0;
	BadpixelRes.MaxClusterSize = 0;
	BadpixelRes.BadPixelMask.BadPixelNum = 0;
	BadpixelRes.BadPixelMask.LocalData.clear();
	BadpixelRes.BadPixelMask.Flag.clear();
	BadpixelRes.BadPixelMask.DiffData.clear();

	BadpixelRes.SubFrameBadpixelData.resize(m_nMaxSubFramesNum);
	if (m_bMultiThreadEnable)
	{
		std::vector<std::thread*> t(m_nMaxSubFramesNum);

		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i] = new std::thread(&CAlp014AAAPSMPAlgorithm::SubFrameBadPixel, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), std::ref(BadpixelRes.SubFrameBadpixelData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			SubFrameBadPixel(nIndexStart, nNumber, ROI, SubFrameIndex(i), BadpixelRes.SubFrameBadpixelData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}

	delete[] bSubRes;

	if (bRet)
	{
			BadpixelRes.BadPixelNum += BadpixelRes.SubFrameBadpixelData[0].BadPixelNum;
			BadpixelRes.ClusterNum += BadpixelRes.SubFrameBadpixelData[0].ClusterNum;
			BadpixelRes.CoupletNum += BadpixelRes.SubFrameBadpixelData[0].CoupletNum;
			BadpixelRes.SingletNum += BadpixelRes.SubFrameBadpixelData[0].SingletNum;
			BadpixelRes.BadPixelMask = BadpixelRes.SubFrameBadpixelData[0].BadPixelMask;
			BadpixelRes.MaxClusterSize = BadpixelRes.SubFrameBadpixelData[0].MaxClusterSize;
			BadpixelRes.LadderNum = 0;
	}
	return bRet;
}

bool CAlp014AAAPSMPAlgorithm::HotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& HotpixelRes)
{
	bool bRet = true;
	bool* bSubRes = new bool[m_nMaxSubFramesNum];
	HotpixelRes.BadPixelNum = 0;
	HotpixelRes.ClusterNum = 0;
	HotpixelRes.CoupletNum = 0;
	HotpixelRes.LadderNum = 0;
	HotpixelRes.SingletNum = 0;
	HotpixelRes.MaxClusterSize = 0;
	HotpixelRes.BadPixelMask.BadPixelNum = 0;
	HotpixelRes.BadPixelMask.LocalData.clear();
	HotpixelRes.BadPixelMask.Flag.clear();
	HotpixelRes.SubFrameBadpixelData.resize(m_nMaxSubFramesNum);
	HotpixelRes.BadPixelMask.DiffData.clear();

	if (m_bMultiThreadEnable)
	{
		std::vector<std::thread*> t(m_nMaxSubFramesNum);

		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i] = new std::thread(&CAlp014AAAPSMPAlgorithm::SubFrameHotPixel, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), std::ref(HotpixelRes.SubFrameBadpixelData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			SubFrameHotPixel(nIndexStart, nNumber, ROI, SubFrameIndex(i), HotpixelRes.SubFrameBadpixelData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	delete[] bSubRes;
	if (bRet)
	{
		HotpixelRes.BadPixelNum += HotpixelRes.SubFrameBadpixelData[0].BadPixelNum;
		HotpixelRes.ClusterNum += HotpixelRes.SubFrameBadpixelData[0].ClusterNum;
		HotpixelRes.CoupletNum += HotpixelRes.SubFrameBadpixelData[0].CoupletNum;
		HotpixelRes.SingletNum += HotpixelRes.SubFrameBadpixelData[0].SingletNum;
		HotpixelRes.BadPixelMask = HotpixelRes.SubFrameBadpixelData[0].BadPixelMask;
		HotpixelRes.MaxClusterSize = HotpixelRes.SubFrameBadpixelData[0].MaxClusterSize;
		HotpixelRes.LadderNum = 0;
	}
	return bRet;
}

bool CAlp014AAAPSMPAlgorithm::BLC(uint32_t nIndexStart, uint32_t nNumber)
{
	bool bRet = true;
	bool* bSubRes = new bool[m_nMaxSubFramesNum];
	std::vector<double> BaseMean(m_nMaxSubFramesNum, m_dPedestal);
	if (m_bMultiThreadEnable)
	{
		std::vector<std::thread*> t(m_nMaxSubFramesNum);

		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i] = new std::thread(&CAlp014AAAPSMPAlgorithm::SubFrameBLC, this, nIndexStart, nNumber, nullptr, SubFrameIndex(i), std::ref(BaseMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			SubFrameBLC(nIndexStart, nNumber, nullptr, SubFrameIndex(i), BaseMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	delete[] bSubRes;
	return bRet;
}

bool CAlp014AAAPSMPAlgorithm::BLC(uint32_t nIndexStart, uint32_t nNumber, APSDataMeanType& BaseMean)
{
	bool bRet = true;
	bool* bSubRes = new bool[m_nMaxSubFramesNum];

	if (m_bMultiThreadEnable)
	{
		std::vector<std::thread*> t(m_nMaxSubFramesNum);

		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i] = new std::thread(&CAlp014AAAPSMPAlgorithm::SubFrameBLC, this, nIndexStart, nNumber, nullptr, SubFrameIndex(i), std::ref(BaseMean.SubFrameDataMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			SubFrameBLC(nIndexStart, nNumber, nullptr, SubFrameIndex(i), BaseMean.SubFrameDataMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	delete[] bSubRes;
	return bRet;
}

bool CAlp014AAAPSMPAlgorithm::BLC(uint32_t nIndexStart, uint32_t nNumber, uint32_t nBaseIndexStart, uint32_t nBaseNumber)
{
	bool bRet = true;
	bool* bSubRes = new bool[m_nMaxSubFramesNum];
	std::vector<double> ColMean[SubFrameIndex::All];

	if (m_bMultiThreadEnable)
	{
		std::vector<std::thread*> t(m_nMaxSubFramesNum);

		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i] = new std::thread(&CAlp014AAAPSMPAlgorithm::SubFrameColMean, this, nBaseIndexStart, nBaseNumber, nullptr, SubFrameIndex(i), std::ref(ColMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			SubFrameColMean(nBaseIndexStart, nBaseNumber, nullptr, SubFrameIndex(i), ColMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	if (!bRet)
	{
		return false;
	}

	if (m_bMultiThreadEnable)
	{
		std::vector<std::thread*> t(m_nMaxSubFramesNum);

		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i] = new std::thread(&CAlp014AAAPSMPAlgorithm::SubFrameBLCByColBase, this, nIndexStart, nNumber, nullptr, SubFrameIndex(i), std::ref(ColMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			SubFrameBLCByColBase(nIndexStart, nNumber, nullptr, SubFrameIndex(i), ColMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	delete[] bSubRes;
	return bRet;
}

bool CAlp014AAAPSMPAlgorithm::DPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<Local>& BadPixelLocal)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];

	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i] = new std::thread(&CAlp014AAAPSMPAlgorithm::SubFrameDPC, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), std::ref(BadPixelLocal), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			SubFrameDPC(nIndexStart, nNumber, ROI, SubFrameIndex(i), BadPixelLocal, bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlp014AAAPSMPAlgorithm::BadPixelLocalToOtpType(std::vector<Local> BadPixelLocal, std::vector<uint8_t>& OtpData)
{
	return false;
}

bool CAlp014AAAPSMPAlgorithm::YShading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSYShadingType& ShadingRes)
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
	for (uint32_t nChannelIndex = 0; nChannelIndex < m_nMaxSubFramesNum; nChannelIndex++)
	{
		if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
		{
			std::string strErr = "YShading: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
			WriteLog(strErr, nChannelIndex);
			return false;
		}
	}

	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "YShading: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, m_nMaxSubFramesNum);
		return false;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;
	std::vector<CAPSDataContainer>BlockData(m_nMaxSubFramesNum);

	if (!GetBlockMean(nIndexStart, nNumber, &RealRoi, m_AlgorithmThre.nYShadingRowBlockNum, m_AlgorithmThre.nYShadingColBlockNum, BlockData))
	{
		std::string strErr = "YShading: GetBlockMean error";
		WriteLog(strErr, m_nMaxSubFramesNum);
		return false;
	}

	double YCenter = 
		BlockData[Gb].m_RawData[m_AlgorithmThre.nYShadingRowBlockNum / 2][m_AlgorithmThre.nYShadingColBlockNum / 2];

	if (YCenter <= 0)
	{
		std::string strErr = "YShading: Y Center error";
		WriteLog(strErr, m_nMaxSubFramesNum);
		return false;
	}

	ShadingRes.YShadingData.resize(m_AlgorithmThre.nYShadingRowBlockNum);
	for (uint32_t nRows = 0; nRows < m_AlgorithmThre.nYShadingRowBlockNum; nRows++)
	{
		ShadingRes.YShadingData[nRows].resize(m_AlgorithmThre.nYShadingColBlockNum);

		for (uint32_t nCols = 0; nCols < m_AlgorithmThre.nYShadingColBlockNum; nCols++)
		{
			double Y = BlockData[Gb].m_RawData[nRows][nCols];

			ShadingRes.YShadingData[nRows][nCols] = Y / YCenter;
		}
	}

	ROIArea LT = { RealRoi.Up, RealRoi.Up + nRow * 0.1 - 1, RealRoi.Left, RealRoi.Left + nCol * 0.1 - 1 };
	ROIArea LB = { RealRoi.Down - nRow * 0.1 + 1, RealRoi.Down , RealRoi.Left, RealRoi.Left + nCol * 0.1 - 1 };
	ROIArea RT = { RealRoi.Up, RealRoi.Up + nRow * 0.1 - 1, RealRoi.Right - nCol * 0.1 + 1 , RealRoi.Right };
	ROIArea RB = { RealRoi.Down - nRow * 0.1 + 1, RealRoi.Down , RealRoi.Right - nCol * 0.1 + 1 , RealRoi.Right };
	ROIArea CT = { RealRoi.Up + nRow * 0.45, RealRoi.Up + nRow * 0.55 - 1, RealRoi.Left + nCol * 0.45, RealRoi.Left + nCol * 0.55 - 1 };

	double YGbLT = 0, YGbLB = 0, YGbRT = 0, YGbRB = 0, YGbCT = 0;
	bool bRes = true;
	SubFrameDataMean(nIndexStart, nNumber, &LT, Gb, YGbLT, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &LB, Gb, YGbLB, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &RT, Gb, YGbRT, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &RB, Gb, YGbRB, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &CT, Gb, YGbCT, bRes, m_RawDataContainer);

	if (YGbCT <= 0)
	{
		std::string strErr = "YShading: Y Center error";
		WriteLog(strErr, m_nMaxSubFramesNum);
		return false;
	}

	ShadingRes.YShadingLT = (YGbLT) / (YGbCT);
	ShadingRes.YShadingLB = (YGbLB) / (YGbCT);
	ShadingRes.YShadingRT = (YGbRT) / (YGbCT);
	ShadingRes.YShadingRB = (YGbRB) / (YGbCT);

	return true;
}

bool CAlp014AAAPSMPAlgorithm::ColorShading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSColorShadingType& ShadingRes)
{
	return false;
}

bool CAlp014AAAPSMPAlgorithm::OpticalCenter(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSOpticalCenterType& OpticalCenterRes)
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
	for (uint32_t nChannelIndex = 0; nChannelIndex < m_nMaxSubFramesNum; nChannelIndex++)
	{
		if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
		{
			std::string strErr = "OpticalCenter: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
			WriteLog(strErr, nChannelIndex);
			return false;
		}
	}

	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "OpticalCenter: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, m_nMaxSubFramesNum);
		return false;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	std::vector<double> RowMean(nRow, 0);
	std::vector<double> ColMean(nCol, 0);

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++)
		{
			double dMeanData = 0;
			for (uint32_t nFrameIndex = 0; nFrameIndex < nNumber; nFrameIndex++)
			{
				for (uint32_t nChannelIndex = 0; nChannelIndex < m_nMaxSubFramesNum; nChannelIndex++)
				{
					dMeanData += m_RawDataContainer[nChannelIndex][nIndexStart + nFrameIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
				}
			}
			dMeanData /= (nNumber * m_nMaxSubFramesNum);
			RowMean[nRows] += dMeanData;
			ColMean[nCols] += dMeanData;
		}
	}

	double dMaxValue = 0;
	Max(dMaxValue, OpticalCenterRes.CenterRow, RowMean, RowMean.size());
	OpticalCenterRes.CenterRow += RealRoi.Up;
	Max(dMaxValue, OpticalCenterRes.CenterCol, ColMean, ColMean.size());
	OpticalCenterRes.CenterCol += RealRoi.Left;
	return true;
}

bool CAlp014AAAPSMPAlgorithm::PedestalVariation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSPedestalVariationType& PedestalVariationRes)
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
	for (uint32_t nChannelIndex = 0; nChannelIndex < m_nMaxSubFramesNum; nChannelIndex++)
	{
		if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
		{
			std::string strErr = "PedestalVariation: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
			WriteLog(strErr, nChannelIndex);
			return false;
		}
	}

	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "PedestalVariation: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, m_nMaxSubFramesNum);
		return false;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	std::vector<CAPSDataContainer>BlockData(m_nMaxSubFramesNum);

	if (!GetBlockMean(nIndexStart, nNumber, &RealRoi, m_AlgorithmThre.nPedestalVariationRowBlockNum, m_AlgorithmThre.nPedestalVariationColBlockNum, BlockData, m_AlgorithmThre.nPedestalVariationRowBlockSize, m_AlgorithmThre.nPedestalVariationColBlockSize))
	{
		std::string strErr = "PedestalVariation: GetBlockMean error";
		WriteLog(strErr, m_nMaxSubFramesNum);
		return false;
	}

	Local temp;
	for (uint32_t nIndex = 0; nIndex < m_nMaxSubFramesNum; nIndex++)
	{
		Max(PedestalVariationRes.PedestalMax[nIndex], temp, BlockData[nIndex]);
		Min(PedestalVariationRes.PedestalMin[nIndex], temp, BlockData[nIndex]);
	}

	return true;
}

bool CAlp014AAAPSMPAlgorithm::ReadNoise(uint32_t nIndex1, uint32_t nIndex2, ROIArea* ROI, APSReadNoiseType& ReadNoiseRes)
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
	for (uint32_t nChannelIndex = 0; nChannelIndex < m_nMaxSubFramesNum; nChannelIndex++)
	{
		if (nIndex1 >= m_RawDataContainer[nChannelIndex].size() || nIndex2 >= m_RawDataContainer[nChannelIndex].size())
		{
			std::string strErr = "ReadNoise: Index error: nIndex1: " + std::to_string(nIndex1) + ", nIndex2: " + std::to_string(nIndex2);
			WriteLog(strErr, nChannelIndex);
			return false;
		}
	}

	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "ReadNoise: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, m_nMaxSubFramesNum);
		return false;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	std::vector<double> AllPixel(nRow * nCol * m_nMaxSubFramesNum);

	uint32_t nCur = 0;
	for (uint32_t nChannelIndex = 0; nChannelIndex < m_nMaxSubFramesNum; nChannelIndex++)
	{
		for (uint32_t nRows = 0; nRows < nRow; nRows++)
		{
			for (uint32_t nCols = 0; nCols < nCol; nCols++)
			{
				double value1 = m_RawDataContainer[nChannelIndex][nIndex1].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
				double value2 = m_RawDataContainer[nChannelIndex][nIndex2].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
				AllPixel[nCur++] = value1 - value2;
			}
		}
	}
	ReadNoiseRes = Std(AllPixel, nCur) / sqrt(2);

	return true;
}

bool CAlp014AAAPSMPAlgorithm::DarkCurrent(std::vector<APSDataMeanType>& DataMean, std::vector<double>& ExpTime, APSDarkCurrentType& DarkCurrentRes)
{
	uint32_t nChannelNum = m_nMaxSubFramesNum;

	if (DataMean.size() != ExpTime.size())
	{
		std::string strErr = "DarkCurrent: Size Error: Data Size: " + std::to_string(DataMean.size()) + ", ExpTime Size: " + std::to_string(ExpTime.size());
		WriteLog(strErr, nChannelNum);
		return false;
	}
	DarkCurrentRes.SubFrameKValue.resize(nChannelNum);
	for (uint32_t nChannelIndex = 0; nChannelIndex < nChannelNum; nChannelIndex++)
	{
		uint32_t nDataNum = DataMean.size();
		std::vector<double> XData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			XData[nIndex] = ExpTime[nIndex];
		}
		std::vector<double> YData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			YData[nIndex] = DataMean[nIndex].SubFrameDataMean[nChannelIndex];
		}
		double k = 0.0, b = 0.0;
		if (LinearityFit(XData, YData, k, b))
		{
			DarkCurrentRes.SubFrameKValue[nChannelIndex] = k * 1000;
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

bool CAlp014AAAPSMPAlgorithm::DarkCurrent(std::vector<APSTNoiseType>& TNoise, std::vector<double>& ExpTime, APSDarkCurrentType& DarkCurrentRes)
{
	uint32_t nChannelNum = m_nMaxSubFramesNum;

	if (TNoise.size() != ExpTime.size())
	{
		std::string strErr = "DarkCurrent: Size Error: Data Size: " + std::to_string(TNoise.size()) + ", ExpTime Size: " + std::to_string(ExpTime.size());
		WriteLog(strErr, nChannelNum);
		return false;
	}
	DarkCurrentRes.SubFrameKValue.resize(nChannelNum);
	for (uint32_t nChannelIndex = 0; nChannelIndex < nChannelNum; nChannelIndex++)
	{
		uint32_t nDataNum = TNoise.size();
		std::vector<double> XData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			XData[nIndex] = ExpTime[nIndex];
		}
		std::vector<double> YData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{

			YData[nIndex] = TNoise[nIndex].SubFrameTNoiseData[nChannelIndex].TempNoise * TNoise[nIndex].SubFrameTNoiseData[nChannelIndex].TempNoise;
		}
		double k = 0.0, b = 0.0;
		if (LinearityFit(XData, YData, k, b))
		{
			DarkCurrentRes.SubFrameKValue[nChannelIndex] = k * 1000;
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

bool CAlp014AAAPSMPAlgorithm::DSNU(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSDSNUType& DSNURes)
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
	for (uint32_t nChannelIndex = 0; nChannelIndex < m_nMaxSubFramesNum; nChannelIndex++)
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
		WriteLog(strErr, m_nMaxSubFramesNum);
		return false;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;
	uint32_t nRowBlockNum = m_AlgorithmThre.nDSNURowBlockNum;
	uint32_t nColBlockNum = m_AlgorithmThre.nDSNUColBlockNum;

	std::vector<CAPSDataContainer>BlockData(m_nMaxSubFramesNum);

	if (!GetBlockMean(nIndexStart, nNumber, &RealRoi, nRowBlockNum, nColBlockNum, BlockData, m_AlgorithmThre.nDSNURowBlockSize, m_AlgorithmThre.nDSNUColBlockSize))
	{
		std::string strErr = "DSNU: GetBlockMean error";
		WriteLog(strErr, m_nMaxSubFramesNum);
		return false;
	}

	CAPSDataContainer G;
	G.Init(nRowBlockNum, nColBlockNum);

	double dPedestal = 0;

	for (uint32_t nRowBlocks = 0; nRowBlocks < nRowBlockNum; nRowBlocks++)
	{
		for (uint32_t nColBlocks = 0; nColBlocks < nColBlockNum; nColBlocks++)
		{
			G.m_RawData[nRowBlocks][nColBlocks] = BlockData[SubFrameIndex::Gb].m_RawData[nRowBlocks][nColBlocks];
			dPedestal += G.m_RawData[nRowBlocks][nColBlocks];
		}
	}

	dPedestal = round(dPedestal / (nRowBlockNum * nColBlockNum));

	double MaxG = G.m_RawData[0][0], MinG = G.m_RawData[0][0];
	double MaxSignal = 0, MinSignal = 10000, MinSignalCorner = 10000, MinSignalCentre = 10000, MinSignalEdge = 10000;
	uint32_t nMinSignalCornerRow = 0, nMinSignalCornerCol = 0;
	uint32_t nMinSignalCentreRow = 0, nMinSignalCentreCol = 0;
	uint32_t nMinSignalEdgeRow = 0, nMinSignalEdgeCol = 0;
	uint32_t nMinSignalRow = 0, nMinSignalCol = 0;

	ROIArea ROICorner[4] = { { 0, 1, 0, 1 }, { 0, 1, nColBlockNum - 2, nColBlockNum - 1 }, { nRowBlockNum - 2, nRowBlockNum - 1, 0, 1 }, { nRowBlockNum - 2, nRowBlockNum - 1, nColBlockNum - 2, nColBlockNum - 1 } };
	ROIArea ROIEdge[4] = { { 0, 1, 2, nColBlockNum - 3 }, { nRowBlockNum - 2, nRowBlockNum - 1, 2, nColBlockNum - 3 }, {2, nRowBlockNum - 3, 0, 1 }, {2, nRowBlockNum - 3, nColBlockNum - 2, nColBlockNum - 1 }, };
	ROIArea ROICenter = { 2, nRowBlockNum - 3, 2, nColBlockNum - 3 };

	for (uint32_t nRowBlockIndex = 0; nRowBlockIndex < nRowBlockNum; nRowBlockIndex++)
	{
		for (uint32_t nColBlockIndex = 0; nColBlockIndex < nColBlockNum; nColBlockIndex++)
		{
			if (G.m_RawData[nRowBlockIndex][nColBlockIndex] > MaxG)
				MaxG = G.m_RawData[nRowBlockIndex][nColBlockIndex];
			if (G.m_RawData[nRowBlockIndex][nColBlockIndex] < MinG)
				MinG = G.m_RawData[nRowBlockIndex][nColBlockIndex];
			double dSignal = fabs(G.m_RawData[nRowBlockIndex][nColBlockIndex] - dPedestal);
			if (dSignal > MaxSignal)
				MaxSignal = dSignal;
			if (dSignal < MinSignal)
			{
				MinSignal = dSignal;
				nMinSignalRow = nRowBlockIndex;
				nMinSignalCol = nColBlockIndex;
			}
			if (PosInRoi(nRowBlockIndex, nColBlockIndex, ROICenter))
			{
				if (dSignal < MinSignalCentre)
				{
					MinSignalCentre = dSignal;
					nMinSignalCentreRow = nRowBlockIndex;
					nMinSignalCentreCol = nColBlockIndex;
				}
			}
			else if (PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[0]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[1]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[2]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[3]))
			{
				if (dSignal < MinSignalCorner)
				{
					MinSignalCorner = dSignal;
					nMinSignalCornerRow = nRowBlockIndex;
					nMinSignalCornerCol = nColBlockIndex;
				}
			}
			else
			{
				if (dSignal < MinSignalEdge)
				{
					MinSignalEdge = dSignal;
					nMinSignalEdgeRow = nRowBlockIndex;
					nMinSignalEdgeCol = nColBlockIndex;
				}
			}
		}
	}

	DSNURes.RangeG = MaxG - MinG;
	DSNURes.SignalMax = MaxSignal;
	DSNURes.GMax = MaxG;
	DSNURes.GMin = MinG;

	double dDeltaSignal = 0;
	DSNURes.DeltaSignalCentreMax = 0;
	DSNURes.DeltaSignalCornerMax = 0;
	DSNURes.DeltaSignalEdgeMax = 0;
	DSNURes.DeltaSignalMax = 0;
	double dGlobalDeltaSignal = 0;

	for (uint32_t nRowBlockIndex = 0; nRowBlockIndex < nRowBlockNum; nRowBlockIndex++)
	{
		for (uint32_t nColBlockIndex = 0; nColBlockIndex < nColBlockNum; nColBlockIndex++)
		{
			dGlobalDeltaSignal = fabs(G.m_RawData[nRowBlockIndex][nColBlockIndex] - G.m_RawData[nMinSignalRow][nMinSignalCol]);
			if (dGlobalDeltaSignal > DSNURes.DeltaSignalMax)
			{
				DSNURes.DeltaSignalMax = dGlobalDeltaSignal;
			}

			if (PosInRoi(nRowBlockIndex, nColBlockIndex, ROICenter))
			{
				dDeltaSignal = fabs(G.m_RawData[nRowBlockIndex][nColBlockIndex] - G.m_RawData[nMinSignalCentreRow][nMinSignalCentreCol]);
				if (dDeltaSignal > DSNURes.DeltaSignalCentreMax)
				{
					DSNURes.DeltaSignalCentreMax = dDeltaSignal;
				}
			}
			else if (PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[0]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[1]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[2]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[3]))
			{
				dDeltaSignal = fabs(G.m_RawData[nRowBlockIndex][nColBlockIndex] - G.m_RawData[nMinSignalCornerRow][nMinSignalCornerCol]);
				if (dDeltaSignal > DSNURes.DeltaSignalCornerMax)
				{
					DSNURes.DeltaSignalCornerMax = dDeltaSignal;
				}
			}
			else
			{
				dDeltaSignal = fabs(G.m_RawData[nRowBlockIndex][nColBlockIndex] - G.m_RawData[nMinSignalEdgeRow][nMinSignalEdgeCol]);
				if (dDeltaSignal > DSNURes.DeltaSignalEdgeMax)
				{
					DSNURes.DeltaSignalEdgeMax = dDeltaSignal;
				}
			}
		}
	}
	return true;
}

bool CAlp014AAAPSMPAlgorithm::DataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSDataMeanType& DataMean)
{
	bool bRet = true;
	uint32_t nChannelNum = m_nMaxSubFramesNum;
	RawDataContainer& DataContainer = m_RawDataContainer;
	bool* bSubRes = new bool[nChannelNum];
	DataMean.SubFrameDataMean.resize(nChannelNum);
	DataMean.DataMeanFrame = 0;

	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}

	if (m_bUse16SubFrame)
	{
		RealRoi.Up /= 2;
		RealRoi.Left /= 2;
		RealRoi.Right = (RealRoi.Right + 1) / 2 - 1;
		RealRoi.Down = (RealRoi.Down + 1) / 2 - 1;
	}

	if (m_bMultiThreadEnable)
	{
		std::vector<std::thread*> t(nChannelNum);

		for (uint32_t i = 0; i < nChannelNum; i++)
		{
			t[i] = new std::thread(&CAlp014AAAPSMPAlgorithm::SubFrameDataMean, this, nIndexStart, nNumber, &RealRoi, SubFrameIndex(i), std::ref(DataMean.SubFrameDataMean[i]), std::ref(bSubRes[i]), std::ref(DataContainer));
		}
		for (uint32_t i = 0; i < nChannelNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < nChannelNum; i++)
		{
			SubFrameDataMean(nIndexStart, nNumber, &RealRoi, SubFrameIndex(i), DataMean.SubFrameDataMean[i], bSubRes[i], DataContainer);
		}
	}
	for (uint32_t i = 0; i < nChannelNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	delete[] bSubRes;

	if (bRet)
	{
		for (uint32_t nChannelIndex = 0; nChannelIndex < nChannelNum; nChannelIndex++)
		{
			DataMean.DataMeanFrame += DataMean.SubFrameDataMean[nChannelIndex];
		}

		DataMean.DataMeanFrame /= nChannelNum;
	}
	return bRet;
}

bool CAlp014AAAPSMPAlgorithm::Linearity(std::vector<APSDataMeanType>& LightMean, std::vector<double>& ExpTime, APSLinearityType& LinearityRes)
{
	uint32_t nChannelNum = m_nMaxSubFramesNum;

	if (LightMean.size() != ExpTime.size() && LightMean.size() != 0)
	{
		std::string strErr = "Linearity: Size Error: LightMean Size: " + std::to_string(LightMean.size()) + ", ExpTime Size: " + std::to_string(ExpTime.size());
		WriteLog(strErr, nChannelNum);
		return false;
	}
	for (uint32_t nIndex = 0; nIndex < LightMean.size(); nIndex++)
	{
		if (LightMean[nIndex].SubFrameDataMean.size() < nChannelNum)
		{
			std::string strErr = "LightMean: Size Error: Index: " + std::to_string(nIndex) + ", Sub Frame Size: " + std::to_string(LightMean[nIndex].SubFrameDataMean.size());
			WriteLog(strErr, nChannelNum);
			return false;
		}
	}

	LinearityRes.SubFrameLinearityData.resize(nChannelNum);
	for (uint32_t nChannelIndex = 0; nChannelIndex < nChannelNum; nChannelIndex++)
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
			YData[nIndex] = LightMean[nIndex].SubFrameDataMean[nChannelIndex];
		}
		if (LinearityFit(XData, YData, LinearityRes.SubFrameLinearityData[nChannelIndex].k, LinearityRes.SubFrameLinearityData[nChannelIndex].b))
		{
			LinearityRes.SubFrameLinearityData[nChannelIndex].LeMax = -100000, LinearityRes.SubFrameLinearityData[nChannelIndex].LeMin = 10000;
			for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
			{
				double FitY = ExpTime[nIndex] * LinearityRes.SubFrameLinearityData[nChannelIndex].k + LinearityRes.SubFrameLinearityData[nChannelIndex].b;
				double LE = 100 * (YData[nIndex] - FitY) / FitY;
				if (LE > LinearityRes.SubFrameLinearityData[nChannelIndex].LeMax)
					LinearityRes.SubFrameLinearityData[nChannelIndex].LeMax = LE;
				if (LE < LinearityRes.SubFrameLinearityData[nChannelIndex].LeMin)
					LinearityRes.SubFrameLinearityData[nChannelIndex].LeMin = LE;
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

bool CAlp014AAAPSMPAlgorithm::OverallSystemGain(std::vector<APSTNoiseType>& LightTNoiseData, std::vector<APSDataMeanType>& LightMean, APSTNoiseType DarkTNoiseBase, APSOverallSystemGainType& GainRes)
{
	uint32_t nChannelNum = m_nMaxSubFramesNum;

	if (LightTNoiseData.size() != LightMean.size())
	{
		std::string strErr = "OverallSystemGain: Size Error: LightTNoiseData Size: " + std::to_string(LightTNoiseData.size()) + ", LightMean Size: " + std::to_string(LightMean.size());
		WriteLog(strErr, nChannelNum);
		return false;
	}
	GainRes.SubFrameGainK.resize(nChannelNum);
	for (uint32_t nChannelIndex = 0; nChannelIndex < nChannelNum; nChannelIndex++)
	{
		uint32_t nDataNum = LightMean.size();
		std::vector<double> XData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			XData[nIndex] = LightMean[nIndex].SubFrameDataMean[nChannelIndex];
		}
		std::vector<double> YData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			YData[nIndex] = LightTNoiseData[nIndex].SubFrameTNoiseData[nChannelIndex].TempNoise * LightTNoiseData[nIndex].SubFrameTNoiseData[nChannelIndex].TempNoise - DarkTNoiseBase.SubFrameTNoiseData[nChannelIndex].TempNoise * DarkTNoiseBase.SubFrameTNoiseData[nChannelIndex].TempNoise;
		}
		double k = 0.0, b = 0.0;
		if (LinearityFit(XData, YData, k, b))
		{
			GainRes.SubFrameGainK[nChannelIndex] = k;
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

bool CAlp014AAAPSMPAlgorithm::Saturation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSaturationType& SaturationRes)
{
	uint32_t nChannelNum = m_nMaxSubFramesNum;
	if (nChannelIndex >= nChannelNum)
	{
		std::string strErr = "Saturation: SubFrameIndex beyond the max num";
		WriteLog(strErr, nChannelIndex);
		return false;
	}

	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}

	RawDataContainer& DataContainer = m_bUse16SubFrame ? m_16SubRawDataContainer : m_RawDataContainer;

	if (m_bUse16SubFrame)
	{
		RealRoi.Up /= 2;
		RealRoi.Left /= 2;
		RealRoi.Right = (RealRoi.Right + 1) / 2 - 1;
		RealRoi.Down = (RealRoi.Down + 1) / 2 - 1;
	}

	bool bRes = false;
	SubFrameDataMean(nIndexStart, nNumber, &RealRoi, nChannelIndex, SaturationRes.SaturationMean, bRes, DataContainer);
	if (!bRes)
	{
		return false;
	}
	APSSubFrameTNoiseType TNoise;
	SubFrameTNoise(nIndexStart, nNumber, &RealRoi, nChannelIndex, TNoise, bRes, DataContainer);
	if (!bRes)
	{
		return false;
	}
	SaturationRes.SaturationTNoise = TNoise.TempNoise;
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

bool CAlp014AAAPSMPAlgorithm::OETC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSOETCType& OETCRes)
{
	uint32_t nChannelNum = m_nMaxSubFramesNum;
	if (nChannelIndex >= nChannelNum)
	{
		std::string strErr = "OETC: SubFrameIndex beyond the max num";
		WriteLog(strErr, nChannelIndex);
		return false;
	}

	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}

	uint32_t nRoiRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nRoiCol = RealRoi.Right - RealRoi.Left + 1;
	ROIArea temp = { 0 };
	temp.Up = RealRoi.Up + nRoiRow / 2 - m_AlgorithmThre.nOETCRadius;
	temp.Down = RealRoi.Up + nRoiRow / 2 + m_AlgorithmThre.nOETCRadius;
	temp.Left = RealRoi.Left + nRoiCol / 2 - m_AlgorithmThre.nOETCRadius;
	temp.Right = RealRoi.Left + nRoiCol / 2 + m_AlgorithmThre.nOETCRadius;

	RealRoi = temp;

	RawDataContainer& DataContainer = m_RawDataContainer;

	OETCRes.ReadNoiseData.resize(nNumber / 2);
	OETCRes.DataMean.resize(nNumber / 2);
	OETCRes.TNoiseData.resize(nNumber / 2);
	bool bRes = false;

	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex += 2)
	{
		SubFrameDataMean(nIndexStart + nIndex, 2, &RealRoi, nChannelIndex, OETCRes.DataMean[nIndex / 2], bRes, DataContainer);
		if (!bRes)
		{
			return false;
		}
		APSSubFrameTNoiseType temp;
		SubFrameTNoise(nIndexStart + nIndex, 2, &RealRoi, nChannelIndex, temp, bRes, DataContainer);
		if (!bRes)
		{
			return false;
		}
		OETCRes.TNoiseData[nIndex / 2] = temp.TempNoise;
		SubFrameReadNoise(nIndexStart + nIndex, nIndexStart + nIndex + 1, &RealRoi, nChannelIndex, OETCRes.ReadNoiseData[nIndex / 2], bRes, DataContainer);
		if (!bRes)
		{
			return false;
		}
	}
	double dMaxValue = 0;
	uint32_t nMaxLocal = 0;
	//Max(dMaxValue, nMaxLocal, TNoiseData, TNoiseData.size());
	//SubFrameReadNoise(0, 1, &RealRoi, nChannelIndex, ReadNoiseData, bRes, DataContainer);

	for (uint32_t nIndex = 0; nIndex < OETCRes.ReadNoiseData.size() - 1; nIndex += 1)
	{
		double U = OETCRes.ReadNoiseData[nIndex] - OETCRes.ReadNoiseData[nIndex + 1];
		if ((U > 1.5) && (nIndex > 10))
		{
			nMaxLocal = nIndex;
			break;
		}
	}

	if (nMaxLocal == 0)
	{
		Max(dMaxValue, nMaxLocal, OETCRes.ReadNoiseData, OETCRes.ReadNoiseData.size());

		nMaxLocal = (nMaxLocal + 2) >= OETCRes.ReadNoiseData.size() ? OETCRes.ReadNoiseData.size() - 1 : nMaxLocal + 2;

	}

	if (0 == OETCRes.ReadNoiseData[0])
	{
		std::string strErr = "OETC:  ReadNoise abnormal";
		WriteLog(strErr, nChannelIndex);
		return false;
	}
	OETCRes.FWC = OETCRes.DataMean[nMaxLocal] - OETCRes.DataMean[0];
	OETCRes.ReadNoise = OETCRes.ReadNoiseData[0];
	OETCRes.DR_dB = 20 * log10(OETCRes.FWC / OETCRes.ReadNoise);

	std::vector<double> XData;
	std::vector<double> YData;

	for (uint32_t n = 0; n < OETCRes.DataMean.size(); n++)
	{
		if ((OETCRes.DataMean[n] - OETCRes.DataMean[0]) >= OETCRes.FWC * 0.1 && (OETCRes.DataMean[n] - OETCRes.DataMean[0]) <= OETCRes.FWC * 0.7)
		{
			XData.push_back(OETCRes.DataMean[n] - OETCRes.DataMean[0]);
			YData.push_back(OETCRes.ReadNoiseData[n] * OETCRes.ReadNoiseData[n] - OETCRes.ReadNoiseData[0] * OETCRes.ReadNoiseData[0]);
		}
	}

	double k = 0.0, b = 0.0;
	if (LinearityFit(XData, YData, k, b))
	{
		OETCRes.ConversionGain = 1 / k;
	}
	else
	{
		std::string strErr = "OETC: LinearityFit Error";
		WriteLog(strErr, nChannelIndex);
		return false;
	}

	OETCRes.FWC_e = OETCRes.FWC * OETCRes.ConversionGain;
	OETCRes.ReadNoise_e = OETCRes.ReadNoise * OETCRes.ConversionGain;
	return true;
}

bool CAlp014AAAPSMPAlgorithm::Linearity(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSSNRType& SSNRRes)
{
	uint32_t nChannelNum = m_nMaxSubFramesNum;
	if (nChannelIndex >= nChannelNum)
	{
		std::string strErr = "Linearity: SubFrameIndex beyond the max num";
		WriteLog(strErr, nChannelIndex);
		return false;
	}

	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}

	RawDataContainer& DataContainer = m_RawDataContainer;

	uint32_t nRoiRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nRoiCol = RealRoi.Right - RealRoi.Left + 1;
	ROIArea temp = { 0 };
	temp.Up = RealRoi.Up + nRoiRow / 2 - m_AlgorithmThre.nLinearityRadius;
	temp.Down = RealRoi.Up + nRoiRow / 2 + m_AlgorithmThre.nLinearityRadius;
	temp.Left = RealRoi.Left + nRoiCol / 2 - m_AlgorithmThre.nLinearityRadius;
	temp.Right = RealRoi.Left + nRoiCol / 2 + m_AlgorithmThre.nLinearityRadius;

	RealRoi = temp;

	SSNRRes.SNoiseData.resize(nNumber);
	SSNRRes.DataMean.resize(nNumber);
	SSNRRes.SSNR.resize(nNumber);
	SSNRRes.MaxSSNR = 0;
	bool bRes = false;

	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex += 1)
	{
		SubFrameDataMean(nIndexStart + nIndex, 1, &RealRoi, nChannelIndex, SSNRRes.DataMean[nIndex], bRes, DataContainer);
		if (!bRes)
		{
			return false;
		}
		APSSubFrameSNoiseType temp;
		SubFrameSNoise(nIndexStart + nIndex, 1, &RealRoi, nChannelIndex, temp, bRes, DataContainer);
		if (!bRes)
		{
			return false;
		}
		SSNRRes.SNoiseData[nIndex] = temp.SNoise;

		if (SSNRRes.SNoiseData[nIndex] != 0)
		{
			double dLinerimg = SSNRRes.DataMean[nIndex] - SSNRRes.DataMean[0];
			if (dLinerimg > 0)
				SSNRRes.SSNR[nIndex] = 20 * log10(dLinerimg / temp.SNoise);
			else
				SSNRRes.SSNR[nIndex] = -1;

		}
		else
		{
			SSNRRes.SSNR[nIndex] = 0;
		}
	}

	double dMax = 0;
	uint32_t nMaxLocal = 0;

	for (uint32_t nIndex = 0; nIndex < nNumber - 1; nIndex += 1)
	{
		double U = abs(SSNRRes.SSNR[nIndex + 1] - SSNRRes.SSNR[nIndex]);
		if ((U > 2.5) && (nIndex > 10) && (nIndex < nNumber - 5))
		{
			nMaxLocal = nIndex;
			break;
		}
	}

	if (nMaxLocal == 0)
	{
		SSNRRes.MaxSSNR = -1; // SSNRRes.SSNR[nMaxLocal];

		//Max(dMax, nMaxLocal, SSNRRes.SSNR, SSNRRes.SSNR.size());

		//nMaxLocal = nMaxLocal >= SSNRRes.SNoiseData.size() ? SSNRRes.SNoiseData.size() - 1 : nMaxLocal;
	}
	else
	{
		SSNRRes.MaxSSNR = SSNRRes.SSNR[nMaxLocal];
	}

	return true;
}

bool CAlp014AAAPSMPAlgorithm::Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, bool bNormalize, ImgType& ImgData)
{
	uint32_t nChannelNum = m_nMaxSubFramesNum;
	if (nChannelIndex >= nChannelNum)
	{
		std::string strErr = "Show: SubFrameIndex beyond the max num";
		WriteLog(strErr, nChannelIndex);
		return false;
	}

	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}

	RawDataContainer& DataContainer = m_bUse16SubFrame ? m_16SubRawDataContainer : m_RawDataContainer;

	if (0 == nNumber || nIndexStart >= DataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > DataContainer[nChannelIndex].size())
	{
		std::string strErr = "Show: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		return false;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "Show: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	uint32_t nChannelRow = m_nChannelRow;
	uint32_t nChannelCol = m_nChannelCol;
	if (m_bUse16SubFrame)
	{
		RealRoi.Up /= 2;
		RealRoi.Left /= 2;
		RealRoi.Right = (RealRoi.Right + 1) / 2 - 1;
		RealRoi.Down = (RealRoi.Down + 1) / 2 - 1;
		nChannelRow /= 2;
		nChannelCol /= 2;
	}

	ImgData.resize(nChannelRow);
	for (uint32_t nIndex = 0; nIndex < nChannelRow; nIndex++)
	{
		ImgData[nIndex].resize(nChannelCol);
	}
	if (!bNormalize)
	{
		for (uint32_t nRows = 0; nRows < nChannelRow; nRows++)
		{
			for (uint32_t nCols = 0; nCols < nChannelCol; nCols++)
			{
				if (PosInRoi(nRows, nCols, RealRoi))
				{
					double temp = 0;
					for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
					{
						temp += DataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols];
					}
					temp /= nNumber;
					if (temp < 0)
					{
						ImgData[nRows][nCols] = 0;
					}
					else
					{
						if (m_RawType == APSRawType::RAW8)
						{
							ImgData[nRows][nCols] = floor(temp);
						}
						else if (m_RawType == APSRawType::RAW10 || m_RawType == APSRawType::UNPACK10)
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
		CAPSDataContainer NormalizeDataContainer;
		NormalizeDataContainer.Init(nChannelRow, nChannelCol, true);
		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			NormalizeDataContainer += DataContainer[nChannelIndex][nIndexStart + nIndex];
		}
		NormalizeDataContainer /= nNumber;
		Max(dMaxValue, temp, NormalizeDataContainer, &RealRoi);
		Min(dMinValue, temp, NormalizeDataContainer, &RealRoi);

		for (uint32_t nRows = 0; nRows < nChannelRow; nRows++)
		{
			for (uint32_t nCols = 0; nCols < nChannelCol; nCols++)
			{
				if (PosInRoi(nRows, nCols, RealRoi))
				{
					double NewValue = (NormalizeDataContainer.m_RawData[nRows][nCols] - dMinValue) / (dMaxValue - dMinValue) * 255;
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

bool CAlp014AAAPSMPAlgorithm::Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSType& ImgData)
{
	uint32_t nChannelNum = m_nMaxSubFramesNum;
	if (nChannelIndex >= nChannelNum)
	{
		std::string strErr = "Show: SubFrameIndex beyond the max num";
		WriteLog(strErr, nChannelIndex);
		return false;
	}

	ROIArea RealRoi = { 0 };
	if (ROI == nullptr)
	{
		RealRoi = m_ActiveArea;
	}
	else
	{
		RealRoi = *ROI;
	}
	RawDataContainer& DataContainer = m_bUse16SubFrame ? m_16SubRawDataContainer : m_RawDataContainer;

	if (0 == nNumber || nIndexStart >= DataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > DataContainer[nChannelIndex].size())
	{
		std::string strErr = "Show: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		return false;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "Show: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	uint32_t nChannelRow = m_nChannelRow;
	uint32_t nChannelCol = m_nChannelCol;
	if (m_bUse16SubFrame)
	{
		RealRoi.Up /= 2;
		RealRoi.Left /= 2;
		RealRoi.Right = (RealRoi.Right + 1) / 2 - 1;
		RealRoi.Down = (RealRoi.Down + 1) / 2 - 1;
		nChannelRow /= 2;
		nChannelCol /= 2;
	}

	ImgData.resize(nChannelRow);
	for (uint32_t nIndex = 0; nIndex < nChannelRow; nIndex++)
	{
		ImgData[nIndex].resize(nChannelCol);
	}
	for (uint32_t nRows = 0; nRows < nChannelRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nChannelCol; nCols++)
		{
			if (PosInRoi(nRows, nCols, RealRoi))
			{
				double temp = 0;
				for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
				{
					temp += DataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols];
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

void CAlp014AAAPSMPAlgorithm::SetRawDataSize(uint32_t nRow, uint32_t nCol)
{
	m_nTotalRow = nRow;
	m_nTotalCol = nCol;
	m_nChannelRow = m_nTotalRow;
	m_nChannelCol = m_nTotalCol;
}

bool CAlp014AAAPSMPAlgorithm::GetBlockMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, uint32_t nRowBlockNum, uint32_t nColBlockNum, std::vector<CAPSDataContainer>& BlockData, uint32_t nSubRowBlockSize, uint32_t nSubColBlockSize)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];

	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i] = new std::thread(&CAlp014AAAPSMPAlgorithm::SubFrameBlockMean, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), nRowBlockNum, nColBlockNum, nSubRowBlockSize, nSubColBlockSize, std::ref(BlockData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
		{
			SubFrameBlockMean(nIndexStart, nNumber, ROI, SubFrameIndex(i), nRowBlockNum, nColBlockNum, nSubRowBlockSize, nSubColBlockSize, BlockData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < m_nMaxSubFramesNum; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}
