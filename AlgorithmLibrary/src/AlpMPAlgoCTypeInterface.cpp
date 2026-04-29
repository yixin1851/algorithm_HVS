#ifdef API_C_TYPE_INTERFACE
#include "AlpMPAlgoCTypeInterface.h"
#include <windows.h>
#include "AlpMPAlgoInterface.h"

HANDLE __stdcall InitHandleDVS(SensorType Sensortype, char *strLogDir, PixelFormatType Pixelformat, int code) {
    CAlpDVSMPAlgoInterface *pInterface = CreateDVSAlgoInterface(Sensortype, strLogDir, Pixelformat, code);
    return reinterpret_cast<HANDLE>(pInterface);
}

void __stdcall DeleteHandleDVS(HANDLE h) {
    if (h) {
        delete reinterpret_cast<CAlpDVSMPAlgoInterface *>(h);
        h = nullptr;
    }
}

uint32_t __stdcall ImportRawDataDVS(HANDLE h, uint8_t *pRawData, uint64_t nLens, uint32_t nIndexStart,
                                    uint32_t nNumber) {
    if (h) {
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->ImportRawData(pRawData, nLens, nIndexStart, nNumber);
        if (bRet) {
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall EventsNumberCountDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                        DVSEventsNumberCountType *EventsNumberCountRes) {
    if (h) {
        DVSEventsNumberCountType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->EventsNumberCount(nIndexStart, nNumber, res);

        if (bRet) {
            if (res.nDataNumber > MAX_DATA_NUMBER) {
                return BEYOND_MAX_RES_NUM;
            }
            EventsNumberCountRes->nDataNumber = res.nDataNumber;
            for (uint32_t nChannel = 0; nChannel < SubFrameIndex::All + 1; nChannel++) {
                EventsNumberCountRes->AllEventsNum[nChannel] = res.AllEventsNum[nChannel];
                EventsNumberCountRes->NoEventsNum[nChannel] = res.NoEventsNum[nChannel];
                EventsNumberCountRes->OffEventsNum[nChannel] = res.OffEventsNum[nChannel];
                EventsNumberCountRes->OnEventsNum[nChannel] = res.OnEventsNum[nChannel];
            }

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall StationaryNoiseDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                      DVSStationaryNoiseType *StationaryNoiseRes) {
    if (h) {
        DVSStationaryNoiseType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->StationaryNoise(nIndexStart, nNumber, res);

        if (bRet) {
            StationaryNoiseRes->dStationaryNoiseMeanOn = res.dStationaryNoiseMeanOn;
            StationaryNoiseRes->dStationaryNoiseMeanOff = res.dStationaryNoiseMeanOff;
            StationaryNoiseRes->dStationaryNoiseMeanAll = res.dStationaryNoiseMeanAll;
            StationaryNoiseRes->dStationaryNoiseStdOn = res.dStationaryNoiseStdOn;
            StationaryNoiseRes->dStationaryNoiseStdOff = res.dStationaryNoiseStdOff;
            StationaryNoiseRes->dStationaryNoiseStdAll = res.dStationaryNoiseStdAll;
            StationaryNoiseRes->dMaxStationaryNoise = res.dMaxStationaryNoise;
            StationaryNoiseRes->dStationaryColTNoise = res.dStationaryColTNoise;
            StationaryNoiseRes->dStationaryRowTNoise = res.dStationaryRowTNoise;
            StationaryNoiseRes->nFlashFrameNumber = res.nFlashFrameNumber;

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall StationaryUniformityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                           DVSStationaryUniformityType *UniformityRes) {
    if (h) {
        DVSStationaryUniformityType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->StationaryUniformity(nIndexStart, nNumber, res);

        if (bRet) {
            UniformityRes->UniformityBlockData.assign(res.UniformityBlockData.begin(), res.UniformityBlockData.end());
            UniformityRes->UniformityRatio = res.UniformityRatio;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

// 释放函数
void __stdcall HotPixelTypeCDVS_Free(DVSHotpixelTypeC *HotpixelRes) {
    if (HotpixelRes) {
        if (HotpixelRes->HotPixelMask.LocalData) {
            delete[] HotpixelRes->HotPixelMask.LocalData;
            HotpixelRes->HotPixelMask.LocalData = nullptr;
            HotpixelRes->HotPixelMask.LocalDataSize = 0;
        }

        if (HotpixelRes->HotPixelMask.Flag) {
            delete[] HotpixelRes->HotPixelMask.Flag;
            HotpixelRes->HotPixelMask.Flag = nullptr;
            HotpixelRes->HotPixelMask.FlagSize = 0;
        }

        if (HotpixelRes->HotPixelMask.DiffData) {
            delete[] HotpixelRes->HotPixelMask.DiffData;
            HotpixelRes->HotPixelMask.DiffData = nullptr;
            HotpixelRes->HotPixelMask.DiffDataSize = 0;
        }
    }
}

uint32_t __stdcall HotPixelTypeCDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                    DVSHotpixelTypeC *HotpixelRes) {
    // 参数检查
    if (!h) {
        return ALGO_HANDLE_ERROR;
    }
    if (!HotpixelRes) {
        return INVALID_PARAMETER_ERROR;
    }

    DVSHotpixelType res;
    bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->HotPixel(nIndexStart, nNumber, res);

    if (!bRet) {
        return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
    }

    try {
        HotpixelRes->HotLineNum = res.HotLineNum;
        HotpixelRes->HotPixelNum = res.HotPixelNum;
        HotpixelRes->SingletNum = res.SingletNum;
        HotpixelRes->CoupletNum = res.CoupletNum;
        HotpixelRes->TripletNum = res.TripletNum;
        HotpixelRes->FourConnectedNum = res.FourConnectedNum;
        HotpixelRes->ClusterNum = res.ClusterNum;
        HotpixelRes->HotPixelMask.BadPixelNum = res.HotPixelMask.BadPixelNum;
        HotpixelRes->MaxClusterSize = res.MaxClusterSize;
        HotpixelRes->SlidingWindowMaxHotPixelNum = res.SlidingWindowMaxHotPixelNum;

        // LocalData
        HotpixelRes->HotPixelMask.LocalDataSize = res.HotPixelMask.LocalData.size();
        if (HotpixelRes->HotPixelMask.LocalDataSize > 0) {
            HotpixelRes->HotPixelMask.LocalData =
                    new Local[HotpixelRes->HotPixelMask.LocalDataSize];
            std::copy(res.HotPixelMask.LocalData.begin(),
                      res.HotPixelMask.LocalData.end(),
                      HotpixelRes->HotPixelMask.LocalData);
        } else {
            HotpixelRes->HotPixelMask.LocalData = nullptr;
        }

        // Flag
        HotpixelRes->HotPixelMask.FlagSize = res.HotPixelMask.Flag.size();
        if (HotpixelRes->HotPixelMask.FlagSize > 0) {
            HotpixelRes->HotPixelMask.Flag =
                    new uint8_t[HotpixelRes->HotPixelMask.FlagSize];
            std::copy(res.HotPixelMask.Flag.begin(),
                      res.HotPixelMask.Flag.end(),
                      HotpixelRes->HotPixelMask.Flag);
        } else {
            HotpixelRes->HotPixelMask.Flag = nullptr;
        }

        // DiffData
        HotpixelRes->HotPixelMask.DiffDataSize = res.HotPixelMask.DiffData.size();
        if (HotpixelRes->HotPixelMask.DiffDataSize > 0) {
            HotpixelRes->HotPixelMask.DiffData =
                    new uint32_t[HotpixelRes->HotPixelMask.DiffDataSize];
            std::copy(res.HotPixelMask.DiffData.begin(),
                      res.HotPixelMask.DiffData.end(),
                      HotpixelRes->HotPixelMask.DiffData);
        } else {
            HotpixelRes->HotPixelMask.DiffData = nullptr;
        }

        return TEST_NO_ERROR;
    } catch (const std::bad_alloc &) {
        // 清理已分配的内存
        HotPixelTypeCDVS_Free(HotpixelRes);
        return MEMORY_ALLOCATION_ERROR;
    }
}

uint32_t __stdcall FindPeakDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, DVSPeakInfo *Peak,
                               DVSLightTrigerType Light) {
    if (h) {
        DVSPeakInfo res;

        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->FindPeak(nIndexStart, nNumber, nPeakNum, res, Light);

        if (bRet) {
            if (res.nOffEventsPeakNumber > MAX_DATA_NUMBER || res.nOnEventsPeakNumber > MAX_DATA_NUMBER) {
                return BEYOND_MAX_RES_NUM;
            }

            Peak->nOffEventsPeakNumber = res.nOffEventsPeakNumber;
            Peak->nOnEventsPeakNumber = res.nOnEventsPeakNumber;
            Peak->OffEventsPeakPos = res.OffEventsPeakPos;
            Peak->OnEventsPeakPos = res.OnEventsPeakPos;
            // for (uint32_t nIndex = 0; nIndex < res.nOffEventsPeakNumber; nIndex++) {
            //     Peak->OffEventsPeakPos[nIndex] = res.OffEventsPeakPos[nIndex];
            // }
            //
            // for (uint32_t nIndex = 0; nIndex < res.nOnEventsPeakNumber; nIndex++) {
            //     Peak->OnEventsPeakPos[nIndex] = res.OnEventsPeakPos[nIndex];
            // }

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ImageContrastSensitivityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                               DVSPeakInfo *Peak,
                                               uint32_t nPeakNum, DVSLightTrigerType Light,
                                               DVSImageContrastSensitivityType *
                                               ImageContrastSensitivityRes) {
    if (h) {
        DVSImageContrastSensitivityType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->ImageContrastSensitivity(
            nIndexStart, nNumber, Peak, nPeakNum, Light, res);

        if (bRet) {
            ImageContrastSensitivityRes->R_Gb_OnEventsRatio = res.R_Gb_OnEventsRatio;
            ImageContrastSensitivityRes->B_Gb_OnEventsRatio = res.B_Gb_OnEventsRatio;
            ImageContrastSensitivityRes->Gr_Gb_OnEventsRatio = res.Gr_Gb_OnEventsRatio;

            ImageContrastSensitivityRes->R_Gb_OffEventsRatio = res.R_Gb_OffEventsRatio;
            ImageContrastSensitivityRes->B_Gb_OffEventsRatio = res.B_Gb_OffEventsRatio;
            ImageContrastSensitivityRes->Gr_Gb_OffEventsRatio = res.Gr_Gb_OffEventsRatio;

            for (uint32_t nChannel = 0; nChannel < SubFrameIndex::All + 1; nChannel++) {
                ImageContrastSensitivityRes->OffEventsRatio[nChannel] = res.OffEventsRatio[nChannel];
                ImageContrastSensitivityRes->OnEventsRatio[nChannel] = res.OnEventsRatio[nChannel];
            }

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall AccompaniedPeakAndDelayedPeakDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                                    DVSPeakInfo *Peak, uint32_t nPeakNum,
                                                    DVSLightTrigerType Light,
                                                    DVSAccompaniedPeakAndDelayedPeakType *
                                                    AccompaniedPeakAndDelayedPeakRes) {
    if (h) {
        DVSAccompaniedPeakAndDelayedPeakType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->AccompaniedPeakAndDelayedPeak(
            nIndexStart, nNumber, Peak, nPeakNum, Light, res);

        if (bRet) {
            for (uint32_t nChannel = 0; nChannel < SubFrameIndex::All + 1; nChannel++) {
                AccompaniedPeakAndDelayedPeakRes->dAccompaniedPeakOffEventsRatio[nChannel] = res.
                        dAccompaniedPeakOffEventsRatio[nChannel];
                AccompaniedPeakAndDelayedPeakRes->dAccompaniedPeakOnEventsRatio[nChannel] = res.
                        dAccompaniedPeakOnEventsRatio[nChannel];
                AccompaniedPeakAndDelayedPeakRes->dDelayedPeakOffEventsRatio[nChannel] = res.dDelayedPeakOffEventsRatio[
                    nChannel];
                AccompaniedPeakAndDelayedPeakRes->dDelayedPeakOnEventsRatio[nChannel] = res.dDelayedPeakOnEventsRatio[
                    nChannel];
            }

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SpatialResponseUniformityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                                ROIArea *ROI, DVSPeakInfo *Peak, uint32_t nPeakNum,
                                                DVSLightTrigerType Light,
                                                DVSSpatialResponseUniformityType *
                                                SpatialResponseUniformityRes) {
    if (h) {
        DVSSpatialResponseUniformityType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->SpatialResponseUniformity(
            nIndexStart, nNumber, ROI, Peak, nPeakNum, Light, res);

        if (bRet) {
            for (uint32_t nChannel = 0; nChannel < SubFrameIndex::All + 1; nChannel++) {
                SpatialResponseUniformityRes->dOnEventsUniformityRatio[nChannel] = res.dOnEventsUniformityRatio[
                    nChannel];
                SpatialResponseUniformityRes->dOffEventsUniformityRatio[nChannel] = res.dOffEventsUniformityRatio[
                    nChannel];
            }
            SpatialResponseUniformityRes->OnEventsUniformityBlockData->assign(
                res.OnEventsUniformityBlockData->begin(), res.OnEventsUniformityBlockData->end());
            SpatialResponseUniformityRes->OffEventsUniformityBlockData->assign(
                res.OffEventsUniformityBlockData->begin(), res.OffEventsUniformityBlockData->end());

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall BadPixelTypeCDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, DVSPeakInfo *Peak,
                                    uint32_t nPeakNum, DVSLightTrigerType Light,
                                    DVSBadpixelTypeC *BadpixelRes) {
    // 参数检查
    if (!h) {
        return ALGO_HANDLE_ERROR;
    }
    if (!BadpixelRes) {
        return INVALID_PARAMETER_ERROR;
    }

    DVSBadpixelType res;
    bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->BadPixel(
        nIndexStart, nNumber, Peak, nPeakNum, Light, res);

    if (!bRet) {
        return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
    }

    try {
        // ========== OffEvents 部分 ==========
        BadpixelRes->nOffEventsClusterNum = res.nOffEventsClusterNum;
        BadpixelRes->nOffEventsDeadPixelNum = res.nOffEventsDeadPixelNum;
        BadpixelRes->nOffEventsDeadLineNum = res.nOffEventsDeadLineNum;
        BadpixelRes->OffEventsBadPixelMask.BadPixelNum = res.OffEventsBadPixelMask.BadPixelNum;
        BadpixelRes->nOffEventsMaxClusterSize = res.nOffEventsMaxClusterSize;
        BadpixelRes->nOffEventsSlidingWindowMaxDeadPixelNum = res.nOffEventsSlidingWindowMaxDeadPixelNum;

        // LocalData
        BadpixelRes->OffEventsBadPixelMask.LocalDataSize = res.OffEventsBadPixelMask.LocalData.size();
        if (BadpixelRes->OffEventsBadPixelMask.LocalDataSize > 0) {
            BadpixelRes->OffEventsBadPixelMask.LocalData =
                    new Local[BadpixelRes->OffEventsBadPixelMask.LocalDataSize];
            std::copy(res.OffEventsBadPixelMask.LocalData.begin(),
                      res.OffEventsBadPixelMask.LocalData.end(),
                      BadpixelRes->OffEventsBadPixelMask.LocalData);
        } else {
            BadpixelRes->OffEventsBadPixelMask.LocalData = nullptr;
        }

        // Flag
        BadpixelRes->OffEventsBadPixelMask.FlagSize = res.OffEventsBadPixelMask.Flag.size();
        if (BadpixelRes->OffEventsBadPixelMask.FlagSize > 0) {
            BadpixelRes->OffEventsBadPixelMask.Flag =
                    new uint8_t[BadpixelRes->OffEventsBadPixelMask.FlagSize];
            std::copy(res.OffEventsBadPixelMask.Flag.begin(),
                      res.OffEventsBadPixelMask.Flag.end(),
                      BadpixelRes->OffEventsBadPixelMask.Flag);
        } else {
            BadpixelRes->OffEventsBadPixelMask.Flag = nullptr;
        }

        // DiffData
        BadpixelRes->OffEventsBadPixelMask.DiffDataSize = res.OffEventsBadPixelMask.DiffData.size();
        if (BadpixelRes->OffEventsBadPixelMask.DiffDataSize > 0) {
            BadpixelRes->OffEventsBadPixelMask.DiffData =
                    new uint32_t[BadpixelRes->OffEventsBadPixelMask.DiffDataSize];
            std::copy(res.OffEventsBadPixelMask.DiffData.begin(),
                      res.OffEventsBadPixelMask.DiffData.end(),
                      BadpixelRes->OffEventsBadPixelMask.DiffData);
        } else {
            BadpixelRes->OffEventsBadPixelMask.DiffData = nullptr;
        }

        // ========== OnEvents 部分 ==========
        BadpixelRes->nOnEventsClusterNum = res.nOnEventsClusterNum;
        BadpixelRes->nOnEventsDeadPixelNum = res.nOnEventsDeadPixelNum;
        BadpixelRes->nOnEventsDeadLineNum = res.nOnEventsDeadLineNum;
        BadpixelRes->OnEventsBadPixelMask.BadPixelNum = res.OnEventsBadPixelMask.BadPixelNum;
        BadpixelRes->nOnEventsMaxClusterSize = res.nOnEventsMaxClusterSize;
        BadpixelRes->nOnEventsSlidingWindowMaxDeadPixelNum = res.nOnEventsSlidingWindowMaxDeadPixelNum;

        // LocalData
        BadpixelRes->OnEventsBadPixelMask.LocalDataSize = res.OnEventsBadPixelMask.LocalData.size();
        if (BadpixelRes->OnEventsBadPixelMask.LocalDataSize > 0) {
            BadpixelRes->OnEventsBadPixelMask.LocalData =
                    new Local[BadpixelRes->OnEventsBadPixelMask.LocalDataSize];
            std::copy(res.OnEventsBadPixelMask.LocalData.begin(),
                      res.OnEventsBadPixelMask.LocalData.end(),
                      BadpixelRes->OnEventsBadPixelMask.LocalData);
        } else {
            BadpixelRes->OnEventsBadPixelMask.LocalData = nullptr;
        }

        // Flag
        BadpixelRes->OnEventsBadPixelMask.FlagSize = res.OnEventsBadPixelMask.Flag.size();
        if (BadpixelRes->OnEventsBadPixelMask.FlagSize > 0) {
            BadpixelRes->OnEventsBadPixelMask.Flag =
                    new uint8_t[BadpixelRes->OnEventsBadPixelMask.FlagSize];
            std::copy(res.OnEventsBadPixelMask.Flag.begin(),
                      res.OnEventsBadPixelMask.Flag.end(),
                      BadpixelRes->OnEventsBadPixelMask.Flag);
        } else {
            BadpixelRes->OnEventsBadPixelMask.Flag = nullptr;
        }

        // DiffData
        BadpixelRes->OnEventsBadPixelMask.DiffDataSize = res.OnEventsBadPixelMask.DiffData.size();
        if (BadpixelRes->OnEventsBadPixelMask.DiffDataSize > 0) {
            BadpixelRes->OnEventsBadPixelMask.DiffData =
                    new uint32_t[BadpixelRes->OnEventsBadPixelMask.DiffDataSize];
            std::copy(res.OnEventsBadPixelMask.DiffData.begin(),
                      res.OnEventsBadPixelMask.DiffData.end(),
                      BadpixelRes->OnEventsBadPixelMask.DiffData);
        } else {
            BadpixelRes->OnEventsBadPixelMask.DiffData = nullptr;
        }

        return TEST_NO_ERROR;
    } catch (const std::bad_alloc &) {
        // 清理已分配的内存
        BadPixelTypeCDVS_Free(BadpixelRes);
        return MEMORY_ALLOCATION_ERROR;
    }
}

// 释放函数
void __stdcall BadPixelTypeCDVS_Free(DVSBadpixelTypeC *BadpixelRes) {
    if (!BadpixelRes) return;

    // ========== OffEvents 部分 ==========
    if (BadpixelRes->OffEventsBadPixelMask.LocalData) {
        delete[] BadpixelRes->OffEventsBadPixelMask.LocalData;
        BadpixelRes->OffEventsBadPixelMask.LocalData = nullptr;
        BadpixelRes->OffEventsBadPixelMask.LocalDataSize = 0;
    }

    if (BadpixelRes->OffEventsBadPixelMask.Flag) {
        delete[] BadpixelRes->OffEventsBadPixelMask.Flag;
        BadpixelRes->OffEventsBadPixelMask.Flag = nullptr;
        BadpixelRes->OffEventsBadPixelMask.FlagSize = 0;
    }

    if (BadpixelRes->OffEventsBadPixelMask.DiffData) {
        delete[] BadpixelRes->OffEventsBadPixelMask.DiffData;
        BadpixelRes->OffEventsBadPixelMask.DiffData = nullptr;
        BadpixelRes->OffEventsBadPixelMask.DiffDataSize = 0;
    }

    // ========== OnEvents 部分 ==========
    if (BadpixelRes->OnEventsBadPixelMask.LocalData) {
        delete[] BadpixelRes->OnEventsBadPixelMask.LocalData;
        BadpixelRes->OnEventsBadPixelMask.LocalData = nullptr;
        BadpixelRes->OnEventsBadPixelMask.LocalDataSize = 0;
    }

    if (BadpixelRes->OnEventsBadPixelMask.Flag) {
        delete[] BadpixelRes->OnEventsBadPixelMask.Flag;
        BadpixelRes->OnEventsBadPixelMask.Flag = nullptr;
        BadpixelRes->OnEventsBadPixelMask.FlagSize = 0;
    }

    if (BadpixelRes->OnEventsBadPixelMask.DiffData) {
        delete[] BadpixelRes->OnEventsBadPixelMask.DiffData;
        BadpixelRes->OnEventsBadPixelMask.DiffData = nullptr;
        BadpixelRes->OnEventsBadPixelMask.DiffDataSize = 0;
    }
}

uint32_t __stdcall ShowDVS(HANDLE h, uint32_t nIndex, uint8_t NoEventFlag, uint8_t OnEventFlag,
                           uint8_t OffEventFlag, ImgType *ImgData) {
    if (h) {
        ImgType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->Show(nIndex, NoEventFlag, OnEventFlag, OffEventFlag,
                                                                        res);

        if (bRet) {
            uint32_t nTotalRow = 0, nTotalCol = 0;
            reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetRawDataSize(nTotalRow, nTotalCol);

            for (uint32_t nRows = 0; nRows < nTotalRow; nRows++) {
                for (uint32_t nCols = 0; nCols < nTotalCol; nCols++) {
                    // ImgData[nRows * nTotalCol + nCols] = res[nRows][nCols];
                    ImgData->assign(res.begin(), res.end());
                }
            }
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SetMultiThreadEnableDVS(HANDLE h, bool bEnable) {
    if (h) {
        reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->SetMultiThreadEnable(bEnable);
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SetAlgorithmThreDVS(HANDLE h, DVSAlgorithmThre *AlgoThre) {
    if (h) {
        DVSAlgorithmThre res;
        res.dHotPixelThre = AlgoThre->dHotPixelThre;
        res.dHotLineThre = AlgoThre->dHotLineThre;
        res.dDeadPixelThre = AlgoThre->dDeadPixelThre;
        res.dDeadLineThre = AlgoThre->dDeadLineThre;
        res.nHotPixelClusterSizeThre = AlgoThre->nHotPixelClusterSizeThre;
        res.nDeadPixelClusterSizeThre = AlgoThre->nDeadPixelClusterSizeThre;
        res.nPeakCycle = AlgoThre->nPeakCycle;
        res.nStationaryUniformityRowBlockNum = AlgoThre->nStationaryUniformityRowBlockNum;
        res.nStationaryUniformityColBlockNum = AlgoThre->nStationaryUniformityColBlockNum;
        res.nSpatialResponseUniformityRowBlockNum = AlgoThre->nSpatialResponseUniformityRowBlockNum;
        res.nSpatialResponseUniformityColBlockNum = AlgoThre->nSpatialResponseUniformityColBlockNum;
        res.dFlashRatioThre = AlgoThre->dFlashRatioThre;

        reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->SetAlgorithmThre(res);
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetAlgorithmThreDVS(HANDLE h, DVSAlgorithmThre *AlgoThre) {
    if (h) {
        DVSAlgorithmThre res;
        res = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetAlgorithmThre();
        AlgoThre->dHotPixelThre = res.dHotPixelThre;
        AlgoThre->dHotLineThre = res.dHotLineThre;
        AlgoThre->dDeadPixelThre = res.dDeadPixelThre;
        AlgoThre->dDeadLineThre = res.dDeadLineThre;
        AlgoThre->nHotPixelClusterSizeThre = res.nHotPixelClusterSizeThre;
        AlgoThre->nDeadPixelClusterSizeThre = res.nDeadPixelClusterSizeThre;
        AlgoThre->nPeakCycle = res.nPeakCycle;
        AlgoThre->nStationaryUniformityRowBlockNum = res.nStationaryUniformityRowBlockNum;
        AlgoThre->nStationaryUniformityColBlockNum = res.nStationaryUniformityColBlockNum;
        AlgoThre->nSpatialResponseUniformityRowBlockNum = res.nSpatialResponseUniformityRowBlockNum;
        AlgoThre->nSpatialResponseUniformityColBlockNum = res.nSpatialResponseUniformityColBlockNum;
        AlgoThre->dFlashRatioThre = res.dFlashRatioThre;

        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetDataNumDVS(HANDLE h, uint32_t *nDataNum) {
    if (h) {
        uint32_t res;
        res = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetDataNum();
        *nDataNum = res;
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetActiveAreaDVS(HANDLE h, ROIArea *ROI) {
    if (h) {
        ROIArea tROI;
        tROI = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetActiveArea();
        ROI->Up = tROI.Up;
        ROI->Down = tROI.Down;
        ROI->Left = tROI.Left;
        ROI->Right = tROI.Right;
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SetActiveAreaDVS(HANDLE h, ROIArea ROI) {
    if (h) {
        reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->SetActiveArea(ROI);
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SetLogEnableDVS(HANDLE h, bool bEnable) {
    if (h) {
        reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->SetLogEnable(bEnable);
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetRawDataSizeDVS(HANDLE h, uint32_t *nRow, uint32_t *nCol) {
    if (h) {
        uint32_t tRow = 0;
        uint32_t tCol = 0;
        reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetRawDataSize(tRow, tCol);
        *nRow = tRow;
        *nCol = tCol;
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall AlpGetVersionDVS(HANDLE h, char *ver, uint32_t nLen) {
    if (h) {
        std::string strVer = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetVersion();

        uint32_t nRealLens = nLen <= strVer.size() + 1 ? nLen : strVer.size() + 1;
        strcpy_s(ver, nRealLens, strVer.c_str());
        ver[nRealLens] = 0;

        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall CalcLightIntensityDVS(HANDLE h, double eventPercent,
                               std::vector<double> vecEvent,
                               std::vector<std::pair<double, double> > vecLightIntensity,
                               double &targetLightIntensity) {
    if (h) {
        double tTargetLightIntensity{0.0};
        bool ret = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->CalcLightIntensity(
            eventPercent, vecEvent, vecLightIntensity, tTargetLightIntensity);
        if (ret) {
            targetLightIntensity = tTargetLightIntensity;
        } else {
            return FUNCTION_ERROR;
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

// | ============= APS =============================================================================================== |
HANDLE __stdcall InitHandleAPS(SensorType Sensortype, APSRawType APSRawtype, char *strLogDir, PixelFormatType Pixelformat, int code) {
    CAlpAPSMPAlgoInterface *pInterface = CreateAPSAlgoInterface(Sensortype, APSRawtype, strLogDir, Pixelformat, code);

    return reinterpret_cast<HANDLE>(pInterface);
}

void __stdcall DeleteHandleAPS(HANDLE h) {
    if (h) {
        delete reinterpret_cast<CAlpAPSMPAlgoInterface *>(h);
        h = nullptr;
    }
}

uint32_t __stdcall ImportRawDataAPS(HANDLE h, uint8_t *pRawData, uint64_t nLens, uint32_t nIndexStart,
                                    uint32_t nNumber) {
    if (h) {
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->ImportRawData(pRawData, nLens, nIndexStart, nNumber);
        if (bRet) {
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode(); // GetErrCode?
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall TNoiseAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, APSTNoiseTypeC *apsTNoiseRes) {
    if (!h) {
        return ALGO_HANDLE_ERROR;
    }

    if (!apsTNoiseRes) {
        return INVALID_PARAMETER_ERROR;
    }

    try {
        APSTNoiseType res;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->TNoise(
            nIndexStart,
            nNumber,
            ROI,
            res
        );

        if (bRet) {
            // 分配内存并复制数据
            size_t dataSize = res.SubFrameTNoiseData.size();

            if (dataSize > 0) {
                APSSubFrameTNoiseType *buffer = new APSSubFrameTNoiseType[dataSize];
                std::copy(
                    res.SubFrameTNoiseData.begin(),
                    res.SubFrameTNoiseData.end(),
                    buffer
                );

                apsTNoiseRes->SubFrameTNoiseData = buffer;
                apsTNoiseRes->SubFrameTNoiseDataCount = dataSize;
            } else {
                apsTNoiseRes->SubFrameTNoiseData = nullptr;
                apsTNoiseRes->SubFrameTNoiseDataCount = 0;
            }

            apsTNoiseRes->TNoiseFrame = res.TNoiseFrame;

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } catch (const std::bad_alloc &) {
        return MEMORY_ALLOCATION_ERROR;
    } catch (...) {
        return UNKNOWN_ERROR;
    }
}

// 释放函数
void __stdcall TNoiseAPS_Free(APSTNoiseTypeC *apsTNoiseRes) {
    if (apsTNoiseRes && apsTNoiseRes->SubFrameTNoiseData) {
        delete[] apsTNoiseRes->SubFrameTNoiseData;
        apsTNoiseRes->SubFrameTNoiseData = nullptr;
        apsTNoiseRes->SubFrameTNoiseDataCount = 0;
        apsTNoiseRes->TNoiseFrame = 0.0;
    }
}

uint32_t __stdcall SNoiseAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, APSSNoiseType *APSSNoiseRes) {
    if (h) {
        APSSNoiseType res;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->SNoise(nIndexStart, nNumber, ROI, res);

        if (bRet) {
            APSSNoiseRes->SubFrameSNoiseData = res.SubFrameSNoiseData;
            APSSNoiseRes->SNoiseFrame = res.SNoiseFrame;

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall BadPixelTypeCAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                    APSBadpixelTypeC *BadpixelRes) {
    // 参数检查
    if (!h) {
        return ALGO_HANDLE_ERROR;
    }
    if (!BadpixelRes) {
        return INVALID_PARAMETER_ERROR;
    }

    APSBadpixelType res;
    bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->BadPixel(nIndexStart, nNumber, ROI, res);

    if (!bRet) {
        return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
    }

    try {
        BadpixelRes->BadPixelNum = res.BadPixelNum;
        BadpixelRes->SingletNum = res.SingletNum;
        BadpixelRes->CoupletNum = res.CoupletNum;
        BadpixelRes->LadderNum = res.LadderNum;
        BadpixelRes->ClusterNum = res.ClusterNum;
        BadpixelRes->MaxClusterSize = res.MaxClusterSize;

        // BadPixelMask
        BadpixelRes->BadPixelMask.BadPixelNum = res.BadPixelMask.BadPixelNum;

        // LocalData
        BadpixelRes->BadPixelMask.LocalDataSize = res.BadPixelMask.LocalData.size();
        if (BadpixelRes->BadPixelMask.LocalDataSize > 0) {
            BadpixelRes->BadPixelMask.LocalData = new Local[BadpixelRes->BadPixelMask.LocalDataSize];
            std::copy(res.BadPixelMask.LocalData.begin(),
                      res.BadPixelMask.LocalData.end(),
                      BadpixelRes->BadPixelMask.LocalData);
        } else {
            BadpixelRes->BadPixelMask.LocalData = nullptr;
        }

        // Flag
        BadpixelRes->BadPixelMask.FlagSize = res.BadPixelMask.Flag.size();
        if (BadpixelRes->BadPixelMask.FlagSize > 0) {
            BadpixelRes->BadPixelMask.Flag = new uint8_t[BadpixelRes->BadPixelMask.FlagSize];
            std::copy(res.BadPixelMask.Flag.begin(),
                      res.BadPixelMask.Flag.end(),
                      BadpixelRes->BadPixelMask.Flag);
        } else {
            BadpixelRes->BadPixelMask.Flag = nullptr;
        }

        // DiffData
        BadpixelRes->BadPixelMask.DiffDataSize = res.BadPixelMask.DiffData.size();
        if (BadpixelRes->BadPixelMask.DiffDataSize > 0) {
            BadpixelRes->BadPixelMask.DiffData = new uint32_t[BadpixelRes->BadPixelMask.DiffDataSize];
            std::copy(res.BadPixelMask.DiffData.begin(),
                      res.BadPixelMask.DiffData.end(),
                      BadpixelRes->BadPixelMask.DiffData);
        } else {
            BadpixelRes->BadPixelMask.DiffData = nullptr;
        }

        // SubFrameBadpixelData
        BadpixelRes->SubFrameBadpixelDataSize = res.SubFrameBadpixelData.size();
        if (BadpixelRes->SubFrameBadpixelDataSize > 0) {
            BadpixelRes->SubFrameBadpixelData = new APSSubFrameBadpixelTypeC[BadpixelRes->SubFrameBadpixelDataSize];

            // 初始化所有指针为 nullptr
            for (size_t i = 0; i < BadpixelRes->SubFrameBadpixelDataSize; i++) {
                BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalData = nullptr;
                BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.Flag = nullptr;
                BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffData = nullptr;
            }

            for (size_t i = 0; i < BadpixelRes->SubFrameBadpixelDataSize; i++) {
                BadpixelRes->SubFrameBadpixelData[i].BadPixelNum = res.SubFrameBadpixelData[i].BadPixelNum;
                BadpixelRes->SubFrameBadpixelData[i].SingletNum = res.SubFrameBadpixelData[i].SingletNum;
                BadpixelRes->SubFrameBadpixelData[i].CoupletNum = res.SubFrameBadpixelData[i].CoupletNum;
                BadpixelRes->SubFrameBadpixelData[i].ClusterNum = res.SubFrameBadpixelData[i].ClusterNum;
                BadpixelRes->SubFrameBadpixelData[i].DefectRowNum = res.SubFrameBadpixelData[i].DefectRowNum;
                BadpixelRes->SubFrameBadpixelData[i].DefectColNum = res.SubFrameBadpixelData[i].DefectColNum;
                BadpixelRes->SubFrameBadpixelData[i].MaxClusterSize = res.SubFrameBadpixelData[i].MaxClusterSize;

                // BadPixelMask.BadPixelNum
                BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.BadPixelNum = res.SubFrameBadpixelData[i].BadPixelMask
                        .BadPixelNum;

                // LocalData
                BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalDataSize =
                        res.SubFrameBadpixelData[i].BadPixelMask.LocalData.size();
                if (BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalDataSize > 0) {
                    BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalData =
                            new Local[BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalDataSize];
                    std::copy(res.SubFrameBadpixelData[i].BadPixelMask.LocalData.begin(),
                              res.SubFrameBadpixelData[i].BadPixelMask.LocalData.end(),
                              BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalData);
                }

                // Flag
                BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.FlagSize =
                        res.SubFrameBadpixelData[i].BadPixelMask.Flag.size();
                if (BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.FlagSize > 0) {
                    BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.Flag =
                            new uint8_t[BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.FlagSize];
                    std::copy(res.SubFrameBadpixelData[i].BadPixelMask.Flag.begin(),
                              res.SubFrameBadpixelData[i].BadPixelMask.Flag.end(),
                              BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.Flag);
                }

                // DiffData
                BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffDataSize =
                        res.SubFrameBadpixelData[i].BadPixelMask.DiffData.size();
                if (BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffDataSize > 0) {
                    BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffData =
                            new uint32_t[BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffDataSize];
                    std::copy(res.SubFrameBadpixelData[i].BadPixelMask.DiffData.begin(),
                              res.SubFrameBadpixelData[i].BadPixelMask.DiffData.end(),
                              BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffData);
                }
            }
        } else {
            BadpixelRes->SubFrameBadpixelData = nullptr;
        }

        return TEST_NO_ERROR;
    } catch (const std::bad_alloc &) {
        // 发生内存分配失败，清理已分配的内存
        BadPixelTypeCAPS_Free(BadpixelRes);
        return MEMORY_ALLOCATION_ERROR;
    }
}

// 释放函数
void __stdcall BadPixelTypeCAPS_Free(APSBadpixelTypeC *BadpixelRes) {
    auto customFree = [&](auto *&array, size_t &arraySize)-> void {
        if (array) {
            delete[] array;
            array = nullptr;
            arraySize = 0;
        }
    };
    if (BadpixelRes != nullptr) {
        customFree(BadpixelRes->BadPixelMask.LocalData, BadpixelRes->BadPixelMask.LocalDataSize);
        customFree(BadpixelRes->BadPixelMask.Flag, BadpixelRes->BadPixelMask.FlagSize);
        customFree(BadpixelRes->BadPixelMask.DiffData, BadpixelRes->BadPixelMask.DiffDataSize);

        if (BadpixelRes->SubFrameBadpixelData != nullptr) {
            int size = BadpixelRes->SubFrameBadpixelDataSize;
            for (size_t i = 0; i < size; i++) {
                customFree(BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalData,
                           BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalDataSize);
                customFree(BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.Flag,
                           BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.FlagSize);
                customFree(BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffData,
                           BadpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffDataSize);
            }

            customFree(BadpixelRes->SubFrameBadpixelData, BadpixelRes->SubFrameBadpixelDataSize);
        }
    }
}

uint32_t __stdcall HotPixelTypeCAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                    ROIArea *ROI, APSBadpixelTypeC *HotpixelRes) {
    // 参数检查
    if (!h) {
        return ALGO_HANDLE_ERROR;
    }
    if (!HotpixelRes) {
        return INVALID_PARAMETER_ERROR;
    }

    // 初始化指针为 nullptr
    HotpixelRes->BadPixelMask.LocalData = nullptr;
    HotpixelRes->BadPixelMask.Flag = nullptr;
    HotpixelRes->BadPixelMask.DiffData = nullptr;
    HotpixelRes->SubFrameBadpixelData = nullptr;

    APSBadpixelType res;
    bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->HotPixel(nIndexStart, nNumber, ROI, res);

    if (!bRet) {
        return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
    }

    try {
        HotpixelRes->BadPixelNum = res.BadPixelNum;
        HotpixelRes->SingletNum = res.SingletNum;
        HotpixelRes->CoupletNum = res.CoupletNum;
        HotpixelRes->LadderNum = res.LadderNum;
        HotpixelRes->ClusterNum = res.ClusterNum;
        HotpixelRes->MaxClusterSize = res.MaxClusterSize;

        HotpixelRes->BadPixelMask.BadPixelNum = res.BadPixelMask.BadPixelNum;
        // LocalData
        HotpixelRes->BadPixelMask.LocalDataSize = res.BadPixelMask.LocalData.size();
        if (HotpixelRes->BadPixelMask.LocalDataSize > 0) {
            HotpixelRes->BadPixelMask.LocalData =
                    new Local[HotpixelRes->BadPixelMask.LocalDataSize];
            std::copy(res.BadPixelMask.LocalData.begin(),
                      res.BadPixelMask.LocalData.end(),
                      HotpixelRes->BadPixelMask.LocalData);
        }

        // Flag
        HotpixelRes->BadPixelMask.FlagSize = res.BadPixelMask.Flag.size();
        if (HotpixelRes->BadPixelMask.FlagSize > 0) {
            HotpixelRes->BadPixelMask.Flag =
                    new uint8_t[HotpixelRes->BadPixelMask.FlagSize];
            std::copy(res.BadPixelMask.Flag.begin(),
                      res.BadPixelMask.Flag.end(),
                      HotpixelRes->BadPixelMask.Flag);
        }

        // DiffData
        HotpixelRes->BadPixelMask.DiffDataSize = res.BadPixelMask.DiffData.size();
        if (HotpixelRes->BadPixelMask.DiffDataSize > 0) {
            HotpixelRes->BadPixelMask.DiffData =
                    new uint32_t[HotpixelRes->BadPixelMask.DiffDataSize];
            std::copy(res.BadPixelMask.DiffData.begin(),
                      res.BadPixelMask.DiffData.end(),
                      HotpixelRes->BadPixelMask.DiffData);
        }

        // SubFrameBadpixelData
        HotpixelRes->SubFrameBadpixelDataSize = res.SubFrameBadpixelData.size();
        if (HotpixelRes->SubFrameBadpixelDataSize > 0) {
            HotpixelRes->SubFrameBadpixelData =
                    new APSSubFrameBadpixelTypeC[HotpixelRes->SubFrameBadpixelDataSize];

            // 初始化所有指针为 nullptr
            for (uint32_t i = 0; i < HotpixelRes->SubFrameBadpixelDataSize; i++) {
                HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalData = nullptr;
                HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.Flag = nullptr;
                HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffData = nullptr;
            }

            for (uint32_t i = 0; i < HotpixelRes->SubFrameBadpixelDataSize; i++) {
                HotpixelRes->SubFrameBadpixelData[i].BadPixelNum = res.SubFrameBadpixelData[i].BadPixelNum;
                HotpixelRes->SubFrameBadpixelData[i].SingletNum = res.SubFrameBadpixelData[i].SingletNum;
                HotpixelRes->SubFrameBadpixelData[i].CoupletNum = res.SubFrameBadpixelData[i].CoupletNum;
                HotpixelRes->SubFrameBadpixelData[i].ClusterNum = res.SubFrameBadpixelData[i].ClusterNum;
                HotpixelRes->SubFrameBadpixelData[i].DefectRowNum = res.SubFrameBadpixelData[i].DefectRowNum;
                HotpixelRes->SubFrameBadpixelData[i].DefectColNum = res.SubFrameBadpixelData[i].DefectColNum;
                HotpixelRes->SubFrameBadpixelData[i].MaxClusterSize = res.SubFrameBadpixelData[i].MaxClusterSize;

                // 使用 [i] 索引
                HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.BadPixelNum =
                        res.SubFrameBadpixelData[i].BadPixelMask.BadPixelNum;

                // LocalData
                HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalDataSize =
                        res.SubFrameBadpixelData[i].BadPixelMask.LocalData.size();
                if (HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalDataSize > 0) {
                    HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalData =
                            new Local[HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalDataSize];
                    std::copy(res.SubFrameBadpixelData[i].BadPixelMask.LocalData.begin(),
                              res.SubFrameBadpixelData[i].BadPixelMask.LocalData.end(),
                              HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalData);
                }

                // Flag
                HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.FlagSize =
                        res.SubFrameBadpixelData[i].BadPixelMask.Flag.size();
                if (HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.FlagSize > 0) {
                    HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.Flag =
                            new uint8_t[HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.FlagSize];
                    std::copy(res.SubFrameBadpixelData[i].BadPixelMask.Flag.begin(),
                              res.SubFrameBadpixelData[i].BadPixelMask.Flag.end(),
                              HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.Flag);
                }

                // DiffData
                HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffDataSize =
                        res.SubFrameBadpixelData[i].BadPixelMask.DiffData.size();
                if (HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffDataSize > 0) {
                    HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffData =
                            new uint32_t[HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffDataSize];
                    std::copy(res.SubFrameBadpixelData[i].BadPixelMask.DiffData.begin(),
                              res.SubFrameBadpixelData[i].BadPixelMask.DiffData.end(),
                              HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffData);
                }
            }
        }

        return TEST_NO_ERROR;
    } catch (const std::bad_alloc &) {
        HotPixelTypeCAPS_Free(HotpixelRes);
        return MEMORY_ALLOCATION_ERROR;
    }
}

// 释放函数
void __stdcall HotPixelTypeCAPS_Free(APSBadpixelTypeC *HotpixelRes) {
    if (!HotpixelRes) return;

    // 释放顶层 BadPixelMask
    if (HotpixelRes->BadPixelMask.LocalData) {
        delete[] HotpixelRes->BadPixelMask.LocalData;
        HotpixelRes->BadPixelMask.LocalData = nullptr;
        HotpixelRes->BadPixelMask.LocalDataSize = 0;
    }

    if (HotpixelRes->BadPixelMask.Flag) {
        delete[] HotpixelRes->BadPixelMask.Flag;
        HotpixelRes->BadPixelMask.Flag = nullptr;
        HotpixelRes->BadPixelMask.FlagSize = 0;
    }

    if (HotpixelRes->BadPixelMask.DiffData) {
        delete[] HotpixelRes->BadPixelMask.DiffData;
        HotpixelRes->BadPixelMask.DiffData = nullptr;
        HotpixelRes->BadPixelMask.DiffDataSize = 0;
    }

    // 遍历所有 SubFrameBadpixelData 元素
    if (HotpixelRes->SubFrameBadpixelData != nullptr) {
        for (size_t i = 0; i < HotpixelRes->SubFrameBadpixelDataSize; i++) {
            if (HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalData) {
                delete[] HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalData;
                HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.LocalData = nullptr;
            }

            if (HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.Flag) {
                delete[] HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.Flag;
                HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.Flag = nullptr;
            }

            if (HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffData) {
                delete[] HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffData;
                HotpixelRes->SubFrameBadpixelData[i].BadPixelMask.DiffData = nullptr;
            }
        }

        delete[] HotpixelRes->SubFrameBadpixelData;
        HotpixelRes->SubFrameBadpixelData = nullptr;
        HotpixelRes->SubFrameBadpixelDataSize = 0;
    }
}

uint32_t __stdcall BLCAPS_1(HANDLE h, uint32_t nIndexStart, uint32_t nNumber) {
    if (h) {
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->BLC(nIndexStart, nNumber);

        if (bRet) {
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall BLCAPS_2(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nBaseIndexStart,
                            uint32_t nBaseNumber, APSDataMeanType &BaseMeanRes) {
    if (h) {
        APSDataMeanType BaseMean;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->DataMean(
                        nBaseIndexStart, nBaseNumber, nullptr, BaseMean)
                    && reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->BLC(nIndexStart, nNumber, BaseMean);

        if (bRet) {
            BaseMeanRes.DataMeanFrame = BaseMean.DataMeanFrame;
            BaseMeanRes.SubFrameDataMean = BaseMean.SubFrameDataMean;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall BLCAPS_3(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nBaseIndexStart,
                            uint32_t nBaseNumber) {
    if (h) {
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->BLC(nIndexStart, nNumber, nBaseIndexStart,
                                                                       nBaseNumber);

        if (bRet) {
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall YShadingAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                               APSYShadingType *YShadingRes) {
    if (h) {
        APSYShadingType YShading;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->YShading(nIndexStart, nNumber, ROI, YShading);

        if (bRet) {
            YShadingRes->YShadingData = YShading.YShadingData;
            YShadingRes->YShadingLB = YShading.YShadingLB;
            YShadingRes->YShadingLT = YShading.YShadingLT;
            YShadingRes->YShadingRB = YShading.YShadingRB;
            YShadingRes->YShadingRT = YShading.YShadingRT;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ColorShadingAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                   APSColorShadingType *ColorShadingRes) {
    if (h) {
        APSColorShadingType ColorShading;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->
                ColorShading(nIndexStart, nNumber, ROI, ColorShading);

        if (bRet) {
            ColorShadingRes->ColorShadingBGData = ColorShading.ColorShadingBGData;
            ColorShadingRes->ColorShadingBGLB = ColorShading.ColorShadingBGLB;
            ColorShadingRes->ColorShadingBGLT = ColorShading.ColorShadingBGLT;
            ColorShadingRes->ColorShadingBGRB = ColorShading.ColorShadingBGRB;
            ColorShadingRes->ColorShadingBGRT = ColorShading.ColorShadingBGRT;

            ColorShadingRes->ColorShadingRGData = ColorShading.ColorShadingRGData;
            ColorShadingRes->ColorShadingRGLB = ColorShading.ColorShadingRGLB;
            ColorShadingRes->ColorShadingRGLT = ColorShading.ColorShadingRGLT;
            ColorShadingRes->ColorShadingRGRB = ColorShading.ColorShadingRGRB;
            ColorShadingRes->ColorShadingRGRT = ColorShading.ColorShadingRGRT;

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ReadNoiseAPS(HANDLE h, uint32_t nIndex1, uint32_t nIndex2, ROIArea *ROI,
                                APSReadNoiseType *ReadNoiseRes) {
    if (h) {
        APSReadNoiseType ReadNoise;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->ReadNoise(nIndex1, nIndex2, ROI, ReadNoise);

        if (bRet) {
            *ReadNoiseRes = ReadNoise;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall OpticalCenterAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                    APSOpticalCenterType *OpticalCenterRes) {
    if (h) {
        APSOpticalCenterType OpticalCenterType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->OpticalCenter(
            nIndexStart, nNumber, ROI, OpticalCenterType);

        if (bRet) {
            OpticalCenterRes->CenterCol = OpticalCenterType.CenterCol;
            OpticalCenterRes->CenterRow = OpticalCenterType.CenterRow;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall PedestalVariationAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                        APSPedestalVariationType *PedestalVariationRes) {
    if (h) {
        APSPedestalVariationType PedestalVariationType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->PedestalVariation(
            nIndexStart, nNumber, ROI, PedestalVariationType);

        if (bRet) {
            PedestalVariationRes->PedestalMax[0] = PedestalVariationType.PedestalMax[0];
            PedestalVariationRes->PedestalMin[0] = PedestalVariationType.PedestalMin[0];
            PedestalVariationRes->PedestalMax[1] = PedestalVariationType.PedestalMax[1];
            PedestalVariationRes->PedestalMin[1] = PedestalVariationType.PedestalMin[1];
            PedestalVariationRes->PedestalMax[2] = PedestalVariationType.PedestalMax[2];
            PedestalVariationRes->PedestalMin[2] = PedestalVariationType.PedestalMin[2];
            PedestalVariationRes->PedestalMax[3] = PedestalVariationType.PedestalMax[3];
            PedestalVariationRes->PedestalMin[3] = PedestalVariationType.PedestalMin[3];
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

// uint32_t DarkCurrentDataMeanAPS(HANDLE h, std::vector<APSDataMeanType> &DataMean, std::vector<double> &ExpTime,
//                                 APSDarkCurrentType &DarkCurrentRes) {
//     if (h) {
//         APSDarkCurrentType DarkCurrentType;
//         bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->DarkCurrent(DataMean, ExpTime, DarkCurrentType);
//
//         if (bRet) {
//             DarkCurrentRes.SubFrameKValue = DarkCurrentType.SubFrameKValue;
//             return TEST_NO_ERROR;
//         } else {
//             return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
//         }
//     } else {
//         return ALGO_HANDLE_ERROR;
//     }
// }
template<typename T>
static uint32_t DarkCurrentImpl(
    HANDLE h,
    const T *data,
    size_t dataCount,
    const double *expTime,
    size_t expTimeCount,
    APSDarkCurrentType *darkCurrentRes) {
    if (!h) return ALGO_HANDLE_ERROR;
    if (!data || dataCount == 0) return INVALID_PARAMETER_ERROR;
    if (!expTime || expTimeCount == 0) return INVALID_PARAMETER_ERROR;
    if (!darkCurrentRes) return INVALID_PARAMETER_ERROR;

    try {
        std::vector<T> dataVec(data, data + dataCount);
        std::vector<double> expTimeVec(expTime, expTime + expTimeCount);

        APSDarkCurrentType darkCurrentType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->DarkCurrent(
            dataVec,
            expTimeVec,
            darkCurrentType
        );

        if (bRet) {
            darkCurrentRes->SubFrameKValue = darkCurrentType.SubFrameKValue;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } catch (const std::bad_alloc &) {
        return MEMORY_ALLOCATION_ERROR;
    } catch (...) {
        return UNKNOWN_ERROR;
    }
}

uint32_t __stdcall DarkCurrentDataMeanAPS(HANDLE h, const APSDataMeanType *dataMean, size_t dataMeanCount,
                                          const double *expTime,
                                          size_t expTimeCount, APSDarkCurrentType *darkCurrentRes) {
    return DarkCurrentImpl(h, dataMean, dataMeanCount, expTime, expTimeCount, darkCurrentRes);
}

uint32_t __stdcall DarkCurrentTNoiseAPS(HANDLE h, const APSTNoiseType *pTNoiseData, size_t iTNoiseDataCount,
                                        const double *expTime,
                                        size_t expTimeCount, APSDarkCurrentType *darkCurrentRes) {
    return DarkCurrentImpl(h, pTNoiseData, iTNoiseDataCount, expTime, expTimeCount, darkCurrentRes);
}

uint32_t __stdcall DSNUAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, APSDSNUType &DSNURes) {
    if (h) {
        APSDSNUType DSNUType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->DSNU(nIndexStart, nNumber, ROI, DSNUType);

        if (bRet) {
            DSNURes.RangeR = DSNUType.RangeR;
            DSNURes.RangeG = DSNUType.RangeG;
            DSNURes.RangeB = DSNUType.RangeB;
            DSNURes.SignalMax = DSNUType.SignalMax;
            DSNURes.DeltaSignalMax = DSNUType.DeltaSignalMax;
            DSNURes.DeltaSignalCentreMax = DSNUType.DeltaSignalCentreMax;
            DSNURes.DeltaSignalEdgeMax = DSNUType.DeltaSignalEdgeMax;
            DSNURes.DeltaSignalCornerMax = DSNUType.DeltaSignalCornerMax;
            DSNURes.RMax = DSNUType.RMax;
            DSNURes.RMin = DSNUType.RMin;
            DSNURes.GMax = DSNUType.GMax;
            DSNURes.GMin = DSNUType.GMin;
            DSNURes.BMax = DSNUType.BMax;
            DSNURes.BMin = DSNUType.BMin;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall DataMeanAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                               APSDataMeanType *DataMeanRes) {
    if (h) {
        APSDataMeanType DataMeanType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->DataMean(nIndexStart, nNumber, ROI, DataMeanType);

        if (bRet) {
            DataMeanRes->DataMeanFrame = DataMeanType.DataMeanFrame;
            DataMeanRes->SubFrameDataMean = DataMeanType.SubFrameDataMean;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall LinearityAPS(HANDLE h, std::vector<APSDataMeanType> *LightMean, std::vector<double> *ExpTime,
                                APSLinearityType *LinearityRes) {
    if (h) {
        APSLinearityType LinearityType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->Linearity(*LightMean, *ExpTime, LinearityType);

        if (bRet) {
            LinearityRes->SubFrameLinearityData = LinearityType.SubFrameLinearityData;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall OverallSystemGainAPS(HANDLE h, std::vector<APSTNoiseType> *LightTNoiseData,
                                        std::vector<APSDataMeanType> *LightMean, APSTNoiseType DarkTNoiseBase,
                                        APSOverallSystemGainType *GainRes) {
    if (h) {
        APSOverallSystemGainType GainType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->OverallSystemGain(
            *LightTNoiseData, *LightMean, DarkTNoiseBase, GainType);

        if (bRet) {
            GainRes->SubFrameGainK = GainType.SubFrameGainK;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SaturationAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                 SubFrameIndex nChannelIndex,
                                 APSSaturationType *SaturationRes) {
    if (h) {
        APSSaturationType SaturationType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->Saturation(
            nIndexStart, nNumber, ROI, nChannelIndex, SaturationType);

        if (bRet) {
            SaturationRes->SaturationMean = SaturationType.SaturationMean;
            SaturationRes->SaturationSNR = SaturationType.SaturationSNR;
            SaturationRes->SaturationTNoise = SaturationType.SaturationTNoise;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall OETCAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nNumberInOneStep, ROIArea *ROI,
                           SubFrameIndex nChannelIndex,
                           APSOETCType *OETCRes) {
    if (h) {
        APSOETCType OETCType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->OETC(nIndexStart, nNumber, nNumberInOneStep, ROI,
                                                                        nChannelIndex, OETCType);

        if (bRet) {
            OETCRes->DataMean = OETCType.DataMean;
            OETCRes->ReadNoise = OETCType.ReadNoise;
            OETCRes->TNoiseData = OETCType.TNoiseData;
            OETCRes->ConversionGain = OETCType.ConversionGain;
            OETCRes->DR_dB = OETCType.DR_dB;
            OETCRes->FWC = OETCType.FWC;
            OETCRes->FWC_e = OETCType.FWC_e;
            OETCRes->ReadNoise_e = OETCType.ReadNoise_e;
            OETCRes->ReadNoiseData = OETCType.ReadNoiseData;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall LinearitySNRAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                   SubFrameIndex nChannelIndex,
                                   APSSSNRType *SSNRRes) {
    if (h) {
        APSSSNRType SSNRType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->Linearity(
            nIndexStart, nNumber, ROI, nChannelIndex, SSNRType);

        if (bRet) {
            SSNRRes->DataMean = SSNRType.DataMean;
            SSNRRes->MaxSSNR = SSNRType.MaxSSNR;
            SSNRRes->SNoiseData = SSNRType.SNoiseData;
            SSNRRes->SSNR = SSNRType.SSNR;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ShowAPS_1(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                             SubFrameIndex nChannelIndex,
                             bool bNormalize, ImgType *ImgData) {
    if (h) {
        ImgType ImgDataType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->Show(nIndexStart, nNumber, ROI, nChannelIndex,
                                                                        bNormalize, ImgDataType);

        if (bRet) {
            ImgData->assign(ImgDataType.begin(), ImgDataType.end());
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ShowAPS_2(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                             SubFrameIndex nChannelIndex,
                             APSType *ImgData) {
    if (h) {
        APSType ImgDataType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->Show(nIndexStart, nNumber, ROI, nChannelIndex,
                                                                        ImgDataType);

        if (bRet) {
            ImgData->assign(ImgDataType.begin(), ImgDataType.end());
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ShowAPS_3(HANDLE h, uint32_t nIndex, uint16_t *RawData) {
    if (h) {
        uint16_t *RawDataTmp = nullptr;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->Show(nIndex, RawDataTmp);

        if (bRet) {
            RawData = RawDataTmp;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SetMultiThreadEnableAPS(HANDLE h, bool bEnable) {
    if (h) {
        reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->SetMultiThreadEnable(bEnable);
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SetLogEnableAPS(HANDLE h, bool bEnable) {
    if (h) {
        reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->SetLogEnable(bEnable);
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SetAlgorithmThreAPS(HANDLE h, APSAlgorithmThre *AlgoThre) {
    if (h) {
        APSAlgorithmThre res;
        res.dHotPixelThre = AlgoThre->dHotPixelThre;
        res.dHotLineThre = AlgoThre->dHotLineThre;
        res.dBadPixelThre = AlgoThre->dBadPixelThre;
        res.dBadLineThre = AlgoThre->dBadLineThre;
        res.nBadPixelRadius = AlgoThre->nBadPixelRadius;
        res.nBadLineRadius = AlgoThre->nBadLineRadius;
        res.nDSNURowBlockNum = AlgoThre->nDSNURowBlockNum;
        res.nDSNUColBlockNum = AlgoThre->nDSNUColBlockNum;
        res.nDSNURowBlockSize = AlgoThre->nDSNURowBlockSize;
        res.nDSNUColBlockSize = AlgoThre->nDSNUColBlockSize;
        res.nYShadingRowBlockNum = AlgoThre->nYShadingRowBlockNum;
        res.nYShadingColBlockNum = AlgoThre->nYShadingColBlockNum;
        res.nColorShadingRowBlockNum = AlgoThre->nColorShadingRowBlockNum;
        res.nColorShadingColBlockNum = AlgoThre->nColorShadingColBlockNum;
        res.nPedestalVariationRowBlockNum = AlgoThre->nPedestalVariationRowBlockNum;
        res.nPedestalVariationColBlockNum = AlgoThre->nPedestalVariationColBlockNum;
        res.nPedestalVariationRowBlockSize = AlgoThre->nPedestalVariationRowBlockSize;
        res.nPedestalVariationColBlockSize = AlgoThre->nPedestalVariationColBlockSize;
        res.nBadPixelMaxLen = AlgoThre->nBadPixelMaxLen;
        res.nBadPixelLocalRowOffset = AlgoThre->nBadPixelLocalRowOffset;
        res.nBadPixelLocalColOffset = AlgoThre->nBadPixelLocalColOffset;
        res.nLinearityRadius = AlgoThre->nLinearityRadius;
        res.nOETCRadius = AlgoThre->nOETCRadius;
        reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->SetAlgorithmThre(res);
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetAlgorithmThreAPS(HANDLE h, APSAlgorithmThre *AlgoThre) {
    if (h) {
        APSAlgorithmThre res;
        res = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetAlgorithmThre();
        AlgoThre->dHotPixelThre = res.dHotPixelThre;
        AlgoThre->dHotLineThre = res.dHotLineThre;
        AlgoThre->dBadPixelThre = res.dBadPixelThre;
        AlgoThre->dBadLineThre = res.dBadLineThre;
        AlgoThre->nBadPixelRadius = res.nBadPixelRadius;
        AlgoThre->nBadLineRadius = res.nBadLineRadius;
        AlgoThre->nDSNURowBlockNum = res.nDSNURowBlockNum;
        AlgoThre->nDSNUColBlockNum = res.nDSNUColBlockNum;
        AlgoThre->nDSNURowBlockSize = res.nDSNURowBlockSize;
        AlgoThre->nDSNUColBlockSize = res.nDSNUColBlockSize;
        AlgoThre->nYShadingRowBlockNum = res.nYShadingRowBlockNum;
        AlgoThre->nYShadingColBlockNum = res.nYShadingColBlockNum;
        AlgoThre->nColorShadingRowBlockNum = res.nColorShadingRowBlockNum;
        AlgoThre->nColorShadingColBlockNum = res.nColorShadingColBlockNum;
        AlgoThre->nPedestalVariationRowBlockNum = res.nPedestalVariationRowBlockNum;
        AlgoThre->nPedestalVariationColBlockNum = res.nPedestalVariationColBlockNum;
        AlgoThre->nPedestalVariationRowBlockSize = res.nPedestalVariationRowBlockSize;
        AlgoThre->nPedestalVariationColBlockSize = res.nPedestalVariationColBlockSize;
        AlgoThre->nBadPixelMaxLen = res.nBadPixelMaxLen;
        AlgoThre->nBadPixelLocalRowOffset = res.nBadPixelLocalRowOffset;
        AlgoThre->nBadPixelLocalColOffset = res.nBadPixelLocalColOffset;
        AlgoThre->nLinearityRadius = res.nLinearityRadius;
        AlgoThre->nOETCRadius = res.nOETCRadius;
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetDataNumAPS(HANDLE h, uint32_t *DataNum) {
    if (h) {
        uint32_t tDataNum = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetDataNum();
        *DataNum = tDataNum;
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SaveBinAPS(HANDLE h, uint8_t *pRawData, uint64_t nLens, const char *strSavePath) {
    if (h) {
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->SaveBin(pRawData, nLens, strSavePath);
        if (bRet) {
            return TEST_NO_ERROR;
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetActiveAreaAPS(HANDLE h, ROIArea *ROIAreaRes) {
    if (h) {
        ROIArea ROIAreaTmp;
        ROIAreaTmp = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetActiveArea();
        ROIAreaRes->Up = ROIAreaTmp.Up;
        ROIAreaRes->Down = ROIAreaTmp.Down;
        ROIAreaRes->Left = ROIAreaTmp.Left;
        ROIAreaRes->Right = ROIAreaTmp.Right;
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetRawDataSizeAPS(HANDLE h, uint32_t *nRow, uint32_t *nCol) {
    if (h) {
        uint32_t tRow = 0;
        uint32_t tCol = 0;
        reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetRawDataSize(tRow, tCol);
        *nRow = tRow;
        *nCol = tCol;
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SetActiveAreaAPS(HANDLE h, ROIArea ActiveArea) {
    if (h) {
        reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->SetActiveArea(ActiveArea);
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SetRawDataSizeAPS(HANDLE h, uint32_t nRow, uint32_t nCol) {
    if (h) {
        reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->SetRawDataSize(nRow, nCol);
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall AlpGetVersionAPS(HANDLE h, char *ver, uint32_t nLen) {
    if (h) {
        std::string strVer = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetVersion();

        uint32_t nRealLens = nLen <= strVer.size() + 1 ? nLen : strVer.size() + 1;
        strcpy_s(ver, nRealLens, strVer.c_str());
        ver[nRealLens] = 0;

        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetCodeAPS(HANDLE h, int *Code) {
    if (h) {
        int tCode = 0;
        tCode = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        *Code = tCode;
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

#endif
