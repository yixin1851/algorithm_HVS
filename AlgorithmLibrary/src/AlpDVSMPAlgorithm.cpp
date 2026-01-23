#include "AlpDVSMPAlgorithm.h"
#include <algorithm>
#include <thread>
#include <stack>
#include <fstream>
#include <windows.h>
#include "Changelist.h"

constexpr uint32_t DVS_MaxThreadNum = 8;

struct PeakAndHeight {
	uint32_t peak;
	uint32_t peekheight;
};

// 比较函数，用于std::sort
bool comparePeakAndHeight(const PeakAndHeight& a, const PeakAndHeight& b) {
	return a.peekheight < b.peekheight;
}

CAlpDVSMPAlgorithm::CAlpDVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
{
	m_nSiteNum = nSiteNum;
	m_bLogEnable = false;
	m_SensorType = Sensortype;
	m_bMultiThreadEnable = false;
	m_AlgorithmThre.dDeadPixelThre = 0.8;
	m_AlgorithmThre.dHotLineThre = 0.5;
	m_AlgorithmThre.dDeadLineThre = 0.5;
	m_AlgorithmThre.dHotPixelThre = 0.8;
	m_AlgorithmThre.nStationaryUniformityRowBlockNum = 5;
	m_AlgorithmThre.nStationaryUniformityColBlockNum = 5;
	m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum = 5;
	m_AlgorithmThre.nSpatialResponseUniformityColBlockNum = 5;
	//m_AlgorithmThre.nFindPeakNum = 3;
	//m_AlgorithmThre.dFindPeakThre = 0.75;
	m_nCode = code;
	m_AlgorithmThre.nPeakCycle = 20;
	m_AlgorithmThre.dFlashRatioThre = 0.15;
	m_AlgorithmThre.nHotPixelClusterSizeThre = 16;
	m_AlgorithmThre.nDeadPixelClusterSizeThre = 16;
	m_PixelFormat = Pixelformat;

	m_RawDataContainer.resize(SubFrameIndex::All);

	if (strLogDir != "")
	{
		std::time_t t = std::time(nullptr);
		std::tm now;
		localtime_s(&now, &t);
		char str_time[100] = { 0 };
		strftime(str_time, sizeof(str_time), "%Y_%m_%d_%H_%M_%S", &now);
		m_strLogFilePath = strLogDir + "\\" + str_time + "_Site" + std::to_string(m_nSiteNum) + "_DVS_Test.log";
	}
	else
	{
		m_strLogFilePath = "";
	}
}

CAlpDVSMPAlgorithm::~CAlpDVSMPAlgorithm()
{
}

bool CAlpDVSMPAlgorithm::EventsNumberCount(uint32_t nIndexStart, uint32_t nNumber, DVSEventsNumberCountType& EventsNumberCountRes)
{
	if (0 == nNumber || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "EventsNumberCount: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}
	EventsNumberCountRes.nDataNumber = 0;
	for (uint32_t i = 0; i <= SubFrameIndex::All; i++)
	{
		EventsNumberCountRes.NoEventsNum[i].resize(nNumber);
		EventsNumberCountRes.OnEventsNum[i].resize(nNumber);
		EventsNumberCountRes.OffEventsNum[i].resize(nNumber);
		EventsNumberCountRes.AllEventsNum[i].resize(nNumber);
	}


	if (m_bMultiThreadEnable)
	{
		std::thread* t[DVS_MaxThreadNum];

		uint32_t BlockNum = nNumber / DVS_MaxThreadNum;

		for (uint32_t nIndex = 0; nIndex < DVS_MaxThreadNum; nIndex++)
		{
			uint32_t nNumStart = 0;
			uint32_t nNumEnd = 0;

			nNumStart = nIndex * BlockNum;
			if (nIndex != DVS_MaxThreadNum - 1)
			{
				nNumEnd = (nIndex + 1) * BlockNum;
			}
			else
			{
				nNumEnd = nNumber;
			}

			t[nIndex] = new std::thread(&CAlpDVSMPAlgorithm::ThreadEventsNumberCount, this, nIndexStart, nNumStart, nNumEnd, std::ref(EventsNumberCountRes));
		}
		for (uint32_t nIndex = 0; nIndex < DVS_MaxThreadNum; nIndex++)
		{
			t[nIndex]->join();
			delete t[nIndex];
		}
	}
	else
	{
		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			m_RawDataContainer[nIndexStart + nIndex].CountEvents(m_ActiveArea);
			for (uint32_t i = 0; i <= SubFrameIndex::All; i++)
			{
				EventsNumberCountRes.NoEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_NoEventsNum[i];
				EventsNumberCountRes.OnEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_OnEventsNum[i];
				EventsNumberCountRes.OffEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_OffEventsNum[i];
				EventsNumberCountRes.AllEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_AllEventsNum[i];
			}
		}
	}
	EventsNumberCountRes.nDataNumber = nNumber;
	return true;
}

// bool CAlpDVSMPAlgorithm::EventsNumberCount(uint32_t nIndexStart, uint32_t nNumber, DVSEventsNumberCountType& EventsNumberCountRes)
// {
//     if (0 == nNumber || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
//     {
//         std::string strErr = "EventsNumberCount: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
//         WriteLog(strErr);
//         m_nErrCode = DATA_INDEX_ERROR;
//         return false;
//     }
//
//     EventsNumberCountRes.nDataNumber = 0;
//
//     // 预分配内存
//     for (uint32_t i = 0; i <= SubFrameIndex::All; i++)
//     {
//         EventsNumberCountRes.NoEventsNum[i].resize(nNumber);
//         EventsNumberCountRes.OnEventsNum[i].resize(nNumber);
//         EventsNumberCountRes.OffEventsNum[i].resize(nNumber);
//         EventsNumberCountRes.AllEventsNum[i].resize(nNumber);
//     }
//
//     if (m_bMultiThreadEnable)
//     {
//         // 使用 vector 管理线程，RAII 自动管理生命周期
//         std::vector<std::thread> threads;
//         threads.reserve(DVS_MaxThreadNum);
//
//         uint32_t BlockNum = nNumber / DVS_MaxThreadNum;
//
//         // 创建线程
//         for (uint32_t nIndex = 0; nIndex < DVS_MaxThreadNum; nIndex++)
//         {
//             uint32_t nNumStart = nIndex * BlockNum;
//             uint32_t nNumEnd = (nIndex != DVS_MaxThreadNum - 1) ? (nIndex + 1) * BlockNum : nNumber;
//
//             threads.emplace_back(&CAlpDVSMPAlgorithm::ThreadEventsNumberCount,
//                                 this,
//                                 nIndexStart,
//                                 nNumStart,
//                                 nNumEnd,
//                                 std::ref(EventsNumberCountRes));
//         }
//
//         // 等待所有线程完成
//         for (auto& t : threads)
//         {
//             if (t.joinable())
//             {
//                 t.join();
//             }
//         }
//         // threads 离开作用域时自动析构，无需手动释放
//     }
//     else
//     {
//         // 单线程处理
//         for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
//         {
//             m_RawDataContainer[nIndexStart + nIndex].CountEvents(m_ActiveArea);
//
//             for (uint32_t i = 0; i <= SubFrameIndex::All; i++)
//             {
//                 EventsNumberCountRes.NoEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_NoEventsNum[i];
//                 EventsNumberCountRes.OnEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_OnEventsNum[i];
//                 EventsNumberCountRes.OffEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_OffEventsNum[i];
//                 EventsNumberCountRes.AllEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_AllEventsNum[i];
//             }
//         }
//     }
//
//     EventsNumberCountRes.nDataNumber = nNumber;
//     return true;
// }

bool CAlpDVSMPAlgorithm::StationaryNoise(uint32_t nIndexStart, uint32_t nNumber, DVSStationaryNoiseType& StationaryNoiseRes)
{
    // 计算DVS传感器静态噪声特征
	if (0 == nNumber || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "StationaryNoise: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}

	DVSEventsNumberCountType EventsNumber;
	if (!EventsNumberCount(nIndexStart, nNumber, EventsNumber))
	{
		std::string strErr = "StationaryNoise: EventsNumberCount error";
		WriteLog(strErr);
		return false;
	}

	uint32_t nRowSize = m_ActiveArea.Down - m_ActiveArea.Up + 1;
	uint32_t nColSize = m_ActiveArea.Right - m_ActiveArea.Left + 1;
	uint32_t nAllSize = nRowSize * nColSize;

	StationaryNoiseRes.nFlashFrameNumber = 0;

    // 计算所有事件、ON事件、OFF事件的平均噪声率**百分比**
    // 公式: (平均事件数 / 像素总数) * 100%
	StationaryNoiseRes.dStationaryNoiseMeanAll = Mean(EventsNumber.AllEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;
	StationaryNoiseRes.dStationaryNoiseMeanOn = Mean(EventsNumber.OnEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;
	StationaryNoiseRes.dStationaryNoiseMeanOff = Mean(EventsNumber.OffEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;

	double dMaxValue;
	uint32_t nLocal;
	Max(dMaxValue, nLocal, EventsNumber.AllEventsNum[SubFrameIndex::All], nNumber);
	StationaryNoiseRes.dMaxStationaryNoise = dMaxValue / nAllSize * 100;

    // 计算标准差
	StationaryNoiseRes.dStationaryNoiseStdAll = Std(EventsNumber.AllEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;
	StationaryNoiseRes.dStationaryNoiseStdOn = Std(EventsNumber.OnEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;
	StationaryNoiseRes.dStationaryNoiseStdOff = Std(EventsNumber.OffEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;

    // Flash Frame Detect
	for (int i = 0; i < nNumber; i++)
	{
	    // 当某帧事件率超过阈值, 判为**Flash Frame**
		if (1.0 * EventsNumber.AllEventsNum[SubFrameIndex::All][i] / nAllSize > m_AlgorithmThre.dFlashRatioThre)
		{
			StationaryNoiseRes.nFlashFrameNumber++;
		}
	}

    // 找到每帧中噪声最大的行
	std::vector<uint32_t> MaxRowEvents(nNumber, 0);
	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
	{
		for (uint32_t nRows = m_ActiveArea.Up; nRows <= m_ActiveArea.Down; nRows++)
		{
			if (m_RawDataContainer[nIndexStart + nIndex].m_RowAllEventsNum[nRows] > MaxRowEvents[nIndex])
			{
				MaxRowEvents[nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_RowAllEventsNum[nRows];
			}
		}
	}
    // 计算噪声最大的行的平均噪声率
	StationaryNoiseRes.dStationaryRowTNoise = Mean(MaxRowEvents, nNumber) / nColSize * 100;

    // 找到每帧中噪声最大的行
	std::vector<uint32_t> MaxColEvents(nNumber, 0);
	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
	{
		for (uint32_t nCols = m_ActiveArea.Left; nCols <= m_ActiveArea.Right; nCols++)
		{
			if (m_RawDataContainer[nIndexStart + nIndex].m_ColAllEventsNum[nCols] > MaxColEvents[nIndex])
			{
				MaxColEvents[nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_ColAllEventsNum[nCols];
			}
		}
	}
    // 计算噪声最大的列的平均噪声率
	StationaryNoiseRes.dStationaryColTNoise = Mean(MaxColEvents, nNumber) / nRowSize * 100;

	double MaxValue = 0;
	uint32_t MaxLocal = 0;

	return true;
}

bool CAlpDVSMPAlgorithm::StationaryUniformity(uint32_t nIndexStart, uint32_t nNumber, DVSStationaryUniformityType& UniformityRes)
{
    // 计算DVS静态均匀性特征
    // 评估DVS传感器在静态场景下的均匀性表现, 通过将传感器活动区域分块, 统计每个块的事件触发率
	if (0 == nNumber || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "StationaryUniformity: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}

	uint32_t nRowSize = m_ActiveArea.Down - m_ActiveArea.Up + 1;
	uint32_t nColSize = m_ActiveArea.Right - m_ActiveArea.Left + 1;
	uint32_t nAllSize = nRowSize * nColSize;

	CAPSDataContainer UniformityBlockData;
	UniformityBlockData.Init(m_AlgorithmThre.nStationaryUniformityRowBlockNum, m_AlgorithmThre.nStationaryUniformityColBlockNum, true);
	UniformityRes.UniformityRatio = 0;

    // 整除计算每个Block包含的像素行数和列数, 余数部分被舍弃
	uint32_t nRowBlockSize = nRowSize / m_AlgorithmThre.nStationaryUniformityRowBlockNum;
	uint32_t nColBlockSize = nColSize / m_AlgorithmThre.nStationaryUniformityColBlockNum;

	for (uint32_t nRows = m_ActiveArea.Up; nRows < m_ActiveArea.Up + nRowBlockSize * m_AlgorithmThre.nStationaryUniformityRowBlockNum; nRows++)
	{
		for (uint32_t nCols = m_ActiveArea.Left; nCols < m_ActiveArea.Left + nColBlockSize * m_AlgorithmThre.nStationaryUniformityColBlockNum; nCols++)
		{
			double dValue = 0;
			for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
			{
			    // 遍历检测像素是否有事件触发, 有(!0)即对应Block的计数器+1
				if (0 != m_RawDataContainer[nIndexStart + nIndex].GetData(nRows, nCols))
				{
					++UniformityBlockData.m_RawData[(nRows - m_ActiveArea.Up) / nRowBlockSize][(nCols - m_ActiveArea.Left) / nColBlockSize];
				}
			}
		}
	}
    // 归一化处理
    // 除以(分块像素数*帧数), 得到平均触发率
	UniformityBlockData /= nRowBlockSize * nColBlockSize * nNumber;
    // *100转换为百分比
	UniformityBlockData *= 100;

	double dMeanValue = Mean(UniformityBlockData);
	double dMaxValue = 0, dMinValue = 0;
	Local temp;
	Max(dMaxValue, temp, UniformityBlockData);
	Min(dMinValue, temp, UniformityBlockData);
	//UniformityRes.UniformityRatio = (dMaxValue - dMinValue) / dMeanValue * 100;
    // 极差对异常数据很敏感, 标准差相对会更稳健
    //UniformityRes.UniformityRatio = Std(UniformityBlockData, nullptr);
    UniformityRes.UniformityRatio = dMaxValue - dMinValue; // 极差作为均匀性指标, 该值越小表示均匀性越好, 各Block事件分布越一致
    UniformityRes.UniformityBlockData.swap(UniformityBlockData.m_RawData);
	return true;
}

bool CAlpDVSMPAlgorithm::HotPixel(uint32_t nIndexStart, uint32_t nNumber, DVSHotpixelType& HotpixelRes)
{
	if (0 == nNumber || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "HotPixel: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}

	uint32_t nRowSize = m_ActiveArea.Down - m_ActiveArea.Up + 1;
	uint32_t nColSize = m_ActiveArea.Right - m_ActiveArea.Left + 1;
	uint32_t nAllSize = nRowSize * nColSize;

	std::vector<uint32_t> RowBadPixelNum(m_nTotalRow, 0);
	std::vector<uint32_t> ColBadPixelNum(m_nTotalCol, 0);
	HotpixelRes.HotPixelNum = 0;
	HotpixelRes.HotPixelMask.LocalData.clear();
	HotpixelRes.HotPixelMask.DiffData.clear();
	HotpixelRes.HotPixelMask.Flag.clear();
	HotpixelRes.HotPixelMask.BadPixelNum = 0;
	HotpixelRes.ClusterNum = 0;
	HotpixelRes.SingletNum = 0;
	HotpixelRes.CoupletNum = 0;
	HotpixelRes.TripletNum = 0;
	HotpixelRes.FourConnectedNum = 0;
	HotpixelRes.HotLineNum = 0;
    HotpixelRes.MaxClusterSize = 0;

	std::vector<std::vector<uint32_t>> BadPixelMask(m_nTotalRow);
	for (uint32_t i = 0; i < m_nTotalRow; i++)
	{
		BadPixelMask[i].resize(m_nTotalCol, 0);
	}

	for (uint32_t nRows = m_ActiveArea.Up; nRows <= m_ActiveArea.Down; nRows++)
	{
		for (uint32_t nCols = m_ActiveArea.Left; nCols <= m_ActiveArea.Right; nCols++)
		{
			double dRatio = 0;
			for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
			{
				if (0 != m_RawDataContainer[nIndexStart + nIndex].GetData(nRows, nCols))
				{
					++dRatio;
				}
			}
			dRatio /= nNumber;
			if (dRatio > m_AlgorithmThre.dHotPixelThre)
			{
				HotpixelRes.HotPixelMask.BadPixelNum++;
				HotpixelRes.HotPixelMask.LocalData.push_back({ nRows, nCols });
				HotpixelRes.HotPixelMask.Flag.push_back(DVS_HOT_PIXEL_FLAG);
				HotpixelRes.HotPixelNum++;
				++RowBadPixelNum[nRows];
				++ColBadPixelNum[nCols];
				BadPixelMask[nRows][nCols] = DVS_HOT_PIXEL_FLAG;
			}
		}
	}

	for (uint32_t nRows = m_ActiveArea.Up; nRows <= m_ActiveArea.Down; nRows++)
	{
		if (RowBadPixelNum[nRows] > nColSize * m_AlgorithmThre.dHotLineThre)
		{
			++HotpixelRes.HotLineNum;
		}
	}
	for (uint32_t nCols = m_ActiveArea.Left; nCols <= m_ActiveArea.Right; nCols++)
	{
		if (ColBadPixelNum[nCols] > nRowSize * m_AlgorithmThre.dHotLineThre)
		{
			++HotpixelRes.HotLineNum;
		}
	}

	uint32_t ConnectedAreaFlag = 0xFFFFFFFF;

	for (uint32_t nRows = m_ActiveArea.Up; nRows <= m_ActiveArea.Down; nRows++)
	{
		for (uint32_t nCols = m_ActiveArea.Left; nCols <= m_ActiveArea.Right; nCols++)
		{
			if (BadPixelMask[nRows][nCols] != 0 && BadPixelMask[nRows][nCols] < ConnectedAreaFlag)
			{
				uint32_t AreaSize = 0;
				std::stack<Local> Search;
				BadPixelMask[nRows][nCols] = ConnectedAreaFlag;
				Search.push({ nRows, nCols });
				while (!Search.empty())
				{
					Local temp = Search.top();
					Search.pop();
					AreaSize++;
					for (int nTempRows = (int)temp.x - 1; nTempRows <= (int)temp.x + 1; nTempRows++)
					{
						if (nTempRows >=0 && nTempRows < m_nTotalRow)
						{
							for (int nTempCols = (int)temp.y - 1; nTempCols <= (int)temp.y + 1; nTempCols++)
							{
								if (nTempCols >= 0 && nTempCols < m_nTotalCol && BadPixelMask[nTempRows][nTempCols] != 0 && BadPixelMask[nTempRows][nTempCols] < ConnectedAreaFlag)
								{
									BadPixelMask[nTempRows][nTempCols] = ConnectedAreaFlag;
									Search.push({ (uint32_t)nTempRows ,(uint32_t)nTempCols });
								}
							}
						}
					}
				}
				if (AreaSize == 1)
				{
					HotpixelRes.SingletNum++;
				}
				else if (AreaSize == 2)
				{
					HotpixelRes.CoupletNum++;
				}
				else if (AreaSize == 3)
				{
					HotpixelRes.TripletNum++;
				}
				else if (AreaSize == 4)
				{
					HotpixelRes.FourConnectedNum++;
				}
				else if (AreaSize >= m_AlgorithmThre.nHotPixelClusterSizeThre)
				{
					HotpixelRes.ClusterNum++;
				}
			    if (AreaSize > HotpixelRes.MaxClusterSize)
			    {
			        HotpixelRes.MaxClusterSize = AreaSize;
			    }
				ConnectedAreaFlag--;
			}
		}
	}

	return true;
}

bool CAlpDVSMPAlgorithm::FindPeak(uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, DVSPeakInfo& Peak, DVSLightTrigerType Light)
{
	if (0 == m_AlgorithmThre.nPeakCycle)
	{
		std::string strErr = "FindPeak: FindPeakNum error";
		WriteLog(strErr);
		m_nErrCode = FIND_PEAK_NUM_SET_ERROR;
		return false;
	}

	if (nNumber < m_AlgorithmThre.nPeakCycle || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "FindPeak: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}

	DVSEventsNumberCountType EventsNumberCountRes;
	EventsNumberCount(nIndexStart, nNumber, EventsNumberCountRes);

	Peak.nOffEventsPeakNumber = 0;
	Peak.OffEventsPeakPos.clear();
	Peak.OffEventsPeakPos.shrink_to_fit();
	Peak.nOnEventsPeakNumber = 0;
	Peak.OnEventsPeakPos.clear();
    Peak.OnEventsPeakPos.shrink_to_fit();

	uint32_t nCycle = m_AlgorithmThre.nPeakCycle;

	if (Light & DVSLightTrigerType::OffEventsOnly)
	{
		std::vector<uint32_t> peaks, left_edge, right_edge, peak_height, keep;
	    // 局部极大值检测
		local_maxima_1d(EventsNumberCountRes.OffEventsNum[SubFrameIndex::All], nNumber, peaks, left_edge, right_edge);
	    // 记录峰值高度
		for (int i = 0; i < peaks.size(); i++)
		{
			peak_height.push_back(EventsNumberCountRes.OffEventsNum[SubFrameIndex::All][peaks[i]]);
		}
	    // 峰值距离过滤, 解决峰值聚集问题, 保留聚类中的最高峰
	    // nDistance = nCycle /2: 确保同一个事件的多个检测被合并; 避免不同事件的峰值被误删
		select_by_peak_distance(peaks, peak_height, nCycle / 2, keep);

	    // 构建有效峰值列表
		std::vector<PeakAndHeight> peak_and_height;
		for (int i = 0; i < peaks.size(); i++)
		{
			if (keep[i] == 1)
			{
				peak_and_height.push_back({ peaks[i], peak_height[i] });
			}
		}

		if (peak_and_height.size() < nPeakNum)
		{
			std::string strErr = "FindPeak: Peak Number error: Find Peak Num: " + std::to_string(peak_and_height.size()) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}

	    // 按高度排序
		std::sort(peak_and_height.begin(), peak_and_height.end(), comparePeakAndHeight);

	    // 从高到低选取 nPeakNum 个峰值
		for (int i = peak_and_height.size() - 1; i >= 0; i--)
		{
			++Peak.nOffEventsPeakNumber;
		    // 将相对位置转换为全局索引 peak_and_height[i].peak + nIndexStart
			Peak.OffEventsPeakPos.push_back(nIndexStart + peak_and_height[i].peak);
			if (Peak.nOffEventsPeakNumber == nPeakNum)
			{
				break;
			}
		}
	}

    // ON 事件处理同理
	if (Light & DVSLightTrigerType::OnEventsOnly)
	{
		std::vector<uint32_t> peaks, left_edge, right_edge, peak_height, keep;
		local_maxima_1d(EventsNumberCountRes.OnEventsNum[SubFrameIndex::All], nNumber, peaks, left_edge, right_edge);
		for (int i = 0; i < peaks.size(); i++)
		{
			peak_height.push_back(EventsNumberCountRes.OnEventsNum[SubFrameIndex::All][peaks[i]]);
		}
		select_by_peak_distance(peaks, peak_height, nCycle / 2, keep);

		std::vector<PeakAndHeight> peak_and_height;
		for (int i = 0; i < peaks.size(); i++)
		{
			if (keep[i] == 1)
			{
				peak_and_height.push_back({ peaks[i], peak_height[i] });
			}
		}

		if (peak_and_height.size() < nPeakNum)
		{
			std::string strErr = "FindPeak: Peak Number error: Find Peak Num: " + std::to_string(peak_and_height.size()) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}

		std::sort(peak_and_height.begin(), peak_and_height.end(), comparePeakAndHeight);

		for (int i = peak_and_height.size() - 1; i >= 0; i--)
		{
			++Peak.nOnEventsPeakNumber;
			Peak.OnEventsPeakPos.push_back(nIndexStart + peak_and_height[i].peak);
			if (Peak.nOnEventsPeakNumber == nPeakNum)
			{
				break;
			}
		}

	}
	return true;
}

bool CAlpDVSMPAlgorithm::ImageContrastSensitivity(uint32_t nIndexStart, uint32_t nNumber, DVSPeakInfo* Peak, uint32_t nPeakNum, DVSLightTrigerType Light, DVSImageContrastSensitivityType& ImageContrastSensitivityRes)
{
    // 计算DVS传感器在特定光照变化下的对比度灵敏度, 通过分析事件峰值来评估传感器响应特性
	if (nNumber < m_AlgorithmThre.nPeakCycle || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "ImageContrastSensitivity: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}
    // 峰值检测
	DVSPeakInfo tempPeak;
	if (Peak == nullptr)
	{
		if (FindPeak(nIndexStart, nNumber, nPeakNum, tempPeak, Light))
		{
			Peak = &tempPeak;
		}
		else
		{
			std::string strErr = "ImageContrastSensitivity: Find Peak error";
			WriteLog(strErr);
			m_nErrCode = FIND_PEAK_ERROR;
			return false;
		}
	}

	uint32_t nRowSize = m_ActiveArea.Down - m_ActiveArea.Up + 1;
	uint32_t nColSize = m_ActiveArea.Right - m_ActiveArea.Left + 1;
	uint32_t nAllSize = nRowSize * nColSize;

	if (Light & DVSLightTrigerType::OffEventsOnly)
	{
		if (Peak->nOffEventsPeakNumber < nPeakNum || 0 == nPeakNum)
		{
			std::string strErr = "ImageContrastSensitivity: Peak Number error: Find Peak Num: " + std::to_string(Peak->nOffEventsPeakNumber) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}

	    // 遍历所有通道
		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
			ImageContrastSensitivityRes.OffEventsRatio[nChannel] = 0;

		    // 对指定数量的最新峰值求和平均
			for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
			{
			    // 从最新的峰值开始向前取nPeakNum个峰值, 确保分析最近的数据. Peak -> OffEventsPeakPos[Peak -> nOffEventsPeakNumber - 1 - nIndex]
				ImageContrastSensitivityRes.OffEventsRatio[nChannel] += m_RawDataContainer[Peak->OffEventsPeakPos[Peak->nOffEventsPeakNumber - 1 - nIndex]].m_OffEventsNum[nChannel];
			}
		    // (∑峰值事件数 / 峰值数量)
			ImageContrastSensitivityRes.OffEventsRatio[nChannel] /= nPeakNum;

            // **ImageContrastSensitivityRes.OffEventsRatio[nChannel]**表示在亮度增加时, 该通道有百分之多少的像素产生了事件响应, 数值越高说明该通道对亮度上升越敏感
            // 归一化为百分比, 除以像素总数*100
            if (nChannel == SubFrameIndex::All)
			{
			    // All通道: (∑峰值事件数 / 峰值数量) / 总像素数量 * 100
				ImageContrastSensitivityRes.OffEventsRatio[nChannel] /= nAllSize;
				ImageContrastSensitivityRes.OffEventsRatio[nChannel] *= 100;
			}
			else
			{
			    // 单通道: (∑峰值事件数 / 峰值数量) / (总像素数量 / 4) * 100
				ImageContrastSensitivityRes.OffEventsRatio[nChannel] /= nAllSize / 4;
				ImageContrastSensitivityRes.OffEventsRatio[nChannel] *= 100;
			}
		}

		if (ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::Gb] != 0)
		{
		    // 以Gb通道为基准, 计算其他通道的相对响应
		    // R 通道相对于 Gb 通道的 Off 事件响应比率, 理想状态下应接近100%
			ImageContrastSensitivityRes.R_Gb_OffEventsRatio = 100 * ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::R] / ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::Gb];
		    // B 通道相对于 Gb 通道的 Off 事件响应比率, 蓝色光子能量高但在自然光中占比少, 该比率可能天然偏低
			ImageContrastSensitivityRes.B_Gb_OffEventsRatio = 100 * ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::B] / ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::Gb];
		    // Gr 通道相对于 Gb 通道的 Off 事件响应比率, 理想状态下应接近100%
			ImageContrastSensitivityRes.Gr_Gb_OffEventsRatio = 100 * ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::Gr] / ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::Gb];
		}
		else
		{
			std::string strErr = "ImageContrastSensitivity: Off events number is zero";
			WriteLog(strErr);
			m_nErrCode = EVENTS_EQU_ZERO;
			return false;
		}
	}

    // On 事件处理同理
	if (Light & DVSLightTrigerType::OnEventsOnly)
	{
		if (Peak->nOnEventsPeakNumber < nPeakNum || 0 == nPeakNum)
		{
			std::string strErr = "ImageContrastSensitivity: Peak Number error: Find Peak Num: " + std::to_string(Peak->nOnEventsPeakNumber) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}

		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
			ImageContrastSensitivityRes.OnEventsRatio[nChannel] = 0;

			for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
			{
				ImageContrastSensitivityRes.OnEventsRatio[nChannel] += m_RawDataContainer[Peak->OnEventsPeakPos[Peak->nOnEventsPeakNumber - 1 - nIndex]].m_OnEventsNum[nChannel];
			}

			ImageContrastSensitivityRes.OnEventsRatio[nChannel] /= nPeakNum;
			if (nChannel == SubFrameIndex::All)
			{
				ImageContrastSensitivityRes.OnEventsRatio[nChannel] /= nAllSize;
				ImageContrastSensitivityRes.OnEventsRatio[nChannel] *= 100;
			}
			else
			{
				ImageContrastSensitivityRes.OnEventsRatio[nChannel] /= nAllSize / 4;
				ImageContrastSensitivityRes.OnEventsRatio[nChannel] *= 100;
			}
		}

		if (ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::Gb] != 0)
		{
			ImageContrastSensitivityRes.R_Gb_OnEventsRatio = 100 * ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::R] / ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::Gb];
			ImageContrastSensitivityRes.B_Gb_OnEventsRatio = 100 * ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::B] / ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::Gb];
			ImageContrastSensitivityRes.Gr_Gb_OnEventsRatio = 100 * ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::Gr] / ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::Gb];
		}
		else
		{
			std::string strErr = "ImageContrastSensitivity: On events number is zero";
			WriteLog(strErr);
			m_nErrCode = EVENTS_EQU_ZERO;
			return false;
		}
	}

	return true;
}

bool CAlpDVSMPAlgorithm::AccompaniedPeakAndDelayedPeak(uint32_t nIndexStart, uint32_t nNumber, DVSPeakInfo* Peak, uint32_t nPeakNum, DVSLightTrigerType Light, DVSAccompaniedPeakAndDelayedPeakType& AccompaniedPeakAndDelayedPeakRes)
{
    // 分析DVS事件数据中峰值位置的 **Accompanied Peak** 和 **Delayed Peak** 的特征, 计算峰值后一帧中 On/Off 事件相对于峰值事件的比率
	if (nNumber < m_AlgorithmThre.nPeakCycle || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "ImageContrastSensitivity: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}
    // 峰值寻找
	DVSPeakInfo tempPeak;
	if (Peak == nullptr)
	{
		if (FindPeak(nIndexStart, nNumber, nPeakNum, tempPeak, Light))
		{
			Peak = &tempPeak;
		}
		else
		{
			std::string strErr = "AccompaniedPeakAndDelayedPeak: Find Peak error";
			WriteLog(strErr);
			m_nErrCode = FIND_PEAK_ERROR;
			return false;
		}
	}
	if (Light & DVSLightTrigerType::OffEventsOnly)
	{
	    // 峰值数量验证
		if (Peak->nOffEventsPeakNumber < nPeakNum || 0 == nPeakNum)
		{
			std::string strErr = "AccompaniedPeakAndDelayedPeak: Peak Number error: Find Peak Num: " + std::to_string(Peak->nOffEventsPeakNumber) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}

	    // 起始峰值位置确定
		uint32_t nPeakStartNum = 0;
		if (Peak->OffEventsPeakPos[Peak->nOffEventsPeakNumber - 1] == nIndexStart + nNumber - 1)
		{
		    // 最后一个峰在边界上, 向前一个
			nPeakStartNum = Peak->nOffEventsPeakNumber - 2;
		}
		else
		{
		    // 从最后一个峰开始
			nPeakStartNum = Peak->nOffEventsPeakNumber - 1;
		}

		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
			double OffEventsPeakNumber = 0;
			double NextOffEventsPeakNumber = 0;
			double NextOnEventsPeakNumber = 0;
			for (uint32_t nIndex = 0; nIndex < nPeakNum && nPeakStartNum >= nIndex; nIndex++)
			{
			    // 累加最近nPeakNum个峰处的OFF事件数
				OffEventsPeakNumber += m_RawDataContainer[Peak->OffEventsPeakPos[nPeakStartNum - nIndex]].m_OffEventsNum[nChannel];
			    // 累加这些峰后一帧的OFF和ON事件数
				NextOffEventsPeakNumber += m_RawDataContainer[Peak->OffEventsPeakPos[nPeakStartNum - nIndex] + 1].m_OffEventsNum[nChannel];
				NextOnEventsPeakNumber += m_RawDataContainer[Peak->OffEventsPeakPos[nPeakStartNum - nIndex] + 1].m_OnEventsNum[nChannel];
			}

			if (OffEventsPeakNumber != 0)
			{
			    // AccompaniedPeakOffEventsRatio = (峰值后ON事件数 / 峰值OFF事件数) * 100%
				AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOffEventsRatio[nChannel] = NextOnEventsPeakNumber / OffEventsPeakNumber * 100;
			    // DelayedPeakOffEventsRatio = (峰值后OFF事件数 / 峰值OFF事件数) * 100%
				AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOffEventsRatio[nChannel] = NextOffEventsPeakNumber / OffEventsPeakNumber * 100;
			}
			else
			{
				std::string strErr = "AccompaniedPeakAndDelayedPeakRes: Off events number is zero";
				WriteLog(strErr);
				m_nErrCode = EVENTS_EQU_ZERO;
				return false;
			}
		}
	}

	if (Light & DVSLightTrigerType::OnEventsOnly)
	{
		if (Peak->nOnEventsPeakNumber < nPeakNum || 0 == nPeakNum)
		{
			std::string strErr = "AccompaniedPeakAndDelayedPeak: Peak Number error: Find Peak Num: " + std::to_string(Peak->nOnEventsPeakNumber) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}

		uint32_t nPeakStartNum = 0;

		if (Peak->OnEventsPeakPos[Peak->nOnEventsPeakNumber - 1] == nIndexStart + nNumber - 1)
		{
			nPeakStartNum = Peak->nOnEventsPeakNumber - 2;
		}
		else
		{
			nPeakStartNum = Peak->nOnEventsPeakNumber - 1;
		}

		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
			double OnEventsPeakNumber = 0;
			double NextOffEventsPeakNumber = 0;
			double NextOnEventsPeakNumber = 0;
			for (uint32_t nIndex = 0; nIndex < nPeakNum && nPeakStartNum >= nIndex; nIndex++)
			{
				OnEventsPeakNumber += m_RawDataContainer[Peak->OnEventsPeakPos[nPeakStartNum - nIndex]].m_OnEventsNum[nChannel];
				NextOffEventsPeakNumber += m_RawDataContainer[Peak->OnEventsPeakPos[nPeakStartNum - nIndex] + 1].m_OffEventsNum[nChannel];
				NextOnEventsPeakNumber += m_RawDataContainer[Peak->OnEventsPeakPos[nPeakStartNum - nIndex] + 1].m_OnEventsNum[nChannel];
			}
			if (OnEventsPeakNumber != 0)
			{
				AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOnEventsRatio[nChannel] = NextOffEventsPeakNumber / OnEventsPeakNumber * 100;
				AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOnEventsRatio[nChannel] = NextOnEventsPeakNumber / OnEventsPeakNumber * 100;
			}
			else
			{
				std::string strErr = "AccompaniedPeakAndDelayedPeakRes: On events number is zero";
				WriteLog(strErr);
				m_nErrCode = EVENTS_EQU_ZERO;
				return false;
			}
		}
	}
	return true;
}

bool CAlpDVSMPAlgorithm::SpatialResponseUniformity(uint32_t nIndexStart, uint32_t nNumber, ROIArea* ROI, DVSPeakInfo* Peak, uint32_t nPeakNum, DVSLightTrigerType Light, DVSSpatialResponseUniformityType& SpatialResponseUniformityRes)
{
    // 分析DVS传感器在空间上的响应均匀性, 分别处理 ON Events 和 OFF Events
	if (nNumber < m_AlgorithmThre.nPeakCycle || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "SpatialResponseUniformity: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}
    // 如果未提供峰值信息, 查找峰值
	DVSPeakInfo tempPeak;
	if (Peak == nullptr)
	{
		if (FindPeak(nIndexStart, nNumber, nPeakNum, tempPeak, Light))
		{
			Peak = &tempPeak;
		}
		else
		{
			std::string strErr = "SpatialResponseUniformity: Find Peak error";
			WriteLog(strErr);
			m_nErrCode = FIND_PEAK_ERROR;
			return false;
		}
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

	if (RealRoi.Down >= m_nTotalRow || RealRoi.Right >= m_nTotalCol || RealRoi.Down < RealRoi.Up || RealRoi.Right < RealRoi.Left)
	{
		std::string strErr = "SpatialResponseUniformity: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " + std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " + std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nTotalRow) + ", Col: " + std::to_string(m_nTotalCol);
		WriteLog(strErr);
		m_nErrCode = DATA_ROI_SET_ERROR;
		return false;
	}

	uint32_t nRowSize = RealRoi.Down - RealRoi.Up + 1;
	uint32_t nColSize = RealRoi.Right - RealRoi.Left + 1;
	uint32_t nAllSize = nRowSize * nColSize;

	if (Light & DVSLightTrigerType::OffEventsOnly)
	{
		if (Peak->nOffEventsPeakNumber < nPeakNum || 0 == nPeakNum)
		{
			std::string strErr = "SpatialResponseUniformity: Peak Number error: Find Peak Num: " + std::to_string(Peak->nOffEventsPeakNumber) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}
	    // 初始化容器
		CAPSDataContainer OffEventsUniformityBlockData[SubFrameIndex::All + 1];
		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
			OffEventsUniformityBlockData[nChannel].Init(m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum, m_AlgorithmThre.nSpatialResponseUniformityColBlockNum, true);
			SpatialResponseUniformityRes.dOffEventsUniformityRatio[nChannel] = 0;
		}

	    // Calc BlockSize
		uint32_t nRowBlockSize = nRowSize / m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum;
		uint32_t nColBlockSize = nColSize / m_AlgorithmThre.nSpatialResponseUniformityColBlockNum;

	    // 遍历ROI, 按Block对齐
		for (uint32_t nRows = RealRoi.Up; nRows < RealRoi.Up + nRowBlockSize * m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum; nRows++)
		{
			for (uint32_t nCols = RealRoi.Left; nCols < RealRoi.Left + nColBlockSize * m_AlgorithmThre.nSpatialResponseUniformityColBlockNum; nCols++)
			{
				double dValue = 0;
				SubFrameIndex nChannel = All;

				GetChannel(nRows, nCols, nChannel);

			    // 统计该像素在最近 nPeakNum 个峰值中的响应此处
				for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
				{
				    // m_RawDataContainer[PeakPos]
					if (0 != m_RawDataContainer[Peak->OffEventsPeakPos[Peak->nOffEventsPeakNumber - 1 - nIndex]].GetData(nRows, nCols))
					{
						++dValue;
					}
				}
			    // 累加到对应Block中
				OffEventsUniformityBlockData[SubFrameIndex::All].m_RawData[(nRows - RealRoi.Up) / nRowBlockSize][(nCols - RealRoi.Left) / nColBlockSize] += dValue / nPeakNum;
				OffEventsUniformityBlockData[nChannel].m_RawData[(nRows - RealRoi.Up) / nRowBlockSize][(nCols - RealRoi.Left) / nColBlockSize] += dValue / nPeakNum;
			}
		}

		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
		    // 归一化并转换为百分比
			OffEventsUniformityBlockData[nChannel] /= nRowBlockSize * nColBlockSize;
			OffEventsUniformityBlockData[nChannel] *= 100;

			double dMeanValue = Mean(OffEventsUniformityBlockData[nChannel]);
			double dMaxValue = 0, dMinValue = 0;
			Local temp;
			Max(dMaxValue, temp, OffEventsUniformityBlockData[nChannel]);
			Min(dMinValue, temp, OffEventsUniformityBlockData[nChannel]);
			if (dMeanValue != 0)
			{
			    // 计算均匀性指标(百分比), 均匀性比率 = (Block最大响应率 - Block最小响应率) / 平均响应率 * 100
				SpatialResponseUniformityRes.dOffEventsUniformityRatio[nChannel] = (dMaxValue - dMinValue) / dMeanValue * 100;
			}
			else
			{
				std::string strErr = "SpatialResponseUniformity: Off events number is zero";
				WriteLog(strErr);
				m_nErrCode = EVENTS_EQU_ZERO;
				return false;
			}

			//SpatialResponseUniformityRes.dOffEventsUniformityRatio[nChannel] = (dMaxValue - dMinValue);
			SpatialResponseUniformityRes.OffEventsUniformityBlockData[nChannel].swap(OffEventsUniformityBlockData[nChannel].m_RawData);
		}
	}

	if (Light & DVSLightTrigerType::OnEventsOnly)
	{
		if (Peak->nOnEventsPeakNumber < nPeakNum || 0 == nPeakNum)
		{
			std::string strErr = "AccompaniedPeakAndDelayedPeak: Peak Number error: Find Peak Num: " + std::to_string(Peak->nOnEventsPeakNumber) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}
		CAPSDataContainer OnEventsUniformityBlockData[SubFrameIndex::All + 1];
		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
			OnEventsUniformityBlockData[nChannel].Init(m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum, m_AlgorithmThre.nSpatialResponseUniformityColBlockNum, true);
			SpatialResponseUniformityRes.dOnEventsUniformityRatio[nChannel] = 0;
		}

		uint32_t nRowBlockSize = nRowSize / m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum;
		uint32_t nColBlockSize = nColSize / m_AlgorithmThre.nSpatialResponseUniformityColBlockNum;

		for (uint32_t nRows = RealRoi.Up; nRows < RealRoi.Up + nRowBlockSize * m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum; nRows++)
		{
			for (uint32_t nCols = RealRoi.Left; nCols < RealRoi.Left + nColBlockSize * m_AlgorithmThre.nSpatialResponseUniformityColBlockNum; nCols++)
			{
				double dValue = 0;
				for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
				{
					if (0 != m_RawDataContainer[Peak->OnEventsPeakPos[Peak->nOnEventsPeakNumber - 1 - nIndex]].GetData(nRows, nCols))
					{
						++dValue;
					}
				}
				SubFrameIndex nChannel = All;
				GetChannel(nRows, nCols, nChannel);

				OnEventsUniformityBlockData[SubFrameIndex::All].m_RawData[(nRows - RealRoi.Up) / nRowBlockSize][(nCols - RealRoi.Left) / nColBlockSize] += dValue / nPeakNum;
				OnEventsUniformityBlockData[nChannel].m_RawData[(nRows - RealRoi.Up) / nRowBlockSize][(nCols - RealRoi.Left) / nColBlockSize] += dValue / nPeakNum;
			}
		}

		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
			OnEventsUniformityBlockData[nChannel] /= nRowBlockSize * nColBlockSize;
			OnEventsUniformityBlockData[nChannel] *= 100;

			double dMeanValue = Mean(OnEventsUniformityBlockData[nChannel]);
			double dMaxValue = 0, dMinValue = 0;
			Local temp;
			Max(dMaxValue, temp, OnEventsUniformityBlockData[nChannel]);
			Min(dMinValue, temp, OnEventsUniformityBlockData[nChannel]);
			if (dMeanValue != 0)
			{
				SpatialResponseUniformityRes.dOnEventsUniformityRatio[nChannel] = (dMaxValue - dMinValue) / dMeanValue * 100;
			}
			else
			{
				std::string strErr = "SpatialResponseUniformity: On events number is zero";
				WriteLog(strErr);
				m_nErrCode = EVENTS_EQU_ZERO;
				return false;
			}

			//SpatialResponseUniformityRes.dOnEventsUniformityRatio[nChannel] = dMaxValue - dMinValue;
			SpatialResponseUniformityRes.OnEventsUniformityBlockData[nChannel].swap(OnEventsUniformityBlockData[nChannel].m_RawData);
		}
	}
	return true;
}

bool CAlpDVSMPAlgorithm::BadPixel(uint32_t nIndexStart, uint32_t nNumber, DVSPeakInfo* Peak, uint32_t nPeakNum, DVSLightTrigerType Light, DVSBadpixelType& BadpixelRes)
{
	if (nNumber < m_AlgorithmThre.nPeakCycle || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
        std::string strErr = "BadPixel: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " +
                             std::to_string(nNumber) + "\n";;
        strErr += "nNumber: " + std::to_string(nNumber) + ", m_AlgorithmThre.nPeakCycle: " + std::to_string(
            m_AlgorithmThre.nPeakCycle) + "\n";
        strErr += "nIndexStart: " + std::to_string(nIndexStart) + ", m_RawDataContainer.size():" + std::to_string(
            m_RawDataContainer.size()) + "\n";
        strErr += "(nIndexStart + nNumber): " + std::to_string(nIndexStart + nNumber) + ", m_RawDataContainer.size(): "
                + std::to_string(m_RawDataContainer.size()) + "\n";
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}
	DVSPeakInfo tempPeak;
	if (Peak == nullptr)
	{
		if (FindPeak(nIndexStart, nNumber, nPeakNum, tempPeak, Light))
		{
			Peak = &tempPeak;
		}
		else
		{
			std::string strErr = "BadPixel: Find Peak error";
			WriteLog(strErr);
			m_nErrCode = FIND_PEAK_ERROR;
			return false;
		}
	}

	uint32_t nRowSize = m_ActiveArea.Down - m_ActiveArea.Up + 1;
	uint32_t nColSize = m_ActiveArea.Right - m_ActiveArea.Left + 1;
	uint32_t nAllSize = nRowSize * nColSize;

	if (Light & DVSLightTrigerType::OffEventsOnly)
	{
		if (Peak->nOffEventsPeakNumber < nPeakNum || 0 == nPeakNum)
		{
			std::string strErr = "BadPixel: Peak Number error: Find Peak Num: " + std::to_string(Peak->nOffEventsPeakNumber) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}
		std::vector<uint32_t> RowBadPixelNum(m_nTotalRow, 0);
		std::vector<uint32_t> ColBadPixelNum(m_nTotalCol, 0);

		BadpixelRes.OffEventsBadPixelMask.BadPixelNum = 0;
		BadpixelRes.OffEventsBadPixelMask.Flag.clear();
		BadpixelRes.OffEventsBadPixelMask.LocalData.clear();
		BadpixelRes.OffEventsBadPixelMask.DiffData.clear();
		BadpixelRes.nOffEventsClusterNum = 0;
		BadpixelRes.nOffEventsDeadPixelNum = 0;
		BadpixelRes.nOffEventsDeadLineNum = 0;
	    BadpixelRes.nOffEventsMaxClusterSize = 0;

		std::vector<std::vector<uint32_t>> BadPixelMask(m_nTotalRow);
		for (uint32_t i = 0; i < m_nTotalRow; i++)
		{
			BadPixelMask[i].resize(m_nTotalCol, 0);
		}

		for (uint32_t nRows = m_ActiveArea.Up; nRows <= m_ActiveArea.Down; nRows++)
		{
			for (uint32_t nCols = m_ActiveArea.Left; nCols <= m_ActiveArea.Right; nCols++)
			{
				double dDeadPixelRatio = 0;

				for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
				{
					uint32_t nValue = m_RawDataContainer[Peak->OffEventsPeakPos[Peak->nOffEventsPeakNumber - 1 - nIndex]].GetData(nRows, nCols);
					if (0 == nValue)
					{
						++dDeadPixelRatio;
					}
				}
				dDeadPixelRatio /= nPeakNum;

				if (dDeadPixelRatio > m_AlgorithmThre.dDeadPixelThre)
				{
					BadpixelRes.OffEventsBadPixelMask.BadPixelNum++;
					BadpixelRes.OffEventsBadPixelMask.LocalData.push_back({ nRows, nCols });
					BadpixelRes.OffEventsBadPixelMask.Flag.push_back(DVS_DEAD_PIXEL_FLAG);
					BadpixelRes.nOffEventsDeadPixelNum++;
					++RowBadPixelNum[nRows];
					++ColBadPixelNum[nCols];
					BadPixelMask[nRows][nCols] = DVS_DEAD_PIXEL_FLAG;
				}
			}
		}

		for (uint32_t nRows = m_ActiveArea.Up; nRows <= m_ActiveArea.Down; nRows++)
		{
			if (RowBadPixelNum[nRows] > nColSize * m_AlgorithmThre.dDeadLineThre)
			{
				++BadpixelRes.nOffEventsDeadLineNum;
			}
		}
		for (uint32_t nCols = m_ActiveArea.Left; nCols <= m_ActiveArea.Right; nCols++)
		{
			if (ColBadPixelNum[nCols] > nRowSize * m_AlgorithmThre.dDeadLineThre)
			{
				++BadpixelRes.nOffEventsDeadLineNum;
			}
		}

		uint32_t ConnectedAreaFlag = 0xFFFFFFFF;

		for (uint32_t nRows = m_ActiveArea.Up; nRows <= m_ActiveArea.Down; nRows++)
		{
			for (uint32_t nCols = m_ActiveArea.Left; nCols <= m_ActiveArea.Right; nCols++)
			{
				if (BadPixelMask[nRows][nCols] != 0 && BadPixelMask[nRows][nCols] < ConnectedAreaFlag)
				{
					uint32_t AreaSize = 0;
					std::stack<Local> Search;
					BadPixelMask[nRows][nCols] = ConnectedAreaFlag;
					Search.push({ nRows, nCols });
					while (!Search.empty())
					{
						Local temp = Search.top();
						Search.pop();
						AreaSize++;
						for (int nTempRows = (int)temp.x - 1; nTempRows <= (int)temp.x + 1; nTempRows++)
						{
							if (nTempRows >= 0 && nTempRows < m_nTotalRow)
							{
								for (int nTempCols = (int)temp.y - 1; nTempCols <= (int)temp.y + 1; nTempCols++)
								{
									if (nTempCols < m_nTotalCol && BadPixelMask[nTempRows][nTempCols] != 0 && BadPixelMask[nTempRows][nTempCols] < ConnectedAreaFlag)
									{
										BadPixelMask[nTempRows][nTempCols] = ConnectedAreaFlag;
										Search.push({ (uint32_t)nTempRows , (uint32_t)nTempCols });
									}
								}
							}
						}
					}
					if (AreaSize >= m_AlgorithmThre.nDeadPixelClusterSizeThre)
					{
						BadpixelRes.nOffEventsClusterNum++;
					}
				    if (AreaSize > BadpixelRes.nOffEventsMaxClusterSize)
				    {
				        BadpixelRes.nOffEventsMaxClusterSize = AreaSize;
				    }
					ConnectedAreaFlag--;
				}
			}
		}
	}

	if (Light & DVSLightTrigerType::OnEventsOnly)
	{
		if (Peak->nOnEventsPeakNumber < nPeakNum || 0 == nPeakNum)
		{
			std::string strErr = "BadPixel: Peak Number error: Find Peak Num: " + std::to_string(Peak->nOnEventsPeakNumber) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}
		std::vector<uint32_t> RowBadPixelNum(m_nTotalRow, 0);
		std::vector<uint32_t> ColBadPixelNum(m_nTotalCol, 0);

		BadpixelRes.OnEventsBadPixelMask.BadPixelNum = 0;
		BadpixelRes.OnEventsBadPixelMask.Flag.clear();
		BadpixelRes.OnEventsBadPixelMask.LocalData.clear();
		BadpixelRes.nOnEventsClusterNum = 0;
		BadpixelRes.nOnEventsDeadPixelNum = 0;
		BadpixelRes.nOnEventsDeadLineNum = 0;
	    BadpixelRes.nOnEventsMaxClusterSize = 0;

		std::vector<std::vector<uint32_t>> BadPixelMask(m_nTotalRow);
		for (uint32_t i = 0; i < m_nTotalRow; i++)
		{
			BadPixelMask[i].resize(m_nTotalCol, 0);
		}

		for (uint32_t nRows = m_ActiveArea.Up; nRows <= m_ActiveArea.Down; nRows++)
		{
			for (uint32_t nCols = m_ActiveArea.Left; nCols <= m_ActiveArea.Right; nCols++)
			{
				double dDeadPixelRatio = 0;

				for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
				{
					uint32_t nValue = m_RawDataContainer[Peak->OnEventsPeakPos[Peak->nOnEventsPeakNumber - 1 - nIndex]].GetData(nRows, nCols);
					if (0 == nValue)
					{
						++dDeadPixelRatio;
					}
				}
				dDeadPixelRatio /= nPeakNum;

				if (dDeadPixelRatio > m_AlgorithmThre.dDeadPixelThre)
				{
					BadpixelRes.OnEventsBadPixelMask.BadPixelNum++;
					BadpixelRes.OnEventsBadPixelMask.LocalData.push_back({ nRows, nCols });
					BadpixelRes.OnEventsBadPixelMask.Flag.push_back(DVS_DEAD_PIXEL_FLAG);
					BadpixelRes.nOnEventsDeadPixelNum++;
					++RowBadPixelNum[nRows];
					++ColBadPixelNum[nCols];
					BadPixelMask[nRows][nCols] = DVS_DEAD_PIXEL_FLAG;
				}
			}
		}

		for (uint32_t nRows = m_ActiveArea.Up; nRows <= m_ActiveArea.Down; nRows++)
		{
			if (RowBadPixelNum[nRows] > nColSize * m_AlgorithmThre.dDeadLineThre)
			{
				++BadpixelRes.nOnEventsDeadLineNum;
			}
		}
		for (uint32_t nCols = m_ActiveArea.Left; nCols <= m_ActiveArea.Right; nCols++)
		{
			if (ColBadPixelNum[nCols] > nRowSize * m_AlgorithmThre.dDeadLineThre)
			{
				++BadpixelRes.nOnEventsDeadLineNum;
			}
		}

		uint32_t ConnectedAreaFlag = 0xFFFFFFFF;

		for (uint32_t nRows = m_ActiveArea.Up; nRows <= m_ActiveArea.Down; nRows++)
		{
			for (uint32_t nCols = m_ActiveArea.Left; nCols <= m_ActiveArea.Right; nCols++)
			{
				if (BadPixelMask[nRows][nCols] != 0 && BadPixelMask[nRows][nCols] < ConnectedAreaFlag)
				{
					uint32_t AreaSize = 0;
					std::stack<Local> Search;
					BadPixelMask[nRows][nCols] = ConnectedAreaFlag;
					Search.push({ nRows, nCols });
					while (!Search.empty())
					{
						Local temp = Search.top();
						Search.pop();
						AreaSize++;
						for (uint32_t nTempRows = temp.x - 1; nTempRows <= temp.x + 1; nTempRows++)
						{
							if (nTempRows < m_nTotalRow)
							{
								for (uint32_t nTempCols = temp.y - 1; nTempCols <= temp.y + 1; nTempCols++)
								{
									if (nTempCols < m_nTotalCol && BadPixelMask[nTempRows][nTempCols] != 0 && BadPixelMask[nTempRows][nTempCols] < ConnectedAreaFlag)
									{
										BadPixelMask[nTempRows][nTempCols] = ConnectedAreaFlag;
										Search.push({ nTempRows , nTempCols });
									}
								}
							}
						}
					}
					if (AreaSize >= m_AlgorithmThre.nDeadPixelClusterSizeThre)
					{
						BadpixelRes.nOnEventsClusterNum++;
					}
				    if (AreaSize > BadpixelRes.nOnEventsMaxClusterSize)
				    {
				        BadpixelRes.nOnEventsMaxClusterSize = AreaSize;
				    }
					ConnectedAreaFlag--;
				}
			}
		}
	}
	return true;
}

bool CAlpDVSMPAlgorithm::Show(uint32_t nIndex, uint8_t NoEventFlag, uint8_t OnEventFlag, uint8_t OffEventFlag, ImgType& ImgData)
{
	if (nIndex >= m_RawDataContainer.size() || m_nTotalRow != m_RawDataContainer[nIndex].m_nRow || m_nTotalCol != m_RawDataContainer[nIndex].m_nCol)
	{
		std::string strErr = "Show: Index error: nIndex: " + std::to_string(nIndex);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}

	ImgData.resize(m_nTotalRow);

	for (uint32_t nRows = 0; nRows < m_nTotalRow; nRows++)
	{
		ImgData[nRows].resize(m_nTotalCol);
		for (uint32_t nCols = 0; nCols < m_nTotalCol; nCols++)
		{
			uint32_t nValue = m_RawDataContainer[nIndex].GetData(nRows, nCols);
			if (nValue == ON_EVENT_FLAG)
			{
				ImgData[nRows][nCols] = OnEventFlag;
			}
			else if (nValue == OFF_EVENT_FLAG)
			{
				ImgData[nRows][nCols] = OffEventFlag;
			}
			else
			{
				ImgData[nRows][nCols] = NoEventFlag;
			}
		}
	}
	return true;
}

void CAlpDVSMPAlgorithm::SetMultiThreadEnable(bool bEnable)
{
	m_bMultiThreadEnable = bEnable;
}

void CAlpDVSMPAlgorithm::SetLogEnable(bool bEnable)
{
	m_bLogEnable = bEnable;
}

void CAlpDVSMPAlgorithm::SetAlgorithmThre(DVSAlgorithmThre& AlgoThre)
{
	m_AlgorithmThre = AlgoThre;
}

DVSAlgorithmThre CAlpDVSMPAlgorithm::GetAlgorithmThre()
{
	return m_AlgorithmThre;
}

uint32_t CAlpDVSMPAlgorithm::GetDataNum()
{
	return m_RawDataContainer.size();
}

void CAlpDVSMPAlgorithm::ThreadEventsNumberCount(uint32_t nIndexStart, uint32_t nNumberStart, uint32_t nNumberEnd, DVSEventsNumberCountType& EventsNumberCountRes)
{
	for (uint32_t nIndex = nNumberStart; nIndex < nNumberEnd; nIndex++)
	{
		m_RawDataContainer[nIndexStart + nIndex].CountEvents(m_ActiveArea);
		for (uint32_t i = 0; i <= SubFrameIndex::All; i++)
		{
			EventsNumberCountRes.NoEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_NoEventsNum[i];
			EventsNumberCountRes.OnEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_OnEventsNum[i];
			EventsNumberCountRes.OffEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_OffEventsNum[i];
			EventsNumberCountRes.AllEventsNum[i][nIndex] = m_RawDataContainer[nIndexStart + nIndex].m_AllEventsNum[i];
		}
	}
}

bool CAlpDVSMPAlgorithm::WriteLog(std::string strMessage)
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

void CAlpDVSMPAlgorithm::GetChannel(uint32_t nRows, uint32_t nCols, SubFrameIndex& nChannel)
{
	switch (m_PixelFormat)
	{
	case BayerGBRG:
		if (0 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gb;
		}
		else if (0 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::B;
		}
		else if (1 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::R;
		}
		else if (1 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gr;
		}
		break;
	case BayerBGGR:
		if (0 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::B;
		}
		else if (0 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gb;
		}
		else if (1 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gr;
		}
		else if (1 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::R;
		}
		break;
	case BayerRGGB:
		if (0 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::R;
		}
		else if (0 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gr;
		}
		else if (1 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gb;
		}
		else if (1 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::B;
		}
		break;
	case BayerGRBG:
		if (0 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gr;
		}
		else if (0 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::R;
		}
		else if (1 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::B;
		}
		else if (1 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gb;
		}
		break;
	}
}

void CAlpDVSMPAlgorithm::local_maxima_1d(std::vector<uint32_t>& RawData, uint32_t nLens, std::vector<uint32_t>& midpoints, std::vector<uint32_t>& left_edges, std::vector<uint32_t>& right_edges)
{
    // 清空并释放旧内存
    left_edges.clear();
    left_edges.shrink_to_fit();
    right_edges.clear();
    right_edges.shrink_to_fit();
    midpoints.clear();
    midpoints.shrink_to_fit();

    // 一维局部极大值检测, 在数据序列中找到所有峰值点及其左右边界
	int i = 1; // 从索引1开始, 跳过边界
	int i_max = nLens - 1; // 结束于倒数第二个元素, 跳过边界
	int m = 0; // 峰值计数器
    // 预分配空间, 最多 n/2个峰
	left_edges.resize(nLens / 2, 0);
	right_edges.resize(nLens / 2, 0);
	midpoints.resize(nLens / 2, 0);
	while (i < i_max)
	{
	    // 条件1: 检测上升沿
		if (RawData[i - 1] < RawData[i])
		{
	        // 条件2: 处理平台区域, 平台区域被识别为单一峰值, 中点作为峰值位置
			int i_ahead = i + 1;
			while (i_ahead < i_max && RawData[i_ahead] == RawData[i])
			{
				i_ahead += 1;
			}
		    // 条件3: 检测下降沿
			if (RawData[i_ahead] < RawData[i])
			{
			    // 找到峰值
			    // 峰值左边界
				left_edges[m] = i;
			    // 峰值右边界
				right_edges[m] = i_ahead - 1;
			    // 峰值中点
				midpoints[m] = (left_edges[m] + right_edges[m]) / 2;
				m += 1;
			    // 跳到下降沿后继续搜索
				i = i_ahead;
			}
		}
		i += 1;
	}
    // 调整 size 并释放多余容量
    left_edges.resize(m);
    left_edges.shrink_to_fit();
    right_edges.resize(m);
    right_edges.shrink_to_fit();
    midpoints.resize(m);
    midpoints.shrink_to_fit();
}

void CAlpDVSMPAlgorithm::select_by_peak_distance(std::vector<uint32_t>& peak, std::vector<uint32_t>& peak_height, uint32_t nDistance, std::vector<uint32_t>& keep)
{
    // 清空并释放旧内存
    keep.clear();
    keep.shrink_to_fit();
    // 当多个峰值距离过近时, 只保留其中高度最大的峰值, 抑制其邻近的较小峰值
    // 初始化峰保留标志, 默认全部保留
	keep.resize(peak_height.size(), 1);
    // 构建: **索引, 高度** key-value
	std::vector<PeakAndHeight> priority_to_position;
	for (uint32_t i = 0; i < peak_height.size(); i++)
	{
        priority_to_position.push_back({i, peak_height[i]});
    }
    // 按高度排序, 从低到高. **排序后高度最大的峰值在vector末尾**
    std::sort(priority_to_position.begin(), priority_to_position.end(), comparePeakAndHeight);

    // 从高度最大的峰值开始处理, 保证保留区域内最显著的峰值
    for (int i = priority_to_position.size() - 1; i >= 0; i--) {
        int j = priority_to_position[i].peak; // 当前峰值的原始索引
        // 如果当前峰值已被抑制, 跳过
        if (keep[j] == 0) {
            continue;
        }

        // 局部抑制: 只影响nDistance内的峰值, 不会误删远处峰值
        // 向左抑制距离过近的峰值
        int k = j - 1;
        while (0 <= k && ((peak[j] - peak[k]) < nDistance)) {
            keep[k] = 0; // 抑制左侧邻近峰值
            k -= 1;
        }

        // 向右抑制距离过近的峰值
        k = j + 1;
        while (k < peak.size() && ((peak[k] - peak[j]) < nDistance)) {
            keep[k] = 0; // 抑制右侧邻近峰值
            k += 1;
        }
    }
    // 释放 priority_to_position 的内存
    priority_to_position.clear();
    priority_to_position.shrink_to_fit();
}

bool CAlpDVSMPAlgorithm::quadratic_fit(const std::vector<std::pair<double, double> > &points, double &a, double &b,
                                       double &c) {
    // 二次多项式拟合：y = a*x? + b*x + c
    // 返回值：是否拟合成功；参数a、b、c为输出的系数
    const int n = points.size();
    if (n < 3) {
        std::string strErr("quadratic_fit requires at least 3 points.");
        WriteLog(strErr);
        return false;
    }

    // 计算求和项
    double sum_x = 0.0, sum_x2 = 0.0, sum_x3 = 0.0, sum_x4 = 0.0;
    double sum_y = 0.0, sum_xy = 0.0, sum_x2y = 0.0;

    for (const auto &p: points) {
        double x = p.first;
        double y = p.second;
        double x2 = x * x;
        double x3 = x2 * x;
        double x4 = x3 * x;

        sum_x += x;
        sum_x2 += x2;
        sum_x3 += x3;
        sum_x4 += x4;
        sum_y += y;
        sum_xy += x * y;
        sum_x2y += x2 * y;
    }

    // 构建方程组：
    // m00*a + m01*b + m02*c = m03
    // m10*a + m11*b + m12*c = m13
    // m20*a + m21*b + m22*c = m23
    double m00 = sum_x4;
    double m01 = sum_x3;
    double m02 = sum_x2;
    double m03 = sum_x2y;

    double m10 = sum_x3;
    double m11 = sum_x2;
    double m12 = sum_x;
    double m13 = sum_xy;

    double m20 = sum_x2;
    double m21 = sum_x;
    double m22 = n;
    double m23 = sum_y;

    // 高斯消元法求解三元一次方程组
    // 第一步：消去第二、三行的a（第一列）
    double factor1 = m10 / m00;
    m11 -= factor1 * m01;
    m12 -= factor1 * m02;
    m13 -= factor1 * m03;

    double factor2 = m20 / m00;
    m21 -= factor2 * m01;
    m22 -= factor2 * m02;
    m23 -= factor2 * m03;

    // 第二步：消去第三行的b（第二列）
    if (fabs(m11) < 1e-12) {
        return false; // 矩阵奇异，无法求解
    }
    double factor3 = m21 / m11;
    m22 -= factor3 * m12;
    m23 -= factor3 * m13;

    // 第三步：回代求解c、b、a
    if (fabs(m22) < 1e-12) {
        return false; // 矩阵奇异，无法求解
    }
    c = m23 / m22;
    b = (m13 - m12 * c) / m11;
    a = (m03 - m01 * b - m02 * c) / m00;

    return true;
}

std::vector<double> CAlpDVSMPAlgorithm::solve_quadratic_equation(double a, double b, double c) {
    // 求解二次方程 ax? + bx + c = 0 的实根
    std::vector<double> roots;
    const double eps = 1e-9;

    // 特殊情况：a接近0，视为一次方程
    if (fabs(a) < eps) {
        if (fabs(b) < eps) {
            return roots; // 无解（0x + 0 = 0 视为无解，除非c=0但无意义）
        }
        roots.push_back(-c / b);
        return roots;
    }

    // 标准二次方程求解
    double discriminant = b * b - 4 * a * c;
    if (discriminant < -eps) {
        return roots; // 无实根
    }
    if (fabs(discriminant) < eps) {
        roots.push_back(-b / (2 * a)); // 唯一实根
        return roots;
    }

    // 两个实根
    double sqrtD = sqrt(discriminant);
    roots.push_back((-b + sqrtD) / (2 * a));
    roots.push_back((-b - sqrtD) / (2 * a));
    return roots;
}

std::vector<double> CAlpDVSMPAlgorithm::get_quadratic_x_value_from_y_value(double y_target, double a, double b,
                                                                           double c) {
    // 根据目标y值和拟合系数, 计算对应的x值
    // ax? + bx + (c - y_target) = 0
    return solve_quadratic_equation(a, b, c - y_target);
}

double CAlpDVSMPAlgorithm::Mean(std::vector<double>& RawData, uint32_t nLens)
{
	if (nLens < 1 || nLens > RawData.size())
	{
		std::string strErr = "Mean: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr);
		m_nErrCode = DATA_LENS_ERROR;

		return 0.0;
	}
	double dMean = 0;
	for (uint32_t nIndex = 0; nIndex < nLens; nIndex++)
	{
		dMean += RawData[nIndex];
	}
	return dMean / nLens;
}

double CAlpDVSMPAlgorithm::Mean(std::vector<uint32_t>& RawData, uint32_t nLens)
{
	if (nLens < 1 || nLens > RawData.size())
	{
		std::string strErr = "Mean: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr);
		m_nErrCode = DATA_LENS_ERROR;
		return 0.0;
	}
	double dMean = 0;
	for (uint32_t nIndex = 0; nIndex < nLens; nIndex++)
	{
		dMean += RawData[nIndex];
	}
	return dMean / nLens;
}

double CAlpDVSMPAlgorithm::Mean(CAPSDataContainer& RawData, ROIArea* ROI)
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
		WriteLog(strErr);
		m_nErrCode = DATA_ROI_SET_ERROR;
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

double CAlpDVSMPAlgorithm::Std(std::vector<double>& RawData, uint32_t nLens)
{
	if (nLens < 2 || nLens > RawData.size())
	{
		std::string strErr = "Std: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr);
		m_nErrCode = DATA_LENS_ERROR;
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

double CAlpDVSMPAlgorithm::Std(CAPSDataContainer& RawData, ROIArea* ROI)
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
		WriteLog(strErr);
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

double CAlpDVSMPAlgorithm::Std(std::vector<uint32_t>& RawData, uint32_t nLens)
{
	if (nLens < 2 || nLens > RawData.size())
	{
		std::string strErr = "Std: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr);
		m_nErrCode = DATA_LENS_ERROR;
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

void CAlpDVSMPAlgorithm::Max(double& dMaxValue, uint32_t& nMaxLocal, std::vector<double>& RawData, uint32_t nLens)
{
	if (nLens < 1 || nLens > RawData.size())
	{
		std::string strErr = "Max: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr);
		m_nErrCode = DATA_LENS_ERROR;
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

void CAlpDVSMPAlgorithm::Max(double& dMaxValue, uint32_t& nMaxLocal, std::vector<uint32_t>& RawData, uint32_t nLens)
{
	if (nLens < 1 || nLens > RawData.size())
	{
		std::string strErr = "Max: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr);
		m_nErrCode = DATA_LENS_ERROR;
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

void CAlpDVSMPAlgorithm::Max(double& dMaxValue, Local& MaxLocal, CAPSDataContainer& RawData, ROIArea* ROI)
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
		WriteLog(strErr);
		m_nErrCode = DATA_ROI_SET_ERROR;
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

void CAlpDVSMPAlgorithm::Min(double& dMinValue, uint32_t& nMinLocal, std::vector<double>& RawData, uint32_t nLens)
{
	if (nLens < 1 || nLens > RawData.size())
	{
		std::string strErr = "Min: Lens error: RawData Lens: " + std::to_string(RawData.size()) + ", Need Lens: " + std::to_string(nLens);
		WriteLog(strErr);
		m_nErrCode = DATA_LENS_ERROR;
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

void CAlpDVSMPAlgorithm::Min(double& dMinValue, Local& MinLocal, CAPSDataContainer& RawData, ROIArea* ROI)
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
		WriteLog(strErr);
		m_nErrCode = DATA_ROI_SET_ERROR;
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

bool CAlpDVSMPAlgorithm::SaveBin(uint8_t* pRawData, uint64_t nLens, std::string strSavePath)
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
	if (!bRet)
	{
		m_nErrCode = SAVE_DATA_ERROR;
	}
	return bRet;
}

ROIArea CAlpDVSMPAlgorithm::GetActiveArea()
{
	return m_ActiveArea;
}

void CAlpDVSMPAlgorithm::GetRawDataSize(uint32_t& nRow, uint32_t& nCol)
{
	nRow = m_nTotalRow;
	nCol = m_nTotalCol;
}

void CAlpDVSMPAlgorithm::SetActiveArea(ROIArea ActiveArea)
{
	m_ActiveArea = ActiveArea;
}

void CAlpDVSMPAlgorithm::SetRawDataSize(uint32_t nRow, uint32_t nCol)
{
	m_nTotalRow = nRow;
	m_nTotalCol = nCol;
}

std::string CAlpDVSMPAlgorithm::GetVersion()
{
	return MP_ALGORITHM_VERSION;
}

int CAlpDVSMPAlgorithm::GetCode()
{
	return m_nCode;
}

uint32_t CAlpDVSMPAlgorithm::GetErrCode()
{
	return m_nErrCode;
}

bool CAlpDVSMPAlgorithm::CalcLightIntensity(double onEventPercent, double offEventPercent,
                                            std::vector<double> vecOnEvent, std::vector<double> vecOffEvent,
                                            std::vector<std::pair<double, double> > vecLightWave,
                                            double &onTargetLightWave, double &offTargetLightWave) {
    if (vecOnEvent.size() != vecOffEvent.size() ||
        vecOnEvent.size() != vecLightWave.size() ||
        vecOffEvent.size() != vecLightWave.size()) {
        std::string str = "vecOnEvent/vecOffEvent/vecLightWave size error.";
        WriteLog(str);
        return false;
    }
    //计算目标事件量的光强跳变点
    onTargetLightWave = 0;
    offTargetLightWave = 0;

    std::vector<std::pair<double, double> > points_on; //一系列用于计算的光源跳变比例和对应的事件量
    std::vector<std::pair<double, double> > points_off; //一系列用于计算的光源跳变比例和对应的事件量

    // std::vector<std::pair<double, double>> vecLightWave = { {100,105}, {100,130}, {100,135}, {100,140}, {100,145}, {100,150}, {100,155}, {100,160}, {100,165}, {100,170}, {100,235} };   //光强跳变点

    for (int var = 1; var < vecLightWave.size() - 1; ++var) {
        points_on.push_back({
            (vecLightWave[var].second - vecLightWave[var].first) / vecLightWave[var].first * 100, vecOnEvent[var]
        });
        points_off.push_back({
            (vecLightWave[var].second - vecLightWave[var].first) / vecLightWave[var].first * 100, vecOffEvent[var]
        });
    }

    //使用二次多项式拟合，根据目标y，寻找x
    std::vector<double> x_roots_on;
    int nRet = 0;
    onTargetLightWave = -999;
    nRet = FindQuadraticXValueFromYValue(points_on, onEventPercent, x_roots_on);
    if (nRet) {
        for (double x: x_roots_on) {
            if (x >= 0) {
                // 找到第一个点
                onTargetLightWave = x;
                break;
            }
        }
    }

    std::vector<double> x_roots_off;
    onTargetLightWave = -999;
    nRet = FindQuadraticXValueFromYValue(points_off, offEventPercent, x_roots_off);
    if (nRet) {
        for (double x: x_roots_off) {
            // 找到第一个点
            if (x >= 0) {
                onTargetLightWave = x;
                break;
            }
        }
    }
    return true;
}

int CAlpDVSMPAlgorithm::FindQuadraticXValueFromYValue(const std::vector<std::pair<double, double> > &points,
                                                      double &y_target, std::vector<double> &x_roots) {
    //使用二次多项式拟合，根据目标Y，寻找X
    // 示例2：带噪声的二次曲线 y = 2x? + 3x + 1（添加±0.2噪声）
    int nRet{0};
    double a, b, c;
    try {
        if (quadratic_fit(points, a, b, c)) {
            // double y_target = 50.0;
            // 理论上2x?+3x+1=38 → 2x?+3x-37=0 → 根≈3.5和-5
            x_roots = get_quadratic_x_value_from_y_value(y_target, a, b, c);
            if (x_roots.empty()) {
                nRet = -1;
                std::string str = "Y_target has no real roots.";
                WriteLog(str);
            }
        } else {
            nRet = -2;
            std::string str = "Quadratic Fitting Error.";
            WriteLog(str);
        }
    } catch (const std::exception &e) {
        std::string str = "FindQuadraticXValueFromYValue error.";
        WriteLog(str);
    }
    return nRet;
}
