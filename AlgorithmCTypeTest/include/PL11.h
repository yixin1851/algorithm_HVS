//
// Created by xinfeng.weng on 2025/12/24.
//

#ifndef ALGORITHMLIBRARY_PL11_H
#define ALGORITHMLIBRARY_PL11_H
#include "public.h"
#include "AlpMPAlgoCTypeInterface.h"
#include <string>
int calcPL11()
{
	//导入数据
	std::string F1  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F1_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
	std::string F2  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F2_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
	std::string F3  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F3_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
	std::string F4  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F4_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F5  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F5_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F6  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F6_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F7  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F7_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F8  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F8_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F9  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F9_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F10 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F10_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F11 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F11_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F12 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F12_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F13 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F13_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F14 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F14_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F15 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F15_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
 	std::string F16 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL11_4000-4120Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F16_W4080H352P10_T2K-CP2-4S-LY_20251205172625.raw";
    int nWidth = 4080;
    int nHeight = 352;
    uint32_t nNumber = 16;
    uint8_t DropFrameN = 0;
    uint8_t UseFrameN = 165;
    uint8_t EVS_cnt = 11;

    long rawDataBufLen = nWidth * nHeight * 2;
    unsigned long rawDataRealLen = 0;
    uint8_t *rawDataBuf = new uint8_t[rawDataBufLen * nNumber];
    ImageCapture_capture(F1, rawDataBuf, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F2, rawDataBuf + rawDataBufLen * 1, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F3, rawDataBuf + rawDataBufLen * 2, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F4, rawDataBuf + rawDataBufLen * 3, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F5, rawDataBuf + rawDataBufLen * 4, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F6, rawDataBuf + rawDataBufLen * 5, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F7, rawDataBuf + rawDataBufLen * 6, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F8, rawDataBuf + rawDataBufLen * 7, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F9, rawDataBuf + rawDataBufLen * 8, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F10, rawDataBuf + rawDataBufLen * 9, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F11, rawDataBuf + rawDataBufLen * 10, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F12, rawDataBuf + rawDataBufLen * 11, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F13, rawDataBuf + rawDataBufLen * 12, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F14, rawDataBuf + rawDataBufLen * 13, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F15, rawDataBuf + rawDataBufLen * 14, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F16, rawDataBuf + rawDataBufLen * 15, rawDataBufLen, rawDataRealLen, 0);
    uint8_t *pbRawData = new uint8_t[nWidth * nHeight * nNumber];
    for (int i = 0; i < nWidth * nHeight * nNumber; i++) {
        *(pbRawData + i) = *(rawDataBuf + i * 2);
    }

    // 3. 调用DLL函数
    HANDLE handle = InitHandleDVS(SensorType::ALP_003CA, "./", BayerGBRG, 1);
    //cout << "句柄：" << handle << endl;
    DVSAlgorithmThre DVSAlgoThre_new;
    check_ret(__func__, GetAlgorithmThreDVS(handle, &DVSAlgoThre_new));
    DVSAlgoThre_new.nPeakCycle = 10;
    check_ret(__func__, SetAlgorithmThreDVS(handle, &DVSAlgoThre_new));
    check_ret(__func__, SetMultiThreadEnableDVS(handle, true));
    //pSetLogEnableDVS(handle, true);

    int framelength = nWidth * nHeight / EVS_cnt;
    if (!check_ret(__func__, ImportRawDataDVS(handle, pbRawData + nWidth * 8 + framelength * DropFrameN,
                                              framelength * UseFrameN, 0, UseFrameN))) {
        std::cout << "Error: fail tp ImportRawdata.\r\n";
        return 1;
    }

    uint32_t nIndexStart = 0;
    DVSEventsNumberCountType EventsNumberCountRes;
    if (!check_ret(__func__, EventsNumberCountDVS(handle, nIndexStart, UseFrameN, &EventsNumberCountRes))) {
        return -1;
    }

    DVSPeakInfo PeakInfoRes;
    if (check_ret(__func__, FindPeakDVS(handle, 0, UseFrameN, 6, &PeakInfoRes, On_OffEvents))) {
        int onNum = PeakInfoRes.nOnEventsPeakNumber;
        int offNum = PeakInfoRes.nOffEventsPeakNumber;
        for (size_t i = 0; i < onNum; i++) {
            setResult("PeakInfo_PL11_OnEventsPeakPos_" + std::to_string(i + 1),
                      (double) PeakInfoRes.OnEventsPeakPos[i]);
        }
        for (size_t i = 0; i < offNum; i++) {
            setResult("PeakInfo_PL11_OffEventsPeakPos_" + std::to_string(i + 1),
                      (double) PeakInfoRes.OffEventsPeakPos[i]);
        }
        for (size_t i = onNum; i < 6; i++) //add other 3 PeakPos to avoid abnormal
        {
            setResult("PeakInfo_PL11_OnEventsPeakPos_" + std::to_string(i + 1), (double) 0);
        }
        for (size_t i = offNum; i < 6; i++) //add other 3 PeakPos to avoid abnormal
        {
            setResult("PeakInfo_PL11_OffEventsPeakPos_" + std::to_string(i + 1), (double) 0);
        }
    } else
        return -1;
    if (PeakInfoRes.nOnEventsPeakNumber >= 3 && PeakInfoRes.nOffEventsPeakNumber >= 3) {
        DVSImageContrastSensitivityType ImageContrastSensitivityRes;
        uint32_t nPeakNum = 3;
        if (check_ret(__func__, ImageContrastSensitivityDVS(handle, 0, UseFrameN, &PeakInfoRes, nPeakNum, On_OffEvents,
                                                            &ImageContrastSensitivityRes))) {
            setResult("OnEvents_EventsRatio_All_PL11", (double) ImageContrastSensitivityRes.OnEventsRatio[All]);
            setResult("OnEvents_EventsRatio_Gb_PL11", (double) ImageContrastSensitivityRes.OnEventsRatio[Gb]);
            setResult("OnEvents_EventsRatio_B_PL11", (double) ImageContrastSensitivityRes.OnEventsRatio[B]);
            setResult("OnEvents_EventsRatio_R_PL11", (double) ImageContrastSensitivityRes.OnEventsRatio[R]);
            setResult("OnEvents_EventsRatio_Gr_PL11", (double) ImageContrastSensitivityRes.OnEventsRatio[Gr]);
            setResult("OnEvents_EventsRatio_R_Gb_PL11", (double) ImageContrastSensitivityRes.R_Gb_OnEventsRatio);
            setResult("OnEvents_EventsRatio_B_Gb_PL11", (double) ImageContrastSensitivityRes.B_Gb_OnEventsRatio);
            setResult("OnEvents_EventsRatio_Gr_Gb_PL11", (double) ImageContrastSensitivityRes.Gr_Gb_OnEventsRatio);
            setResult("OffEvents_EventsRatio_All_PL11", (double) ImageContrastSensitivityRes.OffEventsRatio[All]);
            setResult("OffEvents_EventsRatio_Gb_PL11", (double) ImageContrastSensitivityRes.OffEventsRatio[Gb]);
            setResult("OffEvents_EventsRatio_B_PL11", (double) ImageContrastSensitivityRes.OffEventsRatio[B]);
            setResult("OffEvents_EventsRatio_R_PL11", (double) ImageContrastSensitivityRes.OffEventsRatio[R]);
            setResult("OffEvents_EventsRatio_Gr_PL11", (double) ImageContrastSensitivityRes.OffEventsRatio[Gr]);
            setResult("OffEvents_EventsRatio_R_Gb_PL11", (double) ImageContrastSensitivityRes.R_Gb_OffEventsRatio);
            setResult("OffEvents_EventsRatio_B_Gb_PL11", (double) ImageContrastSensitivityRes.B_Gb_OffEventsRatio);
            setResult("OffEvents_EventsRatio_Gr_Gb_PL11", (double) ImageContrastSensitivityRes.Gr_Gb_OffEventsRatio);
        } else
            return -1;
    }

    DVSSpatialResponseUniformityType SpatialResponseUniformityRes;
    int nPeakNum = 3;
    //bRet = gDVSInterface->SpatialResponseUniformity(0, nEvsSaveNum, nullptr, nPeakNum, On_OffEvents, SpatialResponseUniformityRes);
    if (check_ret(__func__, SpatialResponseUniformityDVS(handle, 0, UseFrameN, nullptr, &PeakInfoRes, nPeakNum,
                                                         On_OffEvents, &SpatialResponseUniformityRes))) {
        setResult("OffEventsUniformityRatio_All_PL11",
                  (double) SpatialResponseUniformityRes.dOffEventsUniformityRatio[All]);
        setResult("OffEventsUniformityRatio_Gb_PL11",
                  (double) SpatialResponseUniformityRes.dOffEventsUniformityRatio[Gb]);
        setResult("OffEventsUniformityRatio_B_PL11",
                  (double) SpatialResponseUniformityRes.dOffEventsUniformityRatio[B]);
        setResult("OffEventsUniformityRatio_R_PL11",
                  (double) SpatialResponseUniformityRes.dOffEventsUniformityRatio[R]);
        setResult("OffEventsUniformityRatio_Gr_PL11",
                  (double) SpatialResponseUniformityRes.dOffEventsUniformityRatio[Gr]);
        setResult("OnEventsUniformityRatio_All_PL11",
                  (double) SpatialResponseUniformityRes.dOnEventsUniformityRatio[All]);
        setResult("OnEventsUniformityRatio_Gb_PL11",
                  (double) SpatialResponseUniformityRes.dOnEventsUniformityRatio[Gb]);
        setResult("OnEventsUniformityRatio_B_PL11", (double) SpatialResponseUniformityRes.dOnEventsUniformityRatio[B]);
        setResult("OnEventsUniformityRatio_R_PL11", (double) SpatialResponseUniformityRes.dOnEventsUniformityRatio[R]);
        setResult("OnEventsUniformityRatio_Gr_PL11",
                  (double) SpatialResponseUniformityRes.dOnEventsUniformityRatio[Gr]);
    } else
        return -1;

    DVSAccompaniedPeakAndDelayedPeakType AccompaniedPeakAndDelayedPeakRes;
    nPeakNum = 3;
    if (check_ret(__func__, AccompaniedPeakAndDelayedPeakDVS(handle, 0, UseFrameN, &PeakInfoRes, nPeakNum, On_OffEvents,
                                                             &AccompaniedPeakAndDelayedPeakRes))) {
        setResult("AccompaniedPeakEventsRatio_On_All_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOnEventsRatio[All]);
        setResult("AccompaniedPeakEventsRatio_On_Gb_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOnEventsRatio[Gb]);
        setResult("AccompaniedPeakEventsRatio_On_B_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOnEventsRatio[B]);
        setResult("AccompaniedPeakEventsRatio_On_R_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOnEventsRatio[R]);
        setResult("AccompaniedPeakEventsRatio_On_Gr_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOnEventsRatio[Gr]);
        setResult("DelayedPeakEventsRatio_On_All_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOnEventsRatio[All]);
        setResult("DelayedPeakEventsRatio_On_Gb_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOnEventsRatio[Gb]);
        setResult("DelayedPeakEventsRatio_On_B_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOnEventsRatio[B]);
        setResult("DelayedPeakEventsRatio_On_R_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOnEventsRatio[R]);
        setResult("DelayedPeakEventsRatio_On_Gr_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOnEventsRatio[Gr]);
        setResult("AccompaniedPeakEventsRatio_Off_All_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOffEventsRatio[All]);
        setResult("AccompaniedPeakEventsRatio_Off_Gb_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOffEventsRatio[Gb]);
        setResult("AccompaniedPeakEventsRatio_Off_B_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOffEventsRatio[B]);
        setResult("AccompaniedPeakEventsRatio_Off_R_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOffEventsRatio[R]);
        setResult("AccompaniedPeakEventsRatio_Off_Gr_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dAccompaniedPeakOffEventsRatio[Gr]);
        setResult("DelayedPeakEventsRatio_Off_All_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOffEventsRatio[All]);
        setResult("DelayedPeakEventsRatio_Off_Gb_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOffEventsRatio[Gb]);
        setResult("DelayedPeakEventsRatio_Off_B_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOffEventsRatio[B]);
        setResult("DelayedPeakEventsRatio_Off_R_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOffEventsRatio[R]);
        setResult("DelayedPeakEventsRatio_Off_Gr_PL11",
                  (double) AccompaniedPeakAndDelayedPeakRes.dDelayedPeakOffEventsRatio[Gr]);
    } else
        return -1;


    DeleteHandleDVS(handle);
    delete [] rawDataBuf;
    rawDataBuf = NULL;
    delete [] pbRawData;
    pbRawData = NULL;
    return 0;
}

#endif //ALGORITHMLIBRARY_PL11_H
