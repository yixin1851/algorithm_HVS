#ifdef API_C_TYPE_INTERFACE
#include "AlpMPAlgoCTypeInterface.h"
#include "AlpMPAlgoInterface.h"

HANDLE __stdcall InitHandleDVS(SensorType Sensortype, PixelFormatType Pixelformat, int code)
{
	CAlpDVSMPAlgoInterface * pInterface = CreateDVSAlgoInterface(Sensortype, "", Pixelformat, code);

	return reinterpret_cast<HANDLE>(pInterface);
}

void __stdcall DeleteHandleDVS(HANDLE h)
{
	if (h)
	{
		delete reinterpret_cast<CAlpDVSMPAlgoInterface *>(h);
		h = nullptr;
	}
}

uint32_t __stdcall ImportRawDataDVS(HANDLE h, uint8_t* pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber)
{
	if (h)
	{
		bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->ImportRawData(pRawData, nLens, nIndexStart, nNumber);
		if (bRet)
		{
			return TEST_NO_ERROR;
		}
		else
		{
			return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
		}
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall EventsNumberCountDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, CEventsNumberCountData* EventsNumberCountRes)
{
	if (h)
	{
		DVSEventsNumberCountType res;
		bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->EventsNumberCount(nIndexStart, nNumber, res);

		if (bRet)
		{
			if (res.nDataNumber > MAX_DATA_NUMBER)
			{
				return BEYOND_MAX_RES_NUM;
			}

			for (uint32_t nIndex = 0; nIndex < res.nDataNumber; nIndex++)
			{
				for (uint32_t nChannel = 0; nChannel < SubFrameIndex::All + 1; nChannel++)
				{
					EventsNumberCountRes->AllEventsNum[nChannel][nIndex] = res.AllEventsNum[nChannel][nIndex];
					EventsNumberCountRes->NoEventsNum[nChannel][nIndex] = res.NoEventsNum[nChannel][nIndex];
					EventsNumberCountRes->OffEventsNum[nChannel][nIndex] = res.OffEventsNum[nChannel][nIndex];
					EventsNumberCountRes->OnEventsNum[nChannel][nIndex] = res.OnEventsNum[nChannel][nIndex];
				}
			}

			return TEST_NO_ERROR;
		}
		else
		{
			return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
		}
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall StationaryNoiseDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, CStationaryNoiseData* StationaryNoiseRes)
{
	if (h)
	{
		DVSStationaryNoiseType res;
		bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->StationaryNoise(nIndexStart, nNumber, res);

		if (bRet)
		{
			StationaryNoiseRes->dStationaryNoiseMeanOn = res.dStationaryNoiseMeanOn;
			StationaryNoiseRes->dStationaryNoiseMeanOff = res.dStationaryNoiseMeanOff;
			StationaryNoiseRes->dStationaryNoiseMeanAll = res.dStationaryNoiseMeanAll;
			StationaryNoiseRes->dStationaryNoiseStdOn = res.dStationaryNoiseStdOn;
			StationaryNoiseRes->dStationaryNoiseStdOff = res.dStationaryNoiseStdOff;
			StationaryNoiseRes->dStationaryNoiseStdAll = res.dStationaryNoiseStdAll;
			StationaryNoiseRes->dStationaryColSNoise = res.dStationaryColTNoise;
			StationaryNoiseRes->dStationaryRowSNoise = res.dStationaryRowTNoise;

			return TEST_NO_ERROR;
		}
		else
		{
			return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
		}
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall StationaryUniformityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, CStationaryUniformityData* UniformityRes)
{
	if (h)
	{
		DVSStationaryUniformityType res;
		bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->StationaryUniformity(nIndexStart, nNumber, res);

		if (bRet)
		{
			UniformityRes->UniformityRatio = res.UniformityRatio;
			return TEST_NO_ERROR;
		}
		else
		{
			return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
		}
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall HotPixelDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, CHotpixelData* HotpixelRes)
{
	if (h)
	{
		DVSHotpixelType res;
		bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->HotPixel(nIndexStart, nNumber, res);

		if (bRet)
		{
			HotpixelRes->HotLineNum = res.HotLineNum;
			HotpixelRes->HotPixelNum = res.HotPixelNum;
			HotpixelRes->SingletNum = res.SingletNum;
			HotpixelRes->CoupletNum = res.CoupletNum;
			HotpixelRes->TripletNum = res.TripletNum;
			HotpixelRes->FourConnectedNum = res.FourConnectedNum;
			HotpixelRes->ClusterNum = res.ClusterNum;

			return TEST_NO_ERROR;
		}
		else
		{
			return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
		}
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall FindPeakDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, CPeakInfo* Peak, DVSLightTrigerType Light)
{
	if (h)
	{
		DVSPeakInfo res;

		bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->FindPeak(nIndexStart, nNumber, nPeakNum, res, Light);

		if (bRet)
		{
			if (res.nOffEventsPeakNumber > MAX_DATA_NUMBER || res.nOnEventsPeakNumber > MAX_DATA_NUMBER)
			{
				return BEYOND_MAX_RES_NUM;
			}

			Peak->nOffEventsPeakNumber = res.nOffEventsPeakNumber;
			Peak->nOnEventsPeakNumber = res.nOnEventsPeakNumber;

			for (uint32_t nIndex = 0; nIndex < res.nOffEventsPeakNumber; nIndex++)
			{
				Peak->OffEventsPeakPos[nIndex] = res.OffEventsPeakPos[nIndex];
			}

			for (uint32_t nIndex = 0; nIndex < res.nOnEventsPeakNumber; nIndex++)
			{
				Peak->OnEventsPeakPos[nIndex] = res.OnEventsPeakPos[nIndex];
			}

			return TEST_NO_ERROR;
		}
		else
		{
			return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
		}
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall ImageContrastSensitivityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, DVSLightTrigerType Light, CImageContrastSensitivityData* ImageContrastSensitivityRes)
{
	if (h)
	{
		DVSImageContrastSensitivityType res;
		bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->ImageContrastSensitivity(nIndexStart, nNumber, nullptr, nPeakNum, Light, res);

		if (bRet)
		{
			ImageContrastSensitivityRes->B_Gb_OffEventsRatio = res.B_Gb_OffEventsRatio;
			ImageContrastSensitivityRes->B_Gb_OnEventsRatio = res.B_Gb_OnEventsRatio;
			ImageContrastSensitivityRes->Gr_Gb_OffEventsRatio = res.Gr_Gb_OffEventsRatio;
			ImageContrastSensitivityRes->Gr_Gb_OnEventsRatio = res.Gr_Gb_OnEventsRatio;
			ImageContrastSensitivityRes->R_Gb_OffEventsRatio = res.R_Gb_OffEventsRatio;
			ImageContrastSensitivityRes->R_Gb_OnEventsRatio = res.R_Gb_OnEventsRatio;

			for (uint32_t nChannel = 0; nChannel < SubFrameIndex::All + 1; nChannel++)
			{
				ImageContrastSensitivityRes->OffEventsRatio[nChannel] = res.OffEventsRatio[nChannel];
				ImageContrastSensitivityRes->OnEventsRatio[nChannel] = res.OnEventsRatio[nChannel];
			}

			return TEST_NO_ERROR;
		}
		else
		{
			return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
		}
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall AccompaniedPeakAndDelayedPeakDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, DVSLightTrigerType Light, CAccompaniedPeakAndDelayedPeakData* AccompaniedPeakAndDelayedPeakRes)
{
	if (h)
	{
		DVSAccompaniedPeakAndDelayedPeakType res;
		bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->AccompaniedPeakAndDelayedPeak(nIndexStart, nNumber, nullptr, nPeakNum, Light, res);

		if (bRet)
		{
			for (uint32_t nChannel = 0; nChannel < SubFrameIndex::All + 1; nChannel++)
			{
				AccompaniedPeakAndDelayedPeakRes->dAccompaniedPeakOffEventsRatio[nChannel] = res.dAccompaniedPeakOffEventsRatio[nChannel];
				AccompaniedPeakAndDelayedPeakRes->dAccompaniedPeakOnEventsRatio[nChannel] = res.dAccompaniedPeakOnEventsRatio[nChannel];
				AccompaniedPeakAndDelayedPeakRes->dDelayedPeakOffEventsRatio[nChannel] = res.dDelayedPeakOffEventsRatio[nChannel];
				AccompaniedPeakAndDelayedPeakRes->dDelayedPeakOnEventsRatio[nChannel] = res.dDelayedPeakOnEventsRatio[nChannel];
			}

			return TEST_NO_ERROR;
		}
		else
		{
			return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
		}
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall SpatialResponseUniformityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, uint32_t nPeakNum, DVSLightTrigerType Light, CSpatialResponseUniformityData* SpatialResponseUniformityRes)
{
	if (h)
	{
		DVSSpatialResponseUniformityType res;
		bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->SpatialResponseUniformity(nIndexStart, nNumber, ROI, nullptr, nPeakNum, Light, res);

		if (bRet)
		{
			for (uint32_t nChannel = 0; nChannel < SubFrameIndex::All + 1; nChannel++)
			{
				SpatialResponseUniformityRes->dOffEventsUniformityRatio[nChannel] = res.dOffEventsUniformityRatio[nChannel];
				SpatialResponseUniformityRes->dOnEventsUniformityRatio[nChannel] = res.dOnEventsUniformityRatio[nChannel];
			}

			return TEST_NO_ERROR;
		}
		else
		{
			return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
		}
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall BadPixelDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, DVSLightTrigerType Light, CDVSBadpixelData* BadpixelRes)
{
	if (h)
	{
		DVSBadpixelType res;
		bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->BadPixel(nIndexStart, nNumber, nullptr, nPeakNum, Light, res);

		if (bRet)
		{
			BadpixelRes->nOffEventsClusterNum = res.nOffEventsClusterNum;
			BadpixelRes->nOffEventsDeadPixelNum = res.nOffEventsDeadPixelNum;
			BadpixelRes->nOnEventsClusterNum = res.nOnEventsClusterNum;
			BadpixelRes->nOnEventsDeadPixelNum = res.nOnEventsDeadPixelNum;

			return TEST_NO_ERROR;
		}
		else
		{
			return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
		}
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall ShowDVS(HANDLE h, uint32_t nIndex, uint8_t NoEventFlag, uint8_t OnEventFlag, uint8_t OffEventFlag, uint8_t * ImgData)
{
	if (h)
	{
		ImgType res;
		bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->Show(nIndex, NoEventFlag, OnEventFlag, OffEventFlag, res);

		if (bRet)
		{
			uint32_t nTotalRow = 0, nTotalCol = 0;
			reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetRawDataSize(nTotalRow, nTotalCol);

			for (uint32_t nRows = 0; nRows < nTotalRow; nRows++)
			{
				for (uint32_t nCols = 0; nCols < nTotalCol; nCols++)
				{
					ImgData[nRows * nTotalCol + nCols] = res[nRows][nCols];
				}
			}
			return TEST_NO_ERROR;
		}
		else
		{
			return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
		}
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall SetMultiThreadEnableDVS(HANDLE h, bool bEnable)
{
	if (h)
	{
		reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->SetMultiThreadEnable(bEnable);		
		return TEST_NO_ERROR;
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall SetAlgorithmThreDVS(HANDLE h, CDVSAlgorithmThre* AlgoThre)
{
	if (h)
	{
		DVSAlgorithmThre res;

		res.dHotPixelThre = AlgoThre->dHotPixelThre;
		res.dHotLineThre = AlgoThre->dHotLineThre;
		res.dDeadPixelThre = AlgoThre->dDeadPixelThre;
		res.nPeakCycle = AlgoThre->nPeakCycle;
		res.nStationaryUniformityRowBlockNum = AlgoThre->nStationaryUniformityRowBlockNum;
		res.nStationaryUniformityColBlockNum = AlgoThre->nStationaryUniformityColBlockNum;
		res.nSpatialResponseUniformityRowBlockNum = AlgoThre->nSpatialResponseUniformityRowBlockNum;
		res.nSpatialResponseUniformityColBlockNum = AlgoThre->nSpatialResponseUniformityColBlockNum;

		reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->SetAlgorithmThre(res);
		return TEST_NO_ERROR;
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall GetAlgorithmThreDVS(HANDLE h, CDVSAlgorithmThre* AlgoThre)
{
	if (h)
	{
		DVSAlgorithmThre res;

		res = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetAlgorithmThre();

		AlgoThre->dDeadPixelThre = res.dDeadPixelThre;
		AlgoThre->dHotLineThre = res.dHotLineThre;
		AlgoThre->dHotPixelThre = res.dHotPixelThre;
		AlgoThre->nPeakCycle = res.nPeakCycle;
		AlgoThre->nSpatialResponseUniformityColBlockNum = res.nSpatialResponseUniformityColBlockNum;
		AlgoThre->nSpatialResponseUniformityRowBlockNum = res.nSpatialResponseUniformityRowBlockNum;
		AlgoThre->nStationaryUniformityColBlockNum = res.nStationaryUniformityColBlockNum;
		AlgoThre->nStationaryUniformityRowBlockNum = res.nStationaryUniformityRowBlockNum;

		return TEST_NO_ERROR;
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall GetDataNumDVS(HANDLE h, uint32_t * nDataNum)
{
	if (h)
	{
		uint32_t res;
		res = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetDataNum();
		*nDataNum = res;
		return TEST_NO_ERROR;
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall GetRawDataSizeDVS(HANDLE h, uint32_t* nRow, uint32_t* nCol)
{
	if (h)
	{
		reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetRawDataSize(*nRow, *nCol);
		return TEST_NO_ERROR;
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}

uint32_t __stdcall AlpGetVersionDVS(HANDLE h, char * ver, uint32_t nLen)
{
	if (h)
	{
		std::string strVer = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetVersion();

		uint32_t nRealLens = nLen <= strVer.size() + 1 ? nLen : strVer.size() + 1;
		strcpy_s(ver, nRealLens, strVer.c_str());
		ver[nRealLens] = 0;

		return TEST_NO_ERROR;
	}
	else
	{
		return ALGO_HANDLE_ERROR;
	}
}
#endif
