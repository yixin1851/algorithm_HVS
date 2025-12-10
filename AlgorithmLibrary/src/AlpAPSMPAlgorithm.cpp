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

typedef struct
{
	Local l;
	uint8_t flag;
	float Diff;
}BadPixelInfo;

bool operator< (const BadPixelInfo& lh, const BadPixelInfo& rh)
{
	return lh.Diff < rh.Diff;
}

CAlpAPSMPAlgorithm::CAlpAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
{
	// 参数初始化
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
	m_AlgorithmThre.nOETCRadius = 64;
	m_AlgorithmThre.nLinearityRadius = 16;
	m_nCode = code;

	m_RawDataContainer.resize(SubFrameIndex::All);

	if ((code & APSCodeType::APS_Code_16_Subframe) == APSCodeType::APS_Code_16_Subframe)
	{
		m_bUse16SubFrame = true;
		m_16SubRawDataContainer.resize(16);
	}
	else
	{
		m_bUse16SubFrame = false;
	}

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
	// 计算时域噪声
	bool bRet = true;
	uint32_t nChannelNum = m_bUse16SubFrame ? 16 : SubFrameIndex::All;
	RawDataContainer& DataContainer = m_bUse16SubFrame ? m_16SubRawDataContainer : m_RawDataContainer;
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
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameTNoise, this, nIndexStart, nNumber, &RealRoi, SubFrameIndex(i), std::ref(TNoiseRes.SubFrameTNoiseData[i]), std::ref(bSubRes[i]), std::ref(DataContainer));
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
		std::vector<double> onePixelInMultiFrames(nNumber);

		uint32_t nCur = 0;
		for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++)
		{
			for (uint32_t nRows = 0; nRows < nRow; nRows++)
			{
				for (uint32_t nCols = 0; nCols < nCol; nCols++)
				{
					for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
					{
						onePixelInMultiFrames[nIndex] = m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
					}
					AllPixel[nCur++] = Std(onePixelInMultiFrames, nNumber);
				}
			}
		}
		TNoiseRes.TNoiseFrame = RMS(AllPixel, nCur);
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::SNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSSNoiseType& SNoiseData)
{
	// 计算空域噪声
	bool bRet = true;
	uint32_t nChannelNum = m_bUse16SubFrame ? 16 : SubFrameIndex::All;
	RawDataContainer& DataContainer = m_bUse16SubFrame ? m_16SubRawDataContainer : m_RawDataContainer;
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
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameSNoise, this, nIndexStart, nNumber, &RealRoi, SubFrameIndex(i), std::ref(SNoiseData.SubFrameSNoiseData[i]), std::ref(bSubRes[i]), std::ref(DataContainer));
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
		// 计算所有像素的标准差作为全帧噪声
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
	BadpixelRes.MaxClusterSize = 0;
	BadpixelRes.BadPixelMask.BadPixelNum = 0;
	BadpixelRes.BadPixelMask.LocalData.clear();
	BadpixelRes.BadPixelMask.Flag.clear();
	BadpixelRes.BadPixelMask.DiffData.clear();

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
		std::vector<BadPixelInfo> BadPixelList;
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			BadpixelRes.BadPixelNum += BadpixelRes.SubFrameBadpixelData[i].BadPixelNum;
			BadpixelRes.ClusterNum += BadpixelRes.SubFrameBadpixelData[i].ClusterNum;
			BadpixelRes.CoupletNum += BadpixelRes.SubFrameBadpixelData[i].CoupletNum;
			BadpixelRes.SingletNum += BadpixelRes.SubFrameBadpixelData[i].SingletNum;
			if (BadpixelRes.SubFrameBadpixelData[i].MaxClusterSize > BadpixelRes.MaxClusterSize)
			{
				BadpixelRes.MaxClusterSize = BadpixelRes.SubFrameBadpixelData[i].MaxClusterSize;
			}
			Local Total;
			for (uint32_t n = 0; n < BadpixelRes.SubFrameBadpixelData[i].BadPixelMask.BadPixelNum; n++)
			{
				BadpixelRes.BadPixelMask.BadPixelNum++;
				SubFrameLocalToTotalLocal(BadpixelRes.SubFrameBadpixelData[i].BadPixelMask.LocalData[n], SubFrameIndex(i), Total);
				BadPixelInfo Temp;
				Temp.l = Total;
				Temp.Diff = BadpixelRes.SubFrameBadpixelData[i].BadPixelMask.DiffData[n];
				Temp.flag = BadpixelRes.SubFrameBadpixelData[i].BadPixelMask.Flag[n];
				BadPixelList.push_back(Temp);
			}
		}
		std::sort(BadPixelList.begin(), BadPixelList.end());
		std::reverse(BadPixelList.begin(), BadPixelList.end());

		for (uint32_t i = 0; i < BadPixelList.size(); i++)
		{
			BadpixelRes.BadPixelMask.LocalData.push_back(BadPixelList[i].l);
			BadpixelRes.BadPixelMask.Flag.push_back(BadPixelList[i].flag);
			BadpixelRes.BadPixelMask.DiffData.push_back(BadPixelList[i].Diff);
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
	HotpixelRes.MaxClusterSize = 0;
	HotpixelRes.BadPixelMask.BadPixelNum = 0;
	HotpixelRes.BadPixelMask.LocalData.clear();
	HotpixelRes.BadPixelMask.Flag.clear();
	HotpixelRes.SubFrameBadpixelData.resize(SubFrameIndex::All);
	HotpixelRes.BadPixelMask.DiffData.clear();

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
		std::vector<BadPixelInfo> BadPixelList;
		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			HotpixelRes.BadPixelNum += HotpixelRes.SubFrameBadpixelData[i].BadPixelNum;
			HotpixelRes.ClusterNum += HotpixelRes.SubFrameBadpixelData[i].ClusterNum;
			HotpixelRes.CoupletNum += HotpixelRes.SubFrameBadpixelData[i].CoupletNum;
			HotpixelRes.SingletNum += HotpixelRes.SubFrameBadpixelData[i].SingletNum;
			if (HotpixelRes.SubFrameBadpixelData[i].MaxClusterSize > HotpixelRes.MaxClusterSize)
			{
				HotpixelRes.MaxClusterSize = HotpixelRes.SubFrameBadpixelData[i].MaxClusterSize;
			}
			Local Total;
			for (uint32_t n = 0; n < HotpixelRes.SubFrameBadpixelData[i].BadPixelMask.BadPixelNum; n++)
			{
				HotpixelRes.BadPixelMask.BadPixelNum++;
				SubFrameLocalToTotalLocal(HotpixelRes.SubFrameBadpixelData[i].BadPixelMask.LocalData[n], SubFrameIndex(i), Total);
				BadPixelInfo Temp;
				Temp.l = Total;
				Temp.Diff = HotpixelRes.SubFrameBadpixelData[i].BadPixelMask.DiffData[n];
				Temp.flag = HotpixelRes.SubFrameBadpixelData[i].BadPixelMask.Flag[n];
				BadPixelList.push_back(Temp);
			}
		}
		std::sort(BadPixelList.begin(), BadPixelList.end());
		std::reverse(BadPixelList.begin(), BadPixelList.end());

		for (uint32_t i = 0; i < BadPixelList.size(); i++)
		{
			HotpixelRes.BadPixelMask.LocalData.push_back(BadPixelList[i].l);
			HotpixelRes.BadPixelMask.Flag.push_back(BadPixelList[i].flag);
			HotpixelRes.BadPixelMask.DiffData.push_back(BadPixelList[i].Diff);
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

bool CAlpAPSMPAlgorithm::DPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, std::vector<Local>& BadPixelLocal)
{
	/*
DPC (主函数)
  │
  ├─> 坐标转换: 全局 → 各子帧局部坐标
  │
  ├─> 多线程模式?
  │    ├─ 是: 创建N个线程并行执行SubFrameDPC
  │    └─ 否: 顺序执行SubFrameDPC
  │
  └─> SubFrameDPC (每个子帧独立执行)
       │
       ├─> 参数验证 (索引、ROI边界)
       │
       ├─> 遍历所有坏点
       │    │
       │    └─> 遍历所有帧
       │         │
       │         ├─> 计算3×3邻域均值 (排除其他坏点)
       │         ├─> 替换坏点值
       │         └─> 如果启用16子帧: 同步更新对应子通道
       │
       └─> 返回处理状态

	*/
	// 核心功能：
	// 1. 坐标转换与任务分配：将全局坐标的坏点位置转换为各子帧的局部坐标；
	// 2. 并行调度：根据配置选择多线程或单线程执行模式；
	// 3. 结果汇总：收集所有子帧的处理状态；
	bool bRet = true;
	bool bSubRes[SubFrameIndex::All];

	std::vector<Local> SubFrameBadPixelLocal[SubFrameIndex::All];

	for (uint32_t n = 0; n < BadPixelLocal.size(); n++)
	{
		auto total = BadPixelLocal[n];
		SubFrameIndex channel = SubFrameIndex::All;
		Local subLocal;
		TotalLocalToSubFrameLocal(total, channel, subLocal);
		SubFrameBadPixelLocal[channel].push_back(subLocal);
	}

	if (m_bMultiThreadEnable)
	{
		std::thread* t[SubFrameIndex::All];

		for (uint32_t i = 0; i < SubFrameIndex::All; i++)
		{
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameDPC, this, nIndexStart, nNumber, ROI, SubFrameIndex(i), std::ref(SubFrameBadPixelLocal[i]), std::ref(bSubRes[i]));
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
			SubFrameDPC(nIndexStart, nNumber, ROI, SubFrameIndex(i), SubFrameBadPixelLocal[i], bSubRes[i]);
		}
	}
	for (uint32_t i = 0; i < SubFrameIndex::All; i++)
	{
		bRet = bRet && bSubRes[i];
	}
	return bRet;
}

bool CAlpAPSMPAlgorithm::BadPixelLocalToOtpType(std::vector<Local> BadPixelLocal, std::vector<uint8_t>& OtpData)
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
		// uData1存储列号的低8位
		uint8_t uData1 = nCol & 0xFF;
		// uData2高4位存储行号的低4位
		// uData2低4位存储列号的高4位
		uint8_t uData2 = ((nRow << 4) & 0xF0) + ((nCol >> 8) & 0x0F);
		// uData3存储列号的高8位
		uint8_t uData3 = (nRow >> 4) & 0xFF;
		OtpData[nCur++] = uData1;
		OtpData[nCur++] = uData2;
		OtpData[nCur++] = uData3;
		// 字节1: [Col[7:0]]
		// 字节2: [row[3:0]] [col[11:8]]
		// 字节3: [row[11:4]]
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

	// 计算中心亮度基准，YCenter = (Gb中心块 + Gr中心块)/2
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
			// 对于每个block，Y = (Gb值 + Gr值) / 2;
			double Y = (BlockData[Gr].m_RawData[nRows][nCols] + BlockData[Gb].m_RawData[nRows][nCols]) / 2;

			// Shading>1表示比中心亮, Shading<1表示比中心暗
			ShadingRes.YShadingData[nRows][nCols] = Y / YCenter;
		}
	}

	// 定义4角和中心亮度比区域
	// 每个角落区域占ROI的10%*10%
	// 中心区域占ROI的10%*10%
	// Left-Top
	ROIArea LT = { RealRoi.Up, RealRoi.Up + nRow * 0.1 - 1, RealRoi.Left, RealRoi.Left + nCol * 0.1 - 1 };
	// Left-Bottom
	ROIArea LB = { RealRoi.Down - nRow * 0.1 + 1, RealRoi.Down , RealRoi.Left, RealRoi.Left + nCol * 0.1 - 1 };
	// Right-Top
	ROIArea RT = { RealRoi.Up, RealRoi.Up + nRow * 0.1 - 1, RealRoi.Right - nCol * 0.1 + 1 , RealRoi.Right };
	// Right-Bottom
	ROIArea RB = { RealRoi.Down - nRow * 0.1 + 1, RealRoi.Down , RealRoi.Right - nCol * 0.1 + 1 , RealRoi.Right };
	// Center
	ROIArea CT = { RealRoi.Up + nRow * 0.45, RealRoi.Up + nRow * 0.55 - 1, RealRoi.Left + nCol * 0.45, RealRoi.Left + nCol * 0.55 - 1 };

	double YGbLT = 0, YGbLB = 0, YGbRT = 0, YGbRB = 0, YGbCT = 0;
	double YGrLT = 0, YGrLB = 0, YGrRT = 0, YGrRB = 0, YGrCT = 0;
	bool bRes = true;
	SubFrameDataMean(nIndexStart, nNumber, &LT, Gb, YGbLT, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &LB, Gb, YGbLB, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &RT, Gb, YGbRT, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &RB, Gb, YGbRB, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &CT, Gb, YGbCT, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &LT, Gr, YGrLT, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &LB, Gr, YGrLB, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &RT, Gr, YGrRT, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &RB, Gr, YGrRB, bRes, m_RawDataContainer);
	SubFrameDataMean(nIndexStart, nNumber, &CT, Gr, YGrCT, bRes, m_RawDataContainer);

	if ((YGbCT + YGrCT) <= 0)
	{
		std::string strErr = "YShading: Y Center error";
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	// 计算4个角的相对亮度：
	// 值接近1.0表示均匀性好，偏离1.0越远表示Shading越严重
	// YShadingLT = (Gb左上 + Gr左上) / (Gb中心 + Gr中心)
	// YShadingLB = (Gb左下 + Gr左下) / (Gb中心 + Gr中心)
	// YShadingRT = (Gb右上 + Gr右上) / (Gb中心 + Gr中心)
	// YShadingRB = (Gb右下 + Gr右下) / (Gb中心 + Gr中心)
	ShadingRes.YShadingLT = (YGbLT + YGrLT) / (YGbCT + YGrCT);
	ShadingRes.YShadingLB = (YGbLB + YGrLB) / (YGbCT + YGrCT);
	ShadingRes.YShadingRT = (YGbRT + YGrRT) / (YGbCT + YGrCT);
	ShadingRes.YShadingRB = (YGbRB + YGrRB) / (YGbCT + YGrCT);

	return true;
}

bool CAlpAPSMPAlgorithm::ColorShading(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSColorShadingType& ShadingRes)
{
	// 该函数分析图像中不同区域的R/G和B/G比值分布，通过与中心区域对比来量化色彩偏移程度
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

	// 分块均值计算
	if (!GetBlockMean(nIndexStart, nNumber, &RealRoi, m_AlgorithmThre.nColorShadingRowBlockNum, m_AlgorithmThre.nColorShadingColBlockNum, BlockData))
	{
		std::string strErr = "ColorShading: GetBlockMean error";
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	// 中心基准计算
	double RCenter = BlockData[R].m_RawData[m_AlgorithmThre.nColorShadingRowBlockNum / 2][m_AlgorithmThre.nColorShadingColBlockNum / 2];
	double BCenter = BlockData[B].m_RawData[m_AlgorithmThre.nColorShadingRowBlockNum / 2][m_AlgorithmThre.nColorShadingColBlockNum / 2];
	// G通道去Gb、Gr的平均值
	double GCenter = (BlockData[Gr].m_RawData[m_AlgorithmThre.nColorShadingRowBlockNum / 2][m_AlgorithmThre.nColorShadingColBlockNum / 2] +
		BlockData[Gb].m_RawData[m_AlgorithmThre.nColorShadingRowBlockNum / 2][m_AlgorithmThre.nColorShadingColBlockNum / 2]) / 2;

	if (GCenter <= 0)
	{
		std::string strErr = "ColorShading: G Center error";
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}
	// 中心R/G比值
	double RGCenter = RCenter / GCenter;
	// 中心B/G比值
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

			// 归一化到中心值
			// 理想情况下，所有块的归一值都应接近1.0。偏离1.0越远，说明该区域色彩偏移越严重。
			double RG = RMean / GMean;
			double BG = BMean / GMean;

			ShadingRes.ColorShadingRGData[nRows][nCols] = RG / RGCenter;
			ShadingRes.ColorShadingBGData[nRows][nCols] = BG / BGCenter;
		}
	}

	// 四个角落数据提取, 四角数据是关键指标，通常用于判断镜头或sensor的色彩均匀性是否合格
	// Left-Top R/G值
	ShadingRes.ColorShadingRGLT = ShadingRes.ColorShadingRGData[0][0];
	// Left-Bottom R/G值
	ShadingRes.ColorShadingRGLB = ShadingRes.ColorShadingRGData[m_AlgorithmThre.nColorShadingRowBlockNum - 1][0];
	// Right-Top R/G值
	ShadingRes.ColorShadingRGRT = ShadingRes.ColorShadingRGData[0][m_AlgorithmThre.nColorShadingColBlockNum - 1];
	// Right-Bottom R/G值
	ShadingRes.ColorShadingRGRB = ShadingRes.ColorShadingRGData[m_AlgorithmThre.nColorShadingRowBlockNum - 1][m_AlgorithmThre.nColorShadingColBlockNum - 1];

	// Left-Top B/G值
	ShadingRes.ColorShadingBGLT = ShadingRes.ColorShadingBGData[0][0];
	// Left-Bottom B/G值
	ShadingRes.ColorShadingBGLB = ShadingRes.ColorShadingBGData[m_AlgorithmThre.nColorShadingRowBlockNum - 1][0];
	// Right-Top B/G值
	ShadingRes.ColorShadingBGRT = ShadingRes.ColorShadingBGData[0][m_AlgorithmThre.nColorShadingColBlockNum - 1];
	// Right-Bottom B/G值
	ShadingRes.ColorShadingBGRB = ShadingRes.ColorShadingBGData[m_AlgorithmThre.nColorShadingRowBlockNum - 1][m_AlgorithmThre.nColorShadingColBlockNum - 1];

	return true;
}

bool CAlpAPSMPAlgorithm::OpticalCenter(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSOpticalCenterType& OpticalCenterType)
{
	// 在指定的ROI区域内，通过多帧图像数据计算出光强最大的位置作为光学中心
	// 基于投影法：
	// 1. 将2D图像投影到行和列两个方向；
	// 2. 最亮的行和列的交点即为光学中心；
	// 3. 通过多帧平均降低噪声影响；
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
			// 计算该像素点在所有帧、所有通道的均值
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
	// 找到行均值最大的行号
	Max(dMaxValue, OpticalCenterType.CenterRow, RowMean, RowMean.size());
	OpticalCenterType.CenterRow += RealRoi.Up;
	// 找到列均值最大的列号
	Max(dMaxValue, OpticalCenterType.CenterCol, ColMean, ColMean.size());
	OpticalCenterType.CenterCol += RealRoi.Left;
	return true;
}

bool CAlpAPSMPAlgorithm::PedestalVariation(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, APSPedestalVariationType& PedestalVariationRes)
{
	// 计算指定帧范围内、指定ROI区域的基底信号变化范围(最大值和最小值)，通常用于评估sensor的噪声特性
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

	// 计算ROI尺寸
	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	// 分块计算均值
	std::vector<CAPSDataContainer>BlockData(SubFrameIndex::All);

	// 将ROI区域分成多个小块，并计算每个块在多帧图像上的时域均值
	if (!GetBlockMean(nIndexStart, nNumber, &RealRoi,
		m_AlgorithmThre.nPedestalVariationRowBlockNum, // 行方向分块数
		m_AlgorithmThre.nPedestalVariationColBlockNum, // 列方向分块数
		BlockData,
		m_AlgorithmThre.nPedestalVariationRowBlockSize, // 行方向块大小
		m_AlgorithmThre.nPedestalVariationColBlockSize)) // 列方向块大小
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
	// 通过分析同一区域两帧图像的差异来评估噪声水平
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
	// 数学公式推导：
	// 1. 两个独立随机变量差值的方差 = 两者方差之和；
	// 2. 若两帧噪声方差相同为σ²，差值方差为2σ²；
	// 3. 差值标准差 = √(2σ²) = σ*√2；
	// 4. 因此单帧噪声σ = 差值标准差 / √2；
	// 即：ReadNoise = 差值标准差 / √2
	ReadNoiseRes = Std(AllPixel, nCur) / sqrt(2);

	return true;
}

bool CAlpAPSMPAlgorithm::DarkCurrent(std::vector<APSDataMeanType>& DataMean, std::vector<double>& ExpTime, APSDarkCurrentType& DarkCurrentRes)
{
	// 通过对多组不同曝光时间的数据进行线性拟合来确定暗电流系数
	// 均值法： 适用于已知基准参考，需要绝对暗电流值的场景
	uint32_t nChannelNum = m_bUse16SubFrame ? 16 : SubFrameIndex::All;

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
			// X: 曝光时间
			XData[nIndex] = ExpTime[nIndex];
		}
		std::vector<double> YData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			// Y: 该通道的均值数据
			YData[nIndex] = DataMean[nIndex].SubFrameDataMean[nChannelIndex];
		}
		double k = 0.0, b = 0.0;
		// 线性拟合，对(曝光数据, 均值)进行拟合，得到斜率k和截距b
		// 拟合公式： DataMean = K*t_exp+b
		if (LinearityFit(XData, YData, k, b))
		{
			DarkCurrentRes.SubFrameKValue[nChannelIndex] = k * 1000;// 为什么*1000再保存，单位转换？
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
	// 使用Temporal Noise数据来计算暗电流
	// 噪声方差法： 适用于相对测量，更关注暗电流引起的图像质量影响的场景
	uint32_t nChannelNum = m_bUse16SubFrame ? 16 : SubFrameIndex::All;

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
			// Y: 使用TNoise²
			YData[nIndex] = TNoise[nIndex].SubFrameTNoiseData[nChannelIndex].TempNoise * TNoise[nIndex].SubFrameTNoiseData[nChannelIndex].TempNoise;
		}
		double k = 0.0, b = 0.0;
		// 线性拟合，对(t_exp, TemproalNoise²)进行拟合
		// TemproalNoise² = DarkCurrent * t_exp;
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
	// 图像分块处理
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
			// G = (Gb + Gr) / 2;
			G.m_RawData[nRowBlocks][nColBlocks] = (BlockData[SubFrameIndex::Gr].m_RawData[nRowBlocks][nColBlocks] + BlockData[SubFrameIndex::Gb].m_RawData[nRowBlocks][nColBlocks]) / 2;
			B.m_RawData[nRowBlocks][nColBlocks] = BlockData[SubFrameIndex::B].m_RawData[nRowBlocks][nColBlocks];
			dPedestal += R.m_RawData[nRowBlocks][nColBlocks];
			dPedestal += G.m_RawData[nRowBlocks][nColBlocks];
			dPedestal += B.m_RawData[nRowBlocks][nColBlocks];
		}
	}

	// 基准值(Pedestal), 计算R、G、B通道的全局平均值作为基准值
	dPedestal = round(dPedestal / (3 * nRowBlockNum * nColBlockNum));

	double MaxR = R.m_RawData[0][0], MinR = R.m_RawData[0][0], MaxG = G.m_RawData[0][0], MinG = G.m_RawData[0][0], MaxB = B.m_RawData[0][0], MinB = B.m_RawData[0][0];
	double MaxSignal = 0, MinSignal = 10000, MinSignalCorner = 10000, MinSignalCentre = 10000, MinSignalEdge = 10000;
	uint32_t nMinSignalCornerRow = 0, nMinSignalCornerCol = 0;
	uint32_t nMinSignalCentreRow = 0, nMinSignalCentreCol = 0;
	uint32_t nMinSignalEdgeRow = 0, nMinSignalEdgeCol = 0;
	uint32_t nMinSignalRow = 0, nMinSignalCol = 0;

	// Corner: 四个角落2*2块区域
	ROIArea ROICorner[4] = { { 0, 1, 0, 1 }, { 0, 1, nColBlockNum - 2, nColBlockNum - 1 }, { nRowBlockNum - 2, nRowBlockNum - 1, 0, 1 }, { nRowBlockNum - 2, nRowBlockNum - 1, nColBlockNum - 2, nColBlockNum - 1 } };
	// Edge: 除Corner外的边缘区域
	ROIArea ROIEdge[4] = { { 0, 1, 2, nColBlockNum - 3 }, { nRowBlockNum - 2, nRowBlockNum - 1, 2, nColBlockNum - 3 }, {2, nRowBlockNum - 3, 0, 1 }, {2, nRowBlockNum - 3, nColBlockNum - 2, nColBlockNum - 1 }, };
	// Center：内部区域(排除最外两圈块)
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
			// 使用欧几里得距离计算每个块相对于基准的偏差
			double dSignal = sqrt(pow(R.m_RawData[nRowBlockIndex][nColBlockIndex] - dPedestal, 2) + pow(G.m_RawData[nRowBlockIndex][nColBlockIndex] - dPedestal, 2) + pow(B.m_RawData[nRowBlockIndex][nColBlockIndex] - dPedestal, 2));
			if (dSignal > MaxSignal)
				MaxSignal = dSignal; // 最大信号强度
			if (dSignal < MinSignal)
			{
				// 最小信号强度及其位置
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

	// RGB各通道范围(Range), Range值越小代表通道均匀性越好
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

	// 增量信号, DeltSignal值越小代表暗信号分布越均匀
	double dDeltaSignal = 0;
	DSNURes.DeltaSignalCentreMax = 0;
	DSNURes.DeltaSignalCornerMax = 0;
	DSNURes.DeltaSignalEdgeMax = 0;
	// 全局最大增量信号
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
	uint32_t nChannelNum = m_bUse16SubFrame ? 16 : SubFrameIndex::All;
	RawDataContainer& DataContainer = m_bUse16SubFrame ? m_16SubRawDataContainer : m_RawDataContainer;
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
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SubFrameDataMean, this, nIndexStart, nNumber, &RealRoi, SubFrameIndex(i), std::ref(DataMean.SubFrameDataMean[i]), std::ref(bSubRes[i]), std::ref(DataContainer));
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

bool CAlpAPSMPAlgorithm::Linearity(std::vector<APSDataMeanType>& LightMean, std::vector<double>& ExpTime, APSLinearityType& LinearityRes)
{
	// 通过拟合曝光时间与信号强度的关系，计算线性度误差
	uint32_t nChannelNum = m_bUse16SubFrame ? 16 : SubFrameIndex::All;

	if (LightMean.size() != ExpTime.size() && LightMean.size() != 0)
	{
		std::string strErr = "Linearity: Size Error: LightMean Size: " + std::to_string(LightMean.size()) + ", ExpTime Size: " + std::to_string(ExpTime.size());
		WriteLog(strErr, nChannelNum);
		return false;
	}
	for (uint32_t nIndex = 0; nIndex < LightMean.size(); nIndex++)
	{
		if (LightMean[nIndex].SubFrameDataMean.size() != nChannelNum)
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
			// 曝光时间t_exp
			XData[nIndex] = ExpTime[nIndex];
		}
		std::vector<double> YData(nDataNum);
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			// 对应的信号强度
			YData[nIndex] = LightMean[nIndex].SubFrameDataMean[nChannelIndex];
		}
		// 线性拟合
		if (LinearityFit(XData, YData, LinearityRes.SubFrameLinearityData[nChannelIndex].k, LinearityRes.SubFrameLinearityData[nChannelIndex].b))
		{
			LinearityRes.SubFrameLinearityData[nChannelIndex].LeMax = -100000, LinearityRes.SubFrameLinearityData[nChannelIndex].LeMin = 10000;
			for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
			{
				// 拟合值FitY
				double FitY = ExpTime[nIndex] * LinearityRes.SubFrameLinearityData[nChannelIndex].k + LinearityRes.SubFrameLinearityData[nChannelIndex].b;
				// 线性度误差LE = (实际值 - 拟合值)/拟合值
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
	// 利用PTC计算系统增益K。方差=K*均值+常数.
	uint32_t nChannelNum = m_bUse16SubFrame ? 16 : SubFrameIndex::All;

	// 确保噪声数据和均值数据的样本数量一致，否则返回错误
	if (LightTNoiseData.size() != LightMean.size())
	{
		std::string strErr = "OverallSystemGain: Size Error: LightTNoiseData Size: " + std::to_string(LightTNoiseData.size()) + ", LightMean Size: " + std::to_string(LightMean.size());
		WriteLog(strErr, nChannelNum);
		return false;
	}
	GainRes.SubFrameGainK.resize(nChannelNum);
	// 逐个通道计算增益
	for (uint32_t nChannelIndex = 0; nChannelIndex < nChannelNum; nChannelIndex++)
	{
		uint32_t nDataNum = LightMean.size();
		std::vector<double> XData(nDataNum);
		// 准备X轴数据(信号均值)
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			XData[nIndex] = LightMean[nIndex].SubFrameDataMean[nChannelIndex];
		}
		std::vector<double> YData(nDataNum);

		// 准备Y轴数据(方差差值)
		for (uint32_t nIndex = 0; nIndex < nDataNum; nIndex++)
		{
			// YData[nIndex] = σ²_light - σ²_dark;
			YData[nIndex] = LightTNoiseData[nIndex].SubFrameTNoiseData[nChannelIndex].TempNoise * LightTNoiseData[nIndex].SubFrameTNoiseData[nChannelIndex].TempNoise - DarkTNoiseBase.SubFrameTNoiseData[nChannelIndex].TempNoise * DarkTNoiseBase.SubFrameTNoiseData[nChannelIndex].TempNoise;
		}
		// 对(均值,方差差值)数据点进行线性拟合, 斜率K即为该通道的增益系数
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
	// 计算饱和度特性，包括均值、时域噪声、信噪比
	uint32_t nChannelNum = m_bUse16SubFrame ? 16 : SubFrameIndex::All;
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

	// 计算均值
	bool bRes = false;
	SubFrameDataMean(nIndexStart, nNumber, &RealRoi, nChannelIndex, SaturationRes.SaturationMean, bRes, DataContainer);
	if (!bRes)
	{
		return false;
	}
	// 计算时域噪声
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

	// 计算信噪比: SNR = Mean / TNoise;
	SaturationRes.SaturationSNR = SaturationRes.SaturationMean / SaturationRes.SaturationTNoise;

	return true;
}

bool CAlpAPSMPAlgorithm::OETC(uint32_t nIndexStart, uint32_t nNumber, uint32_t nNumberInOneStep, ROIArea* ROI, SubFrameIndex nChannelIndex, APSOETCType& OETCRes)
{
	uint32_t nChannelNum = SubFrameIndex::All;
	if (nChannelIndex >= nChannelNum)
	{
		std::string strErr = "OETC: SubFrameIndex beyond the max num";
		WriteLog(strErr, nChannelIndex);
		return false;
	}

	if (nNumberInOneStep == 0)
	{
		std::string strErr = "OETC: NumberInOneStep is 0";
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

	// 添加参数验证
	if (nNumber % 2 != 0 || nNumber < 4) {
		std::string strErr = "OETC: nNumber must be even and >= 4";
		WriteLog(strErr, nChannelIndex);
		return false;
	}

	// 计算ROI中心区域(半径为nOETCRaduis的正方形区域)
	uint32_t nRoiRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nRoiCol = RealRoi.Right - RealRoi.Left + 1;
	ROIArea temp = { 0 };
	temp.Up = RealRoi.Up + nRoiRow / 2 - m_AlgorithmThre.nOETCRadius;
	temp.Down = RealRoi.Up + nRoiRow / 2 + m_AlgorithmThre.nOETCRadius;
	temp.Left = RealRoi.Left + nRoiCol / 2 - m_AlgorithmThre.nOETCRadius;
	temp.Right = RealRoi.Left + nRoiCol / 2 + m_AlgorithmThre.nOETCRadius;

	RealRoi = temp;

	RawDataContainer& DataContainer = m_RawDataContainer;

	OETCRes.ReadNoiseData.resize(nNumber / nNumberInOneStep);
	OETCRes.DataMean.resize(nNumber / nNumberInOneStep);
	OETCRes.TNoiseData.resize(nNumber / nNumberInOneStep);
	bool bRes = false;

    // 双采样, 每次处理2帧消除固定模式噪声
    for (uint32_t nIndex = 0; nIndex < nNumber; nIndex += nNumberInOneStep)
    {
		// 1. 计算平均信号值
		SubFrameDataMean(nIndexStart + nIndex, nNumberInOneStep, &RealRoi, nChannelIndex, OETCRes.DataMean[nIndex / nNumberInOneStep], bRes, DataContainer);
		SubFrameDataMean(nIndexStart + nIndex, 2, &RealRoi, nChannelIndex, OETCRes.DataMean[nIndex / 2], bRes, DataContainer);
        if (!bRes)
        {
            return false;
        }
		// 2. 计算时域噪声(Temporal Noise)
        APSSubFrameTNoiseType temp;
		SubFrameTNoise(nIndexStart + nIndex, nNumberInOneStep, &RealRoi, nChannelIndex, temp, bRes, DataContainer);
        if (!bRes)
        {
            return false;
        }
		OETCRes.TNoiseData[nIndex / nNumberInOneStep] = temp.TempNoise;
		OETCRes.ReadNoiseData[nIndex / nNumberInOneStep] = temp.TempNoise;
		// 3. 计算Read Noise(通过相邻2帧差分)
		//SubFrameReadNoise(nIndexStart + nIndex, nIndexStart + nIndex + 1, &RealRoi, nChannelIndex, OETCRes.ReadNoiseData[nIndex / 2], bRes, DataContainer);
		OETCRes.TNoiseData[nIndex / 2] = temp.TempNoise;
        if (!bRes)
        {
            return false;
        }
    }

    double dMaxValue = 0;
	uint32_t nMaxLocal = 0;
	//Max(dMaxValue, nMaxLocal, TNoiseData, TNoiseData.size());
	//SubFrameReadNoise(0, 1, &RealRoi, nChannelIndex, ReadNoiseData, bRes, DataContainer);

	// FWC检测
	// 策略1：检测Read Noise突变, 原理：像素饱和后，噪声会突然下降(因为信号被限幅)
	for (uint32_t nIndex = 0; nIndex < OETCRes.ReadNoiseData.size() - 1; nIndex += 1)
	{
		double U = OETCRes.ReadNoiseData[nIndex] - OETCRes.ReadNoiseData[nIndex + 1];
		if ((U > 1.5) && (nIndex > 10)) // 噪声突降>1.5且帧数>10
		{
			nMaxLocal = nIndex;
			break;
		}
	}

	// 策略2：寻找Read Noise峰值, 如果上面没有检测到突变, 将Read Noise峰值+2作为饱和点
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
	// 计算FWC
	OETCRes.FWC = OETCRes.DataMean[nMaxLocal] - OETCRes.DataMean[0];
	// 计算Read Noise
	OETCRes.ReadNoise = OETCRes.ReadNoiseData[0];
	// 公式：DR = 20*log10(FWC/Read Noise)
	OETCRes.DR_dB = 20 * log10(OETCRes.FWC / OETCRes.ReadNoise);

	std::vector<double> XData;
	std::vector<double> YData;

	// 计算 Conversion Gain, 使用PTC方法
	for (uint32_t n = 0; n < OETCRes.DataMean.size(); n++)
	{
		// 只使用FWC的10%~70%的线性区域数据
		if ((OETCRes.DataMean[n] - OETCRes.DataMean[0]) >= OETCRes.FWC * 0.1 && (OETCRes.DataMean[n] - OETCRes.DataMean[0]) <= OETCRes.FWC * 0.7)
		{
			XData.push_back(OETCRes.DataMean[n] - OETCRes.DataMean[0]); // 信号值
			YData.push_back(OETCRes.ReadNoiseData[n] * OETCRes.ReadNoiseData[n] - OETCRes.ReadNoiseData[0] * OETCRes.ReadNoiseData[0]); // 方差
		}
	}

	double k = 0.0, b = 0.0;
	// 线性拟合, 最下二乘法线性回归
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

bool CAlpAPSMPAlgorithm::Linearity(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSSNRType& SSNRRes)
{
	// 分析CIS Linearity，通过计算SSNR来评估性能
	uint32_t nChannelNum = SubFrameIndex::All;
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
		// 计算帧平均值
		SubFrameDataMean(nIndexStart + nIndex, 1, &RealRoi, nChannelIndex, SSNRRes.DataMean[nIndex], bRes, DataContainer);
		if (!bRes)
		{
			return false;
		}
		APSSubFrameSNoiseType temp;
		// 计算空域噪声
		SubFrameSNoise(nIndexStart + nIndex, 1, &RealRoi, nChannelIndex, temp, bRes, DataContainer);
		if (!bRes)
		{
			return false;
		}
		SSNRRes.SNoiseData[nIndex] = temp.SNoise;

		// 计算空域SNR
		// SSNR计算公式：20*log10(信号差值 / 空域噪声)
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

	// 寻找最大SSNR位置
	double dMax = 0;
	uint32_t nMaxLocal = 0;

	for (uint32_t nIndex = 0; nIndex < nNumber - 1; nIndex += 1)
	{
		double U = abs(SSNRRes.SSNR[nIndex + 1] - SSNRRes.SSNR[nIndex]);
		// 检测SSNR突变点
		// 检测逻辑：
		// 1. 相邻帧SSNR差值超过2.5dB;
		// 2. 排除前10帧和后5帧;
		// 3. 找到第一个满足条件的位置作为线性度上限
		if ((U > 2.5) && (nIndex > 10) && (nIndex < nNumber - 5))
		{
			nMaxLocal = nIndex;
			break;
		}
	}

	if (nMaxLocal == 0)
	{
		// 如果整个序列都是线性的，nMaxLocal保持为0，MaxSSNR = -1 是否合理？
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

bool CAlpAPSMPAlgorithm::Show(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, bool bNormalize, ImgType& ImgData)
{
	uint32_t nChannelNum = m_bUse16SubFrame ? 16 : SubFrameIndex::All;
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
		// 降采样
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
		// 对每个像素点：
		// 多帧平均：累加指定数量的帧数据并求平均
		// 数据类型转换:
		// Raw8：直接取整(floor(temp))
		// Raw10/UNPACK10：除以4后直接取整(floor(temp/4))
		// 其他类型：除以16后直接取整(floor(temp/16))
		// 负值处理：小于0的值设为0
		// ROI外区域：设为0
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
		// 归一化处理
		// 1. 多帧平均：使用临时容器累加并平均
		// 2. 找到最值: 在ROI内找到最大值、最小值
		// 3. 线性归一化： NewValue = (原始值-最小值) / (最大值-最小值) * 255;
		// 4. 四舍五入：round(NewValue)
		// 5. ROI外区域：设为0
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
					double NewValue = (NormalizeDataContainer.m_RawData[nRows][nCols] - dMinValue) / (dMaxValue - dMinValue) * 255; // BUG: (dMaxValue - dMinValue)==0时会有除0错误
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
	// 从原始数据容器中提取指定帧范围的图像数据，对ROI内的像素进行求平均，生成合成图像
	uint32_t nChannelNum = m_bUse16SubFrame ? 16 : SubFrameIndex::All;
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
		// m_bUse16SubFrame 是什么模式？
		RealRoi.Up /= 2; // 向下取整
		RealRoi.Left /= 2;
		RealRoi.Right = (RealRoi.Right + 1) / 2 - 1; // 向上取整后减1
		RealRoi.Down = (RealRoi.Down + 1) / 2 - 1;
		nChannelRow /= 2;
		nChannelCol /= 2;
	}

	// 创建二维数组，大小为nChannelRow*nChannelCol
	ImgData.resize(nChannelRow);
	for (uint32_t nIndex = 0; nIndex < nChannelRow; nIndex++)
	{
		ImgData[nIndex].resize(nChannelCol);
	}

	// 遍历每个像素位置
	// ROI内：对指定帧范围(nIndexStart到nIndexStart+nNumber-1)的数据求平均
	// ROI外：填充0值
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

void CAlpAPSMPAlgorithm::SetDataToFrame(uint32_t nIndex, uint32_t nRowStart, uint32_t nRows, uint16_t* RawData)
{
	for (uint32_t nRowIndex = nRowStart; nRowIndex < nRowStart + nRows; nRowIndex++)
	{
		uint32_t nBase = nRowIndex * m_nTotalCol;
		for (uint32_t nColIndex = 0; nColIndex < m_nTotalCol; nColIndex++)
		{
			double dValue = 0;
			GetDataFromSubFrame(nIndex, nRowIndex, nColIndex, dValue);
			RawData[nBase + nColIndex] = uint16_t(dValue);
		}
	}
}

bool CAlpAPSMPAlgorithm::Show(uint32_t nIndex, uint16_t* RawData)
{
	// 将指定nIndex的帧数据提取到RawData数组中，支持单线程和多线程两种模式
	if (nIndex >= m_RawDataContainer[0].size())
	{
		std::string strErr = "Show: Index error: nIndex: " + std::to_string(nIndex);
		WriteLog(strErr, SubFrameIndex::All);
		return false;
	}

	if (m_bMultiThreadEnable)
	{
		// 多线程划分策略：将图像按行分成8份
		int RowDiv = 8;
		// 计算实际需要的线程数：
		// 如果总行数能被8整除，使用8个线程，否则使用9个线程，多余的一个线程用于处理余数部分
		int ThreadNum = m_nTotalRow % RowDiv == 0 ? RowDiv : RowDiv + 1;
		std::vector<std::thread*> t(ThreadNum);

		uint32_t nRowStart = 0;
		// 每个线程处理的行数
		uint32_t nRows = m_nTotalRow / RowDiv;

		//
		for (int i = 0; i < ThreadNum; i++)
		{
			if ((nRowStart + nRows) > m_nTotalRow)
			{
				nRows = m_nTotalRow - nRowStart;
			}
			t[i] = new std::thread(&CAlpAPSMPAlgorithm::SetDataToFrame, this, nIndex, nRowStart, nRows, RawData);
			nRowStart += nRows;
		}

		for (uint32_t i = 0; i < ThreadNum; i++)
		{
			t[i]->join(); // 等待线程完成
			delete t[i]; // 释放线程对象
		}
	}
	else
	{
		// 单线程
		SetDataToFrame(nIndex, 0, m_nTotalRow, RawData);
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
	return MP_ALGORITHM_VERSION;
}

int CAlpAPSMPAlgorithm::GetCode()
{
	return m_nCode;
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
	// 判断给定坐标是否在ROI内
	if (nRows >= ROI.Up && nRows <= ROI.Down && nCols >= ROI.Left && nCols <= ROI.Right)
	{
		return true;
	}
	return false;
}

bool CAlpAPSMPAlgorithm::LinearityFit(std::vector<double>& XData, std::vector<double>& YData, double& k, double& b)
{
	// 这是一个最小二乘法线性回归函数的实现，用于拟合一条直线`y=kx+b`。
	// 函数功能：计算给定数据点的最佳拟合直线的斜率k和截距b。

	// 参数检查:
	// 1. 检查数据是否为空；
	// 2. 检查X和Y数据点数量是否一致；
	// 没有检查数据点数量是否至少为2，至少需要2个点才能拟合直线
	if (0 == XData.size() || XData.size() != YData.size())
	{
		return false;
	}
	double dSumX = 0, dSumXY = 0, dSumY = 0, dSumX2 = 0;
	uint32_t nSize = XData.size();
	// 累积计算最小二乘法所需的四个统计量
	for (uint32_t nIndex = 0; nIndex < nSize; nIndex++)
	{
		dSumX += XData[nIndex]; // Σx
		dSumY += YData[nIndex]; // Σy
		dSumXY += XData[nIndex] * YData[nIndex]; // Σ(xy)
		dSumX2 += XData[nIndex] * XData[nIndex]; // Σ(x²)
	}

	// 奇异性检查：
	// 1. 检查分母是否为0；
	// 2. 当所有X值相同时会出现：垂直线，无法用`y=kx+b`表示
	// 使用浮点数直接与0比较不够鲁棒，应该用阈值判断比较好
	if (0 == nSize * dSumX2 - dSumX * dSumX)
	{
		return false;
	}

	k = (nSize * dSumXY - dSumX * dSumY) / (nSize * dSumX2 - dSumX * dSumX);
	// b = dSumY / nSize - dSumX / nSize * k;
	b = (dSumY - k * dSumX) / nSize;
	return true;
}

void CAlpAPSMPAlgorithm::SubFrameTNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameTNoiseType& TNoise, bool& bRes, RawDataContainer& DataContainer)
{
	// 分析多帧图像数据，计算指定区域内的时域噪声：
	// 1. 像素级时域噪声；
	// 2. 行级时域噪声；
	// 3. 列级时域噪声；
	// 4. 以及它们之间的关系；
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
	// 验证：
	// 1. 帧数必须 >= 2，至少需要两帧才能计算标准差；
	// 2. 索引范围必须有效；
	// 3. ROI 边界必须合法；
	if (nNumber < 2 || nIndexStart >= DataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > DataContainer[nChannelIndex].size())
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

	CAPSDataContainer PixelTNoiseArray; // 存储每个像素的时域噪声
	PixelTNoiseArray.Init(nRow, nCol);

	CAPSDataContainer RowDataArray; // 存储每行在各帧的累加值
	RowDataArray.Init(nRow, nNumber, true);

	std::vector<double> RowNoise(nRow, 0); // 每行的噪声
	std::vector<double> ColNoise(nCol, 0); // 每列的噪声

	CAPSDataContainer ColDataArray; // 存储每列在各帧的累加值
	ColDataArray.Init(nCol, nNumber, true);

	std::vector<double> onePixelInMultiFrames(nNumber);

	for (uint32_t nRows = 0; nRows < nRow; nRows++) // 每一行
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++) // 每一列
		{
			for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++) // 每一帧
			{
				// 提取同一像素位置在不同帧的值
				double dValue = DataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
				onePixelInMultiFrames[nIndex] = dValue;
				// 累加到行数据和列数据
				RowDataArray.m_RawData[nRows][nIndex] += dValue;
				ColDataArray.m_RawData[nCols][nIndex] += dValue;
			}
			// 计算该像素的时域标准差
			PixelTNoiseArray.m_RawData[nRows][nCols] = Std(onePixelInMultiFrames, nNumber);
		}
	}
	// 将累加值转换为平均值
	RowDataArray /= nCol;
	ColDataArray /= nRow;

	// 计算每行的时域标准差
	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		RowNoise[nRows] = Std(RowDataArray.m_RawData[nRows], nNumber);
	}

	// 计算每列的时域标准差
	for (uint32_t nCols = 0; nCols < nCol; nCols++)
	{
		ColNoise[nCols] = Std(ColDataArray.m_RawData[nCols], nNumber);
	}

	TNoise.RowTemp = RMS(RowNoise, nRow); // 行噪声的RMS值, 行方向的系统性噪声(可能由于读出电路)
	TNoise.ColTemp = RMS(ColNoise, nCol); // 列噪声的RMS值, 列方向的系统噪声(可能由于列放大器)
	TNoise.TempNoise = RMS(PixelTNoiseArray); // 总时域噪声
	// 分离出Pixel噪声(减去行、列相关噪声) , 纯随机像素噪声(光子散粒噪声等)
	TNoise.PixelTemp = sqrt(TNoise.TempNoise * TNoise.TempNoise - TNoise.RowTemp * TNoise.RowTemp - TNoise.ColTemp * TNoise.ColTemp);
	// 计算噪声比例, 用于评估噪声的主要来源
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
	// 物理意义：
	// 实现了噪声分解模型：总时域噪声² = 像素级噪声² + 行相关噪声² + 列相关噪声²
	return;
}

void CAlpAPSMPAlgorithm::SubFrameSNoise(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, APSSubFrameSNoiseType& SNoiseData, bool& bRes, RawDataContainer& DataContainer)
{
	// 计算子帧数据的空间噪声统计特性, 包括整体噪声、行噪声、列噪声
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
	if (0 == nNumber || nIndexStart >= DataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > DataContainer[nChannelIndex].size())
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
	CAPSDataContainer PixelSNoiseArray; // 存储平均后的像素值
	PixelSNoiseArray.Init(nRow, nCol, true);

	std::vector<double> RowMean(nRow, 0); // 每行的平均值
	std::vector<double> ColMean(nCol, 0);// 每列的平均值

	// 对所有帧求平均
	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++)
		{
			double dValue = 0;
			for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
			{
				dValue += DataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			}
			dValue = round(dValue / nNumber); // 四舍五入
			PixelSNoiseArray.m_RawData[nRows][nCols] = dValue;
			RowMean[nRows] += dValue; // 累加行和
			ColMean[nCols] += dValue; // 累加列和
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

	SNoiseData.SNoise = Std(PixelSNoiseArray); // 整体Pixel标准差
	SNoiseData.RowSNoise = Std(RowMean, nRow); // 行平均的标准差
	SNoiseData.ColSNoise = Std(ColMean, nCol); // 列平均的标准差
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
	BadpixelRes.MaxClusterSize = 0;
	BadpixelRes.BadPixelMask.LocalData.clear();
	BadpixelRes.BadPixelMask.Flag.clear();
	BadpixelRes.BadPixelMask.DiffData.clear();
	BadpixelRes.BadPixelMask.BadPixelNum = 0;
	std::map<Local, float> DiffMap;

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

				// 比值法
				if (abs(dCurrentPixel - dSurroundPixle) / dSurroundPixle > m_AlgorithmThre.dBadPixelThre)
				{
					//BadpixelRes.BadPixelMask.BadPixelNum++;
					//BadpixelRes.BadPixelMask.LocalData.push_back({ nRows - m_AlgorithmThre.nBadPixelRadius + RealRoi.Up, nCols - m_AlgorithmThre.nBadPixelRadius + RealRoi.Left });
					//BadpixelRes.BadPixelMask.Flag.push_back(APS_BAD_PIXEL_FLAG);

					BadpixelRes.BadPixelNum++;
					BadPixelMask[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius] = 1;
					Local temp = { nRows - m_AlgorithmThre.nBadPixelRadius , nCols - m_AlgorithmThre.nBadPixelRadius };
					DiffMap[temp] = abs(dCurrentPixel - dSurroundPixle) / dSurroundPixle;
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
				BadPixelMask[nRows][nCols] = ConnectedAreaFlag;
				Search.push_back({ nRows, nCols });
				while (nCur != Search.size())
				{
					Local temp = Search[nCur];
					nCur++;
					AreaSize++;
					for (int nTempRows = (int)temp.x - 1; nTempRows <= (int)temp.x + 1; nTempRows++)
					{
						if (nTempRows >= 0 && nTempRows < nRow)
						{
							for (int nTempCols = (int)temp.y - 1; nTempCols <= (int)temp.y + 1; nTempCols++)
							{
								if (nTempCols >= 0 && nTempCols < nCol && BadPixelMask[nTempRows][nTempCols] != 0 && BadPixelMask[nTempRows][nTempCols] < ConnectedAreaFlag)
								{
									BadPixelMask[nTempRows][nTempCols] = ConnectedAreaFlag;
									Search.push_back({ (uint32_t)nTempRows , (uint32_t)nTempCols });
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
				if (AreaSize > BadpixelRes.MaxClusterSize )
				{
					BadpixelRes.MaxClusterSize = AreaSize;
				}
				for (uint32_t n = 0; n < Search.size(); n++)
				{
					BadpixelRes.BadPixelMask.LocalData.push_back(Search[n]);
					BadpixelRes.BadPixelMask.Flag.push_back(uFlag);
					BadpixelRes.BadPixelMask.DiffData.push_back(DiffMap[Search[n]]);
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
		// 计算局部邻域的行均值(半径m_AlgorithmThre.nBadLineRadius范围内，排除当前行)
		double dBaseMean = 0;
		for (uint32_t n = nRows - m_AlgorithmThre.nBadLineRadius; n <= nRows + m_AlgorithmThre.nBadLineRadius; n++)
		{
			if (n != nRows)
			{
				dBaseMean += RowMean[n];
			}
		}
		dBaseMean /= 2 * m_AlgorithmThre.nBadLineRadius;
		// 比值判断
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
	// 检测并统计坏点、坏行、坏列
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
	HotpixelRes.MaxClusterSize = 0;
	HotpixelRes.BadPixelMask.LocalData.clear();
	HotpixelRes.BadPixelMask.Flag.clear();
	HotpixelRes.BadPixelMask.DiffData.clear();
	HotpixelRes.BadPixelMask.BadPixelNum = 0;
	std::map<Local, float> DiffMap;

	// 对指定的多帧图像，计算每个像素的均值

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
				// 邻域比较，对每个像素：
				// 1. 获取其周围半径为nBadPixelRadius的邻域像素；
				// 2. 计算邻域均值时，去掉最大值和最小值，避免异常值影响；
				// 3. 比较当前像素与邻域均值的差异；
				// 4. 如果差异超过阈值dHotPixelThre，标记为坏点；
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
					Local temp = { nRows - m_AlgorithmThre.nBadPixelRadius , nCols - m_AlgorithmThre.nBadPixelRadius };
					DiffMap[temp] = abs(dCurrentPixel - dSurroundPixle);
				}
			}
		}
	}

	// 使用广度优先搜索(BFS)算法，将相邻的坏点聚合成连通区域：
	// 1. Singlet, 单点: 孤立的单个坏点；
	// 2. Couplet, 双点：两个相邻的坏点；
	// 3. Cluster, 簇：3个或以上相邻的坏点；
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
				BadPixelMask[nRows][nCols] = ConnectedAreaFlag;
				Search.push_back({ nRows, nCols });
				while (nCur != Search.size())
				{
					Local temp = Search[nCur];
					nCur++;
					AreaSize++;
					// BFS搜索8邻域连通性
					for (int nTempRows = (int)temp.x - 1; nTempRows <= (int)temp.x + 1; nTempRows++)
					{
						// 检查相邻坏点并加入连通域
						if (nTempRows >= 0 && nTempRows < nRow)
						{
							for (int nTempCols = (int)temp.y - 1; nTempCols <= (int)temp.y + 1; nTempCols++)
							{
								if (nTempCols >= 0 && nTempCols < nCol && BadPixelMask[nTempRows][nTempCols] != 0 && BadPixelMask[nTempRows][nTempCols] < ConnectedAreaFlag)
								{
									BadPixelMask[nTempRows][nTempCols] = ConnectedAreaFlag;
									Search.push_back({ (uint32_t)nTempRows , (uint32_t)nTempCols });
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
				if (AreaSize > HotpixelRes.MaxClusterSize)
				{
					HotpixelRes.MaxClusterSize = AreaSize;
				}
				for (uint32_t n = 0; n < Search.size(); n++)
				{
					HotpixelRes.BadPixelMask.LocalData.push_back(Search[n]);
					HotpixelRes.BadPixelMask.Flag.push_back(uFlag);

					HotpixelRes.BadPixelMask.DiffData.push_back(DiffMap[Search[n]]);
					HotpixelRes.BadPixelMask.BadPixelNum++;
				}
				ConnectedAreaFlag--;
			}
		}
	}

	// 坏行、坏列检测：
	// 1. 计算每行每列的像素均值；
	// 2. 与整体基准均值dBaseMean比较；
	// 3. 如果差异超过阈值dHotLineThre，标记为坏行或坏列；
	std::vector<double> RowMean(nRow, 0);
	std::vector<double> ColMean(nCol, 0);

	double dBaseMean = 0;
	SubFrameDataMean(nIndexStart, nNumber, &RealRoi, nChannelIndex, dBaseMean, bRes, m_RawDataContainer);

	if (!bRes)
	{
		return;
	}

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
		// 行均值检测
		if (abs(RowMean[nRows] - dBaseMean) > m_AlgorithmThre.dHotLineThre)
		{
			HotpixelRes.DefectRowNum++;
		}
	}
	for (uint32_t nCols = 0; nCols < nCol; nCols++)
	{
		ColMean[nCols] /= nRow;
		// 列均值检测
		if (abs(ColMean[nCols] - dBaseMean) > m_AlgorithmThre.dHotLineThre)
		{
			HotpixelRes.DefectColNum++;
		}

	}
	//for (uint32_t nRows = m_AlgorithmThre.nBadLineRadius; nRows < nRow - m_AlgorithmThre.nBadLineRadius; nRows++)
	//{
	//	double dBaseMean = 0;
	//	for (uint32_t n = nRows - m_AlgorithmThre.nBadLineRadius; n <= nRows + m_AlgorithmThre.nBadLineRadius; n++)
	//	{
	//		if (n != nRows)
	//		{
	//			dBaseMean += RowMean[n];
	//		}
	//	}
	//	dBaseMean /= 2 * m_AlgorithmThre.nBadLineRadius;
	//	if (abs(RowMean[nRows] - dBaseMean) > m_AlgorithmThre.dHotLineThre)
	//	{
	//		HotpixelRes.DefectRowNum++;
	//	}
	//}
	//for (uint32_t nCols = m_AlgorithmThre.nBadLineRadius; nCols < nCol - m_AlgorithmThre.nBadLineRadius; nCols++)
	//{
	//	double dBaseMean = 0;
	//	for (uint32_t n = nCols - m_AlgorithmThre.nBadLineRadius; n <= nCols + m_AlgorithmThre.nBadLineRadius; n++)
	//	{
	//		if (n != nCols)
	//		{
	//			dBaseMean += ColMean[n];
	//		}
	//	}
	//	dBaseMean /= 2 * m_AlgorithmThre.nBadLineRadius;
	//	if (abs(ColMean[nCols] - dBaseMean) > m_AlgorithmThre.dHotLineThre)
	//	{
	//		HotpixelRes.DefectColNum++;
	//	}
	//}
	return;
}

/*
## 两个函数的对比分析

| 对比维度 | SubFrameBLC | SubFrameBLCByColBase |
|---------|-------------|---------------------|
| **基准值参数** | `double& BaseMean` (单一值) | `std::vector<double>& BaseMean` (向量) |
| **校正方式** | 全局统一校正 | 逐列独立校正 |
| **适用场景** | 传感器暗电流均匀分布 | 传感器存在列间差异/列固定模式噪声(CFPN) |
| **参数检查** | 无需检查向量大小 | 需验证 `BaseMean.size() == ROI列数` |
| **校正精度** | 较低（忽略列间差异） | 较高（考虑每列特性） |
| **计算复杂度** | 较低 | 稍高（需索引向量） |
| **内存占用** | 单个double值 | 需存储整列的基准值向量 |

### 核心差异示意

**SubFrameBLC (全局校正):**
```
所有像素 -= 100  (假设BaseMean = 100)

[120, 150, 130]    [20, 50, 30]
[110, 140, 125] -> [10, 40, 25]
[115, 145, 135]    [15, 45, 35]
```

**SubFrameBLCByColBase (按列校正):**
```
第0列 -= 100, 第1列 -= 105, 第2列 -= 95

[120, 150, 130]    [20, 45, 35]
[110, 140, 125] -> [10, 35, 30]
[115, 145, 135]    [15, 40, 40]
*/
void CAlpAPSMPAlgorithm::SubFrameBLC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, double& BaseMean, bool& bRes)
{
	// 对指定范围内的图像帧进行全局统一的黑电平校正，所有像素减去同一个基准值
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
		std::string strErr = "SubFrameBLC: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameBLC: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
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
				// 每个像素减去统一标量
				m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols] -= BaseMean;
				if (m_bUse16SubFrame)
				{
					uint32_t nSubFrameChannel = nChannelIndex * 4;
					if ((nRows & 1) == 0 && (nCols & 1) == 0)
					{
						// (偶行, 偶列): +0
					}
					else if ((nRows & 1) == 0 && (nCols & 1) == 1)
					{
						// (偶行, 奇列): +1
						nSubFrameChannel += 1;
					}
					else if ((nRows & 1) == 1 && (nCols & 1) == 0)
					{
						// (奇行, 偶列): +2
						nSubFrameChannel += 2;
					}
					else
					{
						// (奇行, 奇列): +3
						nSubFrameChannel += 3;
					}
					// 然后对子帧进行相同的校正
					m_16SubRawDataContainer[nSubFrameChannel][nIndexStart + nIndex].m_RawData[nRows >> 1][nCols >> 1] -= BaseMean;
				}
			}
		}
	}
	return;
}

void CAlpAPSMPAlgorithm::SubFrameDPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, std::vector<Local>& BadPixelList, bool& bRes)
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

	for (uint32_t nBadPixelIndex = 0; nBadPixelIndex < BadPixelList.size(); nBadPixelIndex++)
	{
		uint32_t nBadpixelRows = BadPixelList[nBadPixelIndex].x;
		uint32_t nBadpixelCols = BadPixelList[nBadPixelIndex].y;

		for (uint32_t nFrameIndex = 0; nFrameIndex < nNumber; nFrameIndex++)
		{
			CAPSDataContainer& CurRawData = m_RawDataContainer[nChannelIndex][nIndexStart + nFrameIndex];
			uint32_t nSize = 0;
			double dMeanData = 0;
			// 3*3邻域均值差值, 对每个坏点执行:
			// 1. 遍历坏点周围的3*3邻域(8个像素);
			// 2. 排除邻域中的其他坏点;
			// 3. 计算有效邻域像素的均值;
			// 4. 用均值替换坏点;
			for (uint32_t nCurRows = nBadpixelRows - 1; nCurRows <= nBadpixelRows + 1; nCurRows++)
			{
				if (nCurRows >= RealRoi.Up && nCurRows <= RealRoi.Down)
				{
					for (uint32_t nCurCols = nBadpixelCols - 1; nCurCols <= nBadpixelCols + 1; nCurCols++)
					{
						if (nCurCols >= RealRoi.Left && nCurCols <= RealRoi.Right && BadPixelList.end() == std::find(BadPixelList.begin(), BadPixelList.end(), Local{ nCurRows, nCurCols }))
						{
							dMeanData += CurRawData.m_RawData[nCurRows][nCurCols];
							nSize++;
						}
					}
				}
			}
			if (nSize > 0)
			{
				// 均值替换坏点
				CurRawData.m_RawData[nBadpixelRows][nBadpixelCols] = round(dMeanData / nSize);
				if (m_bUse16SubFrame)
				{
					uint32_t nSubFrameChannel = nChannelIndex * 4;
					if ((nBadpixelRows & 1) == 0 && (nBadpixelCols & 1) == 0)
					{

					}
					else if ((nBadpixelRows & 1) == 0 && (nBadpixelCols & 1) == 1)
					{
						nSubFrameChannel += 1;
					}
					else if ((nBadpixelRows & 1) == 1 && (nBadpixelCols & 1) == 0)
					{
						nSubFrameChannel += 2;
					}
					else
					{
						nSubFrameChannel += 3;
					}
					m_16SubRawDataContainer[nSubFrameChannel][nIndexStart + nFrameIndex].m_RawData[nBadpixelRows >> 1][nBadpixelCols >> 1] = round(dMeanData / nSize);
				}
			}
		}
	}
	return;
}

void CAlpAPSMPAlgorithm::SubFrameDataMean(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, SubFrameIndex nChannelIndex, double& DataMean, bool& bRes, RawDataContainer& DataContainer)
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
	if (0 == nNumber || nIndexStart >= DataContainer[nChannelIndex].size() || (nIndexStart + nNumber) > DataContainer[nChannelIndex].size())
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
				dValue += DataContainer[nChannelIndex][nIndexStart + nFrameIndex].m_RawData[nRows][nCols];
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
	// 按列基准进行黑电平校正(Black Level Correction, BLC)
	// 对指定范围内的图像帧进行黑电平校正，逐列减去对应的基准值(暗电流/偏置)
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
				// 每个像素减去对应列的基准值
				m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols] -= BaseMean[nCols - RealRoi.Left];
				if (m_bUse16SubFrame)
				{
					// 需要处理拜耳模式的4个子通道
					uint32_t nSubFrameChannel = nChannelIndex * 4;
					if ((nRows & 1) == 0 && (nCols & 1) == 0)
					{

					}
					else if ((nRows & 1) == 0 && (nCols & 1) == 1)
					{
						nSubFrameChannel += 1;
					}
					else if ((nRows & 1) == 1 && (nCols & 1) == 0)
					{
						nSubFrameChannel += 2;
					}
					else
					{
						nSubFrameChannel += 3;
					}
					m_16SubRawDataContainer[nSubFrameChannel][nIndexStart + nIndex].m_RawData[nRows >> 1][nCols >> 1] -= BaseMean[nCols - RealRoi.Left];
				}
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
	// 检查参数.
	// 1. index error; 2. ROI border overflow error; 3. BlockSize calc error;
	if (0 == nNumber || // 帧数为0
		nIndexStart >= m_RawDataContainer[nChannelIndex].size() || // 起始索引越界
		(nIndexStart + nNumber) > m_RawDataContainer[nChannelIndex].size()) // 结束索引越界
	{
		std::string strErr = "SubFrameBlockMean: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}

	if (RealRoi.Down >= m_nChannelRow || // 下边界超出图像行数
		RealRoi.Right >= m_nChannelCol ||  // 右边界超出图像列数
		RealRoi.Down < RealRoi.Up || // 下边界在上边界之上（逻辑错误）
		RealRoi.Right < RealRoi.Left) // 右边界在左边界之左（逻辑错误）
	{
		std::string strErr = "SubFrameBlockMean: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}

	// 计算ROI区域的实际行数和列数
	int32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	int32_t nCol = RealRoi.Right - RealRoi.Left + 1;

	// 计算每个Block 的基础大小
	int32_t nRowBlockSizeBase = nSubRowBlockSize != 0 ? nSubRowBlockSize : nRow / nRowBlockNum;
	int32_t nColBlockSizeBase = nSubColBlockSize != 0 ? nSubColBlockSize : nCol / nColBlockNum;
	// 计算无法均分的余数
	int32_t nRowMod = nRow - nRowBlockNum * nRowBlockSizeBase;
	int32_t nColMod = nCol - nColBlockNum * nColBlockSizeBase;
	// 比如：如果ROI有100行，要分成3块，每块基础大小33行，余数=100-3x33=1

	// 初始化输出容器为 nRowBlockNum*nColBlockNum
	BlockData.Init(nRowBlockNum, nColBlockNum);

	// double loop, 遍历所有的block
	/*
	余数分配示例：

		假设100行分3块，基础块33行，余数1行
		第0块：33 + 1 / 2 = 33行（整除向下取整）
		第1块：33行
		第2块：33 + 1 / 2 = 33行
		实际总和：33 + 33 + 33 = 99行（会有1行未分配，这是整除导致的精度损失）

		注意：如果余数是奇数，这种分配方式会丢失1个像素
	*/
	int32_t nRowBlockSize = 0, nColBlockSize = 0, nRowIndex = RealRoi.Up, nColIndex = RealRoi.Left;
	for (uint32_t nRowBlockIndex = 0; nRowBlockIndex < nRowBlockNum; nRowBlockIndex++)
	{
		// 余数分配策略：首尾block分别获得一半余数
		if (0 == nRowBlockIndex || nRowBlockNum - 1 == nRowBlockIndex)
		{
			nRowBlockSize = nRowBlockSizeBase + nRowMod / 2; // 第一块或最后一块
		}
		else
		{
			nRowBlockSize = nRowBlockSizeBase; // 中间块
		}
		nColIndex = RealRoi.Left; // 每行开始时重置列索引
		for (uint32_t nColBlockIndex = 0; nColBlockIndex < nColBlockNum; nColBlockIndex++)
		{
			// 使用同样的余数分配策略
			if (0 == nColBlockIndex || nColBlockNum - 1 == nColBlockIndex)
			{
				nColBlockSize = nColBlockSizeBase + nColMod / 2;
			}
			else
			{
				nColBlockSize = nColBlockSizeBase; // 中间块
			}

			// block size 有效性检测
			if (nRowBlockSize <= 0 || nColBlockSize <= 0)
			{
				std::string strErr = "SubFrameBlockMean: BlockSize error: RowBlockSize: " + std::to_string(nRowBlockSize) + ", ColBlockSize: " + std::to_string(nColBlockSize);
				WriteLog(strErr, nChannelIndex);
				bRes = false;
				return;
			}

			ROIArea temp = {
				nRowIndex, // Up: 起始行；
				nRowIndex + nRowBlockSize - 1, // Down: 结束行;
				nColIndex, // Left: 起始列；
				nColIndex + nColBlockSize - 1  // Right: 结束列
			};

			double dValue = 0; // 声明重复
			BlockData.m_RawData[nRowBlockIndex][nColBlockIndex] = 0;
			// 遍历当前block的每一行
			for (uint32_t nRows = temp.Up; nRows <= temp.Down; nRows++)
			{
				// 遍历当前block的每一列
				for (uint32_t nCols = temp.Left; nCols <= temp.Right; nCols++)
				{
					double dValue = 0; // 声明重复, 循环外层声明失效
					// 遍历多帧, 对同一像素求平均
					for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
					{
						// 累加从nIndexStart开始的nNumber中, 位置(nRows,nCols) 的像素值
						// 对于块内的某个像素位置 (nRows, nCols)，取所有帧（从 nIndexStart 到 nIndexStart + nNumber - 1）在该位置的像素值，求平均值。
						dValue += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows][nCols];
					}
					// 将多帧平均值(四舍五入后)累加到块数据中
					BlockData.m_RawData[nRowBlockIndex][nColBlockIndex] += round(dValue / nNumber);
				}
			}
			// 循环后, BlockData.m_RawData[nRowBlockIndex][nColBlockIndex]累加了该block内所有像素的多帧平均值
			// 除以block内的像素总数，得到该block的最终平均值
			BlockData.m_RawData[nRowBlockIndex][nColBlockIndex] /= (nRowBlockSize * nColBlockSize);
			nColIndex += nColBlockSize; // 移动到下一列block的起始位置
		}
		nRowIndex += nRowBlockSize; // 移动到下一行block的起始位置
	}

	/*
	计算流程
	假设：

		有3帧图像（帧0、帧1、帧2）
		ROI区域是4×4像素
		分成2×2块（每块2×2像素）

		第一个块（左上角）的计算：

		对块内第一个像素(0,0)：

		帧0的(0,0) = 100
		帧1的(0,0) = 102
		帧2的(0,0) = 98
		多帧平均 = round((100+102+98)/3) = round(100) = 100

		对块内所有4个像素重复上述过程，假设得到：

		(0,0): 100
		(0,1): 105
		(1,0): 95
		(1,1): 100

		块平均 = (100+105+95+100) / 4 = 100
	*/
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
	// 根据不同的Bayer模式和颜色通道，将Bayer图像中某个颜色通道的子帧坐标转换为完整图像的总体坐标。
	uint32_t nSubRows = SubLocal.x, nSubCols = SubLocal.y;
	uint32_t nTotalRows = 0, nTotalCols = 0;

	switch (m_PixelFormat)
	{
	case BayerGBRG:
		if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols << 1);
		}
		else if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols << 1) + 1;
		}
		else if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols << 1);
		}
		else if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols << 1) + 1;
		}
		break;
	case BayerBGGR:
		if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols << 1);
		}
		else if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols << 1) + 1;
		}
		else if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols << 1);
		}
		else if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols << 1) + 1;
		}
		break;
	case BayerRGGB:
		if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols << 1);
		}
		else if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols << 1) + 1;
		}
		else if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols << 1);
		}
		else if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols << 1) + 1;
		}
		break;
	case BayerGRBG:
		if (nChannelIndex == Gr)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols << 1);
		}
		else if (nChannelIndex == R)
		{
			nTotalRows = (nSubRows << 1);
			nTotalCols = (nSubCols << 1) + 1;
		}
		else if (nChannelIndex == B)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols << 1);
		}
		else if (nChannelIndex == Gb)
		{
			nTotalRows = (nSubRows << 1) + 1;
			nTotalCols = (nSubCols << 1) + 1;
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

void CAlpAPSMPAlgorithm::GetDataFromSubFrame(uint32_t nIndex, uint32_t nRows, uint32_t nCols, double& dValue)
{
	switch (m_PixelFormat)
	{
	case BayerGBRG:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		break;
	case BayerBGGR:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		break;
	case BayerRGGB:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		break;
	case BayerGRBG:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[nRows >> 1][nCols >> 1];
		}
		break;
	case QuadBayerGBRG:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		break;
	case QuadBayerBGGR:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		break;
	case QuadBayerRGGB:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		break;
	case QuadBayerGRBG:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[Gr][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[R][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE)][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			dValue = m_RawDataContainer[B][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE)];
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			dValue = m_RawDataContainer[Gb][nIndex].m_RawData[((nRows >> 1) & 0xFFFE) + 1][((nCols >> 1) & 0xFFFE) + 1];
		}
		break;
	}
}

void CAlpAPSMPAlgorithm::TotalLocalToSubFrameLocal(Local TotalLocal, SubFrameIndex& nChannelIndex, Local& SubLocal)
{
	uint32_t nRows = TotalLocal.x;
	uint32_t nCols = TotalLocal.y;
	uint32_t nSubRows = 0;
	uint32_t nSubCols = 0;

	switch (m_PixelFormat)
	{
	case BayerGBRG:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			nChannelIndex = SubFrameIndex::Gb;
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			nChannelIndex = SubFrameIndex::B;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			nChannelIndex = SubFrameIndex::R;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			nChannelIndex = SubFrameIndex::Gr;
		}
		nSubRows = nRows >> 1;
		nSubCols = nCols >> 1;
		break;
	case BayerBGGR:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			nChannelIndex = SubFrameIndex::B;
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			nChannelIndex = SubFrameIndex::Gb;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			nChannelIndex = SubFrameIndex::Gr;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			nChannelIndex = SubFrameIndex::R;
		}
		nSubRows = nRows >> 1;
		nSubCols = nCols >> 1;
		break;
	case BayerRGGB:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			nChannelIndex = SubFrameIndex::R;
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			nChannelIndex = SubFrameIndex::Gr;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			nChannelIndex = SubFrameIndex::Gb;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			nChannelIndex = SubFrameIndex::B;
		}
		nSubRows = nRows >> 1;
		nSubCols = nCols >> 1;
		break;
	case BayerGRBG:
		if ((nRows & 1) == 0 && (nCols & 1) == 0)
		{
			nChannelIndex = SubFrameIndex::Gr;
		}
		else if ((nRows & 1) == 0 && (nCols & 1) == 1)
		{
			nChannelIndex = SubFrameIndex::R;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 0)
		{
			nChannelIndex = SubFrameIndex::B;
		}
		else if ((nRows & 1) == 1 && (nCols & 1) == 1)
		{
			nChannelIndex = SubFrameIndex::Gb;
		}
		nSubRows = nRows >> 1;
		nSubCols = nCols >> 1;
		break;
	case QuadBayerGBRG:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		break;
	case QuadBayerBGGR:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		break;
	case QuadBayerRGGB:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		break;
	case QuadBayerGRBG:
		if ((nRows & 3) == 0 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 0 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::Gr;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 1 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::R;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 2 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE);
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 0)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 1)
		{
			nChannelIndex = SubFrameIndex::B;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 2)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE);
		}
		else if ((nRows & 3) == 3 && (nCols & 3) == 3)
		{
			nChannelIndex = SubFrameIndex::Gb;
			nSubRows = ((nRows >> 1) & 0xFFFE) + 1;
			nSubCols = ((nCols >> 1) & 0xFFFE) + 1;
		}
		break;
	}
	SubLocal.x = nSubRows;
	SubLocal.y = nSubCols;
}

void CAlpAPSMPAlgorithm::SubFrameReadNoise(uint32_t nIndex1, uint32_t nIndex2, ROIArea* ROI, SubFrameIndex nChannelIndex, APSReadNoiseType& ReadNoiseRes, bool& bRes, RawDataContainer& DataContainer)
{
	// 计算两帧暗场图像之间的差异来测量ReadNoise
	// init && ROI setting
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
	// 帧索引边界检测
	if (nIndex1 > DataContainer[nChannelIndex].size() || nIndex2 > DataContainer[nChannelIndex].size())
	{
		std::string strErr = "SubFrameReadNoise: Index error: nIndex1: " + std::to_string(nIndex1) + ", nIndex2: " + std::to_string(nIndex2);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	// ROI 边界检查
	if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SubFrameTNoise: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " + std::to_string(m_nChannelCol);
		WriteLog(strErr, nChannelIndex);
		bRes = false;
		return;
	}
	uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;
	std::vector<double> AllPixel(nRow * nCol * SubFrameIndex::All);

	// 逐个像素计算两帧图像的差值
	uint32_t nCur = 0;
	for (uint32_t nRows = 0; nRows < nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nCol; nCols++)
		{
			double value1 = m_RawDataContainer[nChannelIndex][nIndex1].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			double value2 = m_RawDataContainer[nChannelIndex][nIndex2].m_RawData[nRows + RealRoi.Up][nCols + RealRoi.Left];
			AllPixel[nCur++] = value1 - value2;
		}
	}
	// ReadNoise = σ_差值 / √2
	ReadNoiseRes = Std(AllPixel, nCur) / sqrt(2);
	// 原理：两帧独立图像的差值方差 = 2σ²(单帧);
	// 差值标准差 = √(2σ²) = √2*σ;
	// therefore, 单帧噪声 = 差值标准差 / √2;
}

void CAlpAPSMPAlgorithm::ImportDataTo16SubFrame(uint32_t nIndexStart, uint32_t nNumber)
{
	for (uint32_t nChannelIndex = 0; nChannelIndex < 16; nChannelIndex++)
	{
		if (m_16SubRawDataContainer[nChannelIndex].size() < nIndexStart + nNumber)
		{
			m_16SubRawDataContainer[nChannelIndex].resize(nIndexStart + nNumber);
		}
	}

	uint32_t nSubFrameRow = (m_nChannelRow >> 1);
	uint32_t nSubFrameCol = (m_nChannelCol >> 1);
	for (uint32_t i = 0; i < nNumber; i++)
	{
		for (uint32_t nChannelIndex = 0; nChannelIndex < 16; nChannelIndex++)
		{
			CAPSDataContainer& CurContainer = m_16SubRawDataContainer[nChannelIndex][nIndexStart + i];
			if (CurContainer.m_nRow != nSubFrameRow || CurContainer.m_nCol != nSubFrameCol)
			{
				CurContainer.Init(nSubFrameRow, nSubFrameCol);
			}
			for (uint32_t nRows = 0; nRows < nSubFrameRow; nRows++)
			{
				for (uint32_t nCols = 0; nCols < nSubFrameCol; nCols++)
				{
					double dValue = 0;
					uint32_t nChannelRow = nRows << 1;
					uint32_t nChannelCol = nCols << 1;

					switch (nChannelIndex & 0x03)
					{
					case 0:
						break;
					case 1:
						nChannelCol += 1;
						break;
					case 2:
						nChannelRow += 1;
						break;
					case 3:
						nChannelRow += 1;
						nChannelCol += 1;
						break;
					}
					CurContainer.m_RawData[nRows][nCols] = m_RawDataContainer[nChannelIndex >> 2][nIndexStart + i].m_RawData[nChannelRow][nChannelCol];
				}
			}
		}
	}
}
