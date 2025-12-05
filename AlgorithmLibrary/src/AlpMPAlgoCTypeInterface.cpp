#ifdef API_C_TYPE_INTERFACE
#include "AlpMPAlgoCTypeInterface.h"
#include "AlpMPAlgoInterface.h"

HANDLE __stdcall InitHandleDVS(SensorType Sensortype, PixelFormatType Pixelformat, int code) {
    CAlpDVSMPAlgoInterface *pInterface = CreateDVSAlgoInterface(Sensortype, "", Pixelformat, code);

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
                                        CEventsNumberCountData *EventsNumberCountRes) {
    if (h) {
        DVSEventsNumberCountType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->EventsNumberCount(nIndexStart, nNumber, res);

        if (bRet) {
            if (res.nDataNumber > MAX_DATA_NUMBER) {
                return BEYOND_MAX_RES_NUM;
            }

            for (uint32_t nIndex = 0; nIndex < res.nDataNumber; nIndex++) {
                for (uint32_t nChannel = 0; nChannel < SubFrameIndex::All + 1; nChannel++) {
                    EventsNumberCountRes->AllEventsNum[nChannel][nIndex] = res.AllEventsNum[nChannel][nIndex];
                    EventsNumberCountRes->NoEventsNum[nChannel][nIndex] = res.NoEventsNum[nChannel][nIndex];
                    EventsNumberCountRes->OffEventsNum[nChannel][nIndex] = res.OffEventsNum[nChannel][nIndex];
                    EventsNumberCountRes->OnEventsNum[nChannel][nIndex] = res.OnEventsNum[nChannel][nIndex];
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

uint32_t __stdcall StationaryNoiseDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                      CStationaryNoiseData *StationaryNoiseRes) {
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
            StationaryNoiseRes->dStationaryColSNoise = res.dStationaryColTNoise;
            StationaryNoiseRes->dStationaryRowSNoise = res.dStationaryRowTNoise;

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall StationaryUniformityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber,
                                           CStationaryUniformityData *UniformityRes) {
    if (h) {
        DVSStationaryUniformityType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->StationaryUniformity(nIndexStart, nNumber, res);

        if (bRet) {
            UniformityRes->UniformityRatio = res.UniformityRatio;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall HotPixelDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, CHotpixelData *HotpixelRes) {
    if (h) {
        DVSHotpixelType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->HotPixel(nIndexStart, nNumber, res);

        if (bRet) {
            HotpixelRes->HotLineNum = res.HotLineNum;
            HotpixelRes->HotPixelNum = res.HotPixelNum;
            HotpixelRes->SingletNum = res.SingletNum;
            HotpixelRes->CoupletNum = res.CoupletNum;
            HotpixelRes->TripletNum = res.TripletNum;
            HotpixelRes->FourConnectedNum = res.FourConnectedNum;
            HotpixelRes->ClusterNum = res.ClusterNum;

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall FindPeakDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum, CPeakInfo *Peak,
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

            for (uint32_t nIndex = 0; nIndex < res.nOffEventsPeakNumber; nIndex++) {
                Peak->OffEventsPeakPos[nIndex] = res.OffEventsPeakPos[nIndex];
            }

            for (uint32_t nIndex = 0; nIndex < res.nOnEventsPeakNumber; nIndex++) {
                Peak->OnEventsPeakPos[nIndex] = res.OnEventsPeakPos[nIndex];
            }

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ImageContrastSensitivityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum,
                                               DVSLightTrigerType Light,
                                               CImageContrastSensitivityData *ImageContrastSensitivityRes) {
    if (h) {
        DVSImageContrastSensitivityType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->ImageContrastSensitivity(
            nIndexStart, nNumber, nullptr, nPeakNum, Light, res);

        if (bRet) {
            ImageContrastSensitivityRes->B_Gb_OffEventsRatio = res.B_Gb_OffEventsRatio;
            ImageContrastSensitivityRes->B_Gb_OnEventsRatio = res.B_Gb_OnEventsRatio;
            ImageContrastSensitivityRes->Gr_Gb_OffEventsRatio = res.Gr_Gb_OffEventsRatio;
            ImageContrastSensitivityRes->Gr_Gb_OnEventsRatio = res.Gr_Gb_OnEventsRatio;
            ImageContrastSensitivityRes->R_Gb_OffEventsRatio = res.R_Gb_OffEventsRatio;
            ImageContrastSensitivityRes->R_Gb_OnEventsRatio = res.R_Gb_OnEventsRatio;

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

uint32_t __stdcall AccompaniedPeakAndDelayedPeakDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum,
                                                    DVSLightTrigerType Light,
                                                    CAccompaniedPeakAndDelayedPeakData *
                                                    AccompaniedPeakAndDelayedPeakRes) {
    if (h) {
        DVSAccompaniedPeakAndDelayedPeakType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->AccompaniedPeakAndDelayedPeak(
            nIndexStart, nNumber, nullptr, nPeakNum, Light, res);

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

uint32_t __stdcall SpatialResponseUniformityDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                                uint32_t nPeakNum, DVSLightTrigerType Light,
                                                CSpatialResponseUniformityData *SpatialResponseUniformityRes) {
    if (h) {
        DVSSpatialResponseUniformityType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->SpatialResponseUniformity(
            nIndexStart, nNumber, ROI, nullptr, nPeakNum, Light, res);

        if (bRet) {
            for (uint32_t nChannel = 0; nChannel < SubFrameIndex::All + 1; nChannel++) {
                SpatialResponseUniformityRes->dOffEventsUniformityRatio[nChannel] = res.dOffEventsUniformityRatio[
                    nChannel];
                SpatialResponseUniformityRes->dOnEventsUniformityRatio[nChannel] = res.dOnEventsUniformityRatio[
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

uint32_t __stdcall BadPixelDVS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, uint32_t nPeakNum,
                               DVSLightTrigerType Light, CDVSBadpixelData *BadpixelRes) {
    if (h) {
        DVSBadpixelType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->BadPixel(
            nIndexStart, nNumber, nullptr, nPeakNum, Light, res);

        if (bRet) {
            BadpixelRes->nOffEventsClusterNum = res.nOffEventsClusterNum;
            BadpixelRes->nOffEventsDeadPixelNum = res.nOffEventsDeadPixelNum;
            BadpixelRes->nOnEventsClusterNum = res.nOnEventsClusterNum;
            BadpixelRes->nOnEventsDeadPixelNum = res.nOnEventsDeadPixelNum;

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetErrCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ShowDVS(HANDLE h, uint32_t nIndex, uint8_t NoEventFlag, uint8_t OnEventFlag, uint8_t OffEventFlag,
                           uint8_t *ImgData) {
    if (h) {
        ImgType res;
        bool bRet = reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->Show(nIndex, NoEventFlag, OnEventFlag, OffEventFlag,
                                                                        res);

        if (bRet) {
            uint32_t nTotalRow = 0, nTotalCol = 0;
            reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetRawDataSize(nTotalRow, nTotalCol);

            for (uint32_t nRows = 0; nRows < nTotalRow; nRows++) {
                for (uint32_t nCols = 0; nCols < nTotalCol; nCols++) {
                    ImgData[nRows * nTotalCol + nCols] = res[nRows][nCols];
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

uint32_t __stdcall SetAlgorithmThreDVS(HANDLE h, CDVSAlgorithmThre *AlgoThre) {
    if (h) {
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
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetAlgorithmThreDVS(HANDLE h, CDVSAlgorithmThre *AlgoThre) {
    if (h) {
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

uint32_t __stdcall GetRawDataSizeDVS(HANDLE h, uint32_t *nRow, uint32_t *nCol) {
    if (h) {
        reinterpret_cast<CAlpDVSMPAlgoInterface *>(h)->GetRawDataSize(*nRow, *nCol);
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

// | ============= APS ================= |
HANDLE __stdcall InitHandleAPS(SensorType Sensortype, APSRawType APSRawtype, PixelFormatType Pixelformat, int code) {
    CAlpAPSMPAlgoInterface *pInterface = CreateAPSAlgoInterface(Sensortype, APSRawtype, "", Pixelformat, code);

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

uint32_t __stdcall TNoiseAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, APSTNoiseTypeC *apsTNoiseRes) {
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
            nullptr,
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

uint32_t __stdcall SNoiseAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, APSSNoiseType *APSSNoiseRes) {
    if (h) {
        APSSNoiseType res;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->SNoise(nIndexStart, nNumber, nullptr, res);

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

uint32_t __stdcall BadPixelAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, APSBadpixelType *BadpixelRes) {
    if (h) {
        APSBadpixelType res;

        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->BadPixel(nIndexStart, nNumber, nullptr, res);

        if (bRet) {
            BadpixelRes->BadPixelNum = res.BadPixelNum;
            BadpixelRes->SingletNum = res.SingletNum;
            BadpixelRes->CoupletNum = res.CoupletNum;
            BadpixelRes->LadderNum = res.LadderNum;
            BadpixelRes->ClusterNum = res.ClusterNum;
            BadpixelRes->MaxClusterSize = res.MaxClusterSize;
            BadpixelRes->SubFrameBadpixelData = res.SubFrameBadpixelData;

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall HotPixelAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, APSBadpixelType *HotpixelRes) {
    if (h) {
        APSBadpixelType res;
        // bRet = m_pAPSAlgoInterface->HotPixel(nIndexStart, nNumber, roi, m_HotPixel);
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->HotPixel(nIndexStart, nNumber, nullptr, res);

        if (bRet) {
            HotpixelRes->BadPixelNum = res.BadPixelNum;
            HotpixelRes->SingletNum = res.SingletNum;
            HotpixelRes->CoupletNum = res.CoupletNum;
            HotpixelRes->LadderNum = res.LadderNum;
            HotpixelRes->ClusterNum = res.ClusterNum;
            HotpixelRes->MaxClusterSize = res.MaxClusterSize;
            HotpixelRes->SubFrameBadpixelData = res.SubFrameBadpixelData;

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
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

uint32_t __stdcall YShadingAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, APSYShadingType &YShadingRes) {
    if (h) {
        APSYShadingType YShading;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->YShading(nIndexStart, nNumber, ROI, YShading);

        if (bRet) {
            YShadingRes.YShadingData = YShading.YShadingData;
            YShadingRes.YShadingLB = YShading.YShadingLB;
            YShadingRes.YShadingLT = YShading.YShadingLT;
            YShadingRes.YShadingRB = YShading.YShadingRB;
            YShadingRes.YShadingRT = YShading.YShadingRT;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ColorShadingAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                         APSColorShadingType &ColorShadingRes) {
    if (h) {
        APSColorShadingType ColorShading;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->
                ColorShading(nIndexStart, nNumber, ROI, ColorShading);

        if (bRet) {
            ColorShadingRes.ColorShadingBGData = ColorShading.ColorShadingBGData;
            ColorShadingRes.ColorShadingBGLB = ColorShading.ColorShadingBGLB;
            ColorShadingRes.ColorShadingBGLT = ColorShading.ColorShadingBGLT;
            ColorShadingRes.ColorShadingBGRB = ColorShading.ColorShadingBGRB;
            ColorShadingRes.ColorShadingBGRT = ColorShading.ColorShadingBGRT;

            ColorShadingRes.ColorShadingRGData = ColorShading.ColorShadingRGData;
            ColorShadingRes.ColorShadingRGLB = ColorShading.ColorShadingRGLB;
            ColorShadingRes.ColorShadingRGLT = ColorShading.ColorShadingRGLT;
            ColorShadingRes.ColorShadingRGRB = ColorShading.ColorShadingRGRB;
            ColorShadingRes.ColorShadingRGRT = ColorShading.ColorShadingRGRT;

            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ReadNoiseAPS(HANDLE h, uint32_t nIndex1, uint32_t nIndex2, ROIArea *ROI, APSReadNoiseType &ReadNoiseRes) {
    if (h) {
        APSReadNoiseType ReadNoise;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->ReadNoise(nIndex1, nIndex2, ROI, ReadNoise);

        if (bRet) {
            ReadNoiseRes = ReadNoise;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall OpticalCenterAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                          APSOpticalCenterType &OpticalCenterRes) {
    if (h) {
        APSOpticalCenterType OpticalCenterType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->OpticalCenter(
            nIndexStart, nNumber, ROI, OpticalCenterType);

        if (bRet) {
            OpticalCenterRes.CenterCol = OpticalCenterType.CenterCol;
            OpticalCenterRes.CenterRow = OpticalCenterType.CenterRow;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall PedestalVariationAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                              APSPedestalVariationType &PedestalVariationRes) {
    if (h) {
        APSPedestalVariationType PedestalVariationType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->PedestalVariation(
            nIndexStart, nNumber, ROI, PedestalVariationType);

        if (bRet) {
            PedestalVariationRes.PedestalMax[0] = PedestalVariationType.PedestalMax[0];
            PedestalVariationRes.PedestalMin[0] = PedestalVariationType.PedestalMin[0];
            PedestalVariationRes.PedestalMax[1] = PedestalVariationType.PedestalMax[1];
            PedestalVariationRes.PedestalMin[1] = PedestalVariationType.PedestalMin[1];
            PedestalVariationRes.PedestalMax[2] = PedestalVariationType.PedestalMax[2];
            PedestalVariationRes.PedestalMin[2] = PedestalVariationType.PedestalMin[2];
            PedestalVariationRes.PedestalMax[3] = PedestalVariationType.PedestalMax[3];
            PedestalVariationRes.PedestalMin[3] = PedestalVariationType.PedestalMin[3];
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

uint32_t __stdcall DarkCurrentDataMeanAPS(HANDLE h, const APSDataMeanType *dataMean, size_t dataMeanCount, const double *expTime,
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

uint32_t __stdcall DataMeanAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, APSDataMeanType &DataMeanRes) {
    if (h) {
        APSDataMeanType DataMeanType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->DataMean(nIndexStart, nNumber, ROI, DataMeanType);

        if (bRet) {
            DataMeanRes.DataMeanFrame = DataMeanType.DataMeanFrame;
            DataMeanRes.SubFrameDataMean = DataMeanType.SubFrameDataMean;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall LinearityAPS(HANDLE h, std::vector<APSDataMeanType> &LightMean, std::vector<double> &ExpTime,
                      APSLinearityType &LinearityRes) {
    if (h) {
        APSLinearityType LinearityType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->Linearity(LightMean, ExpTime, LinearityType);

        if (bRet) {
            LinearityRes.SubFrameLinearityData = LinearityType.SubFrameLinearityData;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall OverallSystemGainAPS(HANDLE h, std::vector<APSTNoiseType> &LightTNoiseData,
                              std::vector<APSDataMeanType> &LightMean, APSTNoiseType DarkTNoiseBase,
                              APSOverallSystemGainType &GainRes) {
    if (h) {
        APSOverallSystemGainType GainType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->OverallSystemGain(
            LightTNoiseData, LightMean, DarkTNoiseBase, GainType);

        if (bRet) {
            GainRes.SubFrameGainK = GainType.SubFrameGainK;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall SaturationAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, SubFrameIndex nChannelIndex,
                       APSSaturationType &SaturationRes) {
    if (h) {
        APSSaturationType SaturationType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->Saturation(
            nIndexStart, nNumber, ROI, nChannelIndex, SaturationType);

        if (bRet) {
            SaturationRes.SaturationMean = SaturationType.SaturationMean;
            SaturationRes.SaturationSNR = SaturationType.SaturationSNR;
            SaturationRes.SaturationTNoise = SaturationType.SaturationTNoise;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall OETCAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, SubFrameIndex nChannelIndex,
                 APSOETCType &OETCRes) {
    if (h) {
        APSOETCType OETCType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->OETC(nIndexStart, nNumber, ROI, nChannelIndex,
                                                                        OETCType);

        if (bRet) {
            OETCRes.DataMean = OETCType.DataMean;
            OETCRes.ReadNoise = OETCType.ReadNoise;
            OETCRes.TNoiseData = OETCType.TNoiseData;
            OETCRes.ConversionGain = OETCType.ConversionGain;
            OETCRes.DR_dB = OETCType.DR_dB;
            OETCRes.FWC = OETCType.FWC;
            OETCRes.FWC_e = OETCType.FWC_e;
            OETCRes.ReadNoise_e = OETCType.ReadNoise_e;
            OETCRes.ReadNoiseData = OETCType.ReadNoiseData;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall LinearitySNRAPS(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, SubFrameIndex nChannelIndex,
                         APSSSNRType &SSNRRes) {
    if (h) {
        APSSSNRType SSNRType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->Linearity(
            nIndexStart, nNumber, ROI, nChannelIndex, SSNRType);

        if (bRet) {
            SSNRRes.DataMean = SSNRType.DataMean;
            SSNRRes.MaxSSNR = SSNRType.MaxSSNR;
            SSNRRes.SNoiseData = SSNRType.SNoiseData;
            SSNRRes.SSNR = SSNRType.SSNR;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ShowAPS_1(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, SubFrameIndex nChannelIndex,
                   bool bNormalize, ImgType &ImgData) {
    if (h) {
        ImgType ImgDataType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->Show(nIndexStart, nNumber, ROI, nChannelIndex,
                                                                        bNormalize, ImgDataType);

        if (bRet) {
            ImgData = ImgDataType;
            return TEST_NO_ERROR;
        } else {
            return reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        }
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall ShowAPS_2(HANDLE h, uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI, SubFrameIndex nChannelIndex,
                   APSType &ImgData) {
    if (h) {
        APSType ImgDataType;
        bool bRet = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->Show(nIndexStart, nNumber, ROI, nChannelIndex,
                                                                        ImgDataType);

        if (bRet) {
            ImgData = ImgDataType;
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

uint32_t __stdcall SetAlgorithmThreAPS(HANDLE h, APSAlgorithmThre &AlgoThre) {
    if (h) {
        reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->SetAlgorithmThre(AlgoThre);
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetAlgorithmThreAPS(HANDLE h, APSAlgorithmThre &AlgoThre) {
    if (h) {
        AlgoThre = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetAlgorithmThre();

        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetDataNumAPS(HANDLE h, uint32_t &DataNum) {
    if (h) {
        DataNum = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetDataNum();
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

uint32_t __stdcall GetActiveAreaAPS(HANDLE h, ROIArea &ROIAreaRes) {
    if (h) {
        ROIAreaRes = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetActiveArea();
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}

uint32_t __stdcall GetRawDataSizeAPS(HANDLE h, uint32_t &nRow, uint32_t &nCol) {
    if (h) {
        reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetRawDataSize(nRow, nCol);
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

uint32_t __stdcall GetCodeAPS(HANDLE h, int &Code) {
    if (h) {
        Code = reinterpret_cast<CAlpAPSMPAlgoInterface *>(h)->GetCode();
        return TEST_NO_ERROR;
    } else {
        return ALGO_HANDLE_ERROR;
    }
}
#endif
