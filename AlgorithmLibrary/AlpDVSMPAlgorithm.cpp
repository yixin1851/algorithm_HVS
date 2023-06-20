#include "AlpDVSMPAlgorithm.h"
#include <algorithm>
#include <thread>
#include <stack>
#include <fstream>
#include <windows.h>
#include "Changelist.h"

constexpr uint32_t DVS_MaxThreadNum = 8;

CAlpDVSMPAlgorithm::CAlpDVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat)
{
	m_nSiteNum = nSiteNum;
	m_bLogEnable = false;
	m_SensorType = Sensortype;
	m_bMultiThreadEnable = false;
	m_AlgorithmThre.dDeadPixelThre = 0.8;
	m_AlgorithmThre.dErrorPixelThre = 0.8;
	m_AlgorithmThre.dHotLineThre = 0.5;
	m_AlgorithmThre.dHotPixelThre = 0.8;
	m_AlgorithmThre.nStationaryUniformityRowBlockNum = 5;
	m_AlgorithmThre.nStationaryUniformityColBlockNum = 5;
	m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum = 5;
	m_AlgorithmThre.nSpatialResponseUniformityColBlockNum = 5;
	//m_AlgorithmThre.nFindPeakNum = 3;
	//m_AlgorithmThre.dFindPeakThre = 0.75;

	m_AlgorithmThre.nPeakCycle = 20;
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

bool CAlpDVSMPAlgorithm::StationaryNoise(uint32_t nIndexStart, uint32_t nNumber, DVSStationaryNoiseType& StationaryNoiseRes)
{
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
	StationaryNoiseRes.dStationaryNoiseMeanAll = Mean(EventsNumber.AllEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;
	StationaryNoiseRes.dStationaryNoiseMeanOn = Mean(EventsNumber.OnEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;
	StationaryNoiseRes.dStationaryNoiseMeanOff = Mean(EventsNumber.OffEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;

	StationaryNoiseRes.dStationaryNoiseStdAll = Std(EventsNumber.AllEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;
	StationaryNoiseRes.dStationaryNoiseStdOn = Std(EventsNumber.OnEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;
	StationaryNoiseRes.dStationaryNoiseStdOff = Std(EventsNumber.OffEventsNum[SubFrameIndex::All], nNumber) / nAllSize * 100;

	std::vector<double> RowMeanAllEvents(nRowSize, 0);
	std::vector<double> ColMeanAllEvents(nColSize, 0);

	for (uint32_t nRows = m_ActiveArea.Up; nRows <= m_ActiveArea.Down; nRows++)
	{
		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			RowMeanAllEvents[nRows - m_ActiveArea.Up] += m_RawDataContainer[nIndexStart + nIndex].m_RowAllEventsNum[nRows];
		}
		RowMeanAllEvents[nRows - m_ActiveArea.Up] /= nNumber;
	}
	for (uint32_t nCols = m_ActiveArea.Left; nCols <= m_ActiveArea.Right; nCols++)
	{
		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			ColMeanAllEvents[nCols - m_ActiveArea.Left] += m_RawDataContainer[nIndexStart + nIndex].m_ColAllEventsNum[nCols];
		}
		ColMeanAllEvents[nCols - m_ActiveArea.Left] /= nNumber;
	}

	double MaxValue = 0;
	uint32_t MaxLocal = 0;

	Max(MaxValue, MaxLocal, RowMeanAllEvents, nRowSize);
	StationaryNoiseRes.dStationaryRowSNoise = 100.0 * (MaxValue - Mean(RowMeanAllEvents, nRowSize)) / nColSize;

	Max(MaxValue, MaxLocal, ColMeanAllEvents, nColSize);
	StationaryNoiseRes.dStationaryColSNoise = 100.0 * (MaxValue - Mean(ColMeanAllEvents, nColSize)) / nRowSize;

	return true;
}

bool CAlpDVSMPAlgorithm::StationaryUniformity(uint32_t nIndexStart, uint32_t nNumber, DVSStationaryUniformityType& UniformityRes)
{
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

	uint32_t nRowBlockSize = nRowSize / m_AlgorithmThre.nStationaryUniformityRowBlockNum;
	uint32_t nColBlockSize = nColSize / m_AlgorithmThre.nStationaryUniformityColBlockNum;

	for (uint32_t nRows = m_ActiveArea.Up; nRows < m_ActiveArea.Up + nRowBlockSize * m_AlgorithmThre.nStationaryUniformityRowBlockNum; nRows++)
	{
		for (uint32_t nCols = m_ActiveArea.Left; nCols < m_ActiveArea.Left + nColBlockSize * m_AlgorithmThre.nStationaryUniformityColBlockNum; nCols++)
		{
			double dValue = 0;
			for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
			{
				if (0 != m_RawDataContainer[nIndexStart + nIndex].GetData(nRows, nCols))
				{
					++UniformityBlockData.m_RawData[(nRows - m_ActiveArea.Up) / nRowBlockSize][(nCols - m_ActiveArea.Left) / nColBlockSize];
				}
			}
		}
	}
	UniformityBlockData /= nRowBlockSize * nColBlockSize * nNumber;
	UniformityBlockData *= 100;

	double dMeanValue = Mean(UniformityBlockData);
	double dMaxValue = 0, dMinValue = 0;
	Local temp;
	Max(dMaxValue, temp, UniformityBlockData);
	Min(dMinValue, temp, UniformityBlockData);
	//UniformityRes.UniformityRatio = (dMaxValue - dMinValue) / dMeanValue * 100;
	//UniformityRes.UniformityRatio = Std(UniformityBlockData, nullptr);
	UniformityRes.UniformityRatio = dMaxValue - dMinValue;
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
	HotpixelRes.HotPixelMask.Flag.clear();
	HotpixelRes.HotPixelMask.BadPixelNum = 0;
	HotpixelRes.ClusterNum = 0;
	HotpixelRes.HotLineNum = 0;

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
				Search.push({ nRows, nCols });
				while (!Search.empty())
				{
					Local temp = Search.top();
					Search.pop();
					BadPixelMask[temp.x][temp.y] = ConnectedAreaFlag;
					AreaSize++;
					for (uint32_t nTempRows = temp.x - 1; nTempRows <= temp.x + 1; nTempRows++)
					{
						if (nTempRows < m_nTotalRow)
						{
							for (uint32_t nTempCols = temp.y - 1; nTempCols <= temp.y + 1; nTempCols++)
							{
								if (nTempCols < m_nTotalCol && BadPixelMask[nTempRows][nTempCols] != 0 && BadPixelMask[nTempRows][nTempCols] < ConnectedAreaFlag)
								{
									Search.push({ nTempRows , nTempCols });
								}
							}
						}
					}
				}
				if (AreaSize > 1)
				{
					HotpixelRes.ClusterNum++;
				}
				ConnectedAreaFlag--;
			}
		}
	}

	return true;
}
#if 0
bool CAlpDVSMPAlgorithm::FindPeak(uint32_t nIndexStart, uint32_t nNumber, PeakInfo& Peak, LightTrigerType Light)
{
	if (0 == m_AlgorithmThre.nFindPeakNum)
	{
		std::string strErr = "FindPeak: FindPeakNum error";
		WriteLog(strErr);
		m_nErrCode = FIND_PEAK_NUM_SET_ERROR;
		return false;
	}

	if (nNumber < m_AlgorithmThre.nFindPeakNum || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "FindPeak: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}

	EventsNumberCountData EventsNumberCountRes;
	EventsNumberCount(nIndexStart, nNumber, EventsNumberCountRes);

	Peak.nOffEventsPeakNumber = 0;
	Peak.OffEventsPeakPos.clear();
	Peak.nOnEventsPeakNumber = 0;
	Peak.OnEventsPeakPos.clear();

	if (Light & LightTrigerType::OffEventsOnly)
	{
		std::vector<int32_t> Gradient(nNumber - 1);
		for (uint32_t nIndex = 0; nIndex < nNumber - 1; nIndex++)
		{
			Gradient[nIndex] = ((int32_t)(EventsNumberCountRes.OffEventsNum[SubFrameIndex::All][nIndex + 1])) - ((int32_t)(EventsNumberCountRes.OffEventsNum[SubFrameIndex::All][nIndex]));
		}
		std::sort(Gradient.begin(), Gradient.end(), std::greater<int32_t>());

		double GradientThre = 0;
		for (uint32_t nIndex = 1; nIndex <= m_AlgorithmThre.nFindPeakNum; nIndex++)
		{
			GradientThre += Gradient[nIndex];
		}
		GradientThre /= m_AlgorithmThre.nFindPeakNum;
		GradientThre *= m_AlgorithmThre.dFindPeakThre;

		for (uint32_t nIndex = 0; nIndex < nNumber - 1; nIndex++)
		{
			double Gradient = ((int32_t)(EventsNumberCountRes.OffEventsNum[SubFrameIndex::All][nIndex + 1])) - ((int32_t)(EventsNumberCountRes.OffEventsNum[SubFrameIndex::All][nIndex]));

			if (Gradient >= GradientThre)
			{
				++Peak.nOffEventsPeakNumber;
				Peak.OffEventsPeakPos.push_back(nIndexStart + nIndex + 1);
			}
		}
	}

	if (Light & LightTrigerType::OnEventsOnly)
	{
		std::vector<int32_t> Gradient(nNumber - 1);
		for (uint32_t nIndex = 0; nIndex < nNumber - 1; nIndex++)
		{
			Gradient[nIndex] = ((int32_t)(EventsNumberCountRes.OnEventsNum[SubFrameIndex::All][nIndex + 1])) - ((int32_t)(EventsNumberCountRes.OnEventsNum[SubFrameIndex::All][nIndex]));
		}
		std::sort(Gradient.begin(), Gradient.end(), std::greater<int32_t>());

		double GradientThre = 0;
		for (uint32_t nIndex = 1; nIndex <= m_AlgorithmThre.nFindPeakNum; nIndex++)
		{
			GradientThre += Gradient[nIndex];
		}
		GradientThre /= m_AlgorithmThre.nFindPeakNum;
		GradientThre *= m_AlgorithmThre.dFindPeakThre;

		for (uint32_t nIndex = 0; nIndex < nNumber - 1; nIndex++)
		{
			double Gradient = ((int32_t)(EventsNumberCountRes.OnEventsNum[SubFrameIndex::All][nIndex + 1])) - ((int32_t)(EventsNumberCountRes.OnEventsNum[SubFrameIndex::All][nIndex]));

			if (Gradient >= GradientThre)
			{
				++Peak.nOnEventsPeakNumber;
				Peak.OnEventsPeakPos.push_back(nIndexStart + nIndex + 1);
			}
		}
	}

	return true;
}
#endif
bool CAlpDVSMPAlgorithm::FindPeak(uint32_t nIndexStart, uint32_t nNumber, DVSPeakInfo& Peak, DVSLightTrigerType Light)
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
	Peak.nOnEventsPeakNumber = 0;
	Peak.OnEventsPeakPos.clear();

	uint32_t nCycle = m_AlgorithmThre.nPeakCycle;

	if (Light & DVSLightTrigerType::OffEventsOnly)
	{
		//std::vector<int32_t> Gradient(nNumber - 1);
		//for (uint32_t nIndex = 0; nIndex < nNumber - 1; nIndex++)
		//{
		//	Gradient[nIndex] = ((int32_t)(EventsNumberCountRes.OffEventsNum[SubFrameIndex::All][nIndex + 1])) - ((int32_t)(EventsNumberCountRes.OffEventsNum[SubFrameIndex::All][nIndex]));
		//}
		//std::sort(Gradient.begin(), Gradient.end(), std::greater<int32_t>());

		//double GradientThre = 0;
		//for (uint32_t nIndex = 1; nIndex <= m_AlgorithmThre.nFindPeakNum; nIndex++)
		//{
		//	GradientThre += Gradient[nIndex];
		//}
		//GradientThre /= m_AlgorithmThre.nFindPeakNum;
		//GradientThre *= m_AlgorithmThre.dFindPeakThre;

		//for (uint32_t nIndex = 0; nIndex < nNumber - 1; nIndex++)
		//{
		//	double Gradient = ((int32_t)(EventsNumberCountRes.OffEventsNum[SubFrameIndex::All][nIndex + 1])) - ((int32_t)(EventsNumberCountRes.OffEventsNum[SubFrameIndex::All][nIndex]));

		//	if (Gradient >= GradientThre)
		//	{
		//		++Peak.nOffEventsPeakNumber;
		//		Peak.OffEventsPeakPos.push_back(nIndexStart + nIndex + 1);
		//	}
		//}
		std::vector<double> EventsData(nCycle, 0);
		std::vector<uint32_t> NumberData(nCycle, 0);

		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			EventsData[(nIndexStart + nIndex) % nCycle] += EventsNumberCountRes.OffEventsNum[SubFrameIndex::All][nIndex];
			NumberData[(nIndexStart + nIndex) % nCycle]++;
		}

		for (uint32_t nIndex = 0; nIndex < nCycle; nIndex++)
		{
			if (NumberData[nIndex] > 0)
				EventsData[nIndex] /= NumberData[nIndex];
		}

		double dMaxValue = 0;
		uint32_t nMaxLocal = 0;
		Max(dMaxValue, nMaxLocal, EventsData, EventsData.size());

		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			if (((nIndexStart + nIndex) % nCycle) == nMaxLocal)
			{
				++Peak.nOffEventsPeakNumber;
				Peak.OffEventsPeakPos.push_back(nIndexStart + nIndex);
			}
		}
	}

	if (Light & DVSLightTrigerType::OnEventsOnly)
	{
		//std::vector<int32_t> Gradient(nNumber - 1);
		//for (uint32_t nIndex = 0; nIndex < nNumber - 1; nIndex++)
		//{
		//	Gradient[nIndex] = ((int32_t)(EventsNumberCountRes.OnEventsNum[SubFrameIndex::All][nIndex + 1])) - ((int32_t)(EventsNumberCountRes.OnEventsNum[SubFrameIndex::All][nIndex]));
		//}
		//std::sort(Gradient.begin(), Gradient.end(), std::greater<int32_t>());

		//double GradientThre = 0;
		//for (uint32_t nIndex = 1; nIndex <= m_AlgorithmThre.nFindPeakNum; nIndex++)
		//{
		//	GradientThre += Gradient[nIndex];
		//}
		//GradientThre /= m_AlgorithmThre.nFindPeakNum;
		//GradientThre *= m_AlgorithmThre.dFindPeakThre;

		//for (uint32_t nIndex = 0; nIndex < nNumber - 1; nIndex++)
		//{
		//	double Gradient = ((int32_t)(EventsNumberCountRes.OnEventsNum[SubFrameIndex::All][nIndex + 1])) - ((int32_t)(EventsNumberCountRes.OnEventsNum[SubFrameIndex::All][nIndex]));

		//	if (Gradient >= GradientThre)
		//	{
		//		++Peak.nOnEventsPeakNumber;
		//		Peak.OnEventsPeakPos.push_back(nIndexStart + nIndex + 1);
		//	}
		//}

		std::vector<double> EventsData(nCycle, 0);
		std::vector<uint32_t> NumberData(nCycle, 0);

		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			EventsData[(nIndexStart + nIndex) % nCycle] += EventsNumberCountRes.OnEventsNum[SubFrameIndex::All][nIndex];
			NumberData[(nIndexStart + nIndex) % nCycle]++;
		}

		for (uint32_t nIndex = 0; nIndex < nCycle; nIndex++)
		{
			if (NumberData[nIndex] > 0)
				EventsData[nIndex] /= NumberData[nIndex];
		}

		double dMaxValue = 0;
		uint32_t nMaxLocal = 0;
		Max(dMaxValue, nMaxLocal, EventsData, EventsData.size());

		for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		{
			if (((nIndexStart + nIndex) % nCycle) == nMaxLocal)
			{
				++Peak.nOnEventsPeakNumber;
				Peak.OnEventsPeakPos.push_back(nIndexStart + nIndex);
			}
		}
	}

	return true;
}

bool CAlpDVSMPAlgorithm::ImageContrastSensitivity(uint32_t nIndexStart, uint32_t nNumber, DVSPeakInfo* Peak, uint32_t nPeakNum, DVSLightTrigerType Light, DVSImageContrastSensitivityType& ImageContrastSensitivityRes)
{
	if (nNumber < m_AlgorithmThre.nPeakCycle || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "ImageContrastSensitivity: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}
	DVSPeakInfo tempPeak;
	if (Peak == nullptr)
	{
		if (FindPeak(nIndexStart, nNumber, tempPeak, Light))
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

		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
			ImageContrastSensitivityRes.OffEventsRatio[nChannel] = 0;

			for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
			{
				ImageContrastSensitivityRes.OffEventsRatio[nChannel] += m_RawDataContainer[Peak->OffEventsPeakPos[Peak->nOffEventsPeakNumber - 1 - nIndex]].m_OffEventsNum[nChannel];
			}

			ImageContrastSensitivityRes.OffEventsRatio[nChannel] /= nPeakNum;
			if (nChannel == SubFrameIndex::All)
			{
				ImageContrastSensitivityRes.OffEventsRatio[nChannel] /= nAllSize;
				ImageContrastSensitivityRes.OffEventsRatio[nChannel] *= 100;
			}
			else
			{
				ImageContrastSensitivityRes.OffEventsRatio[nChannel] /= nAllSize / 4;
				ImageContrastSensitivityRes.OffEventsRatio[nChannel] *= 100;
			}
		}

		ImageContrastSensitivityRes.R_Gb_OffEventsRatio = 100 * ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::R] / ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::Gb];
		ImageContrastSensitivityRes.B_Gb_OffEventsRatio = 100 * ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::B] / ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::Gb];
		ImageContrastSensitivityRes.Gr_Gb_OffEventsRatio = 100 * ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::Gr] / ImageContrastSensitivityRes.OffEventsRatio[SubFrameIndex::Gb];
	}

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

		ImageContrastSensitivityRes.R_Gb_OnEventsRatio = 100 * ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::R] / ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::Gb];
		ImageContrastSensitivityRes.B_Gb_OnEventsRatio = 100 * ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::B] / ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::Gb];
		ImageContrastSensitivityRes.Gr_Gb_OnEventsRatio = 100 * ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::Gr] / ImageContrastSensitivityRes.OnEventsRatio[SubFrameIndex::Gb];

	}

	return true;
}

bool CAlpDVSMPAlgorithm::AccompaniedPeakAndDelayedPeak(uint32_t nIndexStart, uint32_t nNumber, DVSPeakInfo* Peak, uint32_t nPeakNum, DVSLightTrigerType Light, DVSAccompaniedPeakAndDelayedPeakType& AccompaniedPeakAndDelayedPeakRes)
{
	if (nNumber < m_AlgorithmThre.nPeakCycle || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "ImageContrastSensitivity: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}
	DVSPeakInfo tempPeak;
	if (Peak == nullptr)
	{
		if (FindPeak(nIndexStart, nNumber, tempPeak, Light))
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
		if (Peak->nOffEventsPeakNumber < nPeakNum || 0 == nPeakNum)
		{
			std::string strErr = "AccompaniedPeakAndDelayedPeak: Peak Number error: Find Peak Num: " + std::to_string(Peak->nOffEventsPeakNumber) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}

		uint32_t nPeakStartNum = 0;

		if (Peak->OffEventsPeakPos[Peak->nOffEventsPeakNumber - 1] == nIndexStart + nNumber - 1)
		{
			nPeakStartNum = Peak->nOffEventsPeakNumber - 2;
		}
		else
		{
			nPeakStartNum = Peak->nOffEventsPeakNumber - 1;
		}

		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
			double OffEventsPeakNumber = 0;
			double NextOffEventsPeakNumber = 0;
			double NextOnEventsPeakNumber = 0;
			for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
			{
				OffEventsPeakNumber += m_RawDataContainer[Peak->OffEventsPeakPos[nPeakStartNum - nIndex]].m_OffEventsNum[nChannel];
				NextOffEventsPeakNumber += m_RawDataContainer[Peak->OffEventsPeakPos[nPeakStartNum - nIndex] + 1].m_OffEventsNum[nChannel];
				NextOnEventsPeakNumber += m_RawDataContainer[Peak->OffEventsPeakPos[nPeakStartNum - nIndex] + 1].m_OnEventsNum[nChannel];
			}

			AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOffEventsRatio[nChannel] = NextOnEventsPeakNumber / OffEventsPeakNumber * 100;
			AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOffEventsRatio[nChannel] = NextOffEventsPeakNumber / OffEventsPeakNumber * 100;
		}
	}

	if (Light & DVSLightTrigerType::OnEventsOnly)
	{
		if (Peak->nOnEventsPeakNumber < nPeakNum || 0 == nPeakNum)
		{
			std::string strErr = "AccompaniedPeakAndDelayedPeak: Peak Number error: Find Peak Num: " + std::to_string(Peak->nOffEventsPeakNumber) + ", Need Peak Num: " + std::to_string(nPeakNum);
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
			for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
			{
				OnEventsPeakNumber += m_RawDataContainer[Peak->OnEventsPeakPos[nPeakStartNum - nIndex]].m_OnEventsNum[nChannel];
				NextOffEventsPeakNumber += m_RawDataContainer[Peak->OnEventsPeakPos[nPeakStartNum - nIndex] + 1].m_OffEventsNum[nChannel];
				NextOnEventsPeakNumber += m_RawDataContainer[Peak->OnEventsPeakPos[nPeakStartNum - nIndex] + 1].m_OnEventsNum[nChannel];
			}

			AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOnEventsRatio[nChannel] = NextOffEventsPeakNumber / OnEventsPeakNumber * 100;
			AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOnEventsRatio[nChannel] = NextOnEventsPeakNumber / OnEventsPeakNumber * 100;
		}
	}
	return true;
}

bool CAlpDVSMPAlgorithm::SpatialResponseUniformity(uint32_t nIndexStart, uint32_t nNumber, DVSPeakInfo* Peak, uint32_t nPeakNum, DVSLightTrigerType Light, DVSSpatialResponseUniformityType& SpatialResponseUniformityRes)
{
	if (nNumber < m_AlgorithmThre.nPeakCycle || nIndexStart >= m_RawDataContainer.size() || (nIndexStart + nNumber) > m_RawDataContainer.size())
	{
		std::string strErr = "SpatialResponseUniformity: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}
	DVSPeakInfo tempPeak;
	if (Peak == nullptr)
	{
		if (FindPeak(nIndexStart, nNumber, tempPeak, Light))
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

	uint32_t nRowSize = m_ActiveArea.Down - m_ActiveArea.Up + 1;
	uint32_t nColSize = m_ActiveArea.Right - m_ActiveArea.Left + 1;
	uint32_t nAllSize = nRowSize * nColSize;

	if (Light & DVSLightTrigerType::OffEventsOnly)
	{
		if (Peak->nOffEventsPeakNumber < nPeakNum || 0 == nPeakNum)
		{
			std::string strErr = "AccompaniedPeakAndDelayedPeak: Peak Number error: Find Peak Num: " + std::to_string(Peak->nOffEventsPeakNumber) + ", Need Peak Num: " + std::to_string(nPeakNum);
			WriteLog(strErr);
			m_nErrCode = PEAK_NUM_ERROR;
			return false;
		}
		CAPSDataContainer OffEventsUniformityBlockData[SubFrameIndex::All + 1];
		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
			OffEventsUniformityBlockData[nChannel].Init(m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum, m_AlgorithmThre.nSpatialResponseUniformityColBlockNum, true);
			SpatialResponseUniformityRes.dOffEventsUniformityRatio[nChannel] = 0;
		}

		uint32_t nRowBlockSize = nRowSize / m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum;
		uint32_t nColBlockSize = nColSize / m_AlgorithmThre.nSpatialResponseUniformityColBlockNum;

		for (uint32_t nRows = m_ActiveArea.Up; nRows < m_ActiveArea.Up + nRowBlockSize * m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum; nRows++)
		{
			for (uint32_t nCols = m_ActiveArea.Left; nCols < m_ActiveArea.Left + nColBlockSize * m_AlgorithmThre.nSpatialResponseUniformityColBlockNum; nCols++)
			{
				double dValue = 0;
				SubFrameIndex nChannel = All;

				GetChannel(nRows, nCols, nChannel);

				for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
				{
					if (0 != m_RawDataContainer[Peak->OffEventsPeakPos[Peak->nOffEventsPeakNumber - 1 - nIndex]].GetData(nRows, nCols))
					{
						++dValue;
					}
				}
				OffEventsUniformityBlockData[SubFrameIndex::All].m_RawData[(nRows - m_ActiveArea.Up) / nRowBlockSize][(nCols - m_ActiveArea.Left) / nColBlockSize] += dValue / nPeakNum;
				OffEventsUniformityBlockData[nChannel].m_RawData[(nRows - m_ActiveArea.Up) / nRowBlockSize][(nCols - m_ActiveArea.Left) / nColBlockSize] += dValue / nPeakNum;
			}
		}

		for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
		{
			OffEventsUniformityBlockData[nChannel] /= nRowBlockSize * nColBlockSize;
			OffEventsUniformityBlockData[nChannel] *= 100;

			double dMeanValue = Mean(OffEventsUniformityBlockData[nChannel]);
			double dMaxValue = 0, dMinValue = 0;
			Local temp;
			Max(dMaxValue, temp, OffEventsUniformityBlockData[nChannel]);
			Min(dMinValue, temp, OffEventsUniformityBlockData[nChannel]);
			SpatialResponseUniformityRes.dOffEventsUniformityRatio[nChannel] = (dMaxValue - dMinValue) / dMeanValue * 100;
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

		for (uint32_t nRows = m_ActiveArea.Up; nRows < m_ActiveArea.Up + nRowBlockSize * m_AlgorithmThre.nSpatialResponseUniformityRowBlockNum; nRows++)
		{
			for (uint32_t nCols = m_ActiveArea.Left; nCols < m_ActiveArea.Left + nColBlockSize * m_AlgorithmThre.nSpatialResponseUniformityColBlockNum; nCols++)
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

				OnEventsUniformityBlockData[SubFrameIndex::All].m_RawData[(nRows - m_ActiveArea.Up) / nRowBlockSize][(nCols - m_ActiveArea.Left) / nColBlockSize] += dValue / nPeakNum;
				OnEventsUniformityBlockData[nChannel].m_RawData[(nRows - m_ActiveArea.Up) / nRowBlockSize][(nCols - m_ActiveArea.Left) / nColBlockSize] += dValue / nPeakNum;
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
			SpatialResponseUniformityRes.dOnEventsUniformityRatio[nChannel] = (dMaxValue - dMinValue) / dMeanValue * 100;
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
		std::string strErr = "BadPixel: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " + std::to_string(nNumber);
		WriteLog(strErr);
		m_nErrCode = DATA_INDEX_ERROR;
		return false;
	}
	DVSPeakInfo tempPeak;
	if (Peak == nullptr)
	{
		if (FindPeak(nIndexStart, nNumber, tempPeak, Light))
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

		BadpixelRes.OffEventsBadPixelMask.BadPixelNum = 0;
		BadpixelRes.OffEventsBadPixelMask.Flag.clear();
		BadpixelRes.OffEventsBadPixelMask.LocalData.clear();
		BadpixelRes.nOffEventsClusterNum = 0;
		BadpixelRes.nOffEventsDeadPixelNum = 0;
		BadpixelRes.nOffEventsErrorPixelNum = 0;

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
				double dErrorPixelRatio = 0;

				for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
				{
					uint32_t nValue = m_RawDataContainer[Peak->OffEventsPeakPos[Peak->nOffEventsPeakNumber - 1 - nIndex]].GetData(nRows, nCols);
					if (0 == nValue)
					{
						++dDeadPixelRatio;
					}
					else if (ON_EVENT_FLAG == nValue)
					{
						++dErrorPixelRatio;
					}
				}
				dDeadPixelRatio /= nPeakNum;
				dErrorPixelRatio /= nPeakNum;

				if (dDeadPixelRatio > m_AlgorithmThre.dDeadPixelThre)
				{
					BadpixelRes.OffEventsBadPixelMask.BadPixelNum++;
					BadpixelRes.OffEventsBadPixelMask.LocalData.push_back({ nRows, nCols });
					BadpixelRes.OffEventsBadPixelMask.Flag.push_back(DVS_DEAD_PIXEL_FLAG);
					BadpixelRes.nOffEventsDeadPixelNum++;
					BadPixelMask[nRows][nCols] = DVS_DEAD_PIXEL_FLAG;
				}
				else if (dErrorPixelRatio > m_AlgorithmThre.dErrorPixelThre)
				{
					BadpixelRes.OffEventsBadPixelMask.BadPixelNum++;
					BadpixelRes.OffEventsBadPixelMask.LocalData.push_back({ nRows, nCols });
					BadpixelRes.OffEventsBadPixelMask.Flag.push_back(DVS_ERROR_PIXEL_FLAG);
					BadpixelRes.nOffEventsErrorPixelNum++;
					BadPixelMask[nRows][nCols] = DVS_ERROR_PIXEL_FLAG;
				}
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
					Search.push({ nRows, nCols });
					while (!Search.empty())
					{
						Local temp = Search.top();
						Search.pop();
						BadPixelMask[temp.x][temp.y] = ConnectedAreaFlag;
						AreaSize++;
						for (uint32_t nTempRows = temp.x - 1; nTempRows <= temp.x + 1; nTempRows++)
						{
							if (nTempRows < m_nTotalRow)
							{
								for (uint32_t nTempCols = temp.y - 1; nTempCols <= temp.y + 1; nTempCols++)
								{
									if (nTempCols < m_nTotalCol && BadPixelMask[nTempRows][nTempCols] != 0 && BadPixelMask[nTempRows][nTempCols] < ConnectedAreaFlag)
									{
										Search.push({ nTempRows , nTempCols });
									}
								}
							}
						}
					}
					if (AreaSize > 1)
					{
						BadpixelRes.nOffEventsClusterNum++;
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

		BadpixelRes.OnEventsBadPixelMask.BadPixelNum = 0;
		BadpixelRes.OnEventsBadPixelMask.Flag.clear();
		BadpixelRes.OnEventsBadPixelMask.LocalData.clear();
		BadpixelRes.nOnEventsClusterNum = 0;
		BadpixelRes.nOnEventsDeadPixelNum = 0;
		BadpixelRes.nOnEventsErrorPixelNum = 0;

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
				double dErrorPixelRatio = 0;

				for (uint32_t nIndex = 0; nIndex < nPeakNum; nIndex++)
				{
					uint32_t nValue = m_RawDataContainer[Peak->OnEventsPeakPos[Peak->nOnEventsPeakNumber - 1 - nIndex]].GetData(nRows, nCols);
					if (0 == nValue)
					{
						++dDeadPixelRatio;
					}
					else if (OFF_EVENT_FLAG == nValue)
					{
						++dErrorPixelRatio;
					}
				}
				dDeadPixelRatio /= nPeakNum;
				dErrorPixelRatio /= nPeakNum;

				if (dDeadPixelRatio > m_AlgorithmThre.dDeadPixelThre)
				{
					BadpixelRes.OnEventsBadPixelMask.BadPixelNum++;
					BadpixelRes.OnEventsBadPixelMask.LocalData.push_back({ nRows, nCols });
					BadpixelRes.OnEventsBadPixelMask.Flag.push_back(DVS_DEAD_PIXEL_FLAG);
					BadpixelRes.nOnEventsDeadPixelNum++;
					BadPixelMask[nRows][nCols] = DVS_DEAD_PIXEL_FLAG;
				}
				else if (dErrorPixelRatio > m_AlgorithmThre.dErrorPixelThre)
				{
					BadpixelRes.OnEventsBadPixelMask.BadPixelNum++;
					BadpixelRes.OnEventsBadPixelMask.LocalData.push_back({ nRows, nCols });
					BadpixelRes.OnEventsBadPixelMask.Flag.push_back(DVS_ERROR_PIXEL_FLAG);
					BadpixelRes.nOnEventsErrorPixelNum++;
					BadPixelMask[nRows][nCols] = DVS_ERROR_PIXEL_FLAG;
				}
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
					Search.push({ nRows, nCols });
					while (!Search.empty())
					{
						Local temp = Search.top();
						Search.pop();
						BadPixelMask[temp.x][temp.y] = ConnectedAreaFlag;
						AreaSize++;
						for (uint32_t nTempRows = temp.x - 1; nTempRows <= temp.x + 1; nTempRows++)
						{
							if (nTempRows < m_nTotalRow)
							{
								for (uint32_t nTempCols = temp.y - 1; nTempCols <= temp.y + 1; nTempCols++)
								{
									if (nTempCols < m_nTotalCol && BadPixelMask[nTempRows][nTempCols] != 0 && BadPixelMask[nTempRows][nTempCols] < ConnectedAreaFlag)
									{
										Search.push({ nTempRows , nTempCols });
									}
								}
							}
						}
					}
					if (AreaSize > 1)
					{
						BadpixelRes.nOnEventsClusterNum++;
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
			else if(nValue == OFF_EVENT_FLAG)
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
	return DVS_MP_ALGORITHM_VERSION;
}

uint32_t CAlpDVSMPAlgorithm::GetErrCode()
{
	return m_nErrCode;
}
