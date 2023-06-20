#include "AlpAPSMPAlgorithm.h"
#include <thread>
#include <algorithm>
#include <stack>
#include <map>
#include <fstream>
#include "Changelist.h"

bool operator< (const Local& lh, const Local& rh)
{
	if (lh.x < rh.x)
	{
		return true;
	}
	else if (lh.x == rh.x)
	{
		return lh.y < rh.y;
	}
	else
	{
		return false;
	}
}

CAlpAPSMPAlgorithm::CAlpAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat)
{
	m_nSiteNum = nSiteNum;
	m_bMultiThreadEnable = false;
	m_bLogEnable = false;
	m_SensorType = Sensortype;
	m_RawType = Rawtype;
	m_AlgorithmThre.dBadPixelThre = 0.19;
	m_AlgorithmThre.dBadLineThre = 0.1;
	m_AlgorithmThre.dHotPixelThre = 120;
	m_AlgorithmThre.dHotLineThre = 60;
	m_AlgorithmThre.nBadPixelRadius = 1;
	m_AlgorithmThre.nBadLineRadius = 2;
	m_AlgorithmThre.nDSNURowBlockNum = 60;
	m_AlgorithmThre.nDSNUColBlockNum = 78;
	m_AlgorithmThre.nDSNURowBlockSize = 20;
	m_AlgorithmThre.nDSNUColBlockSize = 20;
	m_AlgorithmThre.nYShadingRowBlockNum = 5;
	m_AlgorithmThre.nYShadingColBlockNum = 5;
	m_AlgorithmThre.nColorShadingRowBlockNum = 13;
	m_AlgorithmThre.nColorShadingColBlockNum = 17;
	m_AlgorithmThre.nPedestalVariationRowBlockNum = 8;
	m_AlgorithmThre.nPedestalVariationColBlockNum = 8;
	m_AlgorithmThre.nPedestalVariationRowBlockSize = 150;
	m_AlgorithmThre.nPedestalVariationColBlockSize = 200;
	m_AlgorithmThre.nBadPixelMaxLen = 200;
	m_AlgorithmThre.nBadPixelLocalRowOffset = 104; // for 003CA
	m_AlgorithmThre.nBadPixelLocalColOffset = 52;  // for 003CA

	m_RawDataContainer.resize(SubFrameIndex::All);
	m_PixelFormat = Pixelformat;

	if (m_RawType == RAW8)
	{
		m_dPedestal = 16;
	}
	else if (m_RawType == RAW10 || m_RawType == UNPACK10)
	{
		m_dPedestal = 64;
	}
	else
	{
		m_dPedestal = 256;
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

bool CAlpAPSMPAlgorithm::TNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSTNoiseType& TNoiseRes)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];
	TNoiseRes.SubFrameTNoiseData.resize(SubFrameIndex::All);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameTNoise, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), std::ref(TNoiseRes.SubFrameTNoiseData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			SubFrameTNoise(nIndexStart, nNumber, ROI, SubFrameIndex(i), TNoiseRes.SubFrameTNoiseData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::SNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSNoiseType& SNoiseData)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];
	SNoiseData.SNoiseFrame = 0;
	SNoiseData.SubFrameSNoiseData.resize(SubFrameIndex::All);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameSNoise, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), std::ref(SNoiseData.SubFrameSNoiseData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			SubFrameSNoise(nIndexStart, nNumber, ROI, SubFrameIndex(i), SNoiseData.SubFrameSNoiseData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	if (bRet)
	{
		ROIArea RealRoi;

		if (ROI == nullptr)
		{
			RealRoi = m_ActiveArea;
		}
		else
		{
			RealRoi = *ROI;
		}
		uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
		uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

		std::vector<double> AllPixel(nRow * nCol * SubFrameIndex::All);

		uint32_t nCur = 0;
		for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
		{
			for (uint32_t nRows = 0; nRows < nRow; nRows++)
			{
				for (uint32_t nCols = 0; nCols < nCol; nCols++)
				{
					double value = 0;
					for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
					{
						value += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
					}
					AllPixel[nCur++] = round(value / nNumber);
				}
			}
		}
		SNoiseData.SNoiseFrame = Std(AllPixel, nCur);
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::BadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& BadpixelRes)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];
	BadpixelRes.BadPixelNum = 0;
	BadpixelRes.ClusterNum = 0;
	BadpixelRes.CoupletNum = 0;
	BadpixelRes.LadderNum = 0;
	BadpixelRes.SingletNum = 0;
	BadpixelRes.BadPixelMask.BadPixelNum = 0;
	BadpixelRes.BadPixelMask.LocalData.clear();
	BadpixelRes.BadPixelMask.Flag.clear();

	BadpixelRes.SubFrameBadpixelData.resize(SubFrameIndex::All);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameBadPixel, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), std::ref(BadpixelRes.SubFrameBadpixelData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			SubFrameBadPixel(nIndexStart, nNumber, ROI, SubFrameIndex(i), BadpixelRes.SubFrameBadpixelData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	if (bRet)
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			BadpixelRes.BadPixelNum += BadpixelRes.SubFrameBadpixelData[i].BadPixelNum;
			BadpixelRes.ClusterNum += BadpixelRes.SubFrameBadpixelData[i].ClusterNum;
			BadpixelRes.CoupletNum += BadpixelRes.SubFrameBadpixelData[i].CoupletNum;
			BadpixelRes.SingletNum += BadpixelRes.SubFrameBadpixelData[i].SingletNum;
			Local Total;
			for (uint32_t n = 0; n < BadpixelRes.SubFrameBadpixelData[i].BadPixelMask.BadPixelNum; n++)
			{
				BadpixelRes.BadPixelMask.BadPixelNum++;
				SubFrameLocalToTotalLocal(BadpixelRes.SubFrameBadpixelData[i].BadPixelMask.LocalData[n], SubFrameIndex(i), Total);
				BadpixelRes.BadPixelMask.LocalData.push_back(Total);
				BadpixelRes.BadPixelMask.Flag.push_back(BadpixelRes.SubFrameBadpixelData[i].BadPixelMask.Flag[n]);
			}
		}

		std::vector<std::vector<uint32_t>> BadPixelMask(m_nTotalRow);
		for (uint32_t i = 0; i < m_nTotalRow; i++)
		{
			BadPixelMask[i].resize(m_nTotalCol, 0);
		}

		for (uint32_t n = 0; n < BadpixelRes.BadPixelMask.BadPixelNum; n++)
		{
			Local temp = BadpixelRes.BadPixelMask.LocalData[n];

			if (m_PixelFormat > 3)
			{
				if (temp.x % 4 == 1)
				{
					++temp.x;
				}
				else if (temp.x % 4 == 2)
				{
					--temp.x;
				}
				if (temp.y % 4 == 1)
				{
					++temp.y;
				}
				else if (temp.y % 4 == 2)
				{
					--temp.y;
				}
			}
			BadPixelMask[temp.x][temp.y] = BadpixelRes.BadPixelMask.Flag[n];
		}

		for (uint32_t nRows = 0; nRows < m_nTotalRow; nRows++)
		{
			for (uint32_t nCols = 0; nCols < m_nTotalCol - 1;)
			{
				if (BadPixelMask[nRows][nCols])
				{
					if (BadPixelMask[nRows][nCols + 1])
					{
						++BadpixelRes.LadderNum;
					}
					nCols += 2;
				}
				else
				{
					++nCols;
				}
			}
		}
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::HotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& HotpixelRes)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];
	HotpixelRes.BadPixelNum = 0;
	HotpixelRes.ClusterNum = 0;
	HotpixelRes.CoupletNum = 0;
	HotpixelRes.LadderNum = 0;
	HotpixelRes.SingletNum = 0;
	HotpixelRes.BadPixelMask.BadPixelNum = 0;
	HotpixelRes.BadPixelMask.LocalData.clear();
	HotpixelRes.BadPixelMask.Flag.clear();
	HotpixelRes.SubFrameBadpixelData.resize(SubFrameIndex::All);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameHotPixel, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), std::ref(HotpixelRes.SubFrameBadpixelData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			SubFrameHotPixel(nIndexStart, nNumber, ROI, SubFrameIndex(i), HotpixelRes.SubFrameBadpixelData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	if (bRet)
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			HotpixelRes.BadPixelNum += HotpixelRes.SubFrameBadpixelData[i].BadPixelNum;
			HotpixelRes.ClusterNum += HotpixelRes.SubFrameBadpixelData[i].ClusterNum;
			HotpixelRes.CoupletNum += HotpixelRes.SubFrameBadpixelData[i].CoupletNum;
			HotpixelRes.SingletNum += HotpixelRes.SubFrameBadpixelData[i].SingletNum;
			Local Global;
			for (uint32_t n = 0; n < HotpixelRes.SubFrameBadpixelData[i].BadPixelMask.BadPixelNum; n++)
			{
				HotpixelRes.BadPixelMask.BadPixelNum++;
				SubFrameLocalToTotalLocal(HotpixelRes.SubFrameBadpixelData[i].BadPixelMask.LocalData[n], SubFrameIndex(i), Global);
				HotpixelRes.BadPixelMask.LocalData.push_back(Global);
				HotpixelRes.BadPixelMask.Flag.push_back(HotpixelRes.SubFrameBadpixelData[i].BadPixelMask.Flag[n]);
			}
		}

		std::vector<std::vector<uint32_t>> BadPixelMask(m_nTotalRow);
		for (uint32_t i = 0; i < m_nTotalRow; i++)
		{
			BadPixelMask[i].resize(m_nTotalCol, 0);
		}

		for (uint32_t n = 0; n < HotpixelRes.BadPixelMask.BadPixelNum; n++)
		{
			Local temp = HotpixelRes.BadPixelMask.LocalData[n];

			if (m_PixelFormat > 3)
			{
				if ((temp.x & 3) == 1)
				{
					++temp.x;
				}
				else if ((temp.x & 3) == 2)
				{
					--temp.x;
				}
				if ((temp.y & 3) == 1)
				{
					++temp.y;
				}
				else if ((temp.y & 3) == 2)
				{
					--temp.y;
				}
			}
			BadPixelMask[temp.x][temp.y] = HotpixelRes.BadPixelMask.Flag[n];
		}

		for (uint32_t nRows = 0; nRows < m_nTotalRow; nRows++)
		{
			for (uint32_t nCols = 0; nCols < m_nTotalCol - 1;)
			{
				if (BadPixelMask[nRows][nCols])
				{
					if (BadPixelMask[nRows][nCols + 1])
					{
						++HotpixelRes.LadderNum;
					}
					nCols += 2;
				}
				else
				{
					++nCols;
				}
			}
		}
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::BLC(uint32_t nIndexStart, uint32_t nNumber)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];
	std::vector<double> BaseMean(SubFrameIndex::All, m_dPedestal);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameBLC, this, nIndexStart, nNumber, nullptr, SubFrameIndex(i), std::ref(BaseMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			SubFrameBLC(nIndexStart, nNumber, nullptr, SubFrameIndex(i), BaseMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::BLC(uint32_t nIndexStart, uint32_t nNumber, APSDataMeanType& BaseMean)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];

	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameBLC, this, nIndexStart, nNumber, nullptr, SubFrameIndex(i), std::ref(BaseMean.SubFrameDataMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			SubFrameBLC(nIndexStart, nNumber, nullptr, SubFrameIndex(i), BaseMean.SubFrameDataMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::BLC(uint32_t nIndexStart, uint32_t nNumber, uint32_t nBaseIndexStart, uint32_t nBaseNumber)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];
	std::vector<double> ColMean[SubFrameIndex::All];

	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameColMean, this, nBaseIndexStart, nBaseNumber, nullptr, SubFrameIndex(i), std::ref(ColMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			SubFrameColMean(nBaseIndexStart, nBaseNumber, nullptr, SubFrameIndex(i), ColMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}

	if (!bRet)
	{
		return false;
	}

	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameBLCByColBase, this, nIndexStart, nNumber, nullptr, SubFrameIndex(i), std::ref(ColMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			SubFrameBLCByColBase(nIndexStart, nNumber, nullptr, SubFrameIndex(i), ColMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::DPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSBadpixelType& BadPixelMask)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];

	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameDPC, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), std::ref(BadPixelMask.SubFrameBadpixelData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			SubFrameDPC(nIndexStart, nNumber, ROI, SubFrameIndex(i), BadPixelMask.SubFrameBadpixelData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::BadPixelLocalToOtpType(std::vector<Local>& BadPixelLocal, std::vector<uint8_t>& OtpData)
{
	if (BadPixelLocal.size() > m_AlgorithmThre.nBadPixelMaxLen)
	{
		std::string strErr = "BadPixelLocalToOtpType: Size error: Bad pixel size: " + std::to_string(BadPixelLocal.size()) + ", Max lens: " + std::to_string(m_AlgorithmThre.nBadPixelMaxLen);
		WriteLog(strErr, All);
		return false;
	}
	std::sort(BadPixelLocal.begin(), BadPixelLocal.end());

	OtpData.resize(BadPixelLocal.size() * 3);

	uint32_t nCur = 0;
	for (uint32_t nIndex = 0; nIndex < BadPixelLocal.size(); nIndex++)
	{
		uint16_t nRow = static_cast<uint16_t>(BadPixelLocal[nIndex].x + m_AlgorithmThre.nBadPixelLocalRowOffset);
		uint16_t nCol = static_cast<uint16_t>(BadPixelLocal[nIndex].y + m_AlgorithmThre.nBadPixelLocalColOffset);
		uint8_t uData1 = nCol & 0xFF;
		uint8_t uData2 = ((nRow << 4) & 0xF0) +((nCol >> 8) & 0x0F);
		uint8_t uData3 = (nRow >> 4) & 0xFF;
		OtpData[nCur++] = uData1;
		OtpData[nCur++] = uData2;
		OtpData[nCur++] = uData3;
	}

	return true;
}

bool CAlpAPSMPAlgorithm::YShading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSYShadingType& ShadingRes)
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
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
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
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;
	std::vector<CAPSDataContainer>BlockData(SubFrameIndex::All);

	if (!GetBlockMean(nIndexStart, nNumber, &RealRoi, m_AlgorithmThre.nYShadingRowBlockNum, m_AlgorithmThre.nYShadingColBlockNum, BlockData))
	{
		std::string strErr = "YShading: GetBlockMean error";
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	double YCenter = (BlockData[Gr].m_RawData[m_AlgorithmThre.nYShadingRowBlockNum / 2][m_AlgorithmThre.nYShadingColBlockNum / 2] +
		BlockData[Gb].m_RawData[m_AlgorithmThre.nYShadingRowBlockNum / 2][m_AlgorithmThre.nYShadingColBlockNum / 2]) / 2;

	if (YCenter <= 0)
	{
		std::string strErr = "YShading: Y Center error";
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	ShadingRes.YShadingData.resize(m_AlgorithmThre.nYShadingRowBlockNum);
	for (uint32_t nRows = 0; nRows < m_AlgorithmThre.nYShadingRowBlockNum; nRows++)
	{
		ShadingRes.YShadingData[nRows].resize(m_AlgorithmThre.nYShadingColBlockNum);

		for (uint32_t nCols = 0; nCols < m_AlgorithmThre.nYShadingColBlockNum; nCols++)
		{
			double Y = (BlockData[Gr].m_RawData[nRows][nCols] + BlockData[Gb].m_RawData[nRows][nCols]) / 2;

			ShadingRes.YShadingData[nRows][nCols] = Y / YCenter;
		}
	}
	ShadingRes.YShadingLT = ShadingRes.YShadingData[0][0];
	ShadingRes.YShadingLB = ShadingRes.YShadingData[m_AlgorithmThre.nYShadingRowBlockNum - 1][0];
	ShadingRes.YShadingRT = ShadingRes.YShadingData[0][m_AlgorithmThre.nYShadingColBlockNum - 1];
	ShadingRes.YShadingLB = ShadingRes.YShadingData[m_AlgorithmThre.nYShadingRowBlockNum - 1][m_AlgorithmThre.nYShadingColBlockNum - 1];

	return true;
}

bool CAlpAPSMPAlgorithm::ColorShading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSColorShadingType& ShadingRes)
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
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
	{
		if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size())
		{
			std::string strErr = "ColorShading: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
			WriteLog(strErr, nChannelIndex);
			return false;
		}
	}

	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "ColorShading: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;
	std::vector<CAPSDataContainer>BlockData(SubFrameIndex::All);

	if (!GetBlockMean(nIndexStart, nNumber, &RealRoi, m_AlgorithmThre.nColorShadingRowBlockNum, m_AlgorithmThre.nColorShadingColBlockNum, BlockData))
	{
		std::string strErr = "ColorShading: GetBlockMean error";
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	double RCenter = BlockData[R].m_RawData[m_AlgorithmThre.nColorShadingRowBlockNum / 2][m_AlgorithmThre.nColorShadingColBlockNum / 2];
	double BCenter = BlockData[B].m_RawData[m_AlgorithmThre.nColorShadingRowBlockNum / 2][m_AlgorithmThre.nColorShadingColBlockNum / 2];
	double GCenter = (BlockData[Gr].m_RawData[m_AlgorithmThre.nColorShadingRowBlockNum / 2][m_AlgorithmThre.nColorShadingColBlockNum / 2] +
		BlockData[Gb].m_RawData[m_AlgorithmThre.nColorShadingRowBlockNum / 2][m_AlgorithmThre.nColorShadingColBlockNum / 2]) / 2;

	if (GCenter <= 0)
	{
		std::string strErr = "ColorShading: G Center error";
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}
	double RGCenter = RCenter / GCenter;
	double BGCenter = BCenter / GCenter;

	ShadingRes.ColorShadingRGData.resize(m_AlgorithmThre.nColorShadingRowBlockNum);
	ShadingRes.ColorShadingBGData.resize(m_AlgorithmThre.nColorShadingRowBlockNum);
	for (uint32_t nRows = 0; nRows < m_AlgorithmThre.nColorShadingRowBlockNum; nRows++)
	{
		ShadingRes.ColorShadingRGData[nRows].resize(m_AlgorithmThre.nColorShadingColBlockNum);
		ShadingRes.ColorShadingBGData[nRows].resize(m_AlgorithmThre.nColorShadingColBlockNum);

		for (uint32_t nCols = 0; nCols < m_AlgorithmThre.nColorShadingColBlockNum; nCols++)
		{
			double GMean = (BlockData[Gr].m_RawData[nRows][nCols] + BlockData[Gb].m_RawData[nRows][nCols]) / 2;
			double RMean = BlockData[R].m_RawData[nRows][nCols];
			double BMean = BlockData[B].m_RawData[nRows][nCols];

			if (GMean <= 0)
			{
				std::string strErr = "ColorShading: G Block error";
				WriteLog(strErr, SubFrameIndex::All);
				return false;
			}

			double RG = RMean / GMean;
			double BG = BMean / GMean;

			ShadingRes.ColorShadingRGData[nRows][nCols] = RG / RGCenter;
			ShadingRes.ColorShadingBGData[nRows][nCols] = BG / BGCenter;
		}
	}

	ShadingRes.ColorShadingRGLT = ShadingRes.ColorShadingRGData[0][0];
	ShadingRes.ColorShadingRGLB = ShadingRes.ColorShadingRGData[m_AlgorithmThre.nColorShadingRowBlockNum - 1][0];
	ShadingRes.ColorShadingRGRT = ShadingRes.ColorShadingRGData[0][m_AlgorithmThre.nColorShadingColBlockNum - 1];
	ShadingRes.ColorShadingRGLB = ShadingRes.ColorShadingRGData[m_AlgorithmThre.nColorShadingRowBlockNum - 1][m_AlgorithmThre.nColorShadingColBlockNum - 1];

	ShadingRes.ColorShadingBGLT = ShadingRes.ColorShadingBGData[0][0];
	ShadingRes.ColorShadingBGLB = ShadingRes.ColorShadingBGData[m_AlgorithmThre.nColorShadingRowBlockNum - 1][0];
	ShadingRes.ColorShadingBGRT = ShadingRes.ColorShadingBGData[0][m_AlgorithmThre.nColorShadingColBlockNum - 1];
	ShadingRes.ColorShadingBGLB = ShadingRes.ColorShadingBGData[m_AlgorithmThre.nColorShadingRowBlockNum - 1][m_AlgorithmThre.nColorShadingColBlockNum - 1];

	return true;
}

bool CAlpAPSMPAlgorithm::OpticalCenter(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSOpticalCenterType& OpticalCenterType)
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
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
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
		WriteLog(strErr, SubFrameIndex::All);
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
				for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
				{
					dMeanData += m_RawDataContainer[nChannelIndex][nIndexStart + nFrameIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
				}
			}
			dMeanData /= (nNumber * SubFrameIndex::All);
			RowMean[nRows] += dMeanData;
			ColMean[nCols] += dMeanData;
		}
	}

	double dMaxValue = 0;
	Max(dMaxValue, OpticalCenterType.CenterRow, RowMean, RowMean.size());
	OpticalCenterType.CenterRow += RealRoi.Up;
	Max(dMaxValue, OpticalCenterType.CenterCol, ColMean, ColMean.size());
	OpticalCenterType.CenterCol += RealRoi.Left;
	return true;
}

bool CAlpAPSMPAlgorithm::PedestalVariation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSPedestalVariationType& PedestalVariationRes)
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
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
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
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	std::vector<CAPSDataContainer>BlockData(SubFrameIndex::All);

	if (!GetBlockMean(nIndexStart, nNumber, &RealRoi, m_AlgorithmThre.nPedestalVariationRowBlockNum, m_AlgorithmThre.nPedestalVariationColBlockNum, BlockData, m_AlgorithmThre.nPedestalVariationRowBlockSize, m_AlgorithmThre.nPedestalVariationColBlockSize))
	{
		std::string strErr = "PedestalVariation: GetBlockMean error";
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	Local temp;
	for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
	{
		Max(PedestalVariationRes.PedestalMax[nIndex], temp, BlockData[nIndex]);
		Min(PedestalVariationRes.PedestalMin[nIndex], temp, BlockData[nIndex]);
	}

	return true;
}

bool CAlpAPSMPAlgorithm::ReadNoise(uint32_t nIndex1, uint32_t nIndex2, ROIArea* ROI, APSReadNoiseType& ReadNoiseRes)
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
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
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
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	std::vector<double> AllPixel(nRow * nCol * SubFrameIndex::All);

	uint32_t nCur = 0;
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
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

bool CAlpAPSMPAlgorithm::DarkCurrent(std::vector<APSDataMeanType>& DataMean, std::vector<double>& ExpTime, APSDarkCurrentType& DarkCurrentRes)
{
	if (DataMean.size() != ExpTime.size())
	{
		std::string strErr = "DarkCurrent: Size Error: Data Size: " + std::to_string(DataMean.size()) + ", ExpTime Size: " + std::to_string(ExpTime.size());
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}
	DarkCurrentRes.SubFrameKValue.resize(SubFrameIndex::All);
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
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

bool CAlpAPSMPAlgorithm::DarkCurrent(std::vector<APSTNoiseType>& TNoise, std::vector<double>& ExpTime, APSDarkCurrentType& DarkCurrentRes)
{
	if (TNoise.size() != ExpTime.size())
	{
		std::string strErr = "DarkCurrent: Size Error: Data Size: " + std::to_string(TNoise.size()) + ", ExpTime Size: " + std::to_string(ExpTime.size());
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}
	DarkCurrentRes.SubFrameKValue.resize(SubFrameIndex::All);
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
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

bool CAlpAPSMPAlgorithm::DSNU(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSDSNUType& DSNURes)
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
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
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
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;
	uint32_t nRowBlockNum = m_AlgorithmThre.nDSNURowBlockNum;
	uint32_t nColBlockNum = m_AlgorithmThre.nDSNUColBlockNum;

	std::vector<CAPSDataContainer>BlockData(SubFrameIndex::All);

	if (!GetBlockMean(nIndexStart, nNumber, &RealRoi, nRowBlockNum, nColBlockNum, BlockData, m_AlgorithmThre.nDSNURowBlockSize, m_AlgorithmThre.nDSNUColBlockSize))
	{
		std::string strErr = "DSNU: GetBlockMean error";
		WriteLog(strErr, SubFrameIndex::All);
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
			R.m_RawData[nRowBlocks][nColBlocks] = BlockData[SubFrameIndex::R].m_RawData[nRowBlocks][nColBlocks];
			//G.m_RawData[nRowBlocks][nColBlocks] = sqrt((BlockData[SubFrameIndex::Gr].m_RawData[nRowBlocks][nColBlocks] * BlockData[SubFrameIndex::Gr].m_RawData[nRowBlocks][nColBlocks] + BlockData[SubFrameIndex::Gb].m_RawData[nRowBlocks][nColBlocks] * BlockData[SubFrameIndex::Gb].m_RawData[nRowBlocks][nColBlocks]) / 2);
			G.m_RawData[nRowBlocks][nColBlocks] = (BlockData[SubFrameIndex::Gr].m_RawData[nRowBlocks][nColBlocks] + BlockData[SubFrameIndex::Gb].m_RawData[nRowBlocks][nColBlocks]) / 2;
			B.m_RawData[nRowBlocks][nColBlocks] = BlockData[SubFrameIndex::B].m_RawData[nRowBlocks][nColBlocks];
			dPedestal += R.m_RawData[nRowBlocks][nColBlocks];
			dPedestal += G.m_RawData[nRowBlocks][nColBlocks];
			dPedestal += B.m_RawData[nRowBlocks][nColBlocks];
		}
	}

	dPedestal = round(dPedestal / (3 * nRowBlockNum * nColBlockNum));

	double MaxR = R.m_RawData[0][0], MinR = R.m_RawData[0][0], MaxG = G.m_RawData[0][0], MinG = G.m_RawData[0][0], MaxB = B.m_RawData[0][0], MinB = B.m_RawData[0][0];
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

	DSNURes.RangeR = MaxR - MinR;
	DSNURes.RangeG = MaxG - MinG;
	DSNURes.RangeB = MaxB - MinB;
	DSNURes.SignalMax = MaxSignal;
	DSNURes.RMax = MaxR;
	DSNURes.RMin = MinR;
	DSNURes.GMax = MaxG;
	DSNURes.GMin = MinG;
	DSNURes.BMax = MaxB;
	DSNURes.BMin = MinB;

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
			dGlobalDeltaSignal = sqrt(pow(R.m_RawData[nRowBlockIndex][nColBlockIndex] - R.m_RawData[nMinSignalRow][nMinSignalCol], 2) + pow(G.m_RawData[nRowBlockIndex][nColBlockIndex] - G.m_RawData[nMinSignalRow][nMinSignalCol], 2) + pow(B.m_RawData[nRowBlockIndex][nColBlockIndex] - B.m_RawData[nMinSignalRow][nMinSignalCol], 2));
			if (dGlobalDeltaSignal > DSNURes.DeltaSignalMax)
			{
				DSNURes.DeltaSignalMax = dGlobalDeltaSignal;
			}

			if (PosInRoi(nRowBlockIndex, nColBlockIndex, ROICenter))
			{
				dDeltaSignal = sqrt(pow(R.m_RawData[nRowBlockIndex][nColBlockIndex] - R.m_RawData[nMinSignalCentreRow][nMinSignalCentreCol], 2) + pow(G.m_RawData[nRowBlockIndex][nColBlockIndex] - G.m_RawData[nMinSignalCentreRow][nMinSignalCentreCol], 2) + pow(B.m_RawData[nRowBlockIndex][nColBlockIndex] - B.m_RawData[nMinSignalCentreRow][nMinSignalCentreCol], 2));
				if (dDeltaSignal > DSNURes.DeltaSignalCentreMax)
				{
					DSNURes.DeltaSignalCentreMax = dDeltaSignal;
				}
			}
			else if (PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[0]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[1]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[2]) || PosInRoi(nRowBlockIndex, nColBlockIndex, ROICorner[3]))
			{
				dDeltaSignal = sqrt(pow(R.m_RawData[nRowBlockIndex][nColBlockIndex] - R.m_RawData[nMinSignalCornerRow][nMinSignalCornerCol], 2) + pow(G.m_RawData[nRowBlockIndex][nColBlockIndex] - G.m_RawData[nMinSignalCornerRow][nMinSignalCornerCol], 2) + pow(B.m_RawData[nRowBlockIndex][nColBlockIndex] - B.m_RawData[nMinSignalCornerRow][nMinSignalCornerCol], 2));
				if (dDeltaSignal > DSNURes.DeltaSignalCornerMax)
				{
					DSNURes.DeltaSignalCornerMax = dDeltaSignal;
				}
			}
			else
			{
				dDeltaSignal = sqrt(pow(R.m_RawData[nRowBlockIndex][nColBlockIndex] - R.m_RawData[nMinSignalEdgeRow][nMinSignalEdgeCol], 2) + pow(G.m_RawData[nRowBlockIndex][nColBlockIndex] - G.m_RawData[nMinSignalEdgeRow][nMinSignalEdgeCol], 2) + pow(B.m_RawData[nRowBlockIndex][nColBlockIndex] - B.m_RawData[nMinSignalEdgeRow][nMinSignalEdgeCol], 2));
				if (dDeltaSignal > DSNURes.DeltaSignalEdgeMax)
				{
					DSNURes.DeltaSignalEdgeMax = dDeltaSignal;
				}
			}
		}
	}
	return true;
}

bool CAlpAPSMPAlgorithm::DataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSDataMeanType& DataMean)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];
	DataMean.SubFrameDataMean.resize(SubFrameIndex::All);
	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameDataMean, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), std::ref(DataMean.SubFrameDataMean[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			SubFrameDataMean(nIndexStart, nNumber, ROI, SubFrameIndex(i), DataMean.SubFrameDataMean[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::Linearity(std::vector<APSDataMeanType>& LightMean, std::vector<double>& ExpTime, APSLinearityType& LinearityRes)
{
	if (LightMean.size() != ExpTime.size() && LightMean.size() != 0)
	{
		std::string strErr = "Linearity: Size Error: LightMean Size: " + std::to_string(LightMean.size()) + ", ExpTime Size: " + std::to_string(ExpTime.size());
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}
	for (uint32_t nIndex = 0; nIndex < LightMean.size(); nIndex++)
	{
		if (LightMean[nIndex].SubFrameDataMean.size() != SubFrameIndex::All)
		{
			std::string strErr = "LightMean: Size Error: Index: " + std::to_string(nIndex) + ", Sub Frame Size: " + std::to_string(LightMean[nIndex].SubFrameDataMean.size());
			WriteLog(strErr, SubFrameIndex::All);
			return false;
		}
	}

	LinearityRes.SubFrameLinearityData.resize(SubFrameIndex::All);
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
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

bool CAlpAPSMPAlgorithm::OverallSystemGain(std::vector<APSTNoiseType>& LightTNoiseData, std::vector<APSDataMeanType>& LightMean, APSTNoiseType DarkTNoiseBase, APSOverallSystemGainType& GainRes)
{
	if (LightTNoiseData.size() != LightMean.size())
	{
		std::string strErr = "OverallSystemGain: Size Error: LightTNoiseData Size: " + std::to_string(LightTNoiseData.size()) + ", LightMean Size: " + std::to_string(LightMean.size());
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}
	GainRes.SubFrameGainK.resize(SubFrameIndex::All);
	for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
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

bool CAlpAPSMPAlgorithm::Saturation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSaturationType& SaturationRes)
{
	bool bRes = false;
	SubFrameDataMean(nIndexStart, nNumber, ROI, nChannelIndex, SaturationRes.SaturationMean, bRes);
	if (!bRes)
	{
		return false;
	}
	APSSubFrameTNoiseType TNoise;
	SubFrameTNoise(nIndexStart, nNumber, ROI, nChannelIndex, TNoise, bRes);
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

bool CAlpAPSMPAlgorithm::Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, bool bNormalize, ImgType& ImgData)
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
		WriteLog(strErr, SubFrameIndex::All);
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

bool CAlpAPSMPAlgorithm::Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSType& ImgData)
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
		WriteLog(strErr, SubFrameIndex::All);
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
	nRow = m_nTotalRow;
	nCol = m_nTotalCol;
}

void CAlpAPSMPAlgorithm::SetActiveArea(ROIArea ActiveArea)
{
	m_ActiveArea = ActiveArea;
}

void CAlpAPSMPAlgorithm::SetRawDataSize(uint32_t nRow, uint32_t nCol)
{
	m_nTotalRow = nRow;
	m_nTotalCol = nCol;
	m_nChannelRow = m_nTotalRow / 2;
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
		WriteLog(strErr, SubFrameIndex::All);
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
		WriteLog(strErr, SubFrameIndex::All);
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
		WriteLog(strErr, SubFrameIndex::All);
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
		WriteLog(strErr, SubFrameIndex::All);
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
		WriteLog(strErr, SubFrameIndex::All);
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
		WriteLog(strErr, SubFrameIndex::All);
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
		WriteLog(strErr, SubFrameIndex::All);
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
		WriteLog(strErr, SubFrameIndex::All);
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
		WriteLog(strErr, SubFrameIndex::All);
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
		WriteLog(strErr, SubFrameIndex::All);
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

void CAlpAPSMPAlgorithm::SubFrameTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameTNoiseType& TNoise, bool& bRes)
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

	CAPSDataContainer RowDataArray;
	RowDataArray.Init(nRow, nNumber, true);

	std::vector<double> RowNoise(nRow, 0);
	std::vector<double> ColNoise(nCol, 0);

	CAPSDataContainer ColDataArray;
	ColDataArray.Init(nCol, nNumber, true);

	std::vector<double> onePixelInMultiFrames(nNumber);

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++)
		{
			for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
			{
				double dValue = m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
				onePixelInMultiFrames[nIndex] = dValue;
				RowDataArray.m_RawData[nRows][nIndex] += dValue;
				ColDataArray.m_RawData[nCols][nIndex] += dValue;
			}
			PixelTNoiseArray.m_RawData[nRows][nCols] = Std(onePixelInMultiFrames, nNumber);
		}
	}
	RowDataArray /= nCol;
	ColDataArray /= nRow;

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		RowNoise[nRows] = Std(RowDataArray.m_RawData[nRows], nNumber);
	}
	for (uint32_t nCols = 0; nCols < nCol; nCols++)
	{
		ColNoise[nCols] = Std(ColDataArray.m_RawData[nCols], nNumber);
	}

	TNoise.RowTemp = RMS(RowNoise, nRow);
	TNoise.ColTemp = RMS(ColNoise, nCol);
	TNoise.TempNoise = RMS(PixelTNoiseArray);
	TNoise.PixelTemp = sqrt(TNoise.TempNoise * TNoise.TempNoise - TNoise.RowTemp * TNoise.RowTemp - TNoise.ColTemp * TNoise.ColTemp);
	if (TNoise.RowTemp > 0)
	{
		TNoise.TempRNRatio = TNoise.PixelTemp / TNoise.RowTemp;
	}
	else
	{
		TNoise.TempRNRatio = -1;
	}
	if (TNoise.ColTemp > 0)
	{
		TNoise.TempCNRatio = TNoise.PixelTemp / TNoise.ColTemp;
	}
	else
	{
		TNoise.TempCNRatio = -1;
	}
	return;
}

void CAlpAPSMPAlgorithm::SubFrameSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameSNoiseType& SNoiseData, bool& bRes)
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

	std::vector<double> RowMean(nRow, 0);
	std::vector<double> ColMean(nCol, 0);

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++)
		{
			double dValue = 0;
			for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
			{
				dValue += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			}
			dValue = round(dValue / nNumber);
			PixelSNoiseArray.m_RawData[nRows][nCols] = dValue;
			RowMean[nRows] += dValue;
			ColMean[nCols] += dValue;
		}
	}
	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		RowMean[nRows] /= nCol;
	}
	for (uint32_t nCols = 0; nCols < nCol; nCols++)
	{
		ColMean[nCols] /= nRow;
	}

	SNoiseData.SNoise = Std(PixelSNoiseArray);
	SNoiseData.RowSNoise = Std(RowMean, nRow);
	SNoiseData.ColSNoise = Std(ColMean, nCol);
	return;
}

void CAlpAPSMPAlgorithm::SubFrameBadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameBadpixelType& BadpixelRes, bool& bRes)
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
		BadPixelMask[i].resize(nCol, 0);
	}
	BadpixelRes.BadPixelNum = 0;
	BadpixelRes.DefectColNum = 0;
	BadpixelRes.DefectRowNum = 0;
	BadpixelRes.SingletNum = 0;
	BadpixelRes.CoupletNum = 0;
	BadpixelRes.ClusterNum = 0;
	BadpixelRes.BadPixelMask.LocalData.clear();
	BadpixelRes.BadPixelMask.Flag.clear();
	BadpixelRes.BadPixelMask.BadPixelNum = 0;

	for (uint32_t nRows = 0; nRows < nRow + m_AlgorithmThre.nBadPixelRadius; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol + m_AlgorithmThre.nBadPixelRadius; nCols++)
		{
			if (nRows < nRow && nCols < nCol)
			{
				double dValue = 0;
				for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
				{
					dValue += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
				}
				PixelMeanArray.m_RawData[nRows][nCols] = round(dValue / nNumber);
			}
			if ((nRows - m_AlgorithmThre.nBadPixelRadius < nRow) && (nCols - m_AlgorithmThre.nBadPixelRadius < nCol))
			{
				uint32_t uSize = 0;
				double dSurroundPixle = 0;
				double dMax = -1;
				double dMin = 10000;
				for (uint32_t i = 0; i < 2 * m_AlgorithmThre.nBadPixelRadius + 1; i++)
				{
					for (uint32_t j = 0; j < 2 * m_AlgorithmThre.nBadPixelRadius + 1; j++)
					{
						if ((nRows - i < nRow) && (nCols - j < nCol) && (i != m_AlgorithmThre.nBadPixelRadius || j != m_AlgorithmThre.nBadPixelRadius))
						{
							dSurroundPixle += PixelMeanArray.m_RawData[nRows - i][nCols - j];
							//SortData[uSize] = PixelMeanArray.m_RawData[nRows - i][nCols - j];
							if (dMax < PixelMeanArray.m_RawData[nRows - i][nCols - j])
							{
								dMax = PixelMeanArray.m_RawData[nRows - i][nCols - j];
							}
							if (dMin > PixelMeanArray.m_RawData[nRows - i][nCols - j])
							{
								dMin = PixelMeanArray.m_RawData[nRows - i][nCols - j];
							}
							uSize++;
						}
					}
				}
				if (uSize > 2)
				{
					dSurroundPixle = (dSurroundPixle - dMax - dMin) / (uSize - 2);
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
					//BadpixelRes.BadPixelMask.BadPixelNum++;
					//BadpixelRes.BadPixelMask.LocalData.push_back({ nRows - m_AlgorithmThre.nBadPixelRadius + RealRoi.Up, nCols - m_AlgorithmThre.nBadPixelRadius + RealRoi.Left });
					//BadpixelRes.BadPixelMask.Flag.push_back(APS_BAD_PIXEL_FLAG);

					BadpixelRes.BadPixelNum++;
					BadPixelMask[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius] = 1;
				}
			}
		}
	}
	uint32_t ConnectedAreaFlag = 0xFFFFFFFF;

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++)
		{
			if (BadPixelMask[nRows][nCols] != 0 && BadPixelMask[nRows][nCols] < ConnectedAreaFlag)
			{
				uint32_t AreaSize = 0;
				uint32_t nCur = 0;
				std::vector<Local> Search;
				Search.push_back({ nRows, nCols });
				while (nCur != Search.size())
				{
					Local temp = Search[nCur];
					nCur++;
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
									Search.push_back({ nTempRows , nTempCols });
								}
							}
						}
					}
				}
				uint8_t uFlag = 0;
				if (AreaSize == 1)
				{
					BadpixelRes.SingletNum++;
					uFlag = APS_BAD_PIXEL_SINGLET_FLAG;
				}
				else if (AreaSize == 2)
				{
					BadpixelRes.CoupletNum++;
					uFlag = APS_BAD_PIXEL_COUPLET_FLAG;
				}
				else
				{
					BadpixelRes.ClusterNum++;
					uFlag = APS_BAD_PIXEL_CLUSTER_FLAG;
				}
				for (uint32_t n = 0; n < Search.size(); n++)
				{
					BadpixelRes.BadPixelMask.LocalData.push_back(Search[n]);
					BadpixelRes.BadPixelMask.Flag.push_back(uFlag);
					BadpixelRes.BadPixelMask.BadPixelNum++;
				}
				ConnectedAreaFlag--;
			}
		}
	}

	std::vector<double> RowMean(nRow, 0);
	std::vector<double> ColMean(nCol, 0);

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++)
		{
			double value = 0;
			for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
			{
				value += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			}
			RowMean[nRows] += round(value / nNumber);
			ColMean[nCols] += round(value / nNumber);
		}
	}
	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		RowMean[nRows] /= nCol;
	}
	for (uint32_t nCols = 0; nCols < nCol; nCols++)
	{
		ColMean[nCols] /= nRow;
	}
	for (uint32_t nRows = m_AlgorithmThre.nBadLineRadius; nRows < nRow - m_AlgorithmThre.nBadLineRadius; nRows++)
	{
		double dBaseMean = 0;
		for (uint32_t n = nRows - m_AlgorithmThre.nBadLineRadius; n <= nRows + m_AlgorithmThre.nBadLineRadius; n++)
		{
			if (n != nRows)
			{
				dBaseMean += RowMean[n];
			}
		}
		dBaseMean /= 2 * m_AlgorithmThre.nBadLineRadius;
		if (abs(RowMean[nRows] - dBaseMean) / dBaseMean > m_AlgorithmThre.dBadLineThre)
		{
			BadpixelRes.DefectRowNum++;
		}
	}
	for (uint32_t nCols = m_AlgorithmThre.nBadLineRadius; nCols < nCol - m_AlgorithmThre.nBadLineRadius; nCols++)
	{
		double dBaseMean = 0;
		for (uint32_t n = nCols - m_AlgorithmThre.nBadLineRadius; n <= nCols + m_AlgorithmThre.nBadLineRadius; n++)
		{
			if (n != nCols)
			{
				dBaseMean += ColMean[n];
			}
		}
		dBaseMean /= 2 * m_AlgorithmThre.nBadLineRadius;
		if (abs(ColMean[nCols] - dBaseMean) / dBaseMean > m_AlgorithmThre.dBadLineThre)
		{
			BadpixelRes.DefectColNum++;
		}
	}
	return;
}

void CAlpAPSMPAlgorithm::SubFrameHotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameBadpixelType& HotpixelRes, bool& bRes)
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
	std::vector<std::vector<uint32_t>> BadPixelMask(nRow);
	for (uint32_t i = 0; i < nRow; i++)
	{
		BadPixelMask[i].resize(nCol, 0);
	}

	HotpixelRes.BadPixelNum = 0;
	HotpixelRes.DefectColNum = 0;
	HotpixelRes.DefectRowNum = 0;
	HotpixelRes.SingletNum = 0;
	HotpixelRes.CoupletNum = 0;
	HotpixelRes.ClusterNum = 0;
	HotpixelRes.BadPixelMask.LocalData.clear();
	HotpixelRes.BadPixelMask.Flag.clear();
	HotpixelRes.BadPixelMask.BadPixelNum = 0;

	for (uint32_t nRows = 0; nRows < nRow + m_AlgorithmThre.nBadPixelRadius; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol + m_AlgorithmThre.nBadPixelRadius; nCols++)
		{
			if (nRows < nRow && nCols < nCol)
			{
				double dValue = 0;
				for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
				{
					dValue += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
				}
				PixelMeanArray.m_RawData[nRows][nCols] = round(dValue / nNumber);
			}
			if ((nRows - m_AlgorithmThre.nBadPixelRadius < nRow) && (nCols - m_AlgorithmThre.nBadPixelRadius < nCol))
			{
				uint32_t uSize = 0;
				double dSurroundPixle = 0;
				double dMax = -1;
				double dMin = 10000;
				for (uint32_t i = 0; i < 2 * m_AlgorithmThre.nBadPixelRadius + 1; i++)
				{
					for (uint32_t j = 0; j < 2 * m_AlgorithmThre.nBadPixelRadius + 1; j++)
					{
						if ((nRows - i < nRow) && (nCols - j < nCol) && (i != m_AlgorithmThre.nBadPixelRadius || j != m_AlgorithmThre.nBadPixelRadius))
						{
							dSurroundPixle += PixelMeanArray.m_RawData[nRows - i][nCols - j];
							//SortData[uSize] = PixelMeanArray.m_RawData[nRows - i][nCols - j];
							if (dMax < PixelMeanArray.m_RawData[nRows - i][nCols - j])
							{
								dMax = PixelMeanArray.m_RawData[nRows - i][nCols - j];
							}
							if (dMin > PixelMeanArray.m_RawData[nRows - i][nCols - j])
							{
								dMin = PixelMeanArray.m_RawData[nRows - i][nCols - j];
							}
							uSize++;
						}
					}
				}
				if (uSize > 2)
				{
					dSurroundPixle = (dSurroundPixle - dMax - dMin) / (uSize - 2);
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
					HotpixelRes.BadPixelNum++;
					//HotpixelRes.BadPixelMask.LocalData.push_back({ nRows - m_AlgorithmThre.nBadPixelRadius + RealRoi.Up, nCols - m_AlgorithmThre.nBadPixelRadius + RealRoi.Left });
					//HotpixelRes.BadPixelMask.Flag.push_back(APS_HOT_PIXEL_FLAG);
					//HotpixelRes.BadPixelMask.BadPixelNum++;
					BadPixelMask[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius] = 1;
				}
			}
		}
	}

	uint32_t ConnectedAreaFlag = 0xFFFFFFFF;

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++)
		{
			if (BadPixelMask[nRows][nCols] != 0 && BadPixelMask[nRows][nCols] < ConnectedAreaFlag)
			{
				uint32_t AreaSize = 0;
				std::vector<Local> Search;
				uint32_t nCur = 0;
				Search.push_back({ nRows, nCols });
				while (nCur != Search.size())
				{
					Local temp = Search[nCur];
					nCur++;
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
									Search.push_back({ nTempRows , nTempCols });
								}
							}
						}
					}
				}
				uint8_t uFlag = 0;
				if (AreaSize == 1)
				{
					HotpixelRes.SingletNum++;
					uFlag = APS_HOT_PIXEL_SINGLET_FLAG;
				}
				else if (AreaSize == 2)
				{
					HotpixelRes.CoupletNum++;
					uFlag = APS_HOT_PIXEL_COUPLET_FLAG;
				}
				else
				{
					HotpixelRes.ClusterNum++;
					uFlag = APS_HOT_PIXEL_CLUSTER_FLAG;
				}
				for (uint32_t n = 0; n < Search.size(); n++)
				{
					HotpixelRes.BadPixelMask.LocalData.push_back(Search[n]);
					HotpixelRes.BadPixelMask.Flag.push_back(uFlag);
					HotpixelRes.BadPixelMask.BadPixelNum++;
				}
				ConnectedAreaFlag--;
			}
		}
	}
	std::vector<double> RowMean(nRow, 0);
	std::vector<double> ColMean(nCol, 0);

	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++)
		{
			double value = 0;
			for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
			{
				value += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			}
			RowMean[nRows] += round(value / nNumber);
			ColMean[nCols] += round(value / nNumber);
		}
	}
	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		RowMean[nRows] /= nCol;
	}
	for (uint32_t nCols = 0; nCols < nCol; nCols++)
	{
		ColMean[nCols] /= nRow;
	}
	for (uint32_t nRows = m_AlgorithmThre.nBadLineRadius; nRows < nRow - m_AlgorithmThre.nBadLineRadius; nRows++)
	{
		double dBaseMean = 0;
		for (uint32_t n = nRows - m_AlgorithmThre.nBadLineRadius; n <= nRows + m_AlgorithmThre.nBadLineRadius; n++)
		{
			if (n != nRows)
			{
				dBaseMean += RowMean[n];
			}
		}
		dBaseMean /= 2 * m_AlgorithmThre.nBadLineRadius;
		if (abs(RowMean[nRows] - dBaseMean) > m_AlgorithmThre.dHotLineThre)
		{
			HotpixelRes.DefectRowNum++;
		}
	}
	for (uint32_t nCols = m_AlgorithmThre.nBadLineRadius; nCols < nCol - m_AlgorithmThre.nBadLineRadius; nCols++)
	{
		double dBaseMean = 0;
		for (uint32_t n = nCols - m_AlgorithmThre.nBadLineRadius; n <= nCols + m_AlgorithmThre.nBadLineRadius; n++)
		{
			if (n != nCols)
			{
				dBaseMean += ColMean[n];
			}
		}
		dBaseMean /= 2 * m_AlgorithmThre.nBadLineRadius;
		if (abs(ColMean[nCols] - dBaseMean) > m_AlgorithmThre.dHotLineThre)
		{
			HotpixelRes.DefectColNum++;
		}
	}
	return;
}

void CAlpAPSMPAlgorithm::SubFrameBLC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, double& BaseMean, bool& bRes)
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

void CAlpAPSMPAlgorithm::SubFrameDPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameBadpixelType& SubFrameBadPixel, bool& bRes)
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

	for (uint32_t nBadPixelIndex = 0; nBadPixelIndex < SubFrameBadPixel.BadPixelMask.BadPixelNum; nBadPixelIndex++)
	{
		uint32_t nBadpixelRows = SubFrameBadPixel.BadPixelMask.LocalData[nBadPixelIndex].x;
		uint32_t nBadpixelCols = SubFrameBadPixel.BadPixelMask.LocalData[nBadPixelIndex].y;

		for (uint32_t nBadPixelIndex = 0; nBadPixelIndex < SubFrameBadPixel.BadPixelMask.BadPixelNum; nBadPixelIndex++)
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
							if (nCurCols >= RealRoi.Left && nCurCols <= RealRoi.Right && SubFrameBadPixel.BadPixelMask.LocalData.end() == std::find(SubFrameBadPixel.BadPixelMask.LocalData.begin(), SubFrameBadPixel.BadPixelMask.LocalData.end(), Local{ nCurRows, nCurCols }))
							{
								dMeanData += CurRawData.m_RawData[nCurRows][nCurCols];
								nSize++;
							}
						}
					}
				}
				if (nSize > 0)
				{
					CurRawData.m_RawData[nBadpixelRows][nBadpixelCols] = round(dMeanData / nSize);
				}
			}
		}
	}
	return;
}

void CAlpAPSMPAlgorithm::SubFrameDataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, double& DataMean, bool& bRes)
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
			double dValue = 0;
			for (uint32_t nFrameIndex = 0; nFrameIndex < nNumber; nFrameIndex++)
			{
				dValue += m_RawDataContainer[nChannelIndex][nIndexStart + nFrameIndex].m_RawData[nRows][nCols];
			}
			dValue = round(dValue / nNumber);
			DataMean += dValue;
		}
	}
	DataMean /= ((RealRoi.Down - RealRoi.Up + 1) * (RealRoi.Right - RealRoi.Left + 1));

	return;
}

void CAlpAPSMPAlgorithm::SubFrameBLCByColBase(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, std::vector<double>& BaseMean, bool& bRes)
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

void CAlpAPSMPAlgorithm::SubFrameColMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, std::vector<double>& DataMean, bool& bRes)
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
		for (uint32_t nCols = RealRoi.Left; nCols <= RealRoi.Right; nCols++)
		{
			DataMean[nCols - RealRoi.Left] = 0;
			for (uint32_t nRows = RealRoi.Up; nRows <= RealRoi.Down; nRows++)
			{
				double dValue = 0;
				for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
				{
					dValue += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols];
				}
				DataMean[nCols - RealRoi.Left] += round(dValue / nNumber);
			}
			DataMean[nCols - RealRoi.Left] /= nAARow;
	}
	return;
}

bool CAlpAPSMPAlgorithm::GetBlockMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, uint32_t nRowBlockNum, uint32_t nColBlockNum, std::vector<CAPSDataContainer>& BlockData, uint32_t nSubRowBlockSize, uint32_t nSubColBlockSize)
{
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];

	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameBlockMean, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), nRowBlockNum, nColBlockNum, nSubRowBlockSize, nSubColBlockSize, std::ref(BlockData[i]), std::ref(bSubRes[i]));
		}
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			SubFrameBlockMean(nIndexStart, nNumber, ROI, SubFrameIndex(i), nRowBlockNum, nColBlockNum, nSubRowBlockSize, nSubColBlockSize, BlockData[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

void CAlpAPSMPAlgorithm::SubFrameBlockMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, uint32_t nRowBlockNum, uint32_t nColBlockNum, uint32_t nSubRowBlockSize, uint32_t nSubColBlockSize, CAPSDataContainer& BlockData, bool& bRes)
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

	int32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	int32_t nCol = RealRoi.Right - RealRoi.Left + 1;
	int32_t nRowBlockSizeBase = nSubRowBlockSize != 0 ? nSubRowBlockSize : nRow / nRowBlockNum;
	int32_t nColBlockSizeBase = nSubColBlockSize != 0 ? nSubColBlockSize : nCol / nColBlockNum;
	int32_t nRowMod = nRow - nRowBlockNum * nRowBlockSizeBase;
	int32_t nColMod = nCol - nColBlockNum * nColBlockSizeBase;

	BlockData.Init(nRowBlockNum, nColBlockNum);

	int32_t nRowBlockSize = 0, nColBlockSize = 0, nRowIndex = RealRoi.Up, nColIndex = RealRoi.Left;
	for (uint32_t nRowBlockIndex = 0; nRowBlockIndex < nRowBlockNum; nRowBlockIndex++)
	{
		if (0 == nRowBlockIndex || nRowBlockNum - 1 == nRowBlockIndex)
		{
			nRowBlockSize = nRowBlockSizeBase + nRowMod / 2;
		}
		else
		{
			nRowBlockSize = nRowBlockSizeBase;
		}
		nColIndex = RealRoi.Left;
		for (uint32_t nColBlockIndex = 0; nColBlockIndex < nColBlockNum; nColBlockIndex++)
		{
			if (0 == nColBlockIndex || nColBlockNum - 1 == nColBlockIndex)
			{
				nColBlockSize = nColBlockSizeBase + nColMod / 2;
			}
			else
			{
				nColBlockSize = nColBlockSizeBase;
			}

			if (nRowBlockSize <= 0 || nColBlockSize <= 0)
			{
				std::string strErr = "SubFrameBlockMean: BlockSize error: RowBlockSize: " + std::to_string(nRowBlockSize) + ", ColBlockSize: " + std::to_string(nColBlockSize);
				WriteLog(strErr, nChannelIndex);
				bRes = false;
				return;
			}

			ROIArea temp = { nRowIndex, nRowIndex + nRowBlockSize - 1, nColIndex, nColIndex + nColBlockSize - 1 };

			double dValue = 0;
			BlockData.m_RawData[nRowBlockIndex][nColBlockIndex] = 0;
			for (uint32_t nRows = temp.Up; nRows <= temp.Down; nRows++)
			{
				for (uint32_t nCols = temp.Left; nCols <= temp.Right; nCols++)
				{
					double dValue = 0;
					for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
					{
						dValue += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols];
					}
					BlockData.m_RawData[nRowBlockIndex][nColBlockIndex] += round(dValue / nNumber);
				}
			}
			BlockData.m_RawData[nRowBlockIndex][nColBlockIndex] /= (nRowBlockSize * nColBlockSize);
			nColIndex += nColBlockSize;
		}
		nRowIndex += nRowBlockSize;
	}
}

void CAlpAPSMPAlgorithm::SetDataToSubFrame(uint32_t nIndex, uint32_t nRows, uint32_t nCols, double dValue)
{
	switch (m_PixelFormat)
	{
	case BayerGBRG:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			m_RawDataContainer[B][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			m_RawDataContainer[R][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		break;
	case BayerBGGR:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			m_RawDataContainer[B][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			m_RawDataContainer[R][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		break;
	case BayerRGGB:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			m_RawDataContainer[R][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			m_RawDataContainer[B][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		break;
	case BayerGRBG:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			m_RawDataContainer[R][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			m_RawDataContainer[B][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[nRows >> 1][nCols >> 1] = dValue;
		}
		break;
	case QuadBayerGBRG:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		break;
	case QuadBayerBGGR:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		break;
	case QuadBayerRGGB:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		break;
	case QuadBayerGRBG:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)] = dValue;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1] = dValue;
		}
		break;
	}
}

void CAlpAPSMPAlgorithm::SubFrameLocalToTotalLocal(Local SubLocal, SubFrameIndex nChannelIndex, Local& TotalLocal)
{
	uint32_t nSubRows = SubLocal.x, nSubCols = SubLocal.y;
	uint32_t nTotalRows = 0, nTotalCols = 0;

	switch (m_PixelFormat)
	{
	case BayerGBRG:
		if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols  << 1);
		}
		else if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols  << 1) + 1;
		}
		else if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols  << 1);
		}
		else if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols  << 1) + 1;
		}
		break;
	case BayerBGGR:
		if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols  << 1);
		}
		else if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols  << 1) + 1;
		}
		else if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols  << 1);
		}
		else if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols  << 1) + 1;
		}
		break;
	case BayerRGGB:
		if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols  << 1);
		}
		else if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols  << 1) + 1;
		}
		else if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols  << 1);
		}
		else if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols  << 1) + 1;
		}
		break;
	case BayerGRBG:
		if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols  << 1);
		}
		else if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols  << 1) + 1;
		}
		else if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols  << 1);
		}
		else if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols  << 1) + 1;
		}
		break;
	case QuadBayerGBRG:
		if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 0;
			}
			else
			{
				nTotalRows += 1;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 0;
			}
			else
			{
				nTotalCols += 1;
			}
		}
		else if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 0;
			}
			else
			{
				nTotalRows += 1;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 2;
			}
			else
			{
				nTotalCols += 3;
			}
		}
		else if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 2;
			}
			else
			{
				nTotalRows += 3;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 0;
			}
			else
			{
				nTotalCols += 1;
			}
		}
		else if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 2;
			}
			else
			{
				nTotalRows += 3;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 2;
			}
			else
			{
				nTotalCols += 3;
			}
		}
		break;
	case QuadBayerBGGR:
		if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 0;
			}
			else
			{
				nTotalRows += 1;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 0;
			}
			else
			{
				nTotalCols += 1;
			}
		}
		else if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 0;
			}
			else
			{
				nTotalRows += 1;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 2;
			}
			else
			{
				nTotalCols += 3;
			}
		}
		else if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 2;
			}
			else
			{
				nTotalRows += 3;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 0;
			}
			else
			{
				nTotalCols += 1;
			}
		}
		else if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 2;
			}
			else
			{
				nTotalRows += 3;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 2;
			}
			else
			{
				nTotalCols += 3;
			}
		}
		break;
	case QuadBayerRGGB:
		if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 0;
			}
			else
			{
				nTotalRows += 1;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 0;
			}
			else
			{
				nTotalCols += 1;
			}
		}
		else if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 0;
			}
			else
			{
				nTotalRows += 1;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 2;
			}
			else
			{
				nTotalCols += 3;
			}
		}
		else if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 2;
			}
			else
			{
				nTotalRows += 3;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 0;
			}
			else
			{
				nTotalCols += 1;
			}
		}
		else if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 2;
			}
			else
			{
				nTotalRows += 3;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 2;
			}
			else
			{
				nTotalCols += 3;
			}
		}
		break;
	case QuadBayerGRBG:
		if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 0;
			}
			else
			{
				nTotalRows += 1;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 0;
			}
			else
			{
				nTotalCols += 1;
			}
		}
		else if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 0;
			}
			else
			{
				nTotalRows += 1;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 2;
			}
			else
			{
				nTotalCols += 3;
			}
		}
		else if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 2;
			}
			else
			{
				nTotalRows += 3;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 0;
			}
			else
			{
				nTotalCols += 1;
			}
		}
		else if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows & 0xFFFE) << 1;
			nTotalCols = (nSubCols & 0xFFFE) << 1;
			if ((nSubRows & 1) == 0)
			{
				nTotalRows += 2;
			}
			else
			{
				nTotalRows += 3;
			}
			if ((nSubCols & 1) == 0)
			{
				nTotalCols += 2;
			}
			else
			{
				nTotalCols += 3;
			}
		}
		break;
	}
	TotalLocal.x = nTotalRows;
	TotalLocal.y = nTotalCols;
}
